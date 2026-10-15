//Подключенные библиотеки
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_SHT4x.h>
#include <LittleFS.h>

// Версия прошивки
#define FW_VERSION "0.2.0"

// СВЕТОДИОД (WS2812 на GPIO48)
#define LED_PIN   48
#define LED_COUNT 1
Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// SHT40 (I2C)
Adafruit_SHT4x sht4 = Adafruit_SHT4x();
#define SDA_PIN 9
#define SCL_PIN 10

// ТОЧКА ДОСТУПА
const char* ap_ssid     = "RONGO_NODE_AP";
const char* ap_password = "12345678";
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway (192, 168, 4, 1);
IPAddress subnet  (255, 255, 255, 0);
WebServer server(80);

// Статус датчика
bool sht4_ok = false;

// ====== МИГАЛКА (неблокирующая) ======
const unsigned long BLINK_PERIOD = 250;
unsigned long lastBlink = 0;
bool          ledState  = false;

void ledOn()  { led.setPixelColor(0, led.Color(0, 0, 120)); led.show(); } // синий
void ledOff() { led.setPixelColor(0, 0);                    led.show(); }

void blinkTick() {
    unsigned long now = millis();
    if (now - lastBlink >= BLINK_PERIOD) {
        lastBlink = now;
        ledState = !ledState;
        if (ledState) ledOn();
        else          ledOff();
    }
}

// ====== ВЕБ ======
void handleData() {
    sensors_event_t humidity, temp;
    bool ok = sht4.getEvent(&humidity, &temp);

    // Если датчик не читается — вернём null вместо чисел
    String tempStr = ok ? String(temp.temperature, 2)      : "null";
    String humStr  = ok ? String(humidity.relative_humidity, 2) : "null";

    String json = "{";
    json += "\"temp\":"   + tempStr + ",";
    json += "\"hum\":"    + humStr  + ",";
    json += "\"uptime\":" + String(millis() / 1000) + ",";
    json += "\"ip\":\""   + WiFi.softAPIP().toString() + "\",";
    json += "\"rssi\":"   + String(WiFi.RSSI()) + ",";
    json += "\"fw\":\""   + String(FW_VERSION) + "\",";
    json += "\"sensor\":" + String(ok ? "true" : "false");
    json += "}";

    server.send(200, "application/json", json);
}

void handleNotFound() {
    server.send(404, "text/plain", "Not found");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    led.begin();
    led.setBrightness(80);
    ledOff();

    // I2C + SHT40
    Wire.begin(SDA_PIN, SCL_PIN);
    if (!sht4.begin()) {
        Serial.println("Couldn't find SHT4x");
        sht4_ok = false;
    } else {
        Serial.println("SHT4x found!");
        sht4.setPrecision(SHT4X_HIGH_PRECISION);
        sht4.setHeater(SHT4X_NO_HEATER);
        sht4_ok = true;
    }

    // LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS mount failed!");
    } else {
        Serial.println("LittleFS mounted");
    }

    // Wi-Fi AP
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(ap_ssid, ap_password);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    // Веб-сервер: статика из LittleFS
    server.serveStatic("/",         LittleFS, "/index.html");
    server.serveStatic("/style.css", LittleFS, "/style.css");
    server.serveStatic("/app.js",   LittleFS, "/app.js");

    // API
    server.on("/data", HTTP_GET, handleData);

    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("Web server started");

    // Стартуем мигалку
    lastBlink = millis();
    ledOn();
}

void loop() {
    server.handleClient();
    blinkTick();

    // Раз в 5 секунд — лог SHT40 в Serial
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 5000) {
        lastLog = millis();
        sensors_event_t humidity, temp;
        if (sht4.getEvent(&humidity, &temp)) {
            Serial.print("Temp: "); Serial.print(temp.temperature, 2); Serial.print(" C  ");
            Serial.print("Hum: ");  Serial.print(humidity.relative_humidity, 2); Serial.println(" %");
        }
    }
}