#include "knitter.h"

#include "api.h"
#include "shield.h"
#include "mcp23008.h"
#include "pcf8574.h"

//----------------------------------------------------------------------------
// Knitter class
//----------------------------------------------------------------------------

Knitter::Knitter(hardwareAbstraction::HalInterface *hal) : API(hal) {
  // Platform
  _hal = hal;

  // Knitter objects
  _machine = new Machine();
  _carriage = new Carriage();
  _beltShift = BeltShift::Unknown;
  _direction = Direction::Unknown;

  // Ayab hardware
  _beeper = new Beeper(_hal, Shield::Piezo::PIEZO_PIN);

  _led_a = new Led(_hal, Shield::Leds::LED_A_PIN, HIGH, LOW);
  _led_b = new Led(_hal, Shield::Leds::LED_B_PIN, HIGH, LOW);

  _resetFromOperate = false;

  _kh970Client.setEventHandler(this);

  reset();
}

void Knitter::reset() {
  _state = KnitterState::Reset;
  _kh970Client.begin();
  _lastRequestedRow = 0;
}

void Knitter::schedule() {
  API::schedule();
  _beeper->schedule();
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
      _carriage->setType(CarriageType::Knit);
      if (_machine->isDefined() && _carriage->isDefined()) {
        _state = KnitterState::Ready;
        _apiRxIndicateState(); // FIXME required for desktop app to start
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

void Knitter::_detectGpioExpanders(hardwareAbstraction::HalInterface *hal, const uint8_t i2cAddress[][2], GpioExpander* gpio_expander[2]) {
  // FIXME: First one is selected when none are detected -> should raise an error towards desktop app instead
  int i2c_address_set = 0;
  for (int id = 0; (i2cAddress[id][0] != 0) || (i2cAddress[id][1] != 0); id++) {
    if (hal->i2c->detect(i2cAddress[id][0]) && hal->i2c->detect(i2cAddress[id][1])) {
      i2c_address_set = id;
      break;
    };
  }

  for (int i = 0; i < 2; i++) {
    // Detect GPIO expander type
    // MCP23008 IOCON.0 always reads as 0 while PCF8574 will latch the last written value
    hal->i2c->write(i2cAddress[i2c_address_set][i], Mcp23008::IOCON, 0x01);
    if ((hal->i2c->read(i2cAddress[i2c_address_set][i], Mcp23008::IOCON) & 0x01) == 0x00) {
      Mcp23008 *mcp23008 = new Mcp23008(hal, i2cAddress[i2c_address_set][i]);
      mcp23008->write(Mcp23008::IODIR, 0);  // Configure as output
      gpio_expander[i] = mcp23008;
    } else {
      gpio_expander[i] = new Pcf8574(hal, i2cAddress[i2c_address_set][i]);
    }
  }
}

void Knitter::_apiRxTrafficIndication() { _led_a->flash(LED_FLASH_DURATION); }

void Knitter::_apiTxTrafficIndication() { _led_b->flash(LED_FLASH_DURATION); }

void Knitter::_apiRequestReset() { reset(); }

ErrorCode Knitter::_apiRequestInit(MachineType machine) {
  if (_state == KnitterState::Init || _state == KnitterState::Operate) {
    _machine->setType(machine);
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
  if (_state == KnitterState::Operate) {
    if (size != 25) {
      return ErrorCode::MessageIncorrectLenght;
    }

    uint8_t reversedPattern[size]; // KH970 byte order does not match AYAB's
    for (int i=0; i<size; i++) {
      reversedPattern[i] = pattern[size-i-1];
    }
    _kh970Client.setPattern(reversedPattern);

    _beeper->beep(BEEPER_NEXT_LINE);
    return ErrorCode::Success;
  }
  return ErrorCode::MachineInvalidState;
}

void Knitter::_apiRxIndicateState() {
  MachineSide hallActive = MachineSide::None;
  CarriageType carriage = CarriageType::Knit;
  _apiIndicateState(_state, 0, 0, carriage,
                    50, _direction, hallActive,
                    BeltShift::Regular);
}

void Knitter::debugLog(const char *msg)
{
  _apiDebugLog(msg);
}

void Knitter::rowCounterHit()
{
  _apiRowCounterHit();
}

void Knitter::_runMachine() {
  if (_machine->isDefined()) {
    _kh970Client.update();
    if (_kh970Client.pattern_row != _lastRequestedRow) {
      _lastRequestedRow = _kh970Client.pattern_row;
      _apiRequestLine(_lastRequestedRow, ErrorCode::Success);
    }

    // Update host SW
    if (_config.continuousReporting) {
      _apiRxIndicateState();
    }
  }
}
