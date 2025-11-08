/*
 * ESP32-S3 LED Matrix Test with Adafruit Protomatter
 * 
 * This sketch tests a 64x32 P3 LED matrix panel with ESP32-S3 using Adafruit Protomatter
 * 
 * INSTALLATION INSTRUCTIONS:
 * 1. Tools → Manage Libraries in Arduino IDE
 * 2. Search for "Protomatter" and install "Adafruit Protomatter"
 * 3. Install "Adafruit GFX Library" (dependency)
 * 
 * Wiring (HUB75 to ESP32-S3):
 * R1  -> GPIO 4
 * G1  -> GPIO 5
 * B1  -> GPIO 6
 * R2  -> GPIO 7
 * G2  -> GPIO 15
 * B2  -> GPIO 16
 * A   -> GPIO 21
 * B   -> GPIO 47
 * C   -> GPIO 48
 * CLK -> GPIO 14
 * LAT -> GPIO 1
 * OE  -> GPIO 0
 */

#include <Adafruit_Protomatter.h>

// Panel configuration
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 32

// Pin definitions for ESP32-S3 - NEW WIRING
uint8_t rgbPins[]  = {4, 5, 6, 7, 15, 16};  // R1, G1, B1, R2, G2, B2 (standard order)
uint8_t addrPins[] = {17, 18, 8};           // A, B, C (NEW)
uint8_t clockPin   = 9;                      // CLK (NEW)
uint8_t latchPin   = 47;                     // LAT (NEW)
uint8_t oePin      = 48;                     // OE (NEW)

// Custom matrix class that remaps rows and columns correctly
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

// Create protomatter instance
Adafruit_Protomatter protomatter(
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

// Create remapped matrix wrapper
RowRemappedMatrix matrix(&protomatter);

// Color definitions
uint16_t RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE, BLACK;

// Scrolling text state
static bool scrollingActive = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32-S3 LED Matrix Test with Adafruit Protomatter");
  Serial.println("===================================================");
  Serial.println();
  
  // Initialize matrix
  Serial.println("Initializing LED matrix...");
  ProtomatterStatus status = protomatter.begin();
  
  if (status != PROTOMATTER_OK) {
    Serial.print("Matrix initialization FAILED with error: ");
    Serial.println(status);
    Serial.println("Try different row pattern or check wiring");
    while (1) {
      delay(1000);
    }
  }
  
  Serial.println("Matrix initialized successfully!");
  Serial.println();
  
  // Initialize color definitions
  RED = matrix.color565(255, 0, 0);
  GREEN = matrix.color565(0, 255, 0);
  BLUE = matrix.color565(0, 0, 255);
  YELLOW = matrix.color565(255, 255, 0);
  CYAN = matrix.color565(0, 255, 255);
  MAGENTA = matrix.color565(255, 0, 255);
  WHITE = matrix.color565(255, 255, 255);
  BLACK = matrix.color565(0, 0, 0);
  
  // Clear display
  matrix.fillScreen(BLACK);
  matrix.show();
  
  // Display startup message
  displayStartupMessage();
  
  // Print help
  printHelp();
}

void loop() {
  // Handle serial commands
  if (Serial.available()) {
    char command = Serial.read();
    handleCommand(command);
  }
  
  // Run scrolling if active
  if (scrollingActive) {
    scrollingTextTest();
  }
  
  delay(10);
}

void handleCommand(char command) {
  switch (command) {
    case 'c':
      colorTest();
      break;
    case 'o': {
      uint8_t r = (matrix.getRotation() + 1) % 4;
      matrix.setRotation(r);
      Serial.print("Rotation set to ");
      Serial.println(r);
      break;
    }
    case 'm':
      mappingTest();
      break;
    case 'x':
      simplePixelTest();
      break;
    case 's':
      scrollingActive = true;
      Serial.println("Scrolling started. Press '0' to stop.");
      break;
    case 'h':
      printHelp();
      break;
    case '0':
      scrollingActive = false;
      clearDisplay();
      break;
    default:
      Serial.println("Unknown command. Type 'h' for help.");
      break;
  }
}

