#include <FastLed.h>
#include <Wifi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define LED_PIN 13
#define NUM_LEDS 5

const char* ssid = "Rogers";
const char* password = "Roger1326";

CRGB leds[NUM_LEDS];
CRGB colour = CRGB(0x80134F);

void setup() {
    Serial.begin(115200);
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(20);
}

void loop() {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = colour;
    }
    FastLED.show();
    Serial.println("LED colour " + String(colour.r) + "," + String(colour.g) + "," + String(colour.b));
    delay(1000);
}