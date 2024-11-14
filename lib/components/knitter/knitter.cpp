#include "knitter.h"

#include "api.h"
#include "shield.h"

//----------------------------------------------------------------------------
// Knitter class
//----------------------------------------------------------------------------

Knitter::Knitter(hardwareAbstraction::HalInterface *hal) : API(hal) {
  // Platform
  _hal = hal;

  // Knitter hardware
  _encoder = new Encoder(_hal, ENC_PIN_A, ENC_PIN_B);
  _hal->pinMode(ENC_PIN_C, INPUT);

  _hall_left = new HallSensor(_hal, EOL_L_PIN);
  _hall_right = new HallSensor(_hal, EOL_R_PIN);

  const uint8_t mcp23008_i2c_addresses[2] = {MCP23008_ADD0, MCP23008_ADD1};
  _solenoids = new Solenoids(_hal, mcp23008_i2c_addresses);

  // Knitter objects
  _machine = new Machine();
  _carriage = new Carriage();
  _beltShift = BeltShift::Unknown;
  _direction = Direction::Unknown;

  // Ayab hardware
  _beeper = new Beeper(_hal, PIEZO_PIN);

  _led_a = new Led(_hal, LED_A_PIN, HIGH, LOW);
  _led_b = new Led(_hal, LED_B_PIN, HIGH, LOW);

  _resetFromOperate = false;
  reset();
}

void Knitter::reset() { _state = KnitterState::Reset; }

void Knitter::schedule() {
  API::schedule();
  _beeper->schedule();
  _encoder->schedule();
  _led_a->schedule();
  _led_b->schedule();

  _runMachine();

  // Finite State Machine of this knitting machine
  bool isStateChange = _state != _lastState;
  _lastState = _state;
  switch (_state) {
    case KnitterState::Reset:
      // Skip machine reset when initiated from reqInit
      if (_resetFromOperate) {
        _resetFromOperate = false;
      } else {
        _machine->reset();
      }
      _carriage->reset();
      _carriage->setPosition(_encoder->getPosition());

      _solenoids->reset();

      _led_a->on();
      _led_b->on();

      _config.valid = false;
      _config.continuousReporting = false;
      _state = KnitterState::Init;

      break;

    case KnitterState::Init:
      if (isStateChange) {
        _led_a->blink(LED_SLOW_ON, LED_SLOW_OFF);
        _beeper->beep(BEEPER_INIT);
      }
      if (_machine->isDefined() && _carriage->isDefined()) {
        _state = KnitterState::Ready;
        _config.valid = false;
      }
      break;

    case KnitterState::Ready:
      if (isStateChange) {
        _led_a->blink(LED_FAST_ON, LED_FAST_OFF);
      }
      if (_config.valid) {
        _state = KnitterState::Operate;
        _currentLine.reset();
      }
      break;

    case KnitterState::Operate:
      if (isStateChange) {
        _led_a->off();  // turn off, used for API Rx indication
        _led_b->off();  // turn off, used for API Tx indication
      }
      if (_currentLine.finished) {
        if (_currentLine.isLastLine()) {
          _state = KnitterState::Reset;
        } else {
          if (!_currentLine.requested) {
            // TODO: Implement a timeout/retry mechanism ?
            _apiRequestLine(_currentLine.getNextLineNumber(), ErrorCode::Success);
            _currentLine.requested = true;
          }
        }
      }
      break;

    default:
      _state = KnitterState::Reset;
      break;
  }
}

void Knitter::_apiRxTrafficIndication() { _led_a->flash(LED_FLASH_DURATION); }

void Knitter::_apiTxTrafficIndication() { _led_b->flash(LED_FLASH_DURATION); }

void Knitter::_apiRequestReset() { reset(); }

ErrorCode Knitter::_apiRequestInit(MachineType machine) {
  if (_state == KnitterState::Init || _state == KnitterState::Operate) {
    _machine->setType(machine);
    _hall_left->config(_machine->getSensorConfig(MachineSide::Left));
    _hall_right->config(_machine->getSensorConfig(MachineSide::Right));
    // Reset machine upon reception of a new reqInit while in Operate state
    // because there is no reqReset API call from ayab-desktop as of today
    // and there is no hardware reset when the serial is open on all platforms
    // e.g. UNO R4
    if (_state == KnitterState::Operate) {
      _resetFromOperate = true;
      reset();
    }
    return ErrorCode::Success;
  }
  return ErrorCode::MachineInvalidState;
}

ErrorCode Knitter::_apiRxSetConfig(uint8_t startNeedle, uint8_t stopNeedle,
                                   bool continuousReporting,
                                   bool beeperEnabled) {
  _config.valid = false;
  if (_state == KnitterState::Ready) {
    if ((startNeedle >= 0) && (stopNeedle < _machine->getNumberofNeedles()) &&
        (startNeedle < stopNeedle)) {
      _config = {.startNeedle = startNeedle,
                 .stopNeedle = stopNeedle,
                 .continuousReporting = continuousReporting};
      _beeper->config(beeperEnabled);
      _config.valid = true;
      return ErrorCode::Success;
    }
    return ErrorCode::MessageInvalidArguments;
  }
  return ErrorCode::MachineInvalidState;
}

