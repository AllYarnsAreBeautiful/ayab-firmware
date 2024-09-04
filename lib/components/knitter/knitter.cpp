#include "knitter.h"

#include <string.h>

#include "api.h"
#include "board.h"

//----------------------------------------------------------------------------
// Line class
//----------------------------------------------------------------------------

void Line::reset() {
  memset(_pattern, 0xff, sizeof(_pattern));
  _number = 0xff;  // First getNextLineNumber is 0
  _isLast = false;
  _enterDirection = Direction::Unknown;
  requested = false;
  finished = true;
}

void Line::workedOn(bool isWorkedOn, Direction direction) {
  if (isWorkedOn) {
    // Register direction when entering the active needle window
    if (_enterDirection == Direction::Unknown) {
      _enterDirection = direction;
      _carriageOverflow = 16;
    }
  } else {
    // Exit direction has to match start direction (other end)
    if (_enterDirection == direction) {
      // Extend by 16 needles to make sure NC handled the last active one
      if (_carriageOverflow-- == 0) {
        finished = true;
      }
    }
  }
}

bool Line::setPattern(uint8_t number, const uint8_t *line, bool isLast) {
  if (number == getNextLineNumber()) {
    memcpy(_pattern, line, sizeof(_pattern));
    _number++;
    _isLast = isLast;
    _enterDirection = Direction::Unknown;
    requested = false;
    finished = false;
    return true;
  }
  return false;
}

bool Line::getNeedleValue(uint8_t position) {
  bool state = true;
  uint8_t byteIndex = position >> 3;
  uint8_t bitIndex = position % 8;

  if ((byteIndex < sizeof(_pattern))) {
    // lsb = leftmost (check)
    state = _pattern[byteIndex] & (1 << bitIndex) ? false : true;
  }
  return state;
}

bool Line::isLastLine() { return _isLast; }

uint8_t Line::getNextLineNumber() { return _number + 1; }

//----------------------------------------------------------------------------
// Knitter class
//----------------------------------------------------------------------------

Knitter::Knitter(hardwareAbstraction::HalInterface *hal) : API(hal) {
  // Platform
  _hal = hal;

  // Knitter hardware
  _encoder = new Encoder(_hal, ENC_PIN_A, ENC_PIN_B);
  _belt = new Belt(_hal, ENC_PIN_C);

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
      _machine->setType(MachineType::Kh910);
      _hall_left->config(_machine->getSensorConfig(Direction::Left));
      _hall_right->config(_machine->getSensorConfig(Direction::Right));

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
        _state = State::Operate;
        _currentLine.reset();
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
                    _belt->getShift());
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

      // When crossing left/right sensors towards the center, update carriage
      // (via isCrossing), encoder and beltshift states
      if ((_hall_left->isDetected(_encoder) &&
           _carriage->isCrossing(_hall_left, Direction::Right)) ||
          (_hall_right->isDetected(_encoder) &&
           _carriage->isCrossing(_hall_right, Direction::Left))) {
        _encoder->setPosition(_carriage->getPosition());
        _belt->setshift(_direction, _carriage->getType());
        _beeper->beep(BEEPER_READY);
      }

      // Get needle to set given current carriage position/type/direction
      uint8_t selectPosition = _carriage->getSelectPosition(_direction);
      // Map needle to set to solenoid
      uint8_t solenoidToSet = _machine->solenoidToSet(selectPosition);
      // Set solenoid according to current machine state
      if (_carriage->isDefined() && (!_currentLine.finished)) {
        // Belt shift handling
        if (_belt->getShift() == BeltShift::Shifted) {
          _machine->solenoidShift(solenoidToSet);
        }
        // Special handling for the L carriage
        if ((_carriage->getType() == CarriageType::Lace) &&
            (_direction == Direction::Left)) {
          _machine->solenoidShift(solenoidToSet);
        }
#ifdef DEBUG
        uint8_t message[] = {(uint8_t)AYAB_API::debugBase, selectPosition,
                             solenoidToSet,
                             _currentLine.getNeedleValue(selectPosition)};
        _hal->packetSerial->send(message, sizeof(message));
#endif
        if ((selectPosition >= _config.startNeedle) &&
            (selectPosition <= _config.stopNeedle)) {
          _solenoids->set(solenoidToSet,
                          _currentLine.getNeedleValue(selectPosition));
          _currentLine.workedOn(true, _direction);
        } else {
          _solenoids->reset(solenoidToSet);
          // Set _currentLine.finished once NC over the last active needle
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