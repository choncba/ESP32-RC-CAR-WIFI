 /*********
  ESP32-C3 websockets/web server to control a RC Car
  https://github.com/choncba/ESP32-RC-CAR-WIFI
  Luciano Bono
*********/

// Import required libraries
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include "ArduinoJson.h"
#include "secrets.h"

// Create the file ./src/secrets.h and define your credentials:
// #define WIFI_SSID   "YOUR_SSID"
// #define WIFI_PWD    "YOUR_PASSWORD"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PWD;
IPAddress ip = (192,168,0,184);
IPAddress gw = (192,168,0,1);
IPAddress mask = (255,255,255,0);

bool ledState = 0;
const int ledPin = 8;

// RC Car Pin Mappings
#define DRIVER_STATUS 0 // Pin Status del DRV8833
#define DRIVER_SLEEP  1 // Pin Sleep del DRV8833
#define BATTERY       4 // Entrada A/D del nivel de batería
#define FRONT_MOTOR_1 5
#define FRONT_MOTOR_2 6
#define SPEAKER       7 // Salida a speaker
#define FRONT_LIGHTS  8 // Onboard LED
#define REAR_MOTOR_1  9 //20
#define REAR_MOTOR_2  10//21

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Commands from web
struct commands
{
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool sound = false;
  bool light = false;
}current_command, prev_command;

// Status for web
struct Status{
  unsigned int battery_level = 0;
  int wifi_signal = 0;
}status;

// Received Joystick values
// int x = 0;
// int y = 0;
// int x_prev = x;
// int y_prev = y;

// PWM Setup
const int FRONT_MOTOR_1_CH = 0;
const int FRONT_MOTOR_2_CH = 1;
const int REAR_MOTOR_1_CH = 2;
const int REAR_MOTOR_2_CH = 3;
const int SPEAKER_CH = 4;
const int frequency = 500;
const int resolution = 8;

unsigned long prev_time = 0;
bool release = false;
unsigned long release_timer = 0;
#define RELEASE_TIME 10
bool ws_connected = false;

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
    Serial.println((char*)data);
    JsonDocument doc;
    if(deserializeJson(doc, data) == DeserializationError::Ok){
      current_command.up = doc["up"];
      current_command.down = doc["down"];
      current_command.left = doc["left"];
      current_command.right = doc["right"];
      current_command.light = doc["lights"];
      current_command.sound = doc["sound"];
    }
    else{
      Serial.println("Json Error");
    }
    // if (strcmp((char*)data, "toggle") == 0) {
    //   ledState = !ledState;
    //   notifyClients();
    // }
    // else{
    //   Serial.println((char*)data);
    //   JsonDocument doc;
    //   deserializeJson(doc, data);
    //   x = constrain(int(doc["x"]),-100,100);
    //   y = constrain(int(doc["y"]),-100,100);
    //   Serial.println(x);
    //   Serial.println(y);
    // }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      ws_connected = true;
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      ws_connected = false;
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

// String processor(const String& var){
//   Serial.println(var);
//   if(var == "STATE"){
//     if (!ledState){
//       return "ON";
//     }
//     else{
//       return "OFF";
//     }
//   }
//   return String();
// }

void handleCommand(){
  // Left
  if(current_command.left != prev_command.left){
    ledcWrite(FRONT_MOTOR_1_CH,(current_command.left)?254:0);
    ledcWrite(FRONT_MOTOR_2_CH,0);
    prev_command.left = current_command.left;
    if(current_command.left){
      Serial.println("LEFT");
      release = true;
      release_timer = millis();
    } 
  }
  // Right
  if(current_command.right != prev_command.right){
    ledcWrite(FRONT_MOTOR_1_CH,0);
    ledcWrite(FRONT_MOTOR_2_CH,(current_command.right)?254:0);
    prev_command.right = current_command.right;
    if(current_command.right){
      Serial.println("RIGHT");
      release = true;
      release_timer = millis();
    }
  }
  // Forward
  if(current_command.up != prev_command.up){
    ledcWrite(REAR_MOTOR_1_CH,(current_command.up)?254:0);
    ledcWrite(REAR_MOTOR_2_CH,0);
    prev_command.up = current_command.up;
  }
  // Reverse
  if(current_command.down != prev_command.down){
    ledcWrite(REAR_MOTOR_1_CH,0);
    ledcWrite(REAR_MOTOR_2_CH,(current_command.down)?254:0);
    prev_command.down = current_command.down;
  }
  // Light
  if(current_command.light != prev_command.light){
    digitalWrite(FRONT_LIGHTS,current_command.light);
    prev_command.light = current_command.light;
  }
  // Sound
  if(current_command.sound != prev_command.sound){
    ledcWrite(SPEAKER_CH,(current_command.sound)?100:0);
    prev_command.sound = current_command.sound;
  }

  if(release){
    if(millis() - release_timer > RELEASE_TIME){
      ledcWrite(FRONT_MOTOR_1_CH,(current_command.left)?100:0);
      ledcWrite(FRONT_MOTOR_2_CH,(current_command.right)?100:0);
      release = false;
      Serial.println("RELEASE");
    }
  }
}

// void handleJoystick(){

