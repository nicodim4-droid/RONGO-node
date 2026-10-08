#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
//Библиотеки

#define LED_PIN   48
#define LED_COUNT 1
//выбор пина и количества светодиодов

Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
//создание объекта для управления светодиодами

void ledOn()  { led.setPixelColor(0, led.Color(0, 165, 255)); led.show(); } // оранжевый
void ledOff() { led.setPixelColor(0, 0);                    led.show(); }

void setup() {
    led.begin();
    led.setBrightness(80);
    ledOff();
    Serial.begin(115200);
    delay(1000);
    Serial.println("SOS Blink Test Started");
}

void dot() {
    ledOn();
    delay(200);
    ledOff();
    delay(200);
}

void dash() {
    ledOn();
    delay(600);
    ledOff();
    delay(200);
}

void loop() {
    dot(); dot(); dot();
    delay(400);
    dash(); dash(); dash();
    delay(400);
    dot(); dot(); dot();
    delay(2000);
}