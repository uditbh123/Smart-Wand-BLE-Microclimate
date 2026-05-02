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

// 3. BLE Service and Characteristics
BLEService wandService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEStringCharacteristic canopyChar("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, 20);
BLEStringCharacteristic fruitChar("19B10002-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, 20);
BLEStringCharacteristic rootChar("19B10003-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, 20);

unsigned long previousMillis = 0;
bool wasConnected = false; // Memory variable to track if a phone is attached

// 4. VPD Calculation (Magnus formula)
float calculateVPD(float tempC, float humidity) {
  float svp = 0.6108 * exp((17.27 * tempC) / (tempC + 237.3));
  float avp = svp * (humidity / 100.0);
  return svp - avp;
}

void setup() {
  Serial.begin(9600);
  
  delay(3000); 
  Serial.println("\n--- Smart Wand Booting Up ---");

  rootSensor.begin();
  fruitSensor.begin();
  canopySensor.begin();

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed! Check board selection.");
    while (1);
  }

  BLE.setLocalName("Smart Wand");
  BLE.setAdvertisedService(wandService);

  wandService.addCharacteristic(canopyChar);
  wandService.addCharacteristic(fruitChar);
  wandService.addCharacteristic(rootChar);
  BLE.addService(wandService);

  canopyChar.writeValue("Waiting...");
  fruitChar.writeValue("Waiting...");
  rootChar.writeValue("Waiting...");

  BLE.advertise();
  Serial.println("Bluetooth Active! Open the nRF Connect app on your phone.");
}

void loop() {
  // 1. Check for a Bluetooth connection
  BLEDevice central = BLE.central();

  // Print a message when a phone connects or disconnects
  if (central && central.connected()) {
    if (!wasConnected) {
      Serial.print("\n[BLE] Connected to phone MAC: ");
      Serial.println(central.address());
      wasConnected = true;
    }
  } else {
    if (wasConnected) {
      Serial.println("\n[BLE] Phone disconnected. Running offline mode...");
      wasConnected = false;
    }
  }

  // 2. Read Sensors EVERY 2.5 seconds (Offline or Online)
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 2500) {
    previousMillis = currentMillis;

    float hc = canopySensor.readHumidity();
    float tc = canopySensor.readTemperature();
    float hf = fruitSensor.readHumidity();
    float tf = fruitSensor.readTemperature();
    float hr = rootSensor.readHumidity();
    float tr = rootSensor.readTemperature();

    bool hasError = false;
    
    if (isnan(tc) || isnan(hc)) { 
      if (wasConnected) canopyChar.writeValue("ERR:Canopy"); 
      Serial.println("  [!] ERROR: Canopy sensor loose/disconnected!");
      hasError = true;
    }
    if (isnan(tf) || isnan(hf)) { 
      if (wasConnected) fruitChar.writeValue("ERR:Fruit");  
      Serial.println("  [!] ERROR: Fruit sensor loose/disconnected!");
      hasError = true;
    }
    if (isnan(tr) || isnan(hr)) { 
      if (wasConnected) rootChar.writeValue("ERR:Root");    
      Serial.println("  [!] ERROR: Root sensor loose/disconnected!");
      hasError = true;
    }

    // 3. Do the math and print/broadcast the results
    if (!hasError) {
      float vpdC = calculateVPD(tc, hc);
      float vpdF = calculateVPD(tf, hf);
      float vpdR = calculateVPD(tr, hr);

      String canopyStr = String(tc, 1) + "C | " + String(hc, 0) + "% | " + String(vpdC, 1) + "kPa";
      String fruitStr  = String(tf, 1) + "C | " + String(hf, 0) + "% | " + String(vpdF, 1) + "kPa";
      String rootStr   = String(tr, 1) + "C | " + String(hr, 0) + "% | " + String(vpdR, 1) + "kPa";

      // ALWAYS print to the Serial Monitor (Even when disconnected)
      Serial.println("Live Data:");
      Serial.println("  Canopy: " + canopyStr);
      Serial.println("  Fruit : " + fruitStr);
      Serial.println("  Root  : " + rootStr);

      if (abs(tc - tr) > 5.0) {
        Serial.println("  ALERT: High zone gradient! Check air circulation.");
      }

      auto vpdHint = [](float vpd, String zone) {
        if      (vpd < 0.4) Serial.println("  " + zone + " Health: VPD LOW  (Risk: mold)");
        else if (vpd > 1.6) Serial.println("  " + zone + " Health: VPD HIGH (Risk: plant wilt)");
        else                Serial.println("  " + zone + " Health: VPD OK   (Healthy)");
      };

      vpdHint(vpdC, "Canopy");
      vpdHint(vpdF, "Fruit ");
      vpdHint(vpdR, "Root  ");

      // ONLY push data to BLE if a phone is actively listening
      if (wasConnected) {
        canopyChar.writeValue(canopyStr);
        fruitChar.writeValue(fruitStr);
        rootChar.writeValue(rootStr);
        Serial.println("  [Status: Beamed to Phone]");
      } else {
        Serial.println("  [Status: Offline Mode - Displaying locally]");
      }
    }
    Serial.println("-------------------------");
  }
}
