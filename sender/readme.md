# 📤 ESP32-CAM Image Sender (ESP-NOW)

This module runs on an **ESP32-CAM** and is responsible for capturing a JPEG image and transmitting it wirelessly to a paired ESP32 receiver using the ESP-NOW protocol.

---

## 📸 Features

- Initializes the onboard OV2640 camera.
- Captures low-resolution JPEG frames (QQVGA, quality 30).
- Splits the image into 128-byte packets.
- Adds metadata and packet indices.
- Sends each packet reliably using a retry mechanism until acknowledged.

---

## 📦 Structure

- `kuldo.ino`: Main program.
- Uses `esp_camera.h` for camera control.
- Uses `esp_now.h` for wireless communication.

---

## ⚙️ Configuration

### MAC Address of the Receiver

In `kuldo.ino`, change the following line to the MAC address of your receiver ESP32:

```cpp
uint8_t receiverMAC[] = {0x98, 0xCD, 0xAC, 0x63, 0x75, 0x1C};
```
You can obtain the MAC address of the receiver by printing WiFi.macAddress().c_str() on the receiver side.

🧪 Test Procedure
Flash this code to an ESP32-CAM using Arduino IDE or PlatformIO.

Open Serial Monitor at 115200 baud.

The device will:

Initialize the camera.

Wait 15 seconds between each image capture.

Send the image in chunks to the paired ESP32.

Serial output confirms packet-by-packet transmission and retry events.

🔍 Troubleshooting
Issue	Cause & Fix
Camera init failed	Check board selection and camera model (AI Thinker)
No packets received	Ensure receiver is powered and paired MAC is correct
Delays too short	Use longer intervals (15+ sec) to avoid overload

📎 Dependencies
esp_camera.h

esp_now.h

WiFi.h

Board must be set to: AI Thinker ESP32-CAM

📄 License
MIT
Author: Bence Sári (2025)
