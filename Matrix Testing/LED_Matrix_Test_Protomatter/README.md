# LED Matrix Test - ESP32-S3 with Adafruit Protomatter

This sketch tests a 64x32 P3 LED matrix panel with ESP32-S3 using Adafruit Protomatter library.

## Hardware

- **Panel**: 64x32 P3 LED Matrix (HUB75 interface, 1/8 scan rate)
- **Controller**: ESP32-S3 Development Board
- **Wiring**: See diagram below

## Installation

1. Install Arduino IDE with ESP32 board support:
   - Add board URL: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   - Board: ESP32S3 Dev Module
   - USB CDC On Boot: Enabled

2. Install required libraries via Library Manager:
   - **Adafruit Protomatter**
   - **Adafruit GFX Library** (dependency)

3. Upload the sketch to your ESP32-S3

## Wiring Diagram

```
HUB75 Panel    ESP32-S3
-----------    --------
R1   ----->    GPIO 4
G1   ----->    GPIO 5
B1   ----->    GPIO 6
R2   ----->    GPIO 7
G2   ----->    GPIO 15
B2   ----->    GPIO 16
A    ----->    GPIO 17
B    ----->    GPIO 18
C    ----->    GPIO 8
CLK  ----->    GPIO 9
LAT  ----->    GPIO 47
OE   ----->    GPIO 48

5V   ----->    5V power supply (external recommended)
GND  ----->    Common ground
```

## Panel Remapping

This specific panel has a non-standard row and column addressing scheme. The sketch includes a custom `RowRemappedMatrix` class that automatically handles the remapping.

### Row Remapping:
- Software rows 0-7 → Physical rows 0-7 (no change)
- Software rows 8-15 → Physical rows 24-31 (reversed)
- Software rows 16-23 → Physical rows 8-15 (no change)
- Software rows 24-31 → Physical rows 16-23 (reversed)

### Column Remapping:
- Columns are reversed for physical rows 16-31

## Usage

### Serial Commands

Open Serial Monitor at 115200 baud and send:

- **c** - Color test (cycles through all colors)
- **o** - Toggle rotation (0-3)
- **m** - Address mapping test (tries different pin combinations)
- **x** - Simple pixel test (corner pixels and lines)
- **h** - Show help
- **0** - Clear display

### Example Output

```
ESP32-S3 LED Matrix Test with Adafruit Protomatter
===================================================

Available Commands:
c - Color test
o - Toggle rotation (0-3)
m - Address mapping test (tries A/B/C orders)
x - Simple pixel test
h - Show this help
0 - Clear display
```

## Custom Remapping Class

The `RowRemappedMatrix` class extends `Adafruit_GFX` and overrides `drawPixel()` to automatically apply the correct remapping:

```cpp
class RowRemappedMatrix : public Adafruit_GFX {
  // Automatically remaps rows and columns
  void drawPixel(int16_t x, int16_t y, uint16_t color) override;
};
```

All standard GFX drawing functions work correctly with this class:
- `drawPixel()`, `fillScreen()`, `show()`
- `drawLine()`, `drawRect()`, `drawCircle()`
- `print()`, `setCursor()`, `setTextColor()`
- Any other `Adafruit_GFX` function

## Troubleshooting

### No display output
- Check wiring connections
- Verify external 5V power supply is connected
- Ensure "USB CDC On Boot" is enabled in board settings
- Try different USB cables

### Jumbled or scrambled display
- Make sure you're using the latest version of this sketch (includes remapping)
- Check that libraries are up to date
- Verify pin definitions match your wiring

### Compilation errors
- Ensure all required libraries are installed
- Check that ESP32 board support is properly installed
- Try cleaning the build cache in Arduino IDE

## Development Notes

This panel required extensive testing to reverse-engineer the row and column mapping. Key discoveries:

1. Panel uses 1/8 scan rate (address pins A, B, C only - no D pin)
2. Physical row blocks appear in non-standard order: 1, 3, 4, 2
3. Some row blocks require reversed column addressing
4. Adafruit Protomatter library handles timing and refresh automatically

## License

This code is provided as-is for testing LED matrix panels with ESP32-S3.

## Related Files

- `LED_Matrix_Test_FIXED.ino` - Simpler test version
- `ROW_MAPPING_TEST.ino` - Diagnostic tool for mapping discovery

