/*
 * Row Mapping Reverse Engineering Test
 * 
 * This sketch will systematically draw pixel rows to reverse-engineer
 * the panel's actual row addressing scheme
 * 
 * Since row 1 works correctly, we'll draw pixels at specific rows
 * to see where they actually appear on the physical panel
 */

#include <Adafruit_Protomatter.h>

#define PANEL_WIDTH 64
#define PANEL_HEIGHT 32

uint8_t rgbPins[]  = {4, 5, 6, 7, 15, 16};  // R1, G1, B1, R2, G2, B2 (standard)
uint8_t addrPins[] = {17, 18, 8};           // A, B, C (NEW WIRING)
uint8_t clockPin   = 9;                      // CLK (NEW WIRING)
uint8_t latchPin   = 47;                     // LAT (NEW WIRING)
uint8_t oePin      = 48;                     // OE (NEW WIRING)

Adafruit_Protomatter matrix(
  PANEL_WIDTH,      // Width
  1,                // Bit depth
  6,                // RGB count
  rgbPins,          // RGB pins
  3,                // Address count
  addrPins,         // Address pins
  clockPin,         // Clock pin
  latchPin,         // Latch pin
  oePin,            // OE pin
  true,             // Double-buffered
  -2                // Row pattern
);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=== ROW MAPPING TEST (manual step) ===");
  Serial.println("Instructions:");
  Serial.println("- After each step, press ENTER (send any character) in Serial Monitor to continue");
  Serial.println("- Describe exactly where you see the red pixels (row number from top)");
  Serial.println();
  
  Serial.println("Row Mapping Reverse Engineering Test");
  Serial.println("====================================");
  Serial.println();
  
  if (matrix.begin() != PROTOMATTER_OK) {
    Serial.println("Failed to initialize matrix!");
    while(1) delay(1000);
  }
  
  Serial.println("Matrix initialized. Starting row mapping test...");
  Serial.println("(Note: Colors reduced to 50% brightness)");
  Serial.println();
  Serial.println("Current pin configuration:");
  Serial.println("RGB: R1=4, G1=5, B1=6, R2=7, G2=15, B2=16");
  Serial.println("Address: A=17, B=18, C=8");
  Serial.println("Control: CLK=9, LAT=47, OE=48");
  Serial.println();
  Serial.println("First we'll clear the display.");
  testRowMapping();
}

void loop() {
  delay(1000);
}

// Remap software row to physical position
int remapRow(int softwareY) {
  // Physical order observed:
  // 0-7: Software 0-7 (no change)
  // 8-15: Software 16-23 (shift -8)
  // 16-23: Software 31-24 (shift -8, reversed)
  // 24-31: Software 15-8 (shift +16, reversed)
  
  if (softwareY < 8) {
    // Software 0-7 -> Physical 0-7
    return softwareY;
  } else if (softwareY < 16) {
    // Software 8-15 -> Physical 24-31 (reversed)
    return 39 - softwareY;  // 8->31, 9->30, ..., 15->24
  } else if (softwareY < 24) {
    // Software 16-23 -> Physical 8-15
    return softwareY - 8;
  } else {
    // Software 24-31 -> Physical 16-23 (reversed)
    return 47 - softwareY;  // 24->23, 25->22, ..., 31->16
  }
}

void testRowMapping() {
  Serial.println("Drawing rows ONE BY ONE with remapping, leaving them on...");
  Serial.println("Watch where each row appears!");
  Serial.println();
  
  // Draw each row one at a time, leaving them visible
  for (int y = 0; y < 32; y++) {
    uint8_t r = (y * 8) % 256;
    uint8_t g = ((y * 8) + 85) % 256;
    uint8_t b = ((y * 8) + 170) % 256;
    uint16_t color = matrix.color565(r, g, b);
    
    // Get remapped physical position
    int physicalY = remapRow(y);
    
    // Draw full row across width at remapped position
    for (int x = 0; x < 64; x++) {
      matrix.drawPixel(x, physicalY, color);
    }
    matrix.show();
    
    Serial.print("Software row "); Serial.print(y); 
    Serial.print(" -> Physical row "); Serial.println(physicalY);
    delay(800);
  }
  
  Serial.println();
  Serial.println("All rows drawn with remapping!");
  Serial.println("Rainbow should now be in correct order from top to bottom");
  
  delay(5000);
  
  // Now test columns
  Serial.println();
  Serial.println("NOW testing COLUMNS...");
  Serial.println("Drawing columns ONE BY ONE with rainbow, leaving them on...");
  Serial.println();
  
  // Clear screen first
  matrix.fillScreen(matrix.color565(0, 0, 0));
  matrix.show();
  delay(1000);
  
  // Draw each column one at a time, leaving them visible
  for (int x = 0; x < 64; x++) {
    uint8_t r = (x * 4) % 256;
    uint8_t g = ((x * 4) + 85) % 256;
    uint8_t b = ((x * 4) + 170) % 256;
    uint16_t color = matrix.color565(r, g, b);
    
    // Draw full column down height
    for (int y = 0; y < 32; y++) {
      int physicalY = remapRow(y);
      
      // Reverse columns for physical rows 16-23 and 24-31 (software rows 24-31 and 8-15)
      int physicalX = x;
      if (physicalY >= 16) {
        physicalX = 63 - x;
      }
      
      matrix.drawPixel(physicalX, physicalY, color);
    }
    matrix.show();
    
    Serial.print("Column "); Serial.println(x);
    delay(200);
  }
  
  Serial.println();
  Serial.println("All columns drawn!");
  Serial.println("Tell me if rainbow appears correctly left to right");
}

void waitForEnter() {
  while (!Serial.available()) {
    delay(10);
  }
  // flush buffer
  while (Serial.available()) {
    Serial.read();
  }
}

