# Smart Wand: Multi-Zone BLE Microclimate Sensor

### Overview
This project is a Proof of Concept (PoC) for a "Smart Wand" designed to eliminate data shadows in commercial greenhouses. It reads temperature and humidity from three vertical crop zones (Root, Fruit, Canopy) and broadcasts the data in real-time to a smartphone via Bluetooth Low Energy (BLE).

### Hardware Used
* Arduino UNO R4 WiFi (Acts as the microcontroller and BLE radio)
* 3x DFRobot DHT22 Temperature & Humidity Sensors
* 1x Breadboard & Jumper Wires

### Pinout Guide
| Sensor Zone | Data Pin (Arduino) | Power |
| :--- | :--- | :--- |
| Root | Digital Pin 2 | 5V / GND |
| Fruit | Digital Pin 3 | 5V / GND |
| Canopy | Digital Pin 4 | 5V / GND |

### How to Use
1. Flash the `Smart_Wand.ino` code to your Arduino UNO R4 WiFi.
2. Download the **nRF Connect for Mobile** app (iOS/Android).
3. Connect to the device named **Smart Wand**.
4. Open the primary service (`19B10000...`) and enable **Notifications** for the three characteristics. 
5. Set the data parser to **UTF-8** to view the live, human-readable data stream (e.g., `26.4C | 23.6%`).
