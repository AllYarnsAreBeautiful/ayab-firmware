#pragma once

#include <beeper.h>
#include <gmock/gmock.h>

class BeeperMock {
public:
  MOCK_METHOD0(ready, void());
  MOCK_METHOD0(finishedLine, void());
  MOCK_METHOD0(endWork, void());
};

BeeperMock *beeperMockInstance();
void releaseBeeperMock();
