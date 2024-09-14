#include "knitter.h"

#include "api.h"
#include "board.h"

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

  // Ayab hardware
  _beeper = new Beeper(_hal, PIEZO_PIN);

  _led_a = new Led(_hal, LED_A_PIN, HIGH, LOW);
  _led_b = new Led(_hal, LED_B_PIN, HIGH, LOW);

  reset();
}

void Knitter::reset() { _state = State::Reset; }

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
    case State::Reset:
      _machine->reset();
      _carriage->reset();
      _carriage->setPosition(_encoder->getPosition());

      _solenoids->reset();

      _led_a->on();
      _led_b->on();

      _config.valid = false;
      _config.continuousReporting = false;
      _state = State::Init;

      // FIXME: API should set 'machine' moving forward
#ifdef KH910
      _machine->setType(MachineType::Kh910);
#else
      _machine->setType(MachineType::Kh930);
#endif
      _hall_left->config(_machine->getSensorConfig(Side::Left));
      _hall_right->config(_machine->getSensorConfig(Side::Right));

      break;

    case State::Init:
      if (isStateChange) {
        _led_a->blink(LED_SLOW_ON, LED_SLOW_OFF);
        _beeper->beep(BEEPER_INIT);
      }
      if (_machine->isDefined() && _carriage->isDefined()) {
        _state = State::Ready;
        _apiRxIndicateState();
        _config.valid = false;
      }
      break;

    case State::Ready:
      if (isStateChange) {
        _led_a->blink(LED_FAST_ON, LED_FAST_OFF);
        _beeper->beep(BEEPER_READY);
      }
      if (_config.valid) {
        // Wait for beep to finish
        if (!_beeper->busy()) {
          _state = State::Operate;
          _currentLine.reset();
        }
      }
      break;

    case State::Operate:
      if (isStateChange) {
        _led_a->off();  // turn off, used for API Rx indication
        _led_b->off();  // turn off, used for API Tx indication
      }
      if (_currentLine.finished) {
        if (_currentLine.isLastLine()) {
          _state = State::Reset;
        }
        if (!_currentLine.requested) {  // Implement a timeout/retry mechanism ?
          _apiRequestLine(_currentLine.getNextLineNumber());
          _currentLine.requested = true;
        }
      }
      break;

    default:
      _state = State::Reset;
      break;
  }
}

void Knitter::_apiRxTrafficIndication() { _led_a->flash(LED_FLASH_DURATION); }

void Knitter::_apiTxTrafficIndication() { _led_b->flash(LED_FLASH_DURATION); }

void Knitter::_apiRequestReset() { reset(); }

bool Knitter::_apiRxSetConfig(uint8_t startNeedle, uint8_t stopNeedle,
                              bool continuousReporting) {
  _config.valid = false;
  if (_state == State::Ready) {
    _config = {.startNeedle = startNeedle,
               .stopNeedle = stopNeedle,
               .continuousReporting = continuousReporting};

    if ((_config.startNeedle >= 0) &&
        (_config.stopNeedle < _machine->getNumberofNeedles()) &&
        (_config.startNeedle < _config.stopNeedle)) {
      _config.valid = true;
    }
  }
  return _config.valid;
}

bool Knitter::_apiRxSetLine(uint8_t lineNumber, const uint8_t *pattern,
                            bool isLastLine) {
  bool success = false;
  if (_state == State::Operate) {
    success = _currentLine.setPattern(lineNumber, pattern, isLastLine);
    if (success) {
      _beeper->beep(BEEPER_NEXT_LINE);
    } else {  // Request line again
      _apiRequestLine(_currentLine.getNextLineNumber());
    }
  }
  return success;
}

void Knitter::_apiRxIndicateState() {
  Direction hallActive = _hall_left->isActive()    ? Direction::Left
                         : _hall_right->isActive() ? Direction::Right
                                                   : Direction::Unknown;
  _apiIndicateState(_hall_left->getSensorValue(), _hall_right->getSensorValue(),
                    _state != State::Init, _carriage->getType(),
                    _carriage->getPosition(), _direction, hallActive,
                    _beltShift);
}

