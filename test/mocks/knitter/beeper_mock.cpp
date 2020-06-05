#include "beeper_mock.h"
#include <beeper.h>

static BeeperMock *gBeeperMock = NULL;
BeeperMock *beeperMockInstance() {
  if (!gBeeperMock) {
    gBeeperMock = new BeeperMock();
  }
  return gBeeperMock;
}

void releaseBeeperMock() {
  if (gBeeperMock) {
    delete gBeeperMock;
    gBeeperMock = NULL;
  }
}

void Beeper::ready() {
  assert(gBeeperMock != NULL);
  gBeeperMock->ready();
}
void Beeper::finishedLine() {
  assert(gBeeperMock != NULL);
  gBeeperMock->finishedLine();
}
void Beeper::endWork() {
  assert(gBeeperMock != NULL);
  gBeeperMock->endWork();
}
