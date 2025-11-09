// esp_now_sender.ino
#include <WiFi.h>
#include <esp_now.h>

uint8_t receiverMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#define TEST_INTERVAL 1000
#define PACKET_SIZE 32

typedef struct TestPacket {
  uint32_t packetID;
  uint32_t timestamp;
  char message[20];
} TestPacket;

TestPacket testData;
uint32_t packetCounter = 0;
uint32_t successCount = 0;
uint32_t failCount = 0;

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println();
  Serial.println("========================================");
  if (status == ESP_NOW_SEND_SUCCESS) {
    successCount++;
    Serial.print("✓ SUCCESS | Packet #");
    Serial.print(testData.packetID);
    Serial.println(" delivered");
  } else {
    failCount++;
    Serial.print("✗ FAILED  | Packet #");
    Serial.print(testData.packetID);
    Serial.println(" not delivered");
  }
  uint32_t totalPackets = successCount + failCount;
  float successRate = (totalPackets == 0) ? 0.0 : (float)successCount / totalPackets * 100.0;
  Serial.println("----------------------------------------");
  Serial.print("Total Sent    : ");
  Serial.println(totalPackets);
  Serial.print("Success       : ");
  Serial.println(successCount);
  Serial.print("Failed        : ");
  Serial.println(failCount);
  Serial.print("Success Rate  : ");
  Serial.print(successRate, 2);
  Serial.println("%");
  Serial.println("========================================");
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║   ESP-NOW SENDER (Transmitter)         ║");
  Serial.println("║   Stephen Chuang - 2702269135          ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println();
  WiFi.mode(WIFI_STA);
  Serial.println("Device Information:");
  Serial.println("-------------------");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Channel    : ");
  Serial.println(WiFi.channel());
  Serial.println();
  Serial.print("Initializing ESP-NOW... ");
  if (esp_now_init() != ESP_OK) {
    Serial.println("FAILED!");
    Serial.println("ERROR: Cannot initialize ESP-NOW");
    while(1);
  }
  Serial.println("SUCCESS");
  esp_now_register_send_cb(onDataSent);
  Serial.print("Adding receiver peer... ");
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("FAILED!");
    Serial.println("ERROR: Cannot add peer");
    while(1);
  }
  Serial.println("SUCCESS");
  Serial.println();
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║  READY TO TRANSMIT                     ║");
  Serial.println("║  Starting in 3 seconds...              ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println();
  delay(3000);
  Serial.println(">>> TRANSMISSION STARTED <<<\n");
}

void loop() {
  packetCounter++;
  testData.packetID = packetCounter;
  testData.timestamp = millis();
  snprintf(testData.message, sizeof(testData.message), "Test-%lu", packetCounter);
  Serial.print("→ Sending Packet #");
  Serial.print(testData.packetID);
  Serial.print(" | Time: ");
  Serial.print(testData.timestamp);
  Serial.print(" ms | Message: ");
  Serial.println(testData.message);
  esp_err_t result = esp_now_send(receiverMAC, (uint8_t *)&testData, sizeof(testData));
  if (result != ESP_OK) {
    Serial.println("✗ ERROR: esp_now_send() failed!");
    failCount++;
  }
  delay(TEST_INTERVAL);
}
