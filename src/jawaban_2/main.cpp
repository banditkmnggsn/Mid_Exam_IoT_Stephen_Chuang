// Remote Actuator Control using FreeRTOS - Stephen Chuang - 2702269135

#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#define builtin_LED_PIN 22
#define LED_PIN 33
const char *mqtt_broker = "v49cca12.ala.asia-southeast1.emqxsl.com";
const char *mqtt_topic_subscribe = "stephenchuang/2702269135/control";
const char *mqtt_username = "2702269135/environment";
const char *mqtt_password = "Stephen Chuang";
const int mqtt_port = 8883;

WiFiClientSecure espClient;
PubSubClient mqtt_client(espClient);

TaskHandle_t blinkTaskHandle = NULL;
SemaphoreHandle_t alertSemaphore;

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("MQTT message received: ");
  Serial.println(message);

  if (message == "ALERT") {
    xSemaphoreGive(alertSemaphore);
  }
}

void connectToMQTT() {
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setCallback(mqttCallback);
  espClient.setInsecure();

  while (!mqtt_client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqtt_client.connect("ESP32Client", mqtt_username, mqtt_password)) {
      Serial.println("MQTT connected");
      mqtt_client.subscribe(mqtt_topic_subscribe);
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.println(mqtt_client.state());
      delay(2000);
    }
  }
}

void mqttTask(void *parameter) {
  WiFiManager wm;
  wm.autoConnect("Stephen-IoT");

  connectToMQTT();

  while (true) {
    mqtt_client.loop();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void blinkTask(void *parameter) {
  pinMode(LED_PIN, OUTPUT);
  pinMode(builtin_LED_PIN, OUTPUT);
  bool alertMode = false;
  
  while (true) {
    if (xSemaphoreTake(alertSemaphore, 0) == pdTRUE) {
      alertMode = true;
      Serial.println("ALERT received: LED ON for 10s");
      digitalWrite(LED_PIN, HIGH); 
      vTaskDelay(10000 / portTICK_PERIOD_MS);
      alertMode = false;
      Serial.println("Returning to blink mode");
      digitalWrite(LED_PIN, LOW);
    }
    
    if (!alertMode) {
      digitalWrite(builtin_LED_PIN, LOW);//lolin32 menggunakan LOW Triggeer pada built in led nya
      vTaskDelay(500 / portTICK_PERIOD_MS);
      digitalWrite(builtin_LED_PIN, HIGH);
      vTaskDelay(500 / portTICK_PERIOD_MS);
    } else {
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  }
}

void setup() {
  Serial.begin(115200);
  alertSemaphore = xSemaphoreCreateBinary();

  xTaskCreatePinnedToCore(mqttTask, "MQTT Task", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(blinkTask, "Blink Task", 4096, NULL, 1, &blinkTaskHandle, 1);
}

void loop() {

}