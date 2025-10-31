/*
 * ESP32-S3 LED Matrix Test Sketch
 * 
 * This sketch tests a 64x32 P3 LED matrix panel with ESP32-S3
 * Compatible with HUB75 interface LED matrices
 * 
 * Hardware Requirements:
 * - ESP32-S3 development board
 * - 64x32 P3 LED matrix panel (HUB75 interface)
 * - External 5V power supply (recommended for larger displays)
 * 
 * Wiring (HUB75 to ESP32-S3):
 * R1  -> GPIO 4
 * G1  -> GPIO 5
 * B1  -> GPIO 6
 * R2  -> GPIO 7
 * G2  -> GPIO 15
 * B2  -> GPIO 16
 * A   -> GPIO 21
 * B   -> GPIO 22
 * C   -> GPIO 23
 * D   -> GPIO 2
 * CLK -> GPIO 14
 * LAT -> GPIO 26
 * OE  -> GPIO 25
 * 
 * Libraries Required:
 * - ESP32-HUB75-MatrixPanel-DMA (install via Library Manager)
 * 
 * Serial Commands:
 * - 'c' - Color test
 * - 't' - Text test
 * - 'p' - Pattern test
 * - 'a' - Animation test
 * - 'r' - Rainbow test
 * - 's' - Scrolling text test
 * - 'h' - Help
 * - '0' - Clear display
 */

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// Panel configuration
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 32
#define PANEL_CHAIN 1

// Create matrix instance
MatrixPanel_I2S_DMA matrix;

// Color definitions
#define RED     matrix.color565(255, 0, 0)
#define GREEN   matrix.color565(0, 255, 0)
#define BLUE    matrix.color565(0, 0, 255)
#define YELLOW  matrix.color565(255, 255, 0)
#define CYAN    matrix.color565(0, 255, 255)
#define MAGENTA matrix.color565(255, 0, 255)
#define WHITE   matrix.color565(255, 255, 255)
#define BLACK   matrix.color565(0, 0, 0)

// Animation variables
int animationStep = 0;
unsigned long lastUpdate = 0;
const unsigned long animationDelay = 100; // milliseconds

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32-S3 LED Matrix Test");
  Serial.println("=========================");
  Serial.println("Initializing LED matrix...");
  
  // Initialize the matrix
  HUB75_I2S_CFG mxconfig(
    PANEL_WIDTH,    // Module width
    PANEL_HEIGHT,   // Module height
    PANEL_CHAIN     // Chain length
  );
  
  // Configure GPIO pins for ESP32-S3
  mxconfig.gpio.r1 = 4;
  mxconfig.gpio.g1 = 5;
  mxconfig.gpio.b1 = 6;
  mxconfig.gpio.r2 = 7;
  mxconfig.gpio.g2 = 15;
  mxconfig.gpio.b2 = 16;
  mxconfig.gpio.a = 21;
  mxconfig.gpio.b = 22;
  mxconfig.gpio.c = 23;
  mxconfig.gpio.d = 2;
  mxconfig.gpio.e = -1; // Not used for 32-pixel height
  mxconfig.gpio.lat = 26;
  mxconfig.gpio.oe = 25;
  mxconfig.gpio.clk = 14;
  
  // Additional configuration
  mxconfig.driver = HUB75_I2S_CFG::FM6126A; // Common driver
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M; // 10MHz clock
  
  // Initialize matrix with configuration
  if (!matrix.begin(mxconfig)) {
    Serial.println("Matrix initialization failed!");
    while (1) {
      delay(1000);
    }
  }
  
  Serial.println("Matrix initialized successfully!");
  
  // Clear display
  matrix.fillScreen(BLACK);
  matrix.flush();
  
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
  
  // Run continuous animations if no specific test is active
  static unsigned long lastAnimation = 0;
  if (millis() - lastAnimation > 2000) {
    runContinuousAnimation();
    lastAnimation = millis();
  }
  
  delay(10);
}