void printHelp() {
  Serial.println("\nAvailable Commands:");
  Serial.println("c - Color test");
  Serial.println("o - Toggle rotation (0-3)");
  Serial.println("m - Address mapping test (tries A/B/C orders)");
  Serial.println("x - Simple pixel test");
  Serial.println("s - Scrolling text test");
  Serial.println("h - Show this help");
  Serial.println("0 - Clear display");
  Serial.println();
}

void displayStartupMessage() {
  matrix.fillScreen(BLACK);
  matrix.setTextColor(GREEN);
  matrix.setTextSize(1);
  matrix.setCursor(2, 0);
  matrix.print("ESP32-S3");
  matrix.setCursor(2, 12);
  matrix.print("PROTOMATTER");
  matrix.show();
  delay(2000);
}

void clearDisplay() {
  matrix.fillScreen(BLACK);
  matrix.show();
  Serial.println("Display cleared");
}

void colorTest() {
  Serial.println("Color Test");
  
  matrix.fillScreen(RED);
  matrix.show();
  Serial.println("Red");
  delay(1000);
  
  matrix.fillScreen(GREEN);
  matrix.show();
  Serial.println("Green");
  delay(1000);
  
  matrix.fillScreen(BLUE);
  matrix.show();
  Serial.println("Blue");
  delay(1000);
  
  matrix.fillScreen(YELLOW);
  matrix.show();
  Serial.println("Yellow");
  delay(1000);
  
  matrix.fillScreen(CYAN);
  matrix.show();
  Serial.println("Cyan");
  delay(1000);
  
  matrix.fillScreen(MAGENTA);
  matrix.show();
  Serial.println("Magenta");
  delay(1000);
  
  matrix.fillScreen(WHITE);
  matrix.show();
  Serial.println("White");
  delay(1000);
  
  matrix.fillScreen(BLACK);
  matrix.show();
  Serial.println("Black");
  delay(1000);
  
  Serial.println("Color test complete");
}

void simplePixelTest() {
  Serial.println("=== SIMPLE PIXEL TEST ===");
  Serial.println("Testing individual pixels");
  Serial.println();
  
  // Test 1: Single red pixel in top-left corner
  Serial.println("Test 1: Single red pixel (0,0)");
  matrix.fillScreen(BLACK);
  matrix.drawPixel(0, 0, RED);
  matrix.show();
  waitForInput();
  
  // Test 2: Single green pixel in top-right corner
  Serial.println("Test 2: Single green pixel (63,0)");
  matrix.fillScreen(BLACK);
  matrix.drawPixel(63, 0, GREEN);
  matrix.show();
  waitForInput();
  
  // Test 3: Single blue pixel in bottom-left corner
  Serial.println("Test 3: Single blue pixel (0,31)");
  matrix.fillScreen(BLACK);
  matrix.drawPixel(0, 31, BLUE);
  matrix.show();
  waitForInput();
  
  // Test 4: Single white pixel in bottom-right corner
  Serial.println("Test 4: Single white pixel (63,31)");
  matrix.fillScreen(BLACK);
  matrix.drawPixel(63, 31, WHITE);
  matrix.show();
  waitForInput();
  
  // Test 5: Horizontal line across top
  Serial.println("Test 5: Red horizontal line across top");
  matrix.fillScreen(BLACK);
  for (int x = 0; x < PANEL_WIDTH; x++) {
    matrix.drawPixel(x, 0, RED);
  }
  matrix.show();
  waitForInput();
  
  // Test 6: Vertical line down left side
  Serial.println("Test 6: Green vertical line down left");
  matrix.fillScreen(BLACK);
  // Both methods: drawFastVLine and per-pixel
  matrix.drawFastVLine(0, 0, PANEL_HEIGHT, GREEN);
  matrix.show();
  waitForInput();
  
  // Alternative: try rotated orientation if vertical still wrong
  Serial.println("Test 6b: Same vertical after rotation (diagnostic)");
  matrix.fillScreen(BLACK);
  matrix.setRotation((matrix.getRotation() + 1) % 4);
  matrix.drawFastVLine(0, 0, PANEL_HEIGHT, GREEN);
  matrix.show();
  waitForInput();
  // Restore rotation
  matrix.setRotation((matrix.getRotation() + 3) % 4);
  
  Serial.println("=== TEST COMPLETE ===");
  Serial.println("If pixels looked correct, Protomatter is working!");
  Serial.println("If still jumbled, we need to adjust the row pattern");
}

