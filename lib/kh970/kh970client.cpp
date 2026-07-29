#include <Arduino.h>

#include "kh970client.h"

#ifndef TXLED0

#define TXLED0
#define TXLED1
#define RXLED0
#define RXLED1

#endif

#define MSG_BEGIN 0x47
#define MSG_END 0x87

#define CLOCK_PERIOD 100

struct CSI {
  uint8_t outByte, inByte;
  uint8_t bitIndex;

  enum State {
    STATE_IDLE,
    STATE_START,
    STATE_TRANSFER,
    STATE_WAIT_CS_UP,
  };

  State state;
  unsigned long delayStartMicros;

  void begin() {
    state = STATE_IDLE;
    digitalWrite(KH_SCK, LOW);
  }

  bool update() {
    if (state == STATE_START) {
      if (digitalRead(KH_CS) == HIGH) {
        return true;
      }
      digitalWrite(KH_MOSI, LOW);
      digitalWrite(KH_SCK, LOW);
      delayStartMicros = micros();
      state = STATE_TRANSFER;
    } else if (state == STATE_TRANSFER) {
      if (micros() - delayStartMicros < CLOCK_PERIOD / 2) {
        return true;
      }
      delayStartMicros = micros();
      if ((bitIndex & 1) == 0) {
        // Setup MOSI on rising edge of SCK
        if (outByte & 1)
          digitalWrite(KH_MOSI, HIGH);
        else
          digitalWrite(KH_MOSI, LOW);
        outByte >>= 1;
        digitalWrite(KH_SCK, HIGH);
      } else {
        // Read MISO on falling edge of SCK
        inByte >>= 1;
        if (digitalRead(KH_MISO) == LOW) {
          inByte |= 0x80;
        }
        digitalWrite(KH_SCK, LOW);
      }
      if (bitIndex >= 15) {
        // In order to pace the bed's communications,
        // we need to lower MOSI as soon as possible,
        // since the bed won't initiate a transfer
        // (by lowering CS) while MOSI is low.
        // However if the last bit to send is high and we lower
        // MOSI right after lowering SCK, there's a risk the bed
        // might latch the wrong bit value (low instead of high).
        // If we return from update() here and wait for the next
        // call, it may be too late (the bed tends to raise CS
        // about 30 µs after the last SCK falling edge, and lower
        // it about 100 µs after that; while the interval
        // between update() calls seems to be at least 50 µs
        // when the firmware is idle and can be much more
        // if something is happening).
        // So instead we force a small delay right here to give
        // the bed time to latch the bit, then lower MOSI.
        // 10 µs is about how soon I've seen the CB-1 flip
        // MOSI after the last SCK falling edge, so that should
        // be safe. The datasheet for the bed's CPU
        // (µPD75004) specifies a hold time of 400 ns.
        // In practice, the delay is unnecessary; I've tested
        // lowering MOSI right after SCK, which I measured as
        // about 500 ns of delay, and the bed still latched a
        // high bit correctly.
        // But to be safe, I'll leave this here.
        delayMicroseconds(10);
        digitalWrite(KH_MOSI, LOW);

        state = STATE_WAIT_CS_UP;
      }
      bitIndex++;
    } else if (state == STATE_WAIT_CS_UP) {
      if (digitalRead(KH_CS) == HIGH) {
        state = STATE_IDLE;
      }
    }
    return state != STATE_IDLE;
  }

  void sendOut(uint8_t mosiVal) {
    inByte = 0;
    outByte = mosiVal;
    bitIndex = 0;
    digitalWrite(KH_MOSI, HIGH);
    state = STATE_START;
  }
};

CSI csi;

void force_reboot() {
  // By signaling readiness but not responding, the CB1 forces the bed to reboot
  digitalWrite(KH_MOSI, HIGH);
  delay(3000);
}

bool check(uint8_t actual, uint8_t expected) {
  if (actual != expected) {
    Serial.print("BAD ACK !!! expected=0x");
    Serial.print(expected, 16);
    Serial.print(" actual=0x");
    Serial.println(actual, 16);
    TXLED1;
    RXLED1;
    force_reboot();
    return false;
  }
  return true;
}

KH970Client::KH970Client() {
  bytesToSend = 0;
  state = STATE_INIT;
  booted = false;
}

void KH970Client::begin() {
  csi.begin();
  pinMode(KH_CS, INPUT_PULLUP);
  pinMode(KH_MOSI, OUTPUT);
  pinMode(KH_SCK, OUTPUT);
}

void KH970Client::append(uint8_t val) {
  if (val != 0x8b) {
    char buf[10];
    sprintf(buf, "C: %02X", val);
    debugLog(buf);
  }
  outBytes[bytesToSend++] = val;
}

