#include <Arduino.h>

#include "knitter.h"
#include "platform.h"

Knitter *knitter;

/*
 * Setup - do once before going to the main loop.
 */
void setup() {
  // Create Hardware Abstraction Layer (HAL) instance
  hardwareAbstraction::Platform *hal = new hardwareAbstraction::Platform();
  // Main knitter instance
  knitter = new Knitter(hal);
}

/*
 * Main Loop - repeat forever.
 */
void loop() {
  // Schedule knitter event loop
  knitter->schedule();
}
