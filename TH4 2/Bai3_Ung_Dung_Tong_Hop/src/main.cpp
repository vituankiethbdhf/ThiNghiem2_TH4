
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================
// 1. WIFI CONFIGURATION
// =====================================

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// =====================================
// 2. MQTT CONFIGURATION
// =====================================

const char* MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

const char* SV_ID = "25112515";

// Topic gửi dữ liệu cảm biến
const char* MQTT_TOPIC = "AET2022/25112515/telemetry";

// Topic nhận lệnh điều khiển LED
const char* LED_CONTROL_TOPIC = "AET2022/25112515/led/set";

// =====================================
// 3. GPIO CONFIGURATION
// THEO diagram.json CỦA BẠN
// =====================================

#define DHT_PIN       15
#define DHT_TYPE      DHT22

#define LED_PIN       2
#define BUTTON_PIN    4

#define OLED_SDA      21
#define OLED_SCL      22

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS  0x3C

// =====================================
// 4. CREATE OBJECTS
// =====================================

DHT dht(DHT_PIN, DHT_TYPE);

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

WiFiClient espClient;
PubSubClient mqtt(espClient);

// =====================================
// 5. VARIABLES
// =====================================

float temperature = NAN;
float humidity = NAN;

bool ledState = false;
bool oledReady = false;

bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;

unsigned long lastDebounceTime = 0;
unsigned long lastSensorTime = 0;
unsigned long lastPublishTime = 0;
unsigned long lastWifiAttempt = 0;
unsigned long lastMqttAttempt = 0;

const unsigned long DEBOUNCE_DELAY = 50;
const unsigned long SENSOR_INTERVAL = 2000;
const unsigned long MQTT_INTERVAL = 2000;
const unsigned long RECONNECT_INTERVAL = 5000;

// =====================================
// 6. UPDATE OLED
// =====================================

void updateOLED() {

  if (!oledReady) return;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("ESP32 - TH4");

  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

  display.setCursor(0, 15);
  display.print("Temp: ");

  if (isnan(temperature)) {
    display.println("Error");
  } else {
    display.print(temperature, 1);
    display.println(" C");
  }

  display.setCursor(0, 27);
  display.print("Hum: ");

  if (isnan(humidity)) {
    display.println("Error");
  } else {
    display.print(humidity, 1);
    display.println(" %");
  }

  display.setCursor(0, 39);
  display.print("MQTT: ");
  display.println(mqtt.connected() ? "CONNECTED" : "OFFLINE");

  display.setCursor(0, 52);
  display.print("LED: ");
  display.println(ledState ? "ON" : "OFF");

  display.display();
}

// =====================================
// 7. READ DHT22 SENSOR
// =====================================

void readSensor() {

  float newHumidity = dht.readHumidity();
  float newTemperature = dht.readTemperature();

  if (isnan(newHumidity) || isnan(newTemperature)) {
    Serial.println("ERROR: DHT22 reading failed!");
    return;
  }

  humidity = newHumidity;
  temperature = newTemperature;

  Serial.println("------------------------");

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}

// =====================================
// 8. SET LED STATE
// =====================================

void setLED(bool state) {

  ledState = state;

  digitalWrite(
    LED_PIN,
    ledState ? HIGH : LOW
  );

  Serial.print("LED STATE: ");
  Serial.println(ledState ? "ON" : "OFF");

  updateOLED();
}

// =====================================
// 9. MQTT RECEIVE CALLBACK
// NHAN LENH TU NODE-RED
// =====================================

void mqttCallback(
  char* topic,
  byte* payload,
  unsigned int length
) {

  String message = "";

  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  message.trim();

  Serial.println("========================");
  Serial.print("MQTT received topic: ");
  Serial.println(topic);

  Serial.print("MQTT received message: ");
  Serial.println(message);

  if (String(topic) == LED_CONTROL_TOPIC) {

    if (message == "ON") {

      setLED(true);

    } else if (message == "OFF") {

      setLED(false);

    } else {

      Serial.println("Unknown LED command!");
    }
  }
}

// =====================================
// 10. BUTTON CONTROL
// BAM NUT TREN WOKWI
// =====================================

void handleButton() {

  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {

    if (reading != stableButtonState) {

      stableButtonState = reading;

      if (stableButtonState == LOW) {

        setLED(!ledState);

        Serial.println("BUTTON PRESSED");
      }
    }
  }

  lastButtonReading = reading;
}

