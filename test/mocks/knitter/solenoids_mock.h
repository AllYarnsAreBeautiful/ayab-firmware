#pragma once

#include "types.h"
#include <gmock/gmock.h>

class SolenoidsMock {
public:
  MOCK_METHOD0(init, void());
  MOCK_METHOD2(setSolenoid, void(byte, bool));
  MOCK_METHOD1(setSolenoids, void(uint16 state));
};

SolenoidsMock *solenoidsMockInstance();
void releaseSolenoidsMock();
