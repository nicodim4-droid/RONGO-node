
//Подключенные библиотеки
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_SHT4x.h>

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

// Сигнал светодиода не блокирующий процессор "НЕБЛОКИРУЮЩИЙ SOS" 
// Паттерн: длительности фаз в мс. Знак: >0 — LED горит, <0 — LED погашен.
// ... --- ...
const int   sosPattern[] = {
    200, -200,  200, -200,  200, -600,   // ... и пауза между буквами
    600, -200,  600, -200,  600, -600,   // --- и пауза между буквами
    200, -200,  200, -200,  200, -2000   // ... и длинная пауза перед повтором
};
const int   sosLength = sizeof(sosPattern) / sizeof(sosPattern[0]);
int         sosIndex  = 0;
unsigned long sosLast = 0;
bool        sosLedOn  = false;

void ledOn()  { led.setPixelColor(0, led.Color(0, 120, 0)); led.show(); } // зелёный
void ledOff() { led.setPixelColor(0, 0);                    led.show(); }

void sosTick() {
    unsigned long now = millis();
    if (now - sosLast < (unsigned long)abs(sosPattern[sosIndex])) return;

    sosLast = now;
    sosIndex = (sosIndex + 1) % sosLength;

    int phase = sosPattern[sosIndex];
    if (phase > 0) { ledOn();  sosLedOn = true;  }
    else           { ledOff(); sosLedOn = false; }
}

// Вебморда
void handleRoot() {
    sensors_event_t humidity, temp;
    sht4.getEvent(&humidity, &temp);

    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>RONGO NODE</title></head>";
    html += "<body style='background:black; color:#00ff00; font-family:monospace; font-size:24px;'>";
    html += "<h1>RONGO NODE ONLINE</h1>";
    html += "<p>Temperature: " + String(temp.temperature, 2) + " &deg;C</p>";
    html += "<p>Humidity: "    + String(humidity.relative_humidity, 2) + " %</p>";
    html += "<p>Uptime: "      + String(millis() / 1000) + " s</p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
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
    } else {
        Serial.println("SHT4x found!");
        sht4.setPrecision(SHT4X_HIGH_PRECISION);
        sht4.setHeater(SHT4X_NO_HEATER);
    }

    // Wi-Fi AP
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(ap_ssid, ap_password);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    // Веб-сервер
    server.on("/", HTTP_GET, handleRoot);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("Web server started");

    // Стартуем SOS
    sosLast = millis();
    ledOn();
}

void loop() {
    server.handleClient();   // обслуживаем клиентов
    sosTick();               // мигаем SOS (не блокирует)

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