// CATEGORY 3: STRICT BLE (BATTERY SAVER MODE)
#include "DHT.h"
#include <ArduinoBLE.h>

#define ROOT_PIN 2
#define FRUIT_PIN 3
#define CANOPY_PIN 4
#define DHTTYPE DHT22

DHT rootSensor(ROOT_PIN, DHTTYPE);
DHT fruitSensor(FRUIT_PIN, DHTTYPE);
DHT canopySensor(CANOPY_PIN, DHTTYPE);

BLEService wandService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEStringCharacteristic canopyChar("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, 20);
BLEStringCharacteristic fruitChar("19B10002-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, 20);
BLEStringCharacteristic rootChar("19B10003-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, 20);

unsigned long previousMillis = 0;

float calculateVPD(float tempC, float humidity) {
  float svp = 0.6108 * exp((17.27 * tempC) / (tempC + 237.3));
  float avp = svp * (humidity / 100.0);
  return svp - avp;
}

void setup() {
  // We don't even need the Serial Monitor in this version!
  rootSensor.begin();
  fruitSensor.begin();
  canopySensor.begin();

  if (!BLE.begin()) { while (1); } // Halt if BLE fails

  BLE.setLocalName("Smart Wand");
  BLE.setAdvertisedService(wandService);

  wandService.addCharacteristic(canopyChar);
  wandService.addCharacteristic(fruitChar);
  wandService.addCharacteristic(rootChar);
  BLE.addService(wandService);

  canopyChar.writeValue("Waiting for sync...");
  fruitChar.writeValue("Waiting for sync...");
  rootChar.writeValue("Waiting for sync...");

  BLE.advertise();
}

void loop() {
  BLEDevice central = BLE.central();

  // ONLY do work if a phone is actively connected
  if (central && central.connected()) {
    
    unsigned long currentMillis = millis();

    // Read sensors every 2.5 seconds while connected
    if (currentMillis - previousMillis >= 2500) {
      previousMillis = currentMillis;

      float hc = canopySensor.readHumidity();
      float tc = canopySensor.readTemperature();
      float hf = fruitSensor.readHumidity();
      float tf = fruitSensor.readTemperature();
      float hr = rootSensor.readHumidity();
      float tr = rootSensor.readTemperature();

      if (isnan(tc) || isnan(hc)) { canopyChar.writeValue("ERR:Canopy"); }
      else {
        float vpdC = calculateVPD(tc, hc);
        canopyChar.writeValue(String(tc, 1) + "C | " + String(hc, 0) + "% | " + String(vpdC, 1) + "kPa");
      }

      if (isnan(tf) || isnan(hf)) { fruitChar.writeValue("ERR:Fruit"); }
      else {
        float vpdF = calculateVPD(tf, hf);
        fruitChar.writeValue(String(tf, 1) + "C | " + String(hf, 0) + "% | " + String(vpdF, 1) + "kPa");
      }

      if (isnan(tr) || isnan(hr)) { rootChar.writeValue("ERR:Root"); }
      else {
        float vpdR = calculateVPD(tr, hr);
        rootChar.writeValue(String(tr, 1) + "C | " + String(hr, 0) + "% | " + String(vpdR, 1) + "kPa");
      }
    }
  }
  // If no phone is connected, the loop just spins silently and the sensors sleep.
}