//   // Handle Front Motor
//   if(x_prev != x){
//     if(x==0){
//       ledcWrite(FRONT_MOTOR_1_CH,0);
//       ledcWrite(FRONT_MOTOR_2_CH,0);
//     }
//     else{
//       if(x<0){
//         ledcWrite(FRONT_MOTOR_1_CH,map(x,0,-100,0,150));  // LEFT
//         ledcWrite(FRONT_MOTOR_2_CH,0);
//       }
//       else{
//         ledcWrite(FRONT_MOTOR_1_CH,0);
//         ledcWrite(FRONT_MOTOR_2_CH,map(x,0,100,0,150)); // RIGHT
//       } 
//     }
//     x_prev = x;
//   }

  
//   // Handle Rear Motor
//   if(y_prev != y){
//     if(y==0){
//       ledcWrite(REAR_MOTOR_1_CH,0);
//       ledcWrite(REAR_MOTOR_2_CH,0);
//     }
//     else{
//       if(y<0){
//         ledcWrite(REAR_MOTOR_1_CH,map(y,0,-100,0,255));  // Front
//         ledcWrite(REAR_MOTOR_2_CH,0);
//       }
//       else{
//         ledcWrite(REAR_MOTOR_1_CH,0);
//         ledcWrite(REAR_MOTOR_2_CH,map(y,0,100,0,255)); // Rear
//       } 
//     }
//     y_prev = y;
//   }

// }

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  // pinMode(GPIO_NUM_0, INPUT_PULLDOWN);
  // pinMode(GPIO_NUM_1, INPUT_PULLDOWN);
  // pinMode(GPIO_NUM_2, INPUT_PULLDOWN);
  // pinMode(GPIO_NUM_3, INPUT_PULLDOWN);
  // pinMode(GPIO_NUM_4, INPUT_PULLDOWN);
  // pinMode(GPIO_NUM_5, INPUT_PULLDOWN);
  // pinMode(GPIO_NUM_6, INPUT_PULLDOWN);
  // pinMode(GPIO_NUM_7, INPUT_PULLDOWN);
  // pinMode(GPIO_NUM_8, OUTPUT);  // BuiltIn led
  // pinMode(GPIO_NUM_9, INPUT_PULLDOWN);
  // pinMode(GPIO_NUM_10, INPUT_PULLDOWN);
  // pinMode(GPIO_NUM_20, INPUT_PULLDOWN);
  // pinMode(GPIO_NUM_21, INPUT_PULLDOWN);

  pinMode(FRONT_LIGHTS, OUTPUT);
  digitalWrite(FRONT_LIGHTS, HIGH);

  // PWM Motor driver setup
  ledcSetup(FRONT_MOTOR_1_CH, frequency, resolution);
  ledcSetup(FRONT_MOTOR_2_CH, frequency, resolution);
  ledcSetup(REAR_MOTOR_1_CH, frequency, resolution);
  ledcSetup(REAR_MOTOR_2_CH, frequency, resolution);

  ledcAttachPin(FRONT_MOTOR_1, FRONT_MOTOR_1_CH);
  ledcAttachPin(FRONT_MOTOR_2, FRONT_MOTOR_2_CH);
  ledcAttachPin(REAR_MOTOR_1, REAR_MOTOR_1_CH);
  ledcAttachPin(REAR_MOTOR_2, REAR_MOTOR_2_CH);

  ledcWrite(FRONT_MOTOR_1_CH,0);
  ledcWrite(FRONT_MOTOR_2_CH,0);
  ledcWrite(REAR_MOTOR_1_CH,0);
  ledcWrite(REAR_MOTOR_2_CH,0);

  // PWM Speker Setup
  ledcSetup(SPEAKER_CH, 1000, resolution);
  ledcAttachPin(SPEAKER_CH, SPEAKER);
  ledcWrite(SPEAKER_CH,0);

  pinMode(BATTERY, INPUT);
  // pinMode(DRIVER_STATUS, INPUT);
  pinMode(DRIVER_SLEEP, OUTPUT);
  digitalWrite(DRIVER_SLEEP, LOW);  // Deshabilito el DRV8833

  initLittleFS();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(FRONT_LIGHTS,HIGH);
    delay(100);
    digitalWrite(FRONT_LIGHTS,LOW);
    delay(900);
    Serial.print("Connecting to WiFi, status: ");
    Serial.print(WiFi.status());
    Serial.print(", RSSI: ");
    Serial.println(WiFi.RSSI());
  }
  digitalWrite(FRONT_LIGHTS,HIGH);

  delay(3000);
  // Print ESP Local IP Address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    // request->send_P(200, "text/html", index_html, processor);
    // request->send(LittleFS, "/ws-server.html", "text/html", false, processor);
    request->send(LittleFS, "/index.html", "text/html", false);
  });

  server.serveStatic("/", LittleFS, "/");

  // Start server
  server.begin();

  digitalWrite(DRIVER_SLEEP, HIGH);  // Habilito el DRV8833

  prev_time = millis();
}



void loop() {
  ws.cleanupClients();
  // handleJoystick();
  // digitalWrite(FRONT_LIGHTS, !ledState);
  handleCommand();

  if(ws_connected){
    if(millis() - prev_time > 1000){
      map(status.battery_level, 0, analogRead(BATTERY), 0, 100);  // Lee el A/D de la bateria y mapea de 0 a 100 %
      status.wifi_signal = WiFi.RSSI();
      JsonDocument status_json;
      status_json["battery"] = status.battery_level;
      status_json["wifi"] = status.wifi_signal;
      char   buffer[200];
      size_t len = serializeJson(status_json, buffer);
      ws.textAll(buffer, len);
      prev_time = millis();
    }
  }

}
