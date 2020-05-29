#include "knitter_mock.h"
#include "../../../knitter.h"

static KnitterMock *gKnitterMock = NULL;
KnitterMock *knitterMockInstance() {
  if (!gKnitterMock) {
    gKnitterMock = new KnitterMock();
  }
  return gKnitterMock;
}

void releaseKnitterMock() {
  if (gKnitterMock) {
    delete gKnitterMock;
    gKnitterMock = NULL;
  }
}

Knitter::Knitter() {
}
bool Knitter::startTest(void) {
  assert(gKnitterMock != NULL);
  return gKnitterMock->startTest();
}
bool Knitter::startOperation(byte startNeedle, byte stopNeedle,
                             bool continuousReportingEnabled, byte(*line)) {
  assert(gKnitterMock != NULL);
  return gKnitterMock->startOperation(startNeedle, stopNeedle,
                                      continuousReportingEnabled, line);
}
bool Knitter::setNextLine(byte lineNumber) {
  assert(gKnitterMock != NULL);
  return gKnitterMock->setNextLine(lineNumber);
}
void Knitter::setLastLine(void) {
  assert(gKnitterMock != NULL);
  gKnitterMock->setLastLine();
}
void Knitter::send(uint8_t payload[]) {
  assert(gKnitterMock != NULL);
  gKnitterMock->send(payload);
}

Knitter *knitter;
