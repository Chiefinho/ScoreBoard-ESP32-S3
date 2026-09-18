# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

Arduino firmware for an ESP32-S3 cricket scoreboard. There is no build system, test suite, or linter in the repo: each directory containing a `.ino` is a standalone Arduino sketch (the folder name must match the `.ino` name). Nothing can be verified without hardware, so compiling is the only local check.

- `Scoreboard_ESP32S3/` — the main application: BLE server driving six NeoPixel 7-segment displays.
- `Matrix Testing/` — separate experimental sketches for a 64x32 P3 HUB75 matrix panel, which is driven by its own ESP32-S3 (not the strip controller). `LED_Matrix_Test_Protomatter/` is the current working one; `LED_Matrix_Test/` (ESP32-HUB75-MatrixPanel-DMA, old wiring), `LED_Matrix_Test_FIXED/` and `ROW_MAPPING_TEST/` are earlier/diagnostic iterations.

## Building and uploading

The documented workflow is the Arduino IDE (see README.md). Board settings: **ESP32S3 Dev Module**, **USB CDC On Boot: Enabled**, **USB Mode: Hardware CDC and JTAG**. Serial monitor at **115200** baud. If upload fails, hold BOOT, tap RESET, release BOOT, then upload.

Libraries: Adafruit NeoPixel (main app); Adafruit Protomatter + Adafruit GFX (matrix sketches). BLE comes from the ESP32 Arduino core.

`arduino-cli` is not installed on this machine, but if it is available the equivalent compile check is:

```bash
arduino-cli compile --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc Scoreboard_ESP32S3
```

Quote the path for matrix sketches (`"Matrix Testing/LED_Matrix_Test_Protomatter"`) since the directory name contains a space.

## Main scoreboard architecture (`Scoreboard_ESP32S3.ino`)

Data flow: BLE write → `MyCallbacks::onWrite` → `processReceivedData` → `parseJsonData` → `updateDisplay` → `displayNumber` → `displayDigit`. All work happens inside the BLE callback; `loop()` only handles re-advertising after disconnect.

**BLE protocol** (device name `Scoreboard`, single read/write characteristic). Note the UUIDs in the code (`12345678-…` / `87654321-…`) differ from the ones listed in README.md — the code is authoritative, and the companion mobile app must match it.
- `{"type":"score","batsmanA":N,"batsmanB":N,"total":N,"overs":N,"wickets":N,"target":N}` — `target` optional; if omitted the target display is left unchanged.
- `{"type":"command","command":"RESET"|"TEST"}`
- Plain `RESET` string also resets. `SCORE:` prefix is recognised but only logged (unimplemented).

JSON is parsed by hand with `indexOf`/`substring` (no JSON library), so it assumes compact JSON with no whitespace after colons (e.g. `"type":"score"`, not `"type": "score"`). Values go through `String::toInt()`, so fractional overs like `12.3` are truncated to `12`.

**LED layout**: each digit is 63 pixels = 7 segments × 9 LEDs (`PIXELSPERDIGIT`). Within a digit, segment index order along the strip is: 0 bottom-right, 1 bottom, 2 bottom-left, 3 top-left, 4 top, 5 top-right, 6 middle — `displayDigit` encodes each numeral as `fill()` calls over these ranges. Strips are chained ones digit first (offset 0), then tens, then hundreds; leading zeros are blanked. Each display is its own `Adafruit_NeoPixel` on its own GPIO (2–7) with a fixed colour set in `updateDisplay`.

## LED matrix sketches

The panel is 1/8 scan (address pins A/B/C only, no D) with non-standard addressing. The `RowRemappedMatrix` class wraps `Adafruit_Protomatter` as an `Adafruit_GFX` subclass and overrides `drawPixel()` to remap software rows 8–15 → physical 31–24, 16–23 → 8–15, 24–31 → 23–16, and mirror columns (`x → 63-x`) on physical rows 16–31. Any drawing must go through this wrapper (not the raw `protomatter` object) or output will be scrambled. Details in `Matrix Testing/LED_MATRIX_SOLUTION.md`.

Current HUB75 wiring is defined by the `rgbPins`/`addrPins`/`clockPin`/`latchPin`/`oePin` variables (RGB 4,5,6,7,15,16; A/B/C 17,18,8; CLK 9; LAT 47; OE 48). The header comment in `LED_Matrix_Test_Protomatter.ino` and its `mappingTest()` still reference the old wiring (21/47/48) — trust the variables and the sub-README's wiring diagram.

The Protomatter sketch is driven by single-character serial commands (`c`, `o`, `m`, `x`, `s`, `h`, `0`) handled in `handleCommand()`; scrolling (`s`) runs non-blocking from `loop()` until `0`. Progress notes live in `LED_Matrix_Test_Protomatter/PROGRESS.md`.

The matrix and the NeoPixel strips run on **separate ESP32-S3 boards**, so the overlapping GPIO numbers (4–7) between the matrix sketches and `Scoreboard_ESP32S3.ino` are not a conflict. Matrix code should become its own firmware, not be merged into the strip sketch.
