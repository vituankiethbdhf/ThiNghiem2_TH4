#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <DHT.h>

// Cau hinh Wi-Fi Wokwi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Ma sinh vien
#define SV_ID "25112515"

// Cam bien DHT22
#define DHT_PIN 15
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

// Web Server cong 80
WebServer server(80);

unsigned long lastCheck = 0;

// Tao chuoi JSON tu cam bien. Tra ve false neu doc cam bien loi.
bool taoJson(char* buf, size_t n) {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    return false;
  }

  snprintf(buf, n,
           "{\"sv_id\":\"%s\",\"temperature\":%.1f,\"humidity\":%.1f}",
           SV_ID, t, h);
  return true;
}

// GET /api/sensor -> tra ve JSON nhiet do, do am
void handleSensor() {
  char json[128];

  server.sendHeader("Access-Control-Allow-Origin", "*");

  if (!taoJson(json, sizeof(json))) {
    server.send(503, "application/json",
                "{\"error\":\"Khong doc duoc cam bien DHT22\"}");
    return;
  }

  server.send(200, "application/json", json);
  Serial.print("GET /api/sensor -> ");
  Serial.println(json);
}

void handleRoot() {
  server.send(200, "text/plain", "ESP32 REST API - thu GET /api/sensor");
}

void handleNotFound() {
  server.send(404, "application/json", "{\"error\":\"Not found\"}");
}

// Thu goi chinh REST API cua ESP32 qua dia chi loopback 127.0.0.1.
// Chay trong task rieng vi WebServer xu ly o loop(), neu goi trong cung
// mot luong thi se bi treo cho den khi het thoi gian cho.
void tuKiemTraTask(void* param) {
  for (;;) {
    vTaskDelay(10000 / portTICK_PERIOD_MS);

    HTTPClient http;
    http.begin("http://127.0.0.1/api/sensor");
    int code = http.GET();

    Serial.print("[Tu goi HTTP] GET http://127.0.0.1/api/sensor -> ");

    if (code > 0) {
      Serial.print("HTTP ");
      Serial.print(code);
      Serial.print(" ");
      Serial.println(http.getString());
    } else {
      Serial.print("Loi: ");
      Serial.println(http.errorToString(code));
    }

    http.end();
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("===== BAI 1: REST API =====");

  dht.begin();

  Serial.println("Dang ket noi Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password, 6);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Da ket noi Wi-Fi!");
  Serial.print("Dia chi IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/sensor", HTTP_GET, handleSensor);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("REST API da san sang: GET /api/sensor");

  xTaskCreate(tuKiemTraTask, "tuKiemTra", 8192, NULL, 1, NULL);
}

void loop() {
  server.handleClient();

  // Moi 10 giay in JSON ma endpoint /api/sensor se tra ve
  if (millis() - lastCheck >= 10000) {
    lastCheck = millis();

    char json[128];
    if (taoJson(json, sizeof(json))) {
      Serial.print("[JSON /api/sensor] ");
      Serial.println(json);
    } else {
      Serial.println("[JSON /api/sensor] Loi doc cam bien DHT22");
    }
  }
}
