#include <esp_now.h>
#include <WiFi.h>
#include "esp_camera.h"

// Define MAC addresses
uint8_t receiverMAC[] = {0x98, 0xCD, 0xAC, 0x63, 0x75, 0x1C};

// ESP32-CAM camera settings
#define PWDN_GPIO_NUM    32
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    0
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27
#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      21
#define Y4_GPIO_NUM      19
#define Y3_GPIO_NUM      18
#define Y2_GPIO_NUM      5
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

volatile bool sendSuccess = false;

void setupCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    
    // Set the frame size to the smallest possible to minimize image size
    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 30; // Set JPEG quality to the lowest to reduce size
    config.fb_count = 1;

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return;
    }
    Serial.println("Camera initialized successfully");
}

void sendPacketWithRetry(const uint8_t *data, size_t len, size_t packetIndex, size_t totalPackets) {
    sendSuccess = false;
    while (!sendSuccess) {
        esp_err_t result = esp_now_send(receiverMAC, data, len);
        if (result == ESP_OK) {
            Serial.printf("Packet %d of %d sent, waiting for confirmation...\n", packetIndex + 1, totalPackets);
            delay(500); // Wait before retrying in case the confirmation is delayed
        } else {
            Serial.printf("Error sending packet %d of %d, retrying...\n", packetIndex + 1, totalPackets);
            delay(500); // Wait before retrying
        }
    }
}

void sendImage() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }

    Serial.println("Camera capture succeeded");
    Serial.printf("Image size: %d bytes\n", fb->len);
    Serial.printf("Image resolution: %dx%d\n", fb->width, fb->height);

    size_t imageSize = fb->len;
    size_t packetSize = 128; // Reduced packet size to improve reliability
    size_t totalPackets = (imageSize + packetSize - 1) / packetSize;
    Serial.printf("Total packets to send: %d\n", totalPackets);
    Serial.printf("Packet size: %d bytes\n", packetSize);

    // Send metadata before image data
    uint8_t metadata[12];
    memcpy(metadata, &imageSize, sizeof(imageSize));
    memcpy(metadata + sizeof(imageSize), &totalPackets, sizeof(totalPackets));
    memcpy(metadata + sizeof(imageSize) + sizeof(totalPackets), &packetSize, sizeof(packetSize));
    sendPacketWithRetry(metadata, sizeof(metadata), 0, 1);
    Serial.println("Metadata sent");

    for (size_t i = 0; i < totalPackets; i++) {
        size_t offset = i * packetSize;
        size_t remaining = imageSize - offset;
        size_t chunkSize = (remaining < packetSize) ? remaining : packetSize;

        // Add packet index to the beginning of each packet
        uint8_t packet[chunkSize + 2];
        packet[0] = (i >> 8) & 0xFF; // High byte of packet index
        packet[1] = i & 0xFF;        // Low byte of packet index
        memcpy(packet + 2, fb->buf + offset, chunkSize);

        Serial.printf("Sending packet %d of %d, size: %d bytes\n", i + 1, totalPackets, chunkSize);
        sendPacketWithRetry(packet, sizeof(packet), i, totalPackets);
    }

    esp_camera_fb_return(fb);
    Serial.println("Image send complete");
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        Serial.println("Delivery Success");
        sendSuccess = true;
    } else {
        Serial.println("Delivery Fail");
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_send_cb(OnDataSent);
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, receiverMAC, 6);
    peerInfo.channel = 0; // Default channel
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    setupCamera();
}

void loop() {
    long rssi = WiFi.RSSI();
    Serial.print("Current RSSI: ");
    Serial.println(rssi);
    sendImage();
    delay(15000); // Send an image every 15 seconds to allow for better stability
}
