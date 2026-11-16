#include <Arduino.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>

namespace {

constexpr char AP_SSID[] = "RONGO";
constexpr char AP_PASSWORD[] = "123456789";
constexpr char INDEX_PATH[] = "/index.html";

const IPAddress localIp(192, 168, 4, 1);
const IPAddress gateway(192, 168, 4, 1);
const IPAddress subnet(255, 255, 255, 0);

WebServer server(80);
bool littleFsReady = false;

void logRequest(const char* path) {
    Serial.print("HTTP GET ");
    Serial.print(path);
    Serial.print(" from ");
    Serial.println(server.client().remoteIP());
}

void handleRoot() {
    logRequest("/");

    if (!littleFsReady) {
        server.send(500, "text/plain", "LittleFS is unavailable. Check Serial Monitor.");
        return;
    }

    File indexFile = LittleFS.open(INDEX_PATH, "r");
    if (!indexFile) {
        Serial.println("ERROR: /index.html is missing from LittleFS.");
        server.send(500, "text/plain", "index.html is missing from LittleFS.");
        return;
    }

    server.streamFile(indexFile, "text/html");
    indexFile.close();
}

void handleStatus() {
    logRequest("/status");
    server.send(200, "application/json", "{\"device\":\"RONGO\",\"status\":\"online\"}");
}

void handleNotFound() {
    Serial.print("HTTP 404 ");
    Serial.println(server.uri());
    server.send(404, "text/plain", "Not found");
}

} // namespace

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println();
    Serial.println("Starting RONGO ESP32 web server...");

    littleFsReady = LittleFS.begin();
    if (littleFsReady) {
        Serial.println("LittleFS mounted successfully.");
        if (!LittleFS.exists(INDEX_PATH)) {
            Serial.println("ERROR: /index.html is missing from LittleFS.");
        }
    } else {
        Serial.println("ERROR: LittleFS mount failed.");
    }

    WiFi.mode(WIFI_AP);
    if (!WiFi.softAPConfig(localIp, gateway, subnet)) {
        Serial.println("ERROR: Failed to configure the Wi-Fi access point IP.");
    }
    if (WiFi.softAP(AP_SSID, AP_PASSWORD)) {
        Serial.println("Wi-Fi access point started: RONGO");
        Serial.print("IP address: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("ERROR: Failed to start the Wi-Fi access point.");
    }

    server.on("/", HTTP_GET, handleRoot);
    server.on("/status", HTTP_GET, handleStatus);
    server.serveStatic("/style.css", LittleFS, "/style.css");
    server.serveStatic("/app.js", LittleFS, "/app.js");
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP server started on port 80.");
}

void loop() {
    server.handleClient();
}
