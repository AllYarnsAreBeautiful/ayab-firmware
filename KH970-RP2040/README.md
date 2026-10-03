# rp2040ayab

RP2040 firmware that lets a **Brother KH-970** (a CSI machine normally driven
by its CB-1 computer) be used with **AYAB Desktop**, presenting as a
**KH-910 / KH-950** (select either the *KH-910, KH-950i* or the
*KH-900, KH-930, KH-940, KH-965i* profile — both are 200-needle machines and
are accepted).

The desktop application is **unmodified** — this firmware speaks AYAB's
existing API v6 protocol and translates it to/from the KH-970's CSI link.

## Architecture

Two cores, split by latency requirements:

```
                    ┌──────────────────────────────────────────────┐
 AYAB Desktop       │                   RP2040                     │
 (unmodified)       │                                              │
                    │  core 1: TinyUSB + AYAB API v6 translation    │
  USB CDC (SLIP) ───┤   ayab.c  slip.c   (SLIP framing, FSM,        │
  115200            │                    line pipeline)             │
                    │        ▲ events            │ pattern rows /   │
                    │        │                   │ reset / ready    │
                    │  shared.h / shared.c  (lock-free SPSC rings,  │
                    │        │  volatile state) │                   │
                    │        ▼                   ▼                   │
                    │  core 0: CSI master bit-bang (timing-critical) │
  KH-970 CSI ───────┤   csi.c   (serves A0/A1 needle rows, owns     │
  (SCK/CS/DIN/DOUT) │             fe7e keep-alive, reports events)  │
                    └──────────────────────────────────────────────┘
```

- **core 0 (`csi.c`)** — byte-exact port of the proven `rp2040/csi_bridge.c`
  wire engine.  Bit-bangs SCK, answers `0x80`/`0x81` keep-alives, serves the
  needle-selection rows `A0`/`A1` from the shared pattern ring, and reports
  machine events (START `0xB0`, row sensors `0xB1`/`0xB2`, carriage mode
  `0xB6`/`0xB7`/`0xB8`, boot config) to core 1.  No USB, no blocking.
- **core 1 (`ayab.c`, `slip.c`)** — TinyUSB CDC + SLIP framing + the AYAB
  API v6 state machine.  Receives needle lines from the desktop, stages them
  into the pattern ring, and translates CSI events into `indState` progress
  reports and `reqLine` requests for the next line.

## Protocol translation

AYAB API v6 (SLIP over USB CDC at 115200):

| Desktop → firmware | firmware → desktop |
|---|---|
| `reqInfo 0x03` | `cnfInfo 0xC3` (API v6, FW version) |
| `reqInit 0x05` (machine=0) | `cnfInit 0xC5` (0 = ok) |
| `reqStart 0x01` | `cnfStart 0xC1` (0 = ok) |
| `cnfLine 0x42` (25 bytes) | — |
| — | `reqLine 0x82` (next line number) |
| — | `indState 0x84` (carriage/position/direction) |

Flow:

1. Desktop → `reqInfo` → `cnfInfo`.
2. Desktop → `reqInit(KH910)` → `cnfInit(0)`.
3. Machine presence detected (config relay or first `0x80` poll) →
   `indState(0)` — this releases the desktop's `REQUEST_START` state.
4. Desktop → `reqStart(start, stop, flags)` → `cnfStart(0)`; core 1 pre-fills
   the first two rows (`reqLine(0)`, `reqLine(1)`) and raises `fe7e = 0xD2`
   so the machine starts knitting.
5. `cnfLine(line, 25 bytes)` → stored into `pattern[line % 22]`.
6. `0xB0` (row complete) → `reqLine(next)`; the blank "last line" sent by the
   desktop ends the job.

Each AYAB line maps to one CSI needle row (`A0`).  Multi-colour patterns are
handled entirely by the desktop (it sends one line per colour pass), so each
`cnfLine` is simply knitted as one row.

## The needle-data polarity

The reference AYAB firmware inverts every line byte (`lineBuffer[i] =
~buffer[i+4]`) because its solenoid outputs are active-low.  The KH-970 CSI
bitmap uses the opposite convention, so this firmware passes the data through
unchanged.  If the machine knits the mirror image of the pattern, change
`AYAB_INVERT_LINE_DATA` to `1` in `ayab.c` and reflash.

## Position reporting

The KH-970 CSI link has no continuous encoder; it only signals the row sensors
(`0xB1` forward, `0xB2` back) and the end-of-pass `0xB0`.  `indState.position`
is therefore synthesised:

| event | position | direction |
|---|---|---|
| `0xB1` | 60 | Right |
| `0xB2` | 190 | Left |
| `0xB0` | 0 or 255 | (start of next pass) |

This is sufficient for the desktop's progress display but is **not** an exact
needle position.  The sensor geometry can be calibrated in `handle_event()`
in `ayab.c`.

## Pin mapping

Same level shifter as the emulator bridge:

| function | GPIO |
|---|---|
| SCK (clock out) | 4 |
| CS  (attention in) | 5 |
| DIN (machine data in) | 7 |
| DOUT (reply out) | 8 |
| reserved (was VCCA) | 6 |

## Build

```sh
set PICO_SDK_PATH=C:\pico-sdk
cmake -G Ninja -B build .
cmake --build build
```

Output: `build/rp2040ayab.uf2`.  Flash by holding BOOTSEL while plugging in,
then dragging the `.uf2` onto the RPI-RP2 drive.

### Debug build (second USB CDC port)

A debug build enables a second USB CDC port (interface 1) that carries human
readable debug prints: `B0/B1/B2` sensor events, `fe7e` transitions, and the
AYAB handshake (`reqInfo`/`reqInit`/`reqStart`/`cnfLine`).  All of it is
compiled out of a normal build via the `RP2040AYAB_DEBUG` flag.

```sh
cmake -G Ninja -B build-debug -DRP2040AYAB_DEBUG=ON .
cmake --build build-debug
```

Flash `build-debug/rp2040ayab.uf2`; the debug port enumerates as
**"AYAB Debug Port"** (open it with any terminal at 115200).  The AYAB data
port is the first CDC interface and is unaffected.

## Sources

- Protocol: `../ayab-firmware/src/ayab/{com,fsm,knitter,encoders}.{h,cpp}`
- Desktop state machine: `../ayab-desktop/src/main/python/main/ayab/engine/engine_fsm.py`
- CSI wire: `../../rp2040/csi_bridge.c` and `../../CSI_PROTOCOL.md`