// (Re)set carriage type/position and beltshift when crossing one sensor 
void Knitter::_checkHallSensors() {
  // When crossing left/right sensors towards the center, update carriage
  // (via isCrossing), encoder and beltshift states
  bool beltPhase = _hal->digitalRead(ENC_PIN_C) != 0;
  if (_hall_left->isDetected(_encoder, beltPhase)) {
    if (_carriage->isCrossing(_hall_left, Direction::Right)) {
      _encoder->setPosition(_carriage->getPosition());
      if (_carriage->getType() == CarriageType::Knit) {
        _beltShift = _hall_left->getDetectedBeltPhase() ? BeltShift::Shifted: BeltShift::Regular;
      } else { // CarriageType::Lace and CarriageType::Gartner
        _beltShift = _hall_left->getDetectedBeltPhase() ? BeltShift::Regular: BeltShift::Shifted;
      }
      _beeper->beep(BEEPER_READY);
    }
  } else if (_hall_right->isDetected(_encoder, beltPhase)) {
    if (_carriage->isCrossing(_hall_right, Direction::Left)) {
      _encoder->setPosition(_carriage->getPosition());
      if (_carriage->getType() == CarriageType::Lace) {
        _beltShift = _hall_right->getDetectedBeltPhase() ? BeltShift::Shifted: BeltShift::Regular;
      } else { // CarriageType::Knit and CarriageType::Gartner
        _beltShift = _hall_right->getDetectedBeltPhase() ? BeltShift::Regular: BeltShift::Shifted;
      }
      _beeper->beep(BEEPER_READY);
    }
  }
}

void Knitter::_runMachine() {
  if (_machine->isDefined()) {
    uint8_t newPosition = _encoder->getPosition();
    if (newPosition != _carriage->getPosition()) {
      // Infer current direction and update carriage position
      _direction = ((uint8_t)(newPosition - _carriage->getPosition()) < 128)
                       ? Direction::Right
                       : Direction::Left;
      _carriage->setPosition(newPosition);

      _checkHallSensors();

      // Get needle to set given current carriage position/type/direction
      uint8_t selectPosition = _carriage->getSelectPosition(_direction);
      // Map needle to set to solenoid
      uint8_t solenoidToSet = _machine->solenoidToSet(selectPosition);
      // Set solenoid according to current machine state
      if (_carriage->isDefined() && (!_currentLine.finished)) {
        // Belt shift handling
        if (_beltShift == BeltShift::Shifted) {
          _machine->solenoidShift(solenoidToSet);
        }
        // Special handling for the L carriage
        if ((_carriage->getType() == CarriageType::Lace) &&
            (_direction == Direction::Left)) {
          _machine->solenoidShift(solenoidToSet);
        }
#ifdef DEBUG
        uint8_t message[] = {(uint8_t)AYAB_API::debugBase,
                            (uint8_t) _hal->digitalRead(ENC_PIN_C) != 0,
                             selectPosition,
                             solenoidToSet,
                             _currentLine.getNeedleValue(selectPosition)};
        _hal->packetSerial->send(message, sizeof(message));
#endif
        // Set solenoid
        if ((selectPosition >= _config.startNeedle) &&
            (selectPosition <= _config.stopNeedle)) {
          _solenoids->set(solenoidToSet,
                          _currentLine.getNeedleValue(selectPosition));
          _currentLine.workedOn(true, _direction);
        } else {
          _solenoids->reset(solenoidToSet);
          // Set _currentLine.finished once last needle selected
          _currentLine.workedOn(false, _direction);
        }
      } else {
        _solenoids->reset(solenoidToSet);
      }

      // Update host SW
      if (_config.continuousReporting) {
        _apiRxIndicateState();
      }
    }
  }
}