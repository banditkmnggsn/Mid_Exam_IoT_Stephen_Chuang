// ble_server_sender.ino
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Arduino.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint32_t packetCounter = 0;

class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("✓ Device Connected!");
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("✗ Device Disconnected!");
    delay(500);
    BLEDevice::startAdvertising();
    Serial.println("→ Advertising restarted");
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("==================================");
  Serial.println("ESP32 BLE SERVER - Testing");
  Serial.println("Stephen Chuang - 2702269135");
  Serial.println("==================================");
  Serial.println();
  BLEDevice::init("ESP32-BLE-Server");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("✓ BLE Server started");
  Serial.println("✓ Advertising...");
  Serial.println();
  Serial.println("[Waiting for client connection...]");
}

void loop() {
  if (deviceConnected) {
    packetCounter++;
    char buffer[32];
    uint32_t timestamp = millis();
    snprintf(buffer, sizeof(buffer), "%lu:%lu", packetCounter, timestamp);
    pCharacteristic->setValue(buffer);
    pCharacteristic->notify();
    Serial.print("→ Sent Packet #");
    Serial.print(packetCounter);
    Serial.print(" at ");
    Serial.print(timestamp);
    Serial.println(" ms");
    delay(1000);
  } else {
    Serial.println("⏳ Waiting for connection...");
    delay(2000);
  }
}
