#include "encoders_mock.h"
#include "encoders.h"

static EncodersMock *gEncodersMock = NULL;
EncodersMock *encodersMockInstance() {
  if (!gEncodersMock) {
    gEncodersMock = new EncodersMock();
  }
  return gEncodersMock;
}

void releaseEncodersMock() {
  if (gEncodersMock) {
    delete gEncodersMock;
    gEncodersMock = NULL;
  }
}

Encoders::Encoders() {
}
void Encoders::encA_interrupt() {
  assert(gEncodersMock != NULL);
  gEncodersMock->encA_interrupt();
}
byte Encoders::getPosition() {
  assert(gEncodersMock != NULL);
  return gEncodersMock->getPosition();
}
Beltshift_t Encoders::getBeltshift() {
  assert(gEncodersMock != NULL);
  return gEncodersMock->getBeltshift();
}
Direction_t Encoders::getDirection() {
  assert(gEncodersMock != NULL);
  return gEncodersMock->getDirection();
}
Direction_t Encoders::getHallActive() {
  assert(gEncodersMock != NULL);
  return gEncodersMock->getHallActive();
}
Carriage_t Encoders::getCarriage() {
  assert(gEncodersMock != NULL);
  return gEncodersMock->getCarriage();
}
uint16 Encoders::getHallValue(Direction_t dir) {
  assert(gEncodersMock != NULL);
  return gEncodersMock->getHallValue(dir);
}
