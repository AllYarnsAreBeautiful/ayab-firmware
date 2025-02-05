#pragma once

#if defined(ARDUINO_ARCH_RP2040)
#define KH_MOSI 7
#define KH_MISO 8
#define KH_SCK 6
#define KH_CS 9
#else
#define KH_MOSI MOSI
#define KH_MISO MISO
#define KH_SCK SCK
#define KH_CS SS
#endif

class KH970ClientEvents {
 public:
  virtual void debugLog(const char *msg) {}
  virtual void rowCounterHit() {}
};

struct KH970Client {
  uint8_t outBytes[32];
  uint8_t bytesToSend;
  uint8_t expectedByte, lastSent;
  uint8_t bedVal;
  bool booted;
  unsigned long delayStartMillis;

  const unsigned long DEBUG_DELAY_MS = 5;
  const uint8_t defaultCb1Val = 0x8b;

  enum State {
    STATE_INIT,
    STATE_INIT_ACK,
    STATE_INIT_ACK_DONE,
    STATE_REPLY,
    STATE_END,
    STATE_END_ACK,
    STATE_END_DELAY,
  };

  State state;

  uint8_t pattern[25];
  int pattern_row = 0;

  KH970Client();
  void begin();
  void append(uint8_t val);
  void update();
  void process(uint8_t bedVal);

  void setPattern(const uint8_t *data);

  void debugLog(const char *msg);

  KH970ClientEvents *_eventHandler = nullptr;
  void setEventHandler(KH970ClientEvents *eventHandler) {
    _eventHandler = eventHandler;
  }
};
