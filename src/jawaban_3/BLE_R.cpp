// ble_client_receiver.ino
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
static boolean doConnect = false;
static boolean connected = false;

uint32_t totalPacketsReceived = 0;
uint32_t lastPacketID = 0;
uint32_t totalPacketLoss = 0;
uint32_t totalLatency = 0;
uint32_t minLatency = 999999;
uint32_t maxLatency = 0;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify
) {

  String dataStr = String((char*)pData, length);
  int separatorIndex = dataStr.indexOf(':');
  if (separatorIndex == -1) return;
  uint32_t packetID = dataStr.substring(0, separatorIndex).toInt();
  uint32_t sendTime = dataStr.substring(separatorIndex + 1).toInt();
  uint32_t receiveTime = millis();
  uint32_t latency = receiveTime - sendTime;
  int rssi = myDevice ? myDevice->getRSSI() : 0;
  totalPacketsReceived++;
  totalLatency += latency;
  if (latency < minLatency) minLatency = latency;
  if (latency > maxLatency) maxLatency = latency;
  if (packetID > lastPacketID + 1) {
    uint32_t lostPackets = packetID - lastPacketID - 1;
    totalPacketLoss += lostPackets;
    Serial.print("⚠ PACKET LOSS! Lost ");
    Serial.print(lostPackets);
    Serial.println(" packet(s)");
  }
  lastPacketID = packetID;
  Serial.println();
  Serial.println("╔═══════════════════════════════════════╗");
  Serial.print("║ Packet #");
  Serial.print(packetID);
  Serial.println(" RECEIVED");
  Serial.println("╠═══════════════════════════════════════╣");
  Serial.print("║ Latency    : ");
  Serial.print(latency);
  Serial.println(" ms");
  Serial.print("║ RSSI       : ");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.println("╠═══════════════════════════════════════╣");
  Serial.print("║ Total RX   : ");
  Serial.println(totalPacketsReceived);
  Serial.print("║ Packet Loss: ");
  Serial.println(totalPacketLoss);
  float lossRate = (float)totalPacketLoss / (totalPacketsReceived + totalPacketLoss) * 100.0;
  Serial.print("║ Loss Rate  : ");
  Serial.print(lossRate, 2);
  Serial.println("%");
  float avgLatency = (totalPacketsReceived == 0) ? 0.0 : (float)totalLatency / totalPacketsReceived;
  Serial.print("║ Avg Latency: ");
  Serial.print(avgLatency, 2);
  Serial.println(" ms");
  Serial.print("║ Min Latency: ");
  Serial.print(minLatency);
  Serial.println(" ms");
  Serial.print("║ Max Latency: ");
  Serial.print(maxLatency);
  Serial.println(" ms");
  float successRate = 100.0 - lossRate;
  Serial.print("║ Success    : ");
  Serial.print(successRate, 2);
  Serial.println("%");
  Serial.println("╚═══════════════════════════════════════╝");
}

class ClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    connected = true;
    Serial.println("✓ Connected to server!");
  }
  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("✗ Disconnected from server!");
  }
};

bool connectToServer() {
  if (!myDevice) return false;
  Serial.print("→ Connecting to ");
  Serial.println(myDevice->getAddress().toString().c_str());
  BLEClient* pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new ClientCallback());
  if (!pClient->connect(myDevice)) {
    Serial.println("✗ Connection failed at client.connect()");
    return false;
  }
  Serial.println("✓ Connected to server");
  BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
  if (pRemoteService == nullptr) {
    Serial.println("✗ Failed to find service UUID");
    pClient->disconnect();
    return false;
  }
  Serial.println("✓ Found service");
  pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.println("✗ Failed to find characteristic UUID");
    pClient->disconnect();
    return false;
  }
  Serial.println("✓ Found characteristic");
  if(pRemoteCharacteristic->canNotify()) {
    pRemoteCharacteristic->registerForNotify(notifyCallback);
    Serial.println("✓ Notifications registered");
  }
  connected = true;
  return true;
}

class AdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      Serial.println("✓ Found target device!");
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("==================================");
  Serial.println("ESP32 BLE CLIENT - Testing");
  Serial.println("Stephen Chuang - 2702269135");
  Serial.println("==================================");
  Serial.println();
  BLEDevice::init("ESP32-BLE-Client");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  Serial.println("✓ Scanning for devices...");
  Serial.println();
}

void loop() {
  if (doConnect) {
    if (connectToServer()) {
      Serial.println();
      Serial.println("[Ready to receive data...]");
    } else {
      Serial.println("✗ Connection failed!");
    }
    doConnect = false;
  }
  if (!connected) {
    Serial.println("⏳ Not connected, rescanning...");
    BLEDevice::getScan()->start(0);
    delay(5000);
  }
  delay(1000);
}
