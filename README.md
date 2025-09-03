# AYAB Async Firmware
Alternative async implementation of [AYAB Firmware](https://github.com/AllYarnsAreBeautiful/ayab-firmware).  
Firmware is compatible with [AYAB Desktop](https://github.com/AllYarnsAreBeautiful/ayab-desktop) (API v5 and v6 using v0.9 and v1.0 branches respectively).

## Main differences
- Robust needle selection algorithm.
   - Based on carriage center position, direction and position of rotary cams.
- Dynamic (while knitting) carriage detection (K, L, G) from both sides.
- Async implementation, no blocking calls.
   - beeper, encoder, knitter, led, ... async scheduling ("event loop")
   - faster & lower latency than legacy implementation
- Hardware Abstraction Layer (hal) isolating platform-specific code (ease porting to e.g. esp32).
   - Support for *Mega2560*, *Uno R3* and *Uno R4 Wifi* hardware.
   - External dependencies limited to PacketSerial & SoftI2CMaster.
- ISR processing limited to carriage position, everything else runs in the main loop.
