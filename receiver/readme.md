# 📥 ESP32 Image Receiver with SPIFFS + HTTP Server

This module runs on a **standard ESP32 DevKit** and acts as the receiver for images transmitted from an ESP32-CAM over ESP-NOW. It stores the received JPEG image on SPIFFS and makes it available via a web server.

---

## 🧰 Features

- Listens for metadata and image packets over ESP-NOW.
- Handles packet numbering, deduplication, and image reconstruction.
- Saves the image as `/received_image.jpg` on the SPIFFS filesystem.
- Hosts an HTTP server at `/image` to serve the image.

---

## 📦 Structure

- `fogado.ino`: Main receiver program.
- Uses `esp_now.h` for wireless reception.
- Uses `SPIFFS.h` and `WebServer.h` for storage and HTTP.

---

## ⚙️ Configuration

### Sender MAC Address

Edit the following line in `fogado.ino` to match your ESP32-CAM’s MAC address:

```cpp
uint8_t senderMAC[] = {0x30, 0xC6, 0xF7, 0x00, 0xC8, 0x8C};
```
WiFi Credentials
Edit this line with your local WiFi SSID and password:

```cpp
WiFi.begin("Zmmm", "14921526");
```
📌 This is only required to access the HTTP image endpoint. ESP-NOW itself does not need WiFi connection.

🧪 Test Procedure
Flash this code to a standard ESP32 board.

Monitor Serial at 115200 baud.

Upon successful connection, it will:

Print ESP-NOW pairing info

Log received packets and metadata

Save the image on SPIFFS

Access the received image via browser at:

```arduino
http://<ESP32-IP>/image
```

🔍 Troubleshooting
| Issue                 | Solution                                             |
| --------------------- | ---------------------------------------------------- |
| SPIFFS error          | Use `SPIFFS.begin(true)` to auto-format              |
| Web endpoint 404      | Ensure image was received before accessing           |
| Duplicate packet spam | Ensure sender delays are sufficient and not too fast |



📎 Dependencies
- esp_now.h
- WiFi.h
- SPIFFS.h
- WebServer.h
- <set> (C++ STL)

📄 License
MIT
Author: Bence Sári (2025)
