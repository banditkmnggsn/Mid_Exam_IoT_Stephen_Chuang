//jawaban nomor 1 - Stephen Chuang - 2702269135
#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <BH1750.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DHTPIN 4      
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

BH1750 lightMeter;

const char *mqtt_broker = "v49cca12.ala.asia-southeast1.emqxsl.com";
const char *mqtt_topic_publish = "stephenchuang/2702269135/sensor";
const char *mqtt_topic_subscribe = "stephenchuang/2702269135/control";
const char *mqtt_username = "2702269135/environment";
const char *mqtt_password = "Stephen Chuang";
const int mqtt_port = 8883;

WiFiClientSecure espClient;
PubSubClient mqtt_client(espClient);

unsigned long lastPublish = 0;
const long publishInterval = 5000;

void connectToMQTT();
void mqttCallback(char *topic, byte *payload, unsigned int length);
void publishSensorData();

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFiManager wm;

  Serial.println("WiFi connecting...");
  bool res = wm.autoConnect("lolin 32 lite", "stephen chuang");

  if (!res) {
    Serial.println("Gagal konek ke WiFi, riset ESP...");
    delay(3000);
    ESP.restart();
  } else {
    Serial.println("Terkoneksi ke WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }

  dht.begin();
  Wire.begin();
  lightMeter.begin();
  espClient.setInsecure();
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

      mqtt_client.subscribe(mqtt_topic_subscribe);
      Serial.printf("Subscribed to topic: %s\n", mqtt_topic_subscribe);

      mqtt_client.publish(mqtt_topic_publish, "ESP32 DHT22 + BH1750 Connected!");
    } else {
      Serial.print("Failed to connect, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("path topik: ");
  Serial.println(topic);
  Serial.print("pesan: ");
  String message = "";
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  Serial.println(message);
  Serial.println("-----------------------");
}

void publishSensorData()
{
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float lux = lightMeter.readLightLevel();

  if (isnan(temperature) || isnan(humidity))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.println("=== Sensor Data ===");
  Serial.print("Suhu (°C): ");
  Serial.println(temperature);
  Serial.print("Kelembaban (%): ");
  Serial.println(humidity);
  Serial.print("Cahaya (lux): ");
  Serial.println(lux);
  Serial.println("====================");

  // struktur data disesuaikan dengan JSON biar gampang diolah di MQTT Broker
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["light"] = lux;
  doc["device"] = "ESP32";
  doc["timestamp"] = millis();

  char jsonBuffer[200];
  serializeJson(doc, jsonBuffer);

  if (mqtt_client.publish(mqtt_topic_publish, jsonBuffer))
  {
    Serial.println("Data published to MQTT!");
    Serial.println(jsonBuffer);
  }
  else
  {
    Serial.println("Failed to publish data");
  }
  Serial.println();
}

void loop()
{
  if (!mqtt_client.connected())
  {
    connectToMQTT();
  }
  mqtt_client.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - lastPublish >= publishInterval)
  {
    lastPublish = currentMillis;
    publishSensorData();
  }
}