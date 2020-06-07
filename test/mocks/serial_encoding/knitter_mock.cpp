#include "knitter_mock.h"
#include "knitter.h"

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

bool Knitter::startOperation(uint8_t startNeedle, uint8_t stopNeedle,
                             bool continuousReportingEnabled, uint8_t *line) {
  assert(gKnitterMock != NULL);
  return gKnitterMock->startOperation(startNeedle, stopNeedle,
                                      continuousReportingEnabled, line);
}

bool Knitter::setNextLine(uint8_t lineNumber) {
  assert(gKnitterMock != NULL);
  return gKnitterMock->setNextLine(lineNumber);
}

void Knitter::setLastLine(void) {
  assert(gKnitterMock != NULL);
  gKnitterMock->setLastLine();
}

void Knitter::send(uint8_t payload[], size_t length) {
  assert(gKnitterMock != NULL);
  gKnitterMock->send(payload, length);
}

void Knitter::onPacketReceived(const uint8_t *buffer, size_t size) {
  assert(gKnitterMock != NULL);
  gKnitterMock->onPacketReceived(buffer, size);
}

Knitter *knitter;
