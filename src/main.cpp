#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_SHT4x.h>

const char* ssid = "TP-Link_14D8";
const char* password = "85814110";

IPAddress local_IP(192, 168, 0, 50);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 0, 1);

WebServer server(80);
Adafruit_SHT4x sht4;

void handleRoot() {
  sensors_event_t humidity, temp;
  sht4.getEvent(&humidity, &temp);

  float t = temp.temperature;
  float h = humidity.relative_humidity;

  String page = R"rawliteral(<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta http-equiv="refresh" content="5">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>RONGO NODE :: SHT40</title>
<style>
  html, body {
    margin: 0; padding: 0; height: 100%;
    background: #000;
    color: #33ff66;
    font-family: "Courier New", Courier, monospace;
    overflow: hidden;
  }
  /* CRT-развёртка */
  body::before {
    content: "";
    position: fixed; inset: 0;
    background: repeating-linear-gradient(
      to bottom,
      rgba(0,255,80,0.06) 0px,
      rgba(0,255,80,0.06) 1px,
      transparent 1px,
      transparent 3px
    );
    pointer-events: none;
    z-index: 10;
  }
  /* Виньетка + лёгкое свечение */
  body::after {
    content: "";
    position: fixed; inset: 0;
    background: radial-gradient(ellipse at center,
                rgba(0,255,80,0.10) 0%,
                rgba(0,0,0,0.55) 70%,
                rgba(0,0,0,0.9) 100%);
    pointer-events: none;
    z-index: 11;
  }
  .screen {
    position: relative;
    max-width: 780px;
    margin: 30px auto;
    padding: 20px 26px 30px;
    border: 1px solid #33ff66;
    box-shadow: 0 0 12px #33ff66, inset 0 0 24px rgba(0,255,80,0.15);
    text-shadow: 0 0 4px #33ff66, 0 0 8px rgba(51,255,102,0.6);
    z-index: 5;
  }
  h1 {
    font-size: 18px;
    letter-spacing: 4px;
    margin: 0 0 4px;
    text-transform: uppercase;
  }
  .sub {
    font-size: 12px;
    letter-spacing: 2px;
    color: #22aa44;
    margin-bottom: 18px;
  }
  pre {
    font-size: 15px;
    line-height: 1.35;
    margin: 0;
    white-space: pre;
    overflow-x: auto;
  }
  .label { color: #22cc55; }
  .value { color: #aaffbb; }
  .warn  { color: #ffcc33; }
  .err   { color: #ff4444; }
  .blink {
    display: inline-block;
    width: 9px;
    height: 15px;
    background: #33ff66;
    box-shadow: 0 0 6px #33ff66;
    vertical-align: -2px;
    animation: blink 1s steps(2, start) infinite;
  }
  @keyframes blink { to { visibility: hidden; } }
  .footer {
    margin-top: 18px;
    font-size: 11px;
    color: #22aa44;
    letter-spacing: 2px;
  }
</style>
</head>
<body>
<div class="screen">
<pre>
<span class="label">┌──────────────────────────────────────────────────────┐</span>
<span class="label">│</span>  <span class="value">WareHausEdition I</span>   <span class="label">::</span> <span class="value">RONGO NODE</span>   <span class="label">│</span>
<span class="label">└──────────────────────────────────────────────────────┘</span>

<span class="label">> ИНИЦИАЛИЗАЦИЯ СЕНСОРА SHT40 ..............</span> <span class="value">[ OK ]</span>
<span class="label">> КАНАЛ СВЯЗИ Wi-Fi ........................</span> <span class="value">[ OK ]</span>
<span class="label">> СЕТЕВОЙ АДРЕС:</span> <span class="value">%IP%</span>
<span class="label">> ШЛЮЗ:</span>         <span class="value">%GW%</span>

<span class="label">─────────────────── ТЕЛЕМЕТРИЯ ───────────────────</span>

  <span class="label">ТЕМПЕРАТУРА .......</span> <span class="value">%TEMP% °C</span>
  <span class="label">ВЛАЖНОСТЬ .........</span> <span class="value">%HUM% %</span>

  <span class="label">СТАТУС ОБЪЕКТА ....</span> <span class="value">НОРМА</span>
  <span class="label">СЛЕДУЮЩИЙ ОПРОС ...</span> <span class="value">5 СЕК</span> <span class="blink"></span>

<span class="label">──────────────────────────────────────────────────</span>
</pre>
<div class="footer">&gt; КОРПОРАЦИЯ «ВЕЙЛАНД-ЮТАНИ» · ОТДЕЛ СПЕЦ. ЗАКАЗОВ · КЛАСС 1</div>
</div>
</body>
</html>)rawliteral";

  page.replace("%IP%",   WiFi.localIP().toString());
  page.replace("%GW%",   WiFi.gatewayIP().toString());
  page.replace("%TEMP%", String(t, 1));
  page.replace("%HUM%",  String(h, 1));

  server.send(200, "text/html; charset=UTF-8", page);
}

void handleNotFound() {
  String page = R"rawliteral(<!DOCTYPE html>
<html><head><meta charset="UTF-8"><title>404</title>
<style>
  body { background:#000; color:#33ff66; font-family:"Courier New", monospace;
         padding:40px; text-shadow:0 0 6px #33ff66; }
  pre { font-size:16px; }
</style></head>
<body><pre>
┌─────────────────────────────────────┐
│   О Ш И Б К А   4 0 4               │
│   ЗАПРОШЕННЫЙ РЕСУРС НЕ НАЙДЕН      │
└─────────────────────────────────────┘
&gt; доступные директивы: /
</pre></body></html>)rawliteral";
  server.send(404, "text/html; charset=UTF-8", page);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("╔════════════════════════════════════════════╗");
  Serial.println("║   RONGO_Project :: RONGO_NODE_SHT40        ║");
  Serial.println("║   BUILD 17/09/2026 :: BOOT SEQUENCE        ║");
  Serial.println("╚════════════════════════════════════════════╝");

  // I2C
  Wire.begin();

  // SHT40
  if (!sht4.begin()) {
    Serial.println("> SHT40 .................. [ FAIL ]");
  } else {
    sht4.setPrecision(SHT4X_HIGH_PRECISION);
    sht4.setHeater(SHT4X_NO_HEATER);
    Serial.println("> SHT40 .................. [ OK ]");
  }

  // Wi-Fi
  WiFi.config(local_IP, gateway, subnet, dns);
  WiFi.begin(ssid, password);

  Serial.print("> КАНАЛ СВЯЗИ Wi-Fi ");
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 30000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("> Wi-Fi .................. [ FAIL ]");
    Serial.println("> ПРОВЕРЬ SSID / ПАРОЛЬ / ДИАПАЗОН 2.4 ГГц");
    return;
  }

  Serial.println("> Wi-Fi .................. [ OK ]");
  Serial.print  ("> IP  .................... "); Serial.println(WiFi.localIP());
  Serial.print  ("> ШЛЮЗ ................... "); Serial.println(WiFi.gatewayIP());
  Serial.print  ("> RSSI ................... "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("> ВЕБ-СЕРВЕР .............. [ OK ]");
  Serial.print  ("> ОТКРОЙ: http://"); Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
}