ErrorCode Knitter::_apiRxSetLine(uint8_t lineNumber, const uint8_t *pattern,
                                 uint8_t size, bool isLastLine) {
  bool success = false;
  if (_state == KnitterState::Operate) {
    if (size != _machine->getNumberofNeedles() >> 3) {
      return ErrorCode::MessageIncorrectLenght;
    }
    success = _currentLine.setPattern(lineNumber, pattern, isLastLine);
    if (success) {
      _beeper->beep(BEEPER_NEXT_LINE);
      return ErrorCode::Success;
    } else {  // Request line again, TODO: Use a different error code
      _apiRequestLine(_currentLine.getNextLineNumber(), ErrorCode::Success);
      return ErrorCode::MessageInvalidArguments;
    }
  }
  return ErrorCode::MachineInvalidState;
}

void Knitter::_apiRxIndicateState() {
  MachineSide hallActive = _hall_left->isActive()    ? MachineSide::Left
                           : _hall_right->isActive() ? MachineSide::Right
                                                     : MachineSide::None;
  CarriageType carriage = _carriage->getType();
  if (carriage == CarriageType::Knit270) {
    // FIXME: APIv6 doesn't know about Knit270
    carriage = CarriageType::Knit;
  }
  _apiIndicateState(_state, _hall_left->getSensorValue(),
                    _hall_right->getSensorValue(), carriage,
                    _carriage->getPosition(), _direction, hallActive,
                    _beltShift);
}

// (Re)set carriage type/position and beltshift when crossing one sensor
void Knitter::_checkHallSensors() {
  // When crossing left/right sensors towards the center, update carriage
  // (via isCrossing), encoder and beltshift states
  bool beltPhase = _hal->digitalRead(ENC_PIN_C) != 0;
  if (_hall_left->isDetected(_encoder, _direction, beltPhase)) {
    if (_carriage->isCrossing(_hall_left, Direction::Right)) {
      _encoder->setPosition(_carriage->getPosition());
      if (_carriage->getType() == CarriageType::Knit) {
        _beltShift = _hall_left->getDetectedBeltPhase() ? BeltShift::Shifted
                                                        : BeltShift::Regular;
      } else if (_carriage->getType() == CarriageType::Knit270) {
        _beltShift = BeltShift::Regular;
      } else {  // CarriageType::Lace and CarriageType::Garter
        _beltShift = _hall_left->getDetectedBeltPhase() ? BeltShift::Regular
                                                        : BeltShift::Shifted;
      }
      _apiRxIndicateState();
      _beeper->beep(BEEPER_CARRIAGE);
    }
  } else if (_hall_right->isDetected(_encoder, _direction, beltPhase)) {
    if (_carriage->isCrossing(_hall_right, Direction::Left)) {
      _encoder->setPosition(_carriage->getPosition());
      if (_carriage->getType() == CarriageType::Lace) {
        _beltShift = _hall_right->getDetectedBeltPhase() ? BeltShift::Shifted
                                                         : BeltShift::Regular;
      } else if (_carriage->getType() == CarriageType::Knit270) {
        _beltShift = BeltShift::Regular;
      } else {  // CarriageType::Knit and CarriageType::Garter
        _beltShift = _hall_right->getDetectedBeltPhase() ? BeltShift::Regular
                                                         : BeltShift::Shifted;
      }
      _apiRxIndicateState();
      _beeper->beep(BEEPER_CARRIAGE);
    }
  }
}

void Knitter::_runMachine() {
  if (_machine->isDefined()) {
    int16_t newPosition = _encoder->getPosition();
    if (newPosition != _carriage->getPosition()) {
      // Infer current direction and update carriage position
      _direction = ((newPosition - _carriage->getPosition()) > 0)
                       ? Direction::Right
                       : Direction::Left;
      _carriage->setPosition(newPosition);

      _checkHallSensors();

      if (_carriage->isDefined()) {
        // Get needle to set given current carriage position/type/direction
        int16_t selectPosition = _carriage->getSelectPosition(_direction);
        // Map needle to set to solenoid
        uint8_t solenoidToSet = _machine->solenoidToSet(selectPosition);
        // Belt shift handling
        if (_beltShift == BeltShift::Shifted) {
          _machine->solenoidShift(solenoidToSet);
        }      
        // For the Lace and KH270, carriage's center is not aligned with belt's elongated holes.
        if (((_carriage->getType() == CarriageType::Lace) ||
              (_carriage->getType() == CarriageType::Knit270)) &&
              (_direction == Direction::Left)) {
          _machine->solenoidShift(solenoidToSet);
        }
        _machine->solenoidMap(solenoidToSet);

        // Set solenoid according to current machine state
        if (!_currentLine.finished) {
          MachineSide machineSide = MachineSide::None;
          // Set solenoid
          if ((selectPosition >= _config.startNeedle) &&
              (selectPosition <= _config.stopNeedle)) {
            _solenoids->set(solenoidToSet,
                            _currentLine.getNeedleValue(selectPosition));
          } else {
            _solenoids->reset(solenoidToSet);
            // Delay _currentLine.finished until safe 
            if (selectPosition < _config.startNeedle) {
              machineSide = MachineSide::Left;    
            } else { // equivalent to > _config.stopNeedle
              machineSide = MachineSide::Right;       
            }
          }
          _currentLine.finished = _carriage->workFinished(machineSide, _direction);
        } else {
          _solenoids->reset(solenoidToSet);
        }
      }
      // Update host SW
      if (_config.continuousReporting) {
        _apiRxIndicateState();
      }
    }
  }
}