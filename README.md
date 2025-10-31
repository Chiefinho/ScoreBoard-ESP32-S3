# Cricket Scoreboard - ESP32-S3

A Bluetooth-controlled cricket scoreboard system using ESP32-S3 and NeoPixel LED strips.

## Project Structure

- **Scoreboard_ESP32S3/** - Main cricket scoreboard application
- **Matrix Testing/** - LED matrix panel test suite
  - **LED_Matrix_Test_Protomatter/** - Working test with remapping (recommended)
  - **LED_Matrix_Test_FIXED/** - Simplified version
  - **LED_Matrix_Test/** - Original test with alternative library
  - **ROW_MAPPING_TEST/** - Diagnostic tool for discovering panel mappings
  - **LED_MATRIX_SOLUTION.md** - Technical documentation

## Hardware Requirements

- **ESP32-S3 Development Board**
- **6x NeoPixel LED Strips** (WS2812B/NeoPixel compatible)
  - Total Score: 3 digits (189 LEDs)
  - Wickets: 1 digit (63 LEDs)
  - Overs: 2 digits (126 LEDs)
  - Target/First Innings: 3 digits (189 LEDs)
  - Batsman A Score: 3 digits (189 LEDs)
  - Batsman B Score: 3 digits (189 LEDs)

### Optional: LED Matrix Panel

The project also includes support for a 64x32 P3 LED matrix panel in `Matrix Testing/`:
- **Adafruit Protomatter** library
- Custom remapping for non-standard panel addressing
- See `Matrix Testing/LED_Matrix_Test_Protomatter/README.md` for details

## Pin Configuration

| Display | GPIO Pin |
|---------|----------|
| Total Score | GPIO 2 |
| Wickets | GPIO 3 |
| Overs | GPIO 4 |
| First Innings | GPIO 5 |
| Batsman A | GPIO 6 |
| Batsman B | GPIO 7 |

## Software Requirements

- **Arduino IDE** (1.8.x or 2.x)
- **ESP32 Board Support**: `https://github.com/espressif/arduino-esp32`
- **Adafruit NeoPixel Library** (1.15.1 or later)

## Installation

1. **Install ESP32 Board Support in Arduino IDE:**
   - Go to **File > Preferences**
   - Add to Additional Boards Manager URLs: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   - Go to **Tools > Board > Boards Manager**
   - Search for "esp32" and install

2. **Install Required Libraries:**
   - Go to **Sketch > Include Library > Manage Libraries**
   - Search and install: **Adafruit NeoPixel**

3. **Configure Board Settings:**
   - **Board:** ESP32S3 Dev Module
   - **USB CDC On Boot:** Enabled
   - **USB Mode:** Hardware CDC and JTAG
   - **Port:** Select your ESP32-S3 port

## Bluetooth Configuration

- **Device Name:** `Scoreboard`
- **Service UUID:** `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Characteristic UUID:** `beb5483e-36e1-4688-b7f5-ea07361b26a8`

## Usage

1. **Upload the sketch** to your ESP32-S3
2. **Power on** the ESP32-S3
3. **Connect via Bluetooth** from your mobile app
4. **Send commands** to update the scoreboard display

## Command Protocol

Send data to the BLE characteristic to control the display. The code includes handlers for:
- Total score updates
- Wickets display
- Overs counter
- Target/first innings score
- Individual batsman scores
- Color customization

## Features

- ✅ Bluetooth Low Energy (BLE) connectivity
- ✅ Six independent 7-segment LED displays
- ✅ Customizable colors for each display
- ✅ Low power consumption
- ✅ Real-time score updates
- ✅ Connection status monitoring

## Uploading to ESP32-S3

If you encounter upload issues:

1. **Hold BOOT button** on ESP32-S3
2. **Press RESET button** while holding BOOT
3. **Release BOOT button**
4. **Click Upload** immediately

## Troubleshooting

### No Serial Output
- Set Serial Monitor baud rate to **115200**
- Ensure USB CDC On Boot is **Enabled**
- Press RESET button after opening Serial Monitor

### Bluetooth Not Visible
- Check Serial Monitor for "Starting BLE work!" message
- Restart Bluetooth on your phone
- Try a BLE scanner app to verify device is advertising

### Upload Fails
- Reduce upload speed to **115200** in Tools menu
- Try different USB cable (must be data cable, not charge-only)
- Put board in boot mode manually before uploading

## License

This project is open source and available for personal and educational use.

## Author

Oliver Haine

## Version History

- **v1.0** - Initial ESP32-S3 port from Arduino Uno R4