void KH970Client::update() {
  if (csi.update())
    return;
  if (state == STATE_INIT) {
    RXLED1;
    csi.sendOut(MSG_BEGIN);
    state = STATE_INIT_ACK;
  } else if (state == STATE_INIT_ACK) {
    bedVal = csi.inByte;
    csi.sendOut(bedVal);
    state = STATE_INIT_ACK_DONE;
  } else if (state == STATE_INIT_ACK_DONE) {
    if (!check(csi.inByte, MSG_BEGIN))
      return;

    process(bedVal);
    expectedByte = MSG_BEGIN;
    csi.sendOut(lastSent = outBytes[bytesToSend - 1]);
    state = STATE_REPLY;
  } else if (state == STATE_REPLY) {
    if (!check(csi.inByte, expectedByte))
      return;
    expectedByte = lastSent;
    bytesToSend--;
    if (bytesToSend > 0) {
      csi.sendOut(lastSent = outBytes[bytesToSend - 1]);
    } else {
      csi.sendOut(MSG_END);
      state = STATE_END_ACK;
    }
  } else if (state == STATE_END_ACK) {
    if (!check(csi.inByte, expectedByte))
      return;
    RXLED0;
    delayStartMillis = millis();
    state = DEBUG_DELAY_MS > 0 ? STATE_END_DELAY : STATE_INIT;
  } else if (state == STATE_END_DELAY) {
    if (millis() - delayStartMillis > DEBUG_DELAY_MS) {
      state = STATE_INIT;
    }
  }
}

void KH970Client::process(uint8_t bedVal) {
  if (bedVal != 0x01) {
    char buf[10];
    sprintf(buf, "B: %02X", bedVal);
    debugLog(buf);
  }
  switch (bedVal) {
  case 0x01:
    // The "ping" message from the bed?
    // Sent very often.
    if (booted) {
      append(0x8b);
    } else {
      // At the end of the boot sequence there's a 0x01 from the bed, but
      // the CB1 replies with 0B instead of the usual 6B.
      append(0x0b);
      booted = true;
    }
    break;

  // 80 D8 14 3C 02 are the initial bytes sent by the bed upon boot.
  // The CB1 appears to always acknowledge them with 07.
  // Could contain bed firmware version?
  case 0x80: // Boot 1
  case 0xD8: // Boot 2
  case 0x14: // Boot 3
  case 0x3C: // Boot 4
  case 0x02: // Boot 5
    append(0x07);
    break;

  // 0A 8A 4A CA are sent in sequence during the boot, and the CB1
  // seems to reply with some information. CB1 version number?
  case 0x0a: // Boot 6
    append(0x50);
    break;

  case 0x8a: // Boot 7
    append(0x30);
    break;

  case 0x4a: // Boot 8
    append(0x00);
    break;

  case 0xca: // Boot 9
    append(0x00);
    break;
    
  case 0x4d: // Sent when the row counter is hit
    append(0x07);
    debugLog("Row counter hit");
    if (_eventHandler) {
      _eventHandler->rowCounterHit();
    }
    break;

  case 0x0d: // Sent (once) when carriage changes direction after crossing the
             // center; will be followed by pattern request (85)
  case 0x8d: // Sent when the bed center is crossed

    // We get 2D 6D 7D 13 D3 when the K carriage crosses the left button.
    // We get CD 6D BD 03 C3 when the K carriage crosses the right button.

  case 0x2d: // Sent when the left button is passed going to the right
  case 0x7d: // Sent after 2D 6D/ED/1D, and after 0D 85 89 if new direction is
             // rightwards.
  case 0x13: // Sent after 2D 6D/ED/1D 7D
  case 0xd3: // Sent after 2D 6D/ED/1D 7D 13

  case 0xcd: // Sent when the right button is passed going to the left
  case 0xbd: // Sent after CD 6D/ED/1D, and after 0D 85 89 if new direction is
             // leftwards.
  case 0x03: // Sent after CD 6D/ED/1D BD
  case 0x43: // Sent instead of 03 after CD 6D/ED/1D BD, sometimes, seen only
             // with L carriage so far
  case 0xc3: // Sent after CD 6D/ED/1D BD 03

  case 0x6d: // Sent after 2D/CD if the carriage is the K carriage
  case 0xed: // Sent after 2D/CD if the carriage is the L carriage
  case 0x1d: // Sent after 2D/CD if the carriage is the G carriage

  case 0x3d: // Sent just before requesting the first row's pattern data

  // 81 5D 49 DD are sent after the first two rows' data has been received
  case 0x81: // 
  case 0x5d:
  case 0x49:
  case 0xdd:
    // There seems to be a clear pattern of the CB1 replying 07 to all
    // query bytes ending in D (and 3?). Possibly this is a filler byte (like
    // 47 and 87, there's a pattern here too) and those *D queries are
    // only informative.
    append(0x07);
    break;

  case 0x05: // Sent after boot sequence, to request the first pattern row
  case 0x85: // Sent after 05 09 to get second pattern row,
             // and after 0D (turnaround) to request next pattern row
  {
    for (int i = 0; i < 25; i++) {
      append(pattern[i]);
    }
    pattern_row++;
    break;
  }

  case 0x09: // Sent after receiving first row's pattern data
  case 0x89: // Sent after receiving subsequent rows' pattern data
    append(0xf0); // cb1 was also seen replying 00
    break;

  default:
    append(defaultCb1Val);
    break;
  }
}

void KH970Client::setPattern(const uint8_t *data) {
  memcpy(pattern, data, sizeof(pattern));
}

void KH970Client::debugLog(const char *msg) {
  if (_eventHandler) {
    _eventHandler->debugLog(msg);
  }
}