void handleCommand(char command) {
  switch (command) {
    case 'c':
      colorTest();
      break;
    case 't':
      textTest();
      break;
    case 'p':
      patternTest();
      break;
    case 'a':
      animationTest();
      break;
    case 'r':
      rainbowTest();
      break;
    case 's':
      scrollingTextTest();
      break;
    case 'h':
      printHelp();
      break;
    case '0':
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
  Serial.println("t - Text test");
  Serial.println("p - Pattern test");
  Serial.println("a - Animation test");
  Serial.println("r - Rainbow test");
  Serial.println("s - Scrolling text test");
  Serial.println("h - Show this help");
  Serial.println("0 - Clear display");
  Serial.println();
}

void displayStartupMessage() {
  matrix.fillScreen(BLACK);
  matrix.setTextColor(GREEN);
  matrix.setTextSize(1);
  matrix.setCursor(2, 8);
  matrix.print("ESP32-S3");
  matrix.setCursor(2, 20);
  matrix.print("LED TEST");
  matrix.flush();
  delay(2000);
}

void clearDisplay() {
  matrix.fillScreen(BLACK);
  matrix.flush();
  Serial.println("Display cleared");
}

void colorTest() {
  Serial.println("Running color test...");
  
  // Test each color
  uint16_t colors[] = {RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE};
  const char* colorNames[] = {"RED", "GREEN", "BLUE", "YELLOW", "CYAN", "MAGENTA", "WHITE"};
  
  for (int i = 0; i < 7; i++) {
    matrix.fillScreen(colors[i]);
    matrix.flush();
    Serial.print("Displaying: ");
    Serial.println(colorNames[i]);
    delay(1000);
  }
  
  clearDisplay();
  Serial.println("Color test complete");
}

void textTest() {
  Serial.println("Running text test...");
  
  matrix.fillScreen(BLACK);
  matrix.setTextColor(WHITE);
  matrix.setTextSize(1);
  
  // Display different text sizes and colors
  matrix.setCursor(5, 8);
  matrix.setTextColor(RED);
  matrix.print("HELLO");
  
  matrix.setCursor(5, 20);
  matrix.setTextColor(GREEN);
  matrix.print("WORLD");
  
  matrix.flush();
  delay(3000);
  
  // Test larger text
  matrix.fillScreen(BLACK);
  matrix.setTextSize(2);
  matrix.setTextColor(CYAN);
  matrix.setCursor(10, 10);
  matrix.print("ESP32");
  matrix.flush();
  delay(2000);
  
  clearDisplay();
  Serial.println("Text test complete");
}

void patternTest() {
  Serial.println("Running pattern test...");
  
  // Checkerboard pattern
  matrix.fillScreen(BLACK);
  for (int y = 0; y < PANEL_HEIGHT; y++) {
    for (int x = 0; x < PANEL_WIDTH; x++) {
      if ((x + y) % 2 == 0) {
        matrix.drawPixel(x, y, RED);
      } else {
        matrix.drawPixel(x, y, BLUE);
      }
    }
  }
  matrix.flush();
  delay(2000);
  
  // Gradient pattern
  matrix.fillScreen(BLACK);
  for (int x = 0; x < PANEL_WIDTH; x++) {
    uint16_t color = matrix.color565(x * 4, 0, 255 - (x * 4));
    for (int y = 0; y < PANEL_HEIGHT; y++) {
      matrix.drawPixel(x, y, color);
    }
  }
  matrix.flush();
  delay(2000);
  
  // Circle pattern
  matrix.fillScreen(BLACK);
  int centerX = PANEL_WIDTH / 2;
  int centerY = PANEL_HEIGHT / 2;
  for (int radius = 1; radius < 16; radius++) {
    uint16_t color = matrix.color565(radius * 16, 0, 255 - (radius * 16));
    drawCircle(centerX, centerY, radius, color);
  }
  matrix.flush();
  delay(2000);
  
  clearDisplay();
  Serial.println("Pattern test complete");
}

void animationTest() {
  Serial.println("Running animation test...");
  
  for (int frame = 0; frame < 30; frame++) {
    matrix.fillScreen(BLACK);
    
    // Moving rectangle
    int x = (frame * 2) % (PANEL_WIDTH - 10);
    int y = (frame * 3) % (PANEL_HEIGHT - 10);
    matrix.fillRect(x, y, 10, 10, RED);
    
    // Pulsing circle
    int centerX = PANEL_WIDTH / 2;
    int centerY = PANEL_HEIGHT / 2;
    int radius = 5 + (sin(frame * 0.2) * 5);
    drawCircle(centerX, centerY, radius, GREEN);
    
    matrix.flush();
    delay(100);
  }
  
  clearDisplay();
  Serial.println("Animation test complete");
}

void rainbowTest() {
  Serial.println("Running rainbow test...");
  
  for (int cycle = 0; cycle < 3; cycle++) {
    for (int hue = 0; hue < 360; hue += 5) {
      matrix.fillScreen(BLACK);
      
      // Create rainbow gradient
      for (int x = 0; x < PANEL_WIDTH; x++) {
        uint16_t color = hsvToRgb((hue + x * 5) % 360, 255, 255);
        for (int y = 0; y < PANEL_HEIGHT; y++) {
          matrix.drawPixel(x, y, color);
        }
      }
      
      matrix.flush();
      delay(50);
    }
  }
  
  clearDisplay();
  Serial.println("Rainbow test complete");
}

void scrollingTextTest() {
  Serial.println("Running scrolling text test...");
  
  const char* text = "ESP32-S3 LED Matrix Test - Scrolling Text Demo!";
  int textLength = strlen(text);
  int scrollWidth = textLength * 6; // Approximate character width
  
  for (int pos = PANEL_WIDTH; pos > -scrollWidth; pos--) {
    matrix.fillScreen(BLACK);
    matrix.setTextColor(WHITE);
    matrix.setTextSize(1);
    matrix.setCursor(pos, 12);
    matrix.print(text);
    matrix.flush();
    delay(50);
  }
  
  clearDisplay();
  Serial.println("Scrolling text test complete");
}

void runContinuousAnimation() {
  static int animStep = 0;
  
  matrix.fillScreen(BLACK);
  
  // Simple breathing effect
  int brightness = 128 + 127 * sin(animStep * 0.1);
  uint16_t color = matrix.color565(brightness, brightness / 2, 0);
  
  // Draw a simple pattern that changes
  for (int i = 0; i < 8; i++) {
    int x = (animStep + i * 8) % PANEL_WIDTH;
    int y = PANEL_HEIGHT / 2 + sin((animStep + i) * 0.2) * 10;
    matrix.drawPixel(x, y, color);
    matrix.drawPixel(x, y + 1, color);
    matrix.drawPixel(x, y - 1, color);
  }
  
  matrix.flush();
  animStep++;
}

// Helper function to draw a circle
void drawCircle(int centerX, int centerY, int radius, uint16_t color) {
  for (int x = -radius; x <= radius; x++) {
    for (int y = -radius; y <= radius; y++) {
      if (x * x + y * y <= radius * radius) {
        int pixelX = centerX + x;
        int pixelY = centerY + y;
        if (pixelX >= 0 && pixelX < PANEL_WIDTH && pixelY >= 0 && pixelY < PANEL_HEIGHT) {
          matrix.drawPixel(pixelX, pixelY, color);
        }
      }
    }
  }
}

// Helper function to convert HSV to RGB
uint16_t hsvToRgb(int h, int s, int v) {
  int r, g, b;
  
  if (s == 0) {
    r = g = b = v;
  } else {
    int region = h / 43;
    int remainder = (h - (region * 43)) * 6;
    
    int p = (v * (255 - s)) >> 8;
    int q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    int t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
    
    switch (region) {
      case 0: r = v; g = t; b = p; break;
      case 1: r = q; g = v; b = p; break;
      case 2: r = p; g = v; b = t; break;
      case 3: r = p; g = q; b = v; break;
      case 4: r = t; g = p; b = v; break;
      default: r = v; g = p; b = q; break;
    }
  }
  
  return matrix.color565(r, g, b);
}
