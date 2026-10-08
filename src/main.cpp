#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

// ====== СВЕТОДИОД (WS2812 на GPIO48) ======
#define LED_PIN   48
#define LED_COUNT 1
Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// ====== ТОЧКА ДОСТУПА ======
const char* ap_ssid     = "RONGO_NODE_AP";
const char* ap_password = "12345678";

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway (192, 168, 4, 1);
IPAddress subnet  (255, 255, 255, 0);

WebServer server(80);

// ====== МИГАЛКА (неблокирующая) ======
// Короткая вспышка раз в секунду — индикатор «прошивка жива»
const unsigned long BLINK_PERIOD = 1000;  // раз в секунду
const unsigned long BLINK_ON_MS  = 60;    // длительность вспышки
unsigned long lastBlink = 0;
bool ledIsOn = false;

void ledOn()  { led.setPixelColor(0, led.Color(0, 120, 0)); led.show(); } // зелёный
void ledOff() { led.setPixelColor(0, 0);                    led.show(); }

void blinkTick() {
    unsigned long now = millis();

    if (!ledIsOn && (now - lastBlink >= BLINK_PERIOD)) {
        ledIsOn = true;
        lastBlink = now;
        ledOn();
    } else if (ledIsOn && (now - lastBlink >= BLINK_ON_MS)) {
        ledIsOn = false;
        ledOff();
    }
}

// ====== ВЕБ ======
void handleRoot() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>RONGO NODE</title></head>";
    html += "<body style='background:black; color:#00ff00; font-family:monospace; font-size:24px;'>";
    html += "<h1>RONGO NODE ONLINE</h1>";
    html += "<p>Status: Web server is working!</p>";
    html += "<p>Uptime: " + String(millis() / 1000) + " s</p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void handleNotFound() {
    server.send(404, "text/plain", "Not found");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Светодиод
    led.begin();
    led.setBrightness(80);
    ledOff();

    // Wi-Fi точка доступа
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(ap_ssid, ap_password);

    Serial.println();
    Serial.println("Access Point started!");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());

    // Веб-сервер
    server.on("/", HTTP_GET, handleRoot);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("Web server started");
}

void loop() {
    server.handleClient();   // обслуживаем клиентов
    blinkTick();             // мигаем (не блокирует)
}