#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid = "TP-Link_14D8";
const char* password = "85814110";

IPAddress local_IP(192, 168, 0, 50);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 0, 1);

WebServer server(80);

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("==============================");
    Serial.println("RONGO NODE");
    Serial.println("Web test");
    Serial.println("==============================");

    // LittleFS
    if (!LittleFS.begin(true))
    {
        Serial.println("LittleFS ERROR");
        while (true)
        {
            delay(1000);
        }
    }

    Serial.println("LittleFS OK");

    // Wi-Fi
    if (!WiFi.config(local_IP, gateway, subnet, dns))
    {
        Serial.println("WiFi config failed");
    }

    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi OK");

    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    // Web server
    server.serveStatic("/", LittleFS, "/index.html");

    server.begin();

    Serial.println("Web server started");
    Serial.println("==============================");
}

void loop()
{
    server.handleClient();
}