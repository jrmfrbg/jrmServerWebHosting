#include <BLEDevice.h>
#include <BLEServer.h>
#include <FastLED.h>

#define DATA_PIN    27
#define NUM_LEDS    48
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

char currentMode = 'R'; // Default to Rainbow (less current than white)
uint8_t staticHue = 0;
uint8_t rainbowHue = 0;

// BLE UUIDs for the Nordic UART Service (standard for Web Bluetooth)
#define SERVICE_UUID           "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e"

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String value = pCharacteristic->getValue(); 
      
      if (value.length() > 0) {
        Serial.print("Received Value: ");
        Serial.println(value);

        if (value[0] == 'H') {
            currentMode = 'S'; 
            staticHue = value.substring(1).toInt(); 
            Serial.print("Mode: Static Hue, Value: ");
            Serial.println(staticHue);
        } else {
            currentMode = value[0]; 
            Serial.print("Mode: ");
            Serial.println(currentMode);
        }
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  
  // Limit power draw to 5V 500mA (Safe for USB power)
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500); 

  BLEDevice::init("NucleusBear");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  pServer->getAdvertising()->start();
}

void loop() {
  switch (currentMode) {
    case 'R': // Rainbow Animation
      fill_rainbow(leds, NUM_LEDS, rainbowHue, 7);
      rainbowHue++; // Increment for movement
      break;
      
    case 'G': fill_solid(leds, NUM_LEDS, CRGB::Green); break;
    case 'D': fill_solid(leds, NUM_LEDS, CRGB::Red); break;
    case 'W': fill_solid(leds, NUM_LEDS, CRGB::White); break;
    case 'S': // Slider/Static Mode
      // Converts the Hue value from the slider into a full color
      fill_solid(leds, NUM_LEDS, CHSV(staticHue, 255, 255)); 
      break;
  }

  /*
  Debug statement:
  Serial.print("LoopMode: ");
  Serial.println(currentMode);
  */

  FastLED.show();
  delay(20); // Smooth 50fps refresh rate
}