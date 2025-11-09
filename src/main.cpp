// jawaban nomor 2 - Stephen Chuang - 2702269135
#include <WiFi.h>
#include <PubSubClient.h>

// Konfigurasi WiFi
const char* ssid = "NAMA_WIFI_KAMU";
const char* password = "PASSWORD_WIFI_KAMU";

// Konfigurasi MQTT
const char* mqtt_server = "broker.emqx.io";
const char* topic_subscribe = "1234567890/control/LED"; // Ganti dengan NIM kamu

WiFiClient espClient;
PubSubClient client(espClient);

// LED onboard ESP32
#define LED_PIN 22

// FreeRTOS handle dan semaphore
TaskHandle_t blinkTaskHandle = NULL;
SemaphoreHandle_t alertSemaphore;

void setup_wifi() {
  delay(10);
  Serial.println("Menghubungkan ke WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi terhubung");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Pesan MQTT diterima: ");
  Serial.println(message);

  if (message == "ALERT") {
    xSemaphoreGive(alertSemaphore); // Kirim sinyal ke task LED
  }
}

void mqttTask(void* parameter) {
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Menghubungkan ke MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("MQTT terhubung");
      client.subscribe(topic_subscribe);
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }

  while (true) {
    client.loop();
    vTaskDelay(10 / portTICK_PERIOD_MS); // Hindari blocking
  }
}

void blinkTask(void* parameter) {
  pinMode(LED_PIN, OUTPUT);

  while (true) {
    if (xSemaphoreTake(alertSemaphore, 0) == pdTRUE) {
      Serial.println("ALERT diterima: LED ON selama 10 detik");
      digitalWrite(LED_PIN, HIGH);
      vTaskDelay(10000 / portTICK_PERIOD_MS);
      Serial.println("Kembali ke mode blink");
    }

    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  alertSemaphore = xSemaphoreCreateBinary();

  xTaskCreatePinnedToCore(
    mqttTask, "MQTT Task", 4096, NULL, 2, NULL, 1
  );

  xTaskCreatePinnedToCore(
    blinkTask, "Blink Task", 2048, NULL, 1, &blinkTaskHandle, 1
  );
}

void loop() {
  // Tidak digunakan karena semua berjalan di FreeRTOS task
}