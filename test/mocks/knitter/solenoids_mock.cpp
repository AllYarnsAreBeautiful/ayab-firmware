#include "solenoids_mock.h"
#include "../../../solenoids.h"

static SolenoidsMock *gSolenoidsMock = NULL;
SolenoidsMock *solenoidsMockInstance() {
  if (!gSolenoidsMock) {
    gSolenoidsMock = new SolenoidsMock();
  }
  return gSolenoidsMock;
}

void releaseSolenoidsMock() {
  if (gSolenoidsMock) {
    delete gSolenoidsMock;
    gSolenoidsMock = NULL;
  }
}

Solenoids::Solenoids() {
}
void Solenoids::init(void) {
  assert(gSolenoidsMock != NULL);
  gSolenoidsMock->init();
}
void Solenoids::setSolenoid(byte solenoid, bool state) {
  assert(gSolenoidsMock != NULL);
  gSolenoidsMock->setSolenoid(solenoid, state);
}
void Solenoids::setSolenoids(uint16 state) {
  assert(gSolenoidsMock != NULL);
  gSolenoidsMock->setSolenoids(state);
}