// =====================================
// 11. WIFI CONNECTION
// =====================================

void connectWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.println("Connecting to WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startTime = millis();

  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - startTime < 15000
  ) {

    delay(300);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println();
    Serial.println("WiFi connected!");

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

  } else {

    Serial.println();
    Serial.println("WiFi connection failed!");
  }

  updateOLED();
}

// =====================================
// 12. MQTT CONNECTION
// =====================================

void reconnectMQTT() {

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (mqtt.connected()) {
    return;
  }

  Serial.println("Connecting to MQTT...");

  char clientId[50];

  uint64_t chipId = ESP.getEfuseMac();

  snprintf(
    clientId,
    sizeof(clientId),
    "ESP32-%s-%04X",
    SV_ID,
    (uint16_t)(chipId & 0xFFFF)
  );

  if (mqtt.connect(clientId)) {

    Serial.println("MQTT connected!");

    // Dang ky nhan lenh dieu khien LED
    mqtt.subscribe(LED_CONTROL_TOPIC);

    Serial.println("Subscribed to LED control topic!");

  } else {

    Serial.print("MQTT connection failed, state: ");
    Serial.println(mqtt.state());
  }

  updateOLED();
}

// =====================================
// 13. MQTT PUBLISH TELEMETRY
// =====================================

void publishTelemetry() {

  if (!mqtt.connected()) {
    return;
  }

  if (isnan(temperature) || isnan(humidity)) {
    return;
  }

  char payload[180];

  snprintf(
    payload,
    sizeof(payload),
    "{\"sv_id\":\"%s\",\"temperature\":%.1f,\"humidity\":%.1f,\"led\":\"%s\"}",
    SV_ID,
    temperature,
    humidity,
    ledState ? "ON" : "OFF"
  );

  bool success = mqtt.publish(
    MQTT_TOPIC,
    payload
  );

  if (success) {

    Serial.println("MQTT PUBLISH SUCCESS");

    Serial.print("Topic: ");
    Serial.println(MQTT_TOPIC);

    Serial.print("Publish: ");
    Serial.println(payload);

  } else {

    Serial.println("MQTT publish failed!");
  }
}

// =====================================
// 14. SETUP
// =====================================

void setup() {

  Serial.begin(115200);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" AET2022 - TH4 - BAI 3");
  Serial.println(" ESP32 APPLICATION");
  Serial.print(" SV_ID: ");
  Serial.println(SV_ID);
  Serial.println("==============================");

  // GPIO
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

  // DHT22
  dht.begin();

  // OLED
  Wire.begin(OLED_SDA, OLED_SCL);

  oledReady = display.begin(
    SSD1306_SWITCHCAPVCC,
    OLED_ADDRESS
  );

  if (oledReady) {

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("ESP32 Starting...");
    display.display();

    Serial.println("OLED initialized!");

  } else {

    Serial.println("OLED initialization failed!");
  }

  // MQTT
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  mqtt.setBufferSize(256);

  // Read sensor
  readSensor();
  updateOLED();

  // Connect WiFi
  connectWiFi();

  // Connect MQTT
  reconnectMQTT();

  Serial.println("System ready!");
}

// =====================================
// 15. MAIN LOOP
// =====================================

void loop() {

  // 1. Button control
  handleButton();

  // 2. Read sensor every 2 seconds
  if (millis() - lastSensorTime >= SENSOR_INTERVAL) {

    lastSensorTime = millis();

    readSensor();
    updateOLED();
  }

  // 3. Reconnect WiFi
  if (WiFi.status() != WL_CONNECTED) {

    if (millis() - lastWifiAttempt >= RECONNECT_INTERVAL) {

      lastWifiAttempt = millis();

      connectWiFi();
    }
  }

  // 4. Reconnect MQTT
  if (
    WiFi.status() == WL_CONNECTED &&
    !mqtt.connected()
  ) {

    if (millis() - lastMqttAttempt >= RECONNECT_INTERVAL) {

      lastMqttAttempt = millis();

      reconnectMQTT();
    }
  }

  // 5. Maintain MQTT connection
  mqtt.loop();

  // 6. Publish telemetry every 2 seconds
  if (
    mqtt.connected() &&
    millis() - lastPublishTime >= MQTT_INTERVAL
  ) {

    lastPublishTime = millis();

    publishTelemetry();
  }
}
