/*
 * ESP32-S3 LED Matrix Test - FIXED VERSION
 * 
 * This version includes custom row remapping to fix the 1/8 scan issue
 * 
 * Panel mapping discovered:
 * - Left half (columns 0-31): Rows 0-15 normal order
 * - Right half (columns 32-63): Rows 16-31 REVERSED order
 */

#include <Adafruit_Protomatter.h>

#define PANEL_WIDTH 64
#define PANEL_HEIGHT 32

// Custom matrix class that remaps rows correctly
class RowRemappedMatrix : public Adafruit_GFX {
public:
  Adafruit_Protomatter* protoMatrix;
  
  RowRemappedMatrix(Adafruit_Protomatter* pm) : Adafruit_GFX(PANEL_WIDTH, PANEL_HEIGHT), protoMatrix(pm) {
  }
  
  // Override drawPixel to remap rows and columns correctly
  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    int16_t remappedY;
    
    // Row remapping:
    // 0-7: Software 0-7 -> Physical 0-7 (no change)
    // 8-15: Software 16-23 -> Physical 8-15 (shift -8)
    // 16-23: Software 31-24 -> Physical 16-23 (shift -8, reversed rows)
    // 24-31: Software 15-8 -> Physical 24-31 (shift +16, reversed rows)
    
    if (y < 8) {
      // Software 0-7 -> Physical 0-7
      remappedY = y;
    } else if (y < 16) {
      // Software 8-15 -> Physical 24-31 (reversed)
      remappedY = 39 - y;  // 8->31, 9->30, ..., 15->24
    } else if (y < 24) {
      // Software 16-23 -> Physical 8-15
      remappedY = y - 8;
    } else {
      // Software 24-31 -> Physical 16-23 (reversed)
      remappedY = 47 - y;  // 24->23, 25->22, ..., 31->16
    }
    
    // Column remapping: reverse columns for physical rows 16-31
    int16_t remappedX = x;
    if (remappedY >= 16) {
      remappedX = 63 - x;
    }
    
    protoMatrix->drawPixel(remappedX, remappedY, color);
  }
  
  void fillScreen(uint16_t color) {
    protoMatrix->fillScreen(color);
  }
  
  void show() {
    protoMatrix->show();
  }
  
  uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return protoMatrix->color565(r, g, b);
  }
};

uint8_t rgbPins[]  = {4, 5, 6, 7, 15, 16};  // R1, G1, B1, R2, G2, B2
uint8_t addrPins[] = {17, 18, 8};           // A, B, C
uint8_t clockPin   = 9;
uint8_t latchPin   = 47;
uint8_t oePin      = 48;

Adafruit_Protomatter protomatter(
  PANEL_WIDTH, 1, 6, rgbPins, 3, addrPins, clockPin, latchPin, oePin, true, -2);

RowRemappedMatrix matrix(&protomatter);

uint16_t RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE, BLACK;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32-S3 LED Matrix Test - FIXED VERSION");
  Serial.println("=======================================");
  Serial.println();
  
  if (protomatter.begin() != PROTOMATTER_OK) {
    Serial.println("Failed to initialize protomatter!");
    while(1) delay(1000);
  }
  
  Serial.println("Matrix initialized successfully!");
  Serial.println();
  
  RED = matrix.color565(255, 0, 0);
  GREEN = matrix.color565(0, 255, 0);
  BLUE = matrix.color565(0, 0, 255);
  YELLOW = matrix.color565(255, 255, 0);
  CYAN = matrix.color565(0, 255, 255);
  MAGENTA = matrix.color565(255, 0, 255);
  WHITE = matrix.color565(255, 255, 255);
  BLACK = matrix.color565(0, 0, 0);
  
  matrix.fillScreen(BLACK);
  matrix.show();
  
  // Test patterns
  Serial.println("Testing corrected display...");
  testCorrectedDisplay();
}

void loop() {
  delay(1000);
}

void testCorrectedDisplay() {
  // Test 1: All four corners
  Serial.println("Test 1: Four corners");
  matrix.fillScreen(BLACK);
  matrix.drawPixel(0, 0, RED);           // Top-left
  matrix.drawPixel(63, 0, GREEN);        // Top-right
  matrix.drawPixel(0, 31, BLUE);         // Bottom-left
  matrix.drawPixel(63, 31, WHITE);       // Bottom-right
  matrix.show();
  delay(3000);
  
  // Test 2: Vertical lines
  Serial.println("Test 2: Vertical lines");
  matrix.fillScreen(BLACK);
  for (int y = 0; y < PANEL_HEIGHT; y++) {
    matrix.drawPixel(0, y, RED);   // Left edge
    matrix.drawPixel(63, y, GREEN); // Right edge
  }
  matrix.show();
  delay(3000);
  
  // Test 3: Horizontal lines
  Serial.println("Test 3: Horizontal lines");
  matrix.fillScreen(BLACK);
  for (int x = 0; x < PANEL_WIDTH; x++) {
    matrix.drawPixel(x, 0, RED);    // Top
    matrix.drawPixel(x, 31, GREEN); // Bottom
  }
  matrix.show();
  delay(3000);
  
  // Test 4: Full screen test
  Serial.println("Test 4: Full screen white");
  matrix.fillScreen(WHITE);
  matrix.show();
  delay(3000);
  
  // Test 5: Text
  Serial.println("Test 5: Text (if library supports it)");
  matrix.fillScreen(BLACK);
  // Try to display text if GFX library is available
  matrix.setTextColor(GREEN);
  matrix.setTextSize(1);
  matrix.setCursor(2, 4);
  matrix.print("WORKING!");
  matrix.show();
  delay(3000);
  
  Serial.println();
  Serial.println("Tests complete. Check if patterns look correct!");
}

