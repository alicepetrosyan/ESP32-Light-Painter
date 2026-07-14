#include <LittleFS.h>
#include <FastLed.h>
#include <Wifi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define LED_PIN 13
#define NUM_LEDS 5
#define NETWORK_SSID "SM-F741W3411"
#define NETWORK_PASSWORD "kkbbiw3qe6cxwwp"

const char* ssid = NETWORK_SSID;
const char* password = NETWORK_PASSWORD;

CRGB leds[NUM_LEDS];
int colour = 0x80134F;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void notifyClients() {
  char hexBuffer[9];
  snprintf(hexBuffer, sizeof(hexBuffer), "#%06X", (unsigned int)colour);
  ws.textAll(hexBuffer);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String msg = String((char*)data);
    if(msg.startsWith("#")) {
      colour = strtol(msg.substring(1).c_str(), NULL, 16);
      Serial.printf("Parsed New Color: 0x%06X\n", (unsigned int)colour);
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
        char hexBuffer[9];
        snprintf(hexBuffer, sizeof(hexBuffer), "#%06X", (unsigned int)colour);
        client->text(hexBuffer);
        break;
    case WS_EVT_DISCONNECT:
      snprintf(hexBuffer, sizeof(hexBuffer), "WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    default:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "COLOUR"){
    char hexBuffer[9];
    snprintf(hexBuffer, sizeof(hexBuffer), "#%06X", (unsigned int)colour);
    return String(hexBuffer);
  }
  return String();
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(20);
    
    // mount filesystem
    if(!LittleFS.begin(true)){
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }
    Serial.println("LittleFS mounted successfully!");
    if (LittleFS.exists("/index.html")) {
        Serial.println("FOUND index.html on the filesystem!");
    } else {
        Serial.println("ERROR: index.html does NOT exist on LittleFS. Did you upload the filesystem image?");
    }

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }

    // Print ESP Local IP Address
    Serial.println(WiFi.localIP());

    initWebSocket();

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/index.html", "text/html", false, processor);
    });

    // Start server
    server.begin();
}

void loop() {
    ws.cleanupClients();
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(colour);
    }
    FastLED.show();
    Serial.println("LED colour " + String(colour));
    delay(50);
}