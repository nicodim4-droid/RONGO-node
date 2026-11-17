#include <Arduino.h>
#include <LittleFS.h>
#include <Wire.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Adafruit_SHT4x.h>
#include <math.h>

namespace {

constexpr char AP_SSID[] = "RONGO";
constexpr char AP_PASSWORD[] = "123456789";
constexpr char INDEX_PATH[] = "/index.html";
constexpr char EN_INDEX_PATH[] = "/en/index.html";
constexpr uint8_t SDA_PIN = 9;
constexpr uint8_t SCL_PIN = 10;
constexpr uint32_t SENSOR_UPDATE_INTERVAL_MS = 2000;

const IPAddress localIp(192, 168, 4, 1);
const IPAddress gateway(192, 168, 4, 1);
const IPAddress subnet(255, 255, 255, 0);

WebServer server(80);
Adafruit_SHT4x sht4;
bool littleFsReady = false;
bool sensorDetected = false;
bool sensorReadingValid = false;
float sensorTemperature = 0.0f;
float sensorHumidity = 0.0f;
uint32_t lastSensorReadMs = 0;

void logRequest(const char* path) {
    Serial.print("HTTP GET ");
    Serial.print(path);
    Serial.print(" from ");
    Serial.println(server.client().remoteIP());
}

void serveIndex(const char* requestPath, const char* filePath) {
    logRequest(requestPath);
    if (!littleFsReady) {
        server.send(500, "text/plain", "LittleFS is unavailable. Check Serial Monitor.");
        return;
    }

    File indexFile = LittleFS.open(filePath, "r");
    if (!indexFile) {
        Serial.print("ERROR: ");
        Serial.print(filePath);
        Serial.println(" is missing from LittleFS.");
        server.send(500, "text/plain", "index.html is missing from LittleFS.");
        return;
    }

    server.streamFile(indexFile, "text/html");
    indexFile.close();
}

void handleRoot() {
    serveIndex("/", INDEX_PATH);
}

void handleEnglishRoot() {
    serveIndex("/en/", EN_INDEX_PATH);
}

void handleStatus() {
    logRequest("/status");
    server.send(200, "application/json", "{\"device\":\"RONGO\",\"status\":\"online\"}");
}

bool readSensor() {
    if (!sensorDetected) {
        sensorReadingValid = false;
        return false;
    }

    sensors_event_t humidityEvent;
    sensors_event_t temperatureEvent;
    if (!sht4.getEvent(&humidityEvent, &temperatureEvent)
        || !isfinite(temperatureEvent.temperature)
        || !isfinite(humidityEvent.relative_humidity)) {
        sensorReadingValid = false;
        return false;
    }

    sensorTemperature = temperatureEvent.temperature;
    sensorHumidity = humidityEvent.relative_humidity;
    sensorReadingValid = true;
    return true;
}

void updateSensor() {
    const uint32_t now = millis();
    if (now - lastSensorReadMs < SENSOR_UPDATE_INTERVAL_MS) {
        return;
    }

    lastSensorReadMs = now;
    if (!sensorDetected) {
        return;
    }

    if (!readSensor()) {
        Serial.println("WARNING: SHT40 reading failed.");
    }
}

void handleDeviceData() {
    logRequest("/api/device");

    const bool readingAvailable = sensorDetected && sensorReadingValid;
    String response;
    response.reserve(384);
    response += "{\"status\":\"online\",\"deviceType\":\"temperature\",";
    response += "\"name\":\"RONGO-node\",\"article\":null,\"ip\":\"";
    response += WiFi.softAPIP().toString();
    response += "\",\"uptimeSeconds\":";
    response += String(millis() / 1000);
    response += ",\"wifiRssiDbm\":null,\"can\":{\"status\":\"waiting\",";
    response += "\"discoveredNodes\":null,\"exchange\":\"waiting\"},";
    response += "\"sensor\":{\"type\":\"SHT40\",\"status\":\"";
    response += !sensorDetected ? "offline" : (readingAvailable ? "online" : "error");
    response += "\",\"temperatureC\":";
    response += readingAvailable ? String(sensorTemperature, 2) : String("null");
    response += ",\"humidityPercent\":";
    response += readingAvailable ? String(sensorHumidity, 2) : String("null");
    response += "}}";

    server.send(200, "application/json", response);
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
        if (!LittleFS.exists(EN_INDEX_PATH)) {
            Serial.println("ERROR: /en/index.html is missing from LittleFS.");
        }
    } else {
        Serial.println("ERROR: LittleFS mount failed.");
    }

    Wire.begin(SDA_PIN, SCL_PIN);
    sensorDetected = sht4.begin();
    if (sensorDetected) {
        sht4.setPrecision(SHT4X_HIGH_PRECISION);
        sht4.setHeater(SHT4X_NO_HEATER);
        Serial.println("SHT40 sensor found.");
        readSensor();
    } else {
        Serial.println("WARNING: SHT40 sensor not found; continuing without sensor data.");
    }
    lastSensorReadMs = millis();

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
    server.on("/en/", HTTP_GET, handleEnglishRoot);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/api/device", HTTP_GET, handleDeviceData);
    server.serveStatic("/style.css", LittleFS, "/style.css");
    server.serveStatic("/app.js", LittleFS, "/app.js");
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP server started on port 80.");
}

void loop() {
    server.handleClient();
    updateSensor();
}
