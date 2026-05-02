# Smart Wand: Multi-Zone BLE Microclimate Sensor

### Overview
This project is a Hardware Proof of Concept (PoC) for a "Smart Wand" designed to eliminate data shadows in commercial greenhouses. It reads temperature and humidity from three vertical crop zones (Root, Fruit, and Canopy), calculates the Vapour Pressure Deficit (VPD), and broadcasts the data in real-time to a smartphone via Bluetooth Low Energy (BLE).

### Hardware Components
* **Microcontroller:** Arduino UNO R4 WiFi (Utilizing onboard ESP32 for BLE)
* **Sensors:** 3x DFRobot DHT22 (SEN0137) Temperature & Humidity Sensors
* **Power:** 5V Power Bank (for untethered deployment) or USB-C
* **Misc:** Breadboard & Jumper Wires

### Physical Pinout Guide
| Sensor Zone | Data Pin (Arduino) | VCC / Power | GND |
| :--- | :--- | :--- | :--- |
| **Root** | Digital Pin 2 | 5V | GND |
| **Fruit** | Digital Pin 3 | 5V | GND |
| **Canopy** | Digital Pin 4 | 5V | GND |

---

### Repository Structure & Firmware Versions
This repository contains three different firmware versions, designed for different stages of testing and deployment. 

* 📁 **`01_Basic_Sensor_Test/`**
  * **Purpose:** Hardware verification. 
  * **Description:** A simple, non-wireless script to verify that all three DHT22 sensors are wired correctly and outputting accurate Temperature and Humidity readings to the laptop's Serial Monitor.

* 📁 **`02_Hybrid_Always_On/`**
  * **Purpose:** Debugging and Data Analysis.
  * **Description:** The Arduino runs continuously. It calculates VPD using the Magnus formula and prints environmental health warnings to the Serial Monitor 24/7. If a smartphone connects via BLE, it simultaneously beams a copy of the data to the phone. 

* 📁 **`03_Strict_BLE_Battery_Saver/`**
  * **Purpose:** Untethered Greenhouse Deployment.
  * **Description:** Designed to run off a portable power bank. To conserve battery, the sensors are put to sleep. The Arduino only wakes up, takes readings, and processes VPD data *when* a smartphone actively connects to it via BLE. 

---

### How to Test the BLE Connection
To view the live data stream from the Hybrid or Battery Saver firmware on your smartphone:

1. Flash the desired `.ino` code to your Arduino UNO R4 WiFi.
2. Download the free **nRF Connect for Mobile** app (available on iOS and Android).
3. Open the app and scan for a device named **Smart Wand**. Connect to it.
4. Expand the Primary Custom Service (`19B10000-E8F2-537E-4F6C-D104768A1214`).
5. You will see three characteristics representing the three crop zones:
   * `...10001` = Canopy Zone
   * `...10002` = Fruit Zone
   * `...10003` = Root Zone
6. Enable **Notifications** (the down arrow icon) for all three characteristics.
7. **Important:** Change the data parser format to **UTF-8** in the app to view the human-readable text strings (e.g., `26.4C | 80% | 0.8kPa`).

### Future Scope
This PoC currently utilizes a generic BLE engineering scanner to verify the hardware-to-mobile data link. The final commercial product will feature a custom-built companion application (e.g., built in Flutter/React Native) capable of reading this BLE stream in the background and issuing push-notification alerts to greenhouse workers if the Tip-Burn risk (VPD) reaches critical thresholds.
