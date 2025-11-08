#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <BH1750.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// DHT22 Setup
#define DHTPIN 4      
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// BH1750 Setup
BH1750 lightMeter;

// MQTT Broker Settings (EMQX Cloud dengan TLS/SSL)
const char *mqtt_broker = "v49cca12.ala.asia-southeast1.emqxsl.com";
const char *mqtt_topic_publish = "emqx/esp32/sensor";  // Topic untuk publish data sensor
const char *mqtt_topic_subscribe = "emqx/esp32/control";  // Topic untuk subscribe
const char *mqtt_username = "2702269135/environment";  // Ganti dengan username EMQX Cloud Anda
const char *mqtt_password = "Stephen Chuang";  // Ganti dengan password EMQX Cloud Anda
const int mqtt_port = 8883;  // Port untuk MQTT over TLS/SSL

// WiFi dan MQTT Client
WiFiClientSecure espClient;
PubSubClient mqtt_client(espClient);

// Interval publish data (5 detik)
unsigned long lastPublish = 0;
const long publishInterval = 5000;

// Function Declarations
void connectToMQTT();
void mqttCallback(char *topic, byte *payload, unsigned int length);
void publishSensorData();

void setup() {
  Serial.begin(115200);
  delay(1000);

  // WiFiManager Setup
  WiFiManager wm;
  
  Serial.println("Memulai WiFi Setup...");
  bool res = wm.autoConnect("ESP32-IoT-Setup", "password123");  // AP dengan password

  if (!res) {
    Serial.println("Gagal konek ke WiFi, restart ESP...");
    delay(3000);
    ESP.restart();
  } else {
    Serial.println("Terkoneksi ke WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }

  // Inisialisasi Sensor
  dht.begin();
  Wire.begin();
  lightMeter.begin();
  Serial.println("Sensor DHT22 dan BH1750 siap!");

  // Setup MQTT
  espClient.setInsecure();  // Untuk testing, skip verifikasi sertifikat SSL
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setKeepAlive(60);
  mqtt_client.setCallback(mqttCallback);
  
  connectToMQTT();
}

void connectToMQTT() {
  while (!mqtt_client.connected()) {
    String client_id = "esp32-client-" + String(WiFi.macAddress());
    Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
    
    if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to EMQX MQTT broker");
      
      // Subscribe ke topic control
      mqtt_client.subscribe(mqtt_topic_subscribe);
      Serial.printf("Subscribed to topic: %s\n", mqtt_topic_subscribe);
      
      // Publish pesan selamat datang
      mqtt_client.publish(mqtt_topic_publish, "ESP32 DHT22 + BH1750 Connected!");
    } else {
      Serial.print("Failed to connect, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  Serial.println("-----------------------");
}

void publishSensorData() {
  // Baca data sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float lux = lightMeter.readLightLevel();

  // Cek apakah pembacaan sensor valid
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Tampilkan di Serial Monitor
  Serial.println("=== Sensor Data ===");
  Serial.print("Suhu (°C): ");
  Serial.println(temperature);
  Serial.print("Kelembaban (%): ");
  Serial.println(humidity);
  Serial.print("Cahaya (lux): ");
  Serial.println(lux);
  Serial.println("====================");

  // Buat JSON payload
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["light"] = lux;
  doc["device"] = "ESP32";
  doc["timestamp"] = millis();

  char jsonBuffer[200];
  serializeJson(doc, jsonBuffer);

  // Publish ke MQTT
  if (mqtt_client.publish(mqtt_topic_publish, jsonBuffer)) {
    Serial.println("Data published to MQTT!");
    Serial.println(jsonBuffer);
  } else {
    Serial.println("Failed to publish data");
  }
  Serial.println();
}

void loop() {
  // Reconnect jika terputus dari MQTT
  if (!mqtt_client.connected()) {
    connectToMQTT();
  }
  mqtt_client.loop();

  // Publish data sensor setiap 5 detik
  unsigned long currentMillis = millis();
  if (currentMillis - lastPublish >= publishInterval) {
    lastPublish = currentMillis;
    publishSensorData();
  }
}