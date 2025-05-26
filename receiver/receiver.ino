#include <esp_now.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <FS.h>
#include <WebServer.h>
#include <set>

// Receiver MAC address
uint8_t senderMAC[] = {0x30, 0xC6, 0xF7, 0x00, 0xC8, 0x8C};

// Buffer to store incoming data
#define PACKET_SIZE 128
uint8_t incomingData[PACKET_SIZE];

// File to store received image
File imageFile;

// Variables to handle image reception
std::vector<uint8_t> imageBuffer;
bool isReceivingImage = false;
bool imageReceived = false;
size_t totalPackets = 0;
size_t receivedPackets = 0;
size_t packetSize = 0;
size_t imageSize = 0;
std::set<uint16_t> receivedPacketNumbers; // Track received packet numbers to avoid duplication

// Create a web server on port 80
WebServer server(80);

void onDataRecv(const esp_now_recv_info_t *recvInfo, const uint8_t *incomingData, int len) {
  if (imageReceived) {
    // If an image is already received, ignore further data
    return;
  }

  // Check if the MAC address matches the sender
  if (memcmp(recvInfo->src_addr, senderMAC, 6) == 0) {
    if (len > 0) {
      // If metadata is being received
      if (!isReceivingImage && len == 12) {
        // Extract metadata: imageSize, totalPackets, packetSize
        memcpy(&imageSize, incomingData, sizeof(imageSize));
        memcpy(&totalPackets, incomingData + sizeof(imageSize), sizeof(totalPackets));
        memcpy(&packetSize, incomingData + sizeof(imageSize) + sizeof(totalPackets), sizeof(packetSize));
        
        Serial.printf("Metadata received - Image Size: %d bytes, Total Packets: %d, Packet Size: %d bytes\n", imageSize, totalPackets, packetSize);
        
        // Initialize variables for image reception
        isReceivingImage = true;
        receivedPackets = 0;
        imageBuffer.clear();
        receivedPacketNumbers.clear();
        return;
      }

      // If the image packets are being received
      if (isReceivingImage && len <= packetSize + 2) { // Adjust for packet number bytes
        // Extract packet number
        uint16_t packetNumber = (incomingData[0] << 8) | incomingData[1];

        // Log packet information
        Serial.printf("Received packet %d of %d, length: %d bytes\n", packetNumber + 1, totalPackets, len);

        // If the packet number has already been received, skip it
        if (receivedPacketNumbers.find(packetNumber) != receivedPacketNumbers.end()) {
          Serial.printf("Duplicate packet %d ignored\n", packetNumber + 1);
          return;
        }

        // Store packet number to avoid future duplication
        receivedPacketNumbers.insert(packetNumber);
        
        // Store image data, ensuring no overflow
        if (len > 2) { // First two bytes are for packet number
          imageBuffer.insert(imageBuffer.end(), incomingData + 2, incomingData + len);
          receivedPackets++;
        }

        // Print progress
        Serial.printf("Successfully received packet %d of %d\n", packetNumber + 1, totalPackets);
        
        // Check if all packets have been received
        if (receivedPackets == totalPackets) {
          Serial.println("All packets received. Image received successfully!");
          saveImage();
          isReceivingImage = false;
          imageReceived = true; // Mark that an image has been successfully received
        }
      } else {
        Serial.printf("Invalid packet length: %d bytes\n", len);
      }
    }
  }
}

void saveImage() {
  // Save image to the file system
  imageFile = SPIFFS.open("/received_image.jpg", FILE_WRITE);
  if (imageFile) {
    imageFile.write(imageBuffer.data(), imageBuffer.size());
    imageFile.close();
    Serial.println("Image saved successfully.");
  } else {
    Serial.println("Failed to open file for writing.");
  }
}

void handleImageDownload() {
  File file = SPIFFS.open("/received_image.jpg", FILE_READ);
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }

  server.streamFile(file, "image/jpeg");
  file.close();
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize WiFi in station mode
  WiFi.mode(WIFI_STA);
  WiFi.begin("Zmmm", "14921526"); // Replace with your network credentials
  WiFi.setSleep(false); // Disable WiFi sleep for more reliable connection

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startAttemptTime >= 20000) { // Timeout after 20 seconds
      Serial.println("Failed to connect to WiFi. Restarting...");
      ESP.restart();
    }
    delay(500);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  // Initialize SPIFFS for storing images
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }

  // Set up ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the receive callback function
  esp_now_register_recv_cb(onDataRecv);

  // Set up the web server
  server.on("/image", HTTP_GET, handleImageDownload);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle web server requests
  server.handleClient();
}
