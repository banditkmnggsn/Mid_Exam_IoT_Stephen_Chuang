// esp_now_receiver.ino
#include <WiFi.h>
#include <esp_now.h>

typedef struct TestPacket {
  uint32_t packetID;
  uint32_t timestamp;
  char message[20];
} TestPacket;

TestPacket receivedData;

uint32_t totalPacketsReceived = 0;
uint32_t lastPacketID = 0;
uint32_t totalPacketLoss = 0;

uint32_t totalLatency = 0;
uint32_t minLatency = 999999;
uint32_t maxLatency = 0;

int currentRSSI = 0;

void onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
  memcpy(&receivedData, data, sizeof(receivedData));
  uint32_t receiveTime = millis();
  uint32_t latency = receiveTime - receivedData.timestamp;
  currentRSSI = -50 - (latency * 2);
  totalPacketsReceived++;
  totalLatency += latency;
  if (latency < minLatency) minLatency = latency;
  if (latency > maxLatency) maxLatency = latency;
  if (receivedData.packetID > lastPacketID + 1) {
    uint32_t lostPackets = receivedData.packetID - lastPacketID - 1;
    totalPacketLoss += lostPackets;
    Serial.println();
    Serial.println("⚠⚠⚠ PACKET LOSS DETECTED! ⚠⚠⚠");
    Serial.print("Missing Packets: ");
    Serial.print(lastPacketID + 1);
    Serial.print(" to ");
    Serial.println(receivedData.packetID - 1);
    Serial.print("Lost Count: ");
    Serial.println(lostPackets);
  }
  lastPacketID = receivedData.packetID;
  uint32_t totalExpected = receivedData.packetID;
  float lossRate = (totalExpected == 0) ? 0.0 : (float)totalPacketLoss / totalExpected * 100.0;
  float successRate = 100.0 - lossRate;
  float avgLatency = (totalPacketsReceived == 0) ? 0.0 : (float)totalLatency / totalPacketsReceived;
  Serial.println();
  Serial.println("╔═══════════════════════════════════════════════╗");
  Serial.print("║ PACKET #");
  Serial.print(receivedData.packetID);
  Serial.println(" RECEIVED                           ║");
  Serial.println("╠═══════════════════════════════════════════════╣");
  Serial.print("║ Message     : ");
  Serial.println(receivedData.message);
  Serial.print("║ Latency     : ");
  Serial.print(latency);
  Serial.println(" ms");
  Serial.print("║ RSSI (est.) : ");
  Serial.print(currentRSSI);
  Serial.println(" dBm");
  Serial.println("╠═══════════════════════════════════════════════╣");
  Serial.println("║ CUMULATIVE STATISTICS                         ║");
  Serial.println("╠═══════════════════════════════════════════════╣");
  Serial.print("║ Total RX        : ");
  Serial.println(totalPacketsReceived);
  Serial.print("║ Expected        : ");
  Serial.println(totalExpected);
  Serial.print("║ Packets Lost    : ");
  Serial.println(totalPacketLoss);
  Serial.print("║ Loss Rate       : ");
  Serial.print(lossRate, 2);
  Serial.println(" %");
  Serial.print("║ Success Rate    : ");
  Serial.print(successRate, 2);
  Serial.println(" %");
  Serial.print("║ Average Latency : ");
  Serial.print(avgLatency, 2);
  Serial.println(" ms");
  Serial.print("║ Min Latency     : ");
  Serial.print(minLatency);
  Serial.println(" ms");
  Serial.print("║ Max Latency     : ");
  Serial.print(maxLatency);
  Serial.println(" ms");
  Serial.println("╚═══════════════════════════════════════════════╝");
  if (lossRate > 10.0) {
    Serial.println();
    Serial.println("⚠ WARNING: High packet loss (>10%)!");
  }
  if (avgLatency > 50) {
    Serial.println("⚠ WARNING: High latency (>50ms)!");
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  Serial.println("╔════════════════════════════════════════════════╗");
  Serial.println("║   ESP-NOW RECEIVER (Listener)                  ║");
  Serial.println("║   Stephen Chuang - 2702269135                  ║");
  Serial.println("╚════════════════════════════════════════════════╝");
  Serial.println();
  WiFi.mode(WIFI_STA);
  Serial.println("╔════════════════════════════════════════════════╗");
  Serial.println("║ ⚠ COPY THIS MAC ADDRESS TO SENDER CODE!       ║");
  Serial.println("╠════════════════════════════════════════════════╣");
  Serial.print("║   ");
  Serial.print(WiFi.macAddress());
  Serial.println("                          ║");
  Serial.println("╠════════════════════════════════════════════════╣");
  Serial.println("║ Example format for Sender:                     ║");
  Serial.println("║ {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}           ║");
  Serial.println("╚════════════════════════════════════════════════╝");
  Serial.println();
  Serial.print("Initializing ESP-NOW... ");
  if (esp_now_init() != ESP_OK) {
    Serial.println("FAILED!");
    while(1);
  }
  Serial.println("SUCCESS");
  esp_now_register_recv_cb(onDataRecv);
  Serial.println();
  Serial.println("╔════════════════════════════════════════════════╗");
  Serial.println("║  READY TO RECEIVE PACKETS                      ║");
  Serial.println("╚════════════════════════════════════════════════╝");
  Serial.println(">>> LISTENING <<<");
}

void loop() {
  delay(100);
}