void waitForInput() {
  Serial.println("Press any key to continue...");
  while (!Serial.available()) {
    delay(10);
  }
  while (Serial.available()) {
    Serial.read();
  }
  Serial.println();
}

void scrollingTextTest() {
  Serial.println("=== SCROLLING TEXT TEST ===");
  
  static int16_t x = PANEL_WIDTH; // Start text off-screen to the right
  const char* text = "ESP32-S3 LED Matrix - Scrolling Text Test! ";
  
  matrix.fillScreen(BLACK);
  matrix.setTextWrap(false); // Important for scrolling!
  matrix.setTextSize(1);
  matrix.setTextColor(CYAN);
  
  // Set cursor position - centered vertically
  // With 8-pixel tall font, center is around HEIGHT/2 - 4
  matrix.setCursor(x, PANEL_HEIGHT / 2 - 4);
  matrix.print(text);
  matrix.show();
  
  x--;
  
  // Reset position when text has scrolled completely off-screen
  if (x < -(int16_t)(strlen(text) * 6)) {
    x = PANEL_WIDTH;
  }
  
  delay(30);
}

// --- Address mapping diagnostic ---
void tryAddrConfig(uint8_t a, uint8_t b, uint8_t c, const char* label) {
  uint8_t order[3] = { a, b, c };
  Serial.print("Testing mapping: ");
  Serial.println(label);
  
  Adafruit_Protomatter testMx(
    PANEL_WIDTH,
    1,
    6,
    rgbPins,
    3,
    order,
    clockPin,
    latchPin,
    oePin,
    true,
    -1
  );
  
  ProtomatterStatus st = testMx.begin();
  if (st != PROTOMATTER_OK) {
    Serial.print("  begin() failed: ");
    Serial.println(st);
    return;
  }
  
  testMx.fillScreen(BLACK);
  // Draw simple diagnostics
  testMx.drawFastHLine(0, 0, PANEL_WIDTH, RED);
  testMx.drawFastHLine(0, PANEL_HEIGHT/2, PANEL_WIDTH, RED);
  testMx.drawFastVLine(0, 0, PANEL_HEIGHT, GREEN);
  testMx.drawFastVLine(PANEL_WIDTH/2, 0, PANEL_HEIGHT, GREEN);
  testMx.setCursor(2, 10);
  testMx.setTextColor(WHITE);
  testMx.setTextSize(1);
  testMx.print(label);
  testMx.show();
  delay(2500);
  testMx.fillScreen(0);
  testMx.show();
}

void mappingTest() {
  Serial.println("=== ADDRESS MAPPING TEST ===");
  Serial.println("Trying different A/B/C pin orders. Watch vertical lines and text.");
  
  tryAddrConfig(21, 47, 48, "A=21, B=47, C=48 (ABC)");
  tryAddrConfig(21, 48, 47, "A=21, B=48, C=47 (ACB)");
  tryAddrConfig(47, 21, 48, "A=47, B=21, C=48 (BAC)");
  tryAddrConfig(47, 48, 21, "A=47, B=48, C=21 (BCA)");
  tryAddrConfig(48, 21, 47, "A=48, B=21, C=47 (CAB)");
  tryAddrConfig(48, 47, 21, "A=48, B=47, C=21 (CBA)");
  
  Serial.println("=== TEST COMPLETE ===");
  Serial.println("Reply with which mapping looked correct.");
}
