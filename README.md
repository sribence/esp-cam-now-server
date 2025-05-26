# esp-cam-now-server
##📡 ESP32-CAM Wireless Image Transfer using ESP-NOW

This project demonstrates how to wirelessly capture and transmit JPEG images from an ESP32-CAM to a receiver ESP32 module using ESP-NOW — a lightweight, connectionless WiFi communication protocol developed by Espressif. The receiver stores the image on SPIFFS and serves it via a local HTTP endpoint.

---

## 🔍 Overview

**Key features:**

- 🖼️ Real-time JPEG image capture with ESP32-CAM
- 📦 Reliable chunk-based image transfer via ESP-NOW
- 📂 Reconstruction and saving of the image on the receiver ESP32 using SPIFFS
- 🌐 HTTP-based access to the received image (`/image`)
- 🛡️ Robust resend mechanism and duplicate packet handling

This is a fully offline peer-to-peer communication system that does **not require an internet connection**. Perfect for remote sensor stations, wildlife monitoring, or surveillance systems where bandwidth and reliability are key.

---

## 🧠 System Architecture

```[ESP32-CAM]
📷 Camera Capture
|
JPEG Image
|
Chunk Sender
(ESP-NOW + Retry)
|
V
[ESP32 Receiver]
Chunk Reassembly
+ De-duplication
|
Image File
|
SPIFFS Storage
|
HTTP File Server
```

---

## 🛠️ Hardware Requirements

| Device         | Quantity | Notes |
|----------------|----------|-------|
| ESP32-CAM      | 1        | With OV2640 camera module |
| ESP32 DevKit   | 1        | Any ESP32 board with enough flash for SPIFFS |
| 5V Power Source| 2        | Stable USB or Li-ion power source recommended |
| Optional: Button or LED | 1+ | For triggering capture or status indicators |

---

## 💾 Software & Library Dependencies

Make sure the following libraries are installed in the Arduino IDE:

- `esp_now.h` (comes with ESP32 board package)
- `WiFi.h`
- `esp_camera.h` (only for ESP32-CAM)
- `SPIFFS.h`
- `WebServer.h`
- `FS.h`
- `set` from `<set>` (C++ STL container for packet tracking)

**Board definitions required:**

- ESP32 by Espressif Systems — version ≥ 2.0.5
- Camera board: **AI Thinker ESP32-CAM**

---

## 🔧 Installation & Usage

### 📸 On the Sender ESP32-CAM:

1. Open `kuldo.ino` in Arduino IDE.
2. Adjust the `receiverMAC` array to match the receiver ESP32's MAC address.
3. Upload the sketch to your ESP32-CAM.
4. Monitor Serial output at **115200 baud**.

### 🖥️ On the Receiver ESP32:

1. Open `fogado.ino` in Arduino IDE.
2. Edit the WiFi SSID & password in `WiFi.begin("Zmmm", "14921526");`.
3. Upload the sketch to your ESP32 DevKit.
4. After boot, check Serial monitor for:
    - `Connected to WiFi`
    - `HTTP server started`
5. Find the device IP in Serial output. Visit: `http://<ESP32-IP>/image`

---

## 📸 Image Flow & Data Strategy

- JPEG image is captured and split into **128-byte packets**.
- Each packet is prefixed with a **2-byte packet index** (big endian).
- First metadata packet (12 bytes) contains:
  - Total image size (4 bytes)
  - Total number of packets (4 bytes)
  - Packet size (4 bytes)
- The receiver reassembles the image, tracking packet indices to avoid duplicates.
- Once complete, the image is written to `/received_image.jpg` on SPIFFS.

---

## ⚙️ Advanced Features

- ✅ **Retry mechanism**: Sender retries each packet until an ACK is received.
- ✅ **Duplicate filtering**: Receiver ignores repeated packets via `std::set<uint16_t>`.
- ✅ **Timeout watchdog** (planned): To reset transmission if interrupted.
- ✅ **Dynamic resolution tuning** (planned): Adjust `FRAMESIZE_QQVGA` for higher quality if needed.

---

## 🧪 Known Limitations & Troubleshooting

| Issue | Explanation / Fix |
|-------|-------------------|
| ❌ Some packets not received | Ensure both devices have good RSSI and same channel |
| ⚠️ "Camera capture failed" | Restart the sender, check power supply |
| 🔁 Receiver does not reset after image | Add logic to timeout or force buffer clear |
| 🌐 No image in browser | Ensure correct IP address and receiver connected to WiFi |

---

## 🔗 References & Further Reading

- [ESP-NOW Protocol Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
- [ESP32-CAM Camera Initialization](https://randomnerdtutorials.com/esp32-cam-camera-module-pinout/)
- [SPIFFS Filesystem](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/spiffs.html)
- [WebServer Library Docs](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer)

---

## 📬 Contact

This project was developed as part of an **experimental wireless image transmission system** for embedded applications.

Feel free to submit pull requests, open issues, or reach out for collaboration.

---
**License:** MIT  
**Author:** Bence Sári  
**Year:** 2025  

