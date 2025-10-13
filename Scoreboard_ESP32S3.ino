#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>

// LED Display Configuration
#define TOTALPIN 2
#define WICKETSPIN 3
#define OVERSPIN 4
#define FIRSTINNINGPIN 5
#define BATAPIN 6
#define BATBPIN 7
#define PIXELSPERDIGIT 63

// LED Strips
Adafruit_NeoPixel totalStrip(PIXELSPERDIGIT * 3, TOTALPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel wicketsStrip(PIXELSPERDIGIT, WICKETSPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel oversStrip(PIXELSPERDIGIT * 2, OVERSPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel targetStrip(PIXELSPERDIGIT * 3, FIRSTINNINGPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel batAStrip(PIXELSPERDIGIT * 3, BATAPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel batBStrip(PIXELSPERDIGIT * 3, BATBPIN, NEO_GRB + NEO_KHZ800);

// Bluetooth service and characteristic UUIDs
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "87654321-4321-4321-4321-cba987654321"

// BLE objects
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Function declarations
void processReceivedData(String data);
void parseJsonData(String jsonData);
void displayNumber(Adafruit_NeoPixel &strip, int digitToDisplay, uint32_t colourToUse);
void displayDigit(Adafruit_NeoPixel &strip, int digit, int offset, uint32_t colour);
void updateDisplay(int batsmanA, int batsmanB, int total, int overs, int wickets, int target = -1);
void resetScoreboard();
void testMode();

// BLE Server Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device disconnected");
    }
};

// BLE Characteristic Callbacks
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String receivedData = pCharacteristic->getValue().c_str();

      if (receivedData.length() > 0) {
        Serial.print("Received data: ");
        Serial.println(receivedData);
        
        // Process the received data
        processReceivedData(receivedData);
      }
    }
};

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32 S3 - Bluetooth Scoreboard");
  
  // Initialize LED strips
  totalStrip.begin();
  wicketsStrip.begin();
  oversStrip.begin();
  targetStrip.begin();
  batAStrip.begin();
  batBStrip.begin();
  
  // Clear all strips
  totalStrip.clear();
  wicketsStrip.clear();
  oversStrip.clear();
  targetStrip.clear();
  batAStrip.clear();
  batBStrip.clear();
  totalStrip.show();
  wicketsStrip.show();
  oversStrip.show();
  targetStrip.show();
  batAStrip.show();
  batBStrip.show();
  
  // Initialize BLE
  BLEDevice::init("Scoreboard");
  
  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // Create BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE
                    );
  
  // Set callbacks for characteristic
  pCharacteristic->setCallbacks(new MyCallbacks());
  
  // Add descriptor for notifications (optional, but good practice)
  pCharacteristic->addDescriptor(new BLE2902());
  
  // Set initial value
  pCharacteristic->setValue("Ready");
  
  // Start the service
  pService->start();
  
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // helps with iPhone connections
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE device is now advertising as 'Scoreboard'");
  Serial.println("Waiting for connections...");
}

void loop() {
  // Handle disconnecting and reconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack time to get ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }
  
  // Handle new connection
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
  
  // Small delay to prevent watchdog issues on ESP32
  delay(10);
}

void processReceivedData(String data) {
  // Add your data processing logic here
  // For example, parsing score updates, commands, etc.
  
  Serial.print("Processing: ");
  Serial.println(data);
  
  // Try to parse as JSON first
  if (data.startsWith("{")) {
    // Handle JSON data
    Serial.println("Received JSON data");
    parseJsonData(data);
  } else if (data.startsWith("SCORE:")) {
    // Handle legacy score update format
    String scoreData = data.substring(6); // Remove "SCORE:" prefix
    Serial.print("Legacy score update: ");
    Serial.println(scoreData);
  } else if (data.startsWith("RESET")) {
    // Handle reset command
    Serial.println("Resetting scoreboard");
    resetScoreboard();
  } else {
    // Handle other data
    Serial.print("General data: ");
    Serial.println(data);
  }
}

