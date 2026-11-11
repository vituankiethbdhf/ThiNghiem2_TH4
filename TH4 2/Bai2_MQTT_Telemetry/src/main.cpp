#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Cau hinh Wi-Fi Wokwi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Cau hinh MQTT Broker
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Ma sinh vien -> topic rieng
#define SV_ID "25112515"
const char* TOPIC_TELEMETRY = "TH4/" SV_ID "/telemetry";

// Cam bien DHT22
#define DHT_PIN 15
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastPublish = 0;

void ketNoiMQTT() {
  while (!client.connected()) {
    Serial.print("Dang ket noi MQTT...");

    String clientId = String("ESP32-" SV_ID "-") + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("Thanh cong!");
    } else {
      Serial.print("That bai, ma loi = ");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("===== BAI 2: MQTT TELEMETRY =====");

  dht.begin();

  Serial.print("Dang ket noi Wi-Fi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password, 6);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Da ket noi Wi-Fi!");

  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    ketNoiMQTT();
  }

  client.loop();

  // Publish telemetry moi 5 giay
  if (millis() - lastPublish >= 5000) {
    lastPublish = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(t) || isnan(h)) {
      Serial.println("Loi doc cam bien DHT22!");
      return;
    }

    char json[128];
    snprintf(json, sizeof(json),
             "{\"sv_id\":\"%s\",\"temperature\":%.1f,\"humidity\":%.1f}",
             SV_ID, t, h);

    client.publish(TOPIC_TELEMETRY, json);

    Serial.print("Publish ");
    Serial.print(TOPIC_TELEMETRY);
    Serial.print(": ");
    Serial.println(json);
  }
}
