
//********************************************************************************************
// Configuration
//********************************************************************************************

#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <VescUart.h>

// Constants
#define LED_PIN 2
#define CURRENT 15.0
#define SSID_NAME "JamesiPhone"
#define PASSWORD  "coolboy69"

// initiate uart class
VescUart UART;

// Web server globals
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
char msg_buf[15];

//Program flow globals
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;

unsigned long lastPingTime = 0;

//********************************************************************************************
// Helper functions
//********************************************************************************************
void panic() {
  digitalWrite(LED_PIN, LOW);
  UART.setCurrent(0);
  UART.setCurrent(0, 108);
}


//********************************************************************************************
// Callbacks
//********************************************************************************************

// Callback: receiving any WebSocket message
void onWsEvent(
  AsyncWebSocket * server,
  AsyncWebSocketClient * client,
  AwsEventType type, void * arg,
  uint8_t *data, size_t len) {

  if (type == WS_EVT_DATA) {
    String dataStr = ((String)(char*)data).substring(0, len);


    if (dataStr == "ping") {
      lastPingTime = millis();


    } else if (dataStr == "forwardOn") {
      digitalWrite(LED_PIN, HIGH);
      UART.setCurrent(CURRENT);
      UART.setCurrent(CURRENT, 108);
    } else if (dataStr == "forwardOff") {
      digitalWrite(LED_PIN, LOW);
      UART.setCurrent(0);
      UART.setCurrent(0, 108);
    } else if (dataStr == "backwardOn") {
      UART.setCurrent(-CURRENT);
      UART.setCurrent(-CURRENT, 108);
    } else if (dataStr == "backwardOff") {
      UART.setCurrent(0);
      UART.setCurrent(0, 108);
    } else if (dataStr == "leftOn") {
      UART.setCurrent(CURRENT, 108);
    } else if (dataStr == "leftOff") {
      UART.setCurrent(0);
      UART.setCurrent(0, 108);
    } else if (dataStr == "rightOn") {
      UART.setCurrent(CURRENT);
    } else if (dataStr == "rightOff") {
      UART.setCurrent(0);
      UART.setCurrent(0, 108);
    }
    
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("[WS] Client disconnected");
    panic();
  } else if (type == WS_EVT_ERROR) {
    panic();
  } else if (type == WS_EVT_CONNECT) {
    Serial.println("[WS] Client connected");
  }
}

// Callback: send homepage
void onIndexRequest(AsyncWebServerRequest * request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                 "] HTTP GET request of " + request->url());
  request->send(SPIFFS, "/index.html", "text/html");
}

// Callback: send style sheet
void onCSSRequest(AsyncWebServerRequest * request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                 "] HTTP GET request of " + request->url());
  request->send(SPIFFS, "/style.css", "text/css");
}

// Callback: send 404 if requested file does not exist
void onPageNotFound(AsyncWebServerRequest * request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                 "] HTTP GET request of " + request->url());
  request->send(404, "text/plain", "404 not found");
}

//********************************************************************************************
// Main program
//********************************************************************************************

void setup() {
  //Initialize outputs and set low
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(9600);
  Serial2.begin(115200);

  while (!Serial2) {;}
  
  UART.setSerialPort(&Serial2) ; 
  
  

  Serial.println("[SPIFFS] Attempting to mount SPIFFS");

  // Make sure we can read the file system
  if (!SPIFFS.begin(true)) {
    Serial.println("[SPIFFS] Error mounting SPIFFS");
    while (1);
  }

  // Attempt to connect to WiFi
  WiFi.mode(WIFI_AP);
  WiFi.softAP(SSID_NAME, PASSWORD);

  //Info
  Serial.print("[WIFI] Network name: ");
  Serial.println(WiFi.SSID());
  Serial.print("[WIFI] IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("[WIFI] RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println("dBm");

  // HTTP bindings
  server.on("/", HTTP_GET, onIndexRequest);
  server.on("/style.css", HTTP_GET, onCSSRequest);
  server.onNotFound(onPageNotFound);

  // Websocket bindings
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // Start webserver
  server.begin();

}

void loop() {
  if (millis() - lastPingTime > 400) {
    Serial.println("[INFO] No connections to server. Entering shutdown mode");
    panic();
    delay(500);
  }
}