void parseJsonData(String jsonData) {
  // Simple JSON parsing for scoreboard data
  // This is a basic implementation - you might want to use a proper JSON library
  
  if (jsonData.indexOf("\"type\":\"score\"") > 0) {
    Serial.println("Score update received");
    
    // Extract batsmanA
    int batsmanAStart = jsonData.indexOf("\"batsmanA\":") + 11;
    int batsmanAEnd = jsonData.indexOf(",", batsmanAStart);
    if (batsmanAEnd == -1) batsmanAEnd = jsonData.indexOf("}", batsmanAStart);
    String batsmanA = jsonData.substring(batsmanAStart, batsmanAEnd);
    
    // Extract batsmanB
    int batsmanBStart = jsonData.indexOf("\"batsmanB\":") + 11;
    int batsmanBEnd = jsonData.indexOf(",", batsmanBStart);
    if (batsmanBEnd == -1) batsmanBEnd = jsonData.indexOf("}", batsmanBStart);
    String batsmanB = jsonData.substring(batsmanBStart, batsmanBEnd);
    
    // Extract total
    int totalStart = jsonData.indexOf("\"total\":") + 8;
    int totalEnd = jsonData.indexOf(",", totalStart);
    if (totalEnd == -1) totalEnd = jsonData.indexOf("}", totalStart);
    String total = jsonData.substring(totalStart, totalEnd);
    
    // Extract overs
    int oversStart = jsonData.indexOf("\"overs\":") + 8;
    int oversEnd = jsonData.indexOf(",", oversStart);
    if (oversEnd == -1) oversEnd = jsonData.indexOf("}", oversStart);
    String overs = jsonData.substring(oversStart, oversEnd);
    
    // Extract wickets
    int wicketsStart = jsonData.indexOf("\"wickets\":") + 10;
    int wicketsEnd = jsonData.indexOf(",", wicketsStart);
    if (wicketsEnd == -1) wicketsEnd = jsonData.indexOf("}", wicketsStart);
    String wickets = jsonData.substring(wicketsStart, wicketsEnd);
    
    // Extract target (optional)
    String target = "";
    if (jsonData.indexOf("\"target\":") > 0) {
      int targetStart = jsonData.indexOf("\"target\":") + 9;
      int targetEnd = jsonData.indexOf(",", targetStart);
      if (targetEnd == -1) targetEnd = jsonData.indexOf("}", targetStart);
      target = jsonData.substring(targetStart, targetEnd);
    }
    
    Serial.print("Batsman A: "); Serial.println(batsmanA);
    Serial.print("Batsman B: "); Serial.println(batsmanB);
    Serial.print("Total: "); Serial.println(total);
    Serial.print("Overs: "); Serial.println(overs);
    Serial.print("Wickets: "); Serial.println(wickets);
    if (target.length() > 0) {
      Serial.print("Target: "); Serial.println(target);
    }
    
    // Update the LED display with the new values
    updateDisplay(batsmanA.toInt(), batsmanB.toInt(), total.toInt(), overs.toInt(), wickets.toInt(), target.length() > 0 ? target.toInt() : -1);
    
  } else if (jsonData.indexOf("\"type\":\"command\"") > 0) {
    Serial.println("Command received");
    
    // Extract command
    int commandStart = jsonData.indexOf("\"command\":\"") + 11;
    int commandEnd = jsonData.indexOf("\"", commandStart);
    String command = jsonData.substring(commandStart, commandEnd);
    
    Serial.print("Command: "); Serial.println(command);
    
    // Handle specific commands
    if (command == "RESET") {
      Serial.println("Resetting scoreboard");
      resetScoreboard();
    } else if (command == "TEST") {
      Serial.println("Running test mode");
      testMode();
    }
  }
}

// LED Display Functions
void displayNumber(Adafruit_NeoPixel &strip, int digitToDisplay, uint32_t colourToUse) {
  int hundreds = digitToDisplay / 100;
  int tens = (digitToDisplay % 100) / 10;
  int ones = digitToDisplay % 10;

  strip.clear();

  displayDigit(strip, ones, 0, colourToUse);
  if (hundreds > 0 || tens > 0) {
    displayDigit(strip, tens, PIXELSPERDIGIT, colourToUse);
  }
  if (hundreds > 0) {
    displayDigit(strip, hundreds, PIXELSPERDIGIT * 2, colourToUse);
  }

  strip.show();
}

void displayDigit(Adafruit_NeoPixel &strip, int digit, int offset, uint32_t colour) {
  switch (digit) {
    case 0: strip.fill(colour, offset, 54); break;
    case 1: strip.fill(colour, offset, 9); strip.fill(colour, offset + 45, 9); break;
    case 2: strip.fill(colour, offset + 9, 18); strip.fill(colour, offset + 36, 27); break;
    case 3: strip.fill(colour, offset, 18); strip.fill(colour, offset + 36, 27); break;
    case 4: strip.fill(colour, offset, 9); strip.fill(colour, offset + 27, 9); strip.fill(colour, offset + 45, 18); break;
    case 5: strip.fill(colour, offset, 18); strip.fill(colour, offset + 27, 18); strip.fill(colour, offset + 54, 9); break;
    case 6: strip.fill(colour, offset, 45); strip.fill(colour, offset + 54, 9); break;
    case 7: strip.fill(colour, offset, 9); strip.fill(colour, offset + 36, 18); break;
    case 8: strip.fill(colour, offset, 63); break;
    case 9: strip.fill(colour, offset, 18); strip.fill(colour, offset + 27, 36); break;
  }
}

void updateDisplay(int batsmanA, int batsmanB, int total, int overs, int wickets, int target) {
  Serial.print("Updating display - Total: "); Serial.print(total);
  Serial.print(", Wickets: "); Serial.print(wickets);
  Serial.print(", Overs: "); Serial.print(overs);
  Serial.print(", BatA: "); Serial.print(batsmanA);
  Serial.print(", BatB: "); Serial.println(batsmanB);
  
  // Update LED displays with different colors
  displayNumber(totalStrip, total, totalStrip.Color(255, 0, 0));      // Red
  displayNumber(wicketsStrip, wickets, wicketsStrip.Color(0, 255, 0)); // Green
  displayNumber(oversStrip, overs, oversStrip.Color(0, 0, 255));       // Blue
  displayNumber(batAStrip, batsmanA, batAStrip.Color(255, 255, 255));  // White
  displayNumber(batBStrip, batsmanB, batBStrip.Color(255, 100, 0));    // Orange
  
  // Update target if provided
  if (target >= 0) {
    displayNumber(targetStrip, target, targetStrip.Color(255, 255, 0)); // Yellow
  }
}

void resetScoreboard() {
  Serial.println("Resetting scoreboard display");
  updateDisplay(0, 0, 0, 0, 0, 0);
}

void testMode() {
  Serial.println("Test Mode Triggered");
  updateDisplay(88, 77, 111, 12, 2, 345);
}

