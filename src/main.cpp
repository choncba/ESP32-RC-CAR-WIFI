/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-websocket-server-arduino/
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include "ArduinoJson.h"

// Replace with your network credentials
const char* ssid = "GuiFi";
const char* password = "paula1548";

bool ledState = 0;
const int ledPin = 8;

// RC Car Pin Mappings
#define FRONT_MOTOR_1 5
#define FRONT_MOTOR_2 6
#define FRONT_LIGHTS  7
#define ONBOARD_LED   8
#define REAR_MOTOR_1  9
#define REAR_MOTOR_2  10

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Received Joystick values
int x = 0;
int y = 0;
int x_prev = x;
int y_prev = y;

// PWM Setup
const int FRONT_MOTOR_1_CH = 0;
const int FRONT_MOTOR_2_CH = 1;
const int REAR_MOTOR_1_CH = 2;
const int REAR_MOTOR_2_CH = 3;
const int frequency = 5000;
const int resolution = 8;

// Initialize LittleFS  
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

// Read File from LittleFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

void notifyClients() {
  ws.textAll(String(ledState));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) {
      ledState = !ledState;
      notifyClients();
    }
    else{
      Serial.println((char*)data);
      JsonDocument doc;
      deserializeJson(doc, data);
      x = constrain(int(doc["x"]),-100,100);
      y = constrain(int(doc["y"]),-100,100);
      Serial.println(x);
      Serial.println(y);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (!ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}


void handleJoystick(){

  // Handle Front Motor
  if(x_prev != x){
    if(x==0){
      ledcWrite(FRONT_MOTOR_1_CH,0);
      ledcWrite(FRONT_MOTOR_2_CH,0);
    }
    else{
      if(x<0){
        ledcWrite(FRONT_MOTOR_1_CH,map(x,0,-100,0,255));  // LEFT
        ledcWrite(FRONT_MOTOR_2_CH,0);
      }
      else{
        ledcWrite(FRONT_MOTOR_1_CH,0);
        ledcWrite(FRONT_MOTOR_2_CH,map(x,0,100,0,255)); // RIGHT
      } 
    }
    x_prev = x;
  }

  
  // Handle Rear Motor
  if(y_prev != y){
    if(y==0){
      ledcWrite(REAR_MOTOR_1_CH,0);
      ledcWrite(REAR_MOTOR_2_CH,0);
    }
    else{
      if(y<0){
        ledcWrite(REAR_MOTOR_1_CH,map(y,0,-100,0,255));  // Front
        ledcWrite(REAR_MOTOR_2_CH,0);
      }
      else{
        ledcWrite(REAR_MOTOR_1_CH,0);
        ledcWrite(REAR_MOTOR_2_CH,map(y,0,100,0,255)); // Rear
      } 
    }
    y_prev = y;
  }

}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  initLittleFS();
  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  pinMode(FRONT_LIGHTS, OUTPUT);
  digitalWrite(FRONT_LIGHTS, LOW);

  ledcSetup(FRONT_MOTOR_1_CH, frequency, resolution);
  ledcSetup(FRONT_MOTOR_2_CH, frequency, resolution);
  ledcSetup(REAR_MOTOR_1_CH, frequency, resolution);
  ledcSetup(REAR_MOTOR_2_CH, frequency, resolution);

  ledcAttachPin(FRONT_MOTOR_1, FRONT_MOTOR_1_CH);
  ledcAttachPin(FRONT_MOTOR_2, FRONT_MOTOR_2_CH);
  ledcAttachPin(REAR_MOTOR_1, REAR_MOTOR_1_CH);
  ledcAttachPin(REAR_MOTOR_2, REAR_MOTOR_2_CH);
  // ledcAttachPin(ONBOARD_LED, REAR_MOTOR_1_CH);

  ledcWrite(FRONT_MOTOR_1_CH,0);
  ledcWrite(FRONT_MOTOR_2_CH,0);
  ledcWrite(REAR_MOTOR_1_CH,0);
  ledcWrite(REAR_MOTOR_2_CH,0);

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
    // request->send_P(200, "text/html", index_html, processor);
    request->send(LittleFS, "/ws-server.html", "text/html", false, processor);
  });

  server.serveStatic("/", LittleFS, "/");

  // Start server
  server.begin();
}

void loop() {
  ws.cleanupClients();
  handleJoystick();
  digitalWrite(ledPin, !ledState);
}
