# AYAB Async Firmware
Async implementation of [AYAB Firmware](https://github.com/AllYarnsAreBeautiful/ayab-firmware) (main branch).
## Main differences
- Compatible with ayab desktop software (API v6).
- Async implementation, no blocking calls.
   - beeper, encoder, knitter, led, ... async scheduling ("event loop")
   - faster & lower latency than legacy implementation
- Hardware Abstraction Layer (hal) isolating platform-specific code (should ease porting to e.g. esp32).
   - Support for Mega2560 and Uno hardware.
   - External dependencies limited to PacketSerial & SoftI2CMaster.
- ISR processing limited to carriage position, everything else runs in the main loop.
- Simplified needle selection algorithm.
   - Using carriage center and position of the 'needle checker'.
- Robust & dynamic (while knitting) carriage detection (K, L, G) from both sides.
- No overhead for googletest support.

## Warning
Work in progress, not yet fully tested/validated (not tested against KH270)!
