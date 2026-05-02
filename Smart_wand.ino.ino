// 1. Include Sensors and Bluetooth Libraries
#include "DHT.h"
#include <ArduinoBLE.h>

// 2. Hardware Setup
#define ROOT_PIN 2
#define FRUIT_PIN 3
#define CANOPY_PIN 4
#define DHTTYPE DHT22

DHT rootSensor(ROOT_PIN, DHTTYPE);
DHT fruitSensor(FRUIT_PIN, DHTTYPE);
DHT canopySensor(CANOPY_PIN, DHTTYPE);

// 3. Create the Bluetooth Service and Data Channels (Characteristics)
// We use custom UUIDs so the phone reads them as standard text
BLEService wandService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEStringCharacteristic canopyChar("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, 20);
BLEStringCharacteristic fruitChar("19B10002-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, 20);
BLEStringCharacteristic rootChar("19B10003-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, 20);

unsigned long previousMillis = 0;

void setup() {
  Serial.begin(9600);
  
  // Turn on sensors
  rootSensor.begin();
  fruitSensor.begin();
  canopySensor.begin();

  // Boot up the Bluetooth chip
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed! Check board selection.");
    while (1);
  }

  // Name the device exactly what we called it in the report!
  BLE.setLocalName("Smart Wand");
  BLE.setAdvertisedService(wandService);

  // Add the three zones to the broadcast
  wandService.addCharacteristic(canopyChar);
  wandService.addCharacteristic(fruitChar);
  wandService.addCharacteristic(rootChar);
  BLE.addService(wandService);

  // Set initial waiting text
  canopyChar.writeValue("Waiting...");
  fruitChar.writeValue("Waiting...");
  rootChar.writeValue("Waiting...");

  // Start broadcasting to the room
  BLE.advertise();
  Serial.println("Bluetooth Active! Open the nRF Connect app on your phone.");
}

void loop() {
  // Check if a phone has connected to the Wand
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to phone MAC: ");
    Serial.println(central.address());

    // While the phone is connected, keep sending data
    while (central.connected()) {
      unsigned long currentMillis = millis();
      
      // Wait 2.5 seconds between readings (DHT22s need time to process)
      if (currentMillis - previousMillis >= 2500) {
        previousMillis = currentMillis;

        // Read all sensors
        float hc = canopySensor.readHumidity();
        float tc = canopySensor.readTemperature();
        float hf = fruitSensor.readHumidity();
        float tf = fruitSensor.readTemperature();
        float hr = rootSensor.readHumidity();
        float tr = rootSensor.readTemperature();

        // Format the data into clean, short text strings (e.g., "25.5C | 80.2%")
        String canopyStr = String(tc, 1) + "C | " + String(hc, 1) + "%";
        String fruitStr = String(tf, 1) + "C | " + String(hf, 1) + "%";
        String rootStr = String(tr, 1) + "C | " + String(hr, 1) + "%";

        // Push the new text to the Bluetooth channels
        canopyChar.writeValue(canopyStr);
        fruitChar.writeValue(fruitStr);
        rootChar.writeValue(rootStr);

        Serial.println("Beamed to Phone: " + canopyStr + " / " + fruitStr + " / " + rootStr);
      }
    }
    Serial.println("Phone disconnected. Waiting for a new connection...");
  }
}