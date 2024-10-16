#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"

// Create the file ./src/secrets.h and define your credentials:
// #define WIFI_SSID   "YOUR_SSID"
// #define WIFI_PWD    "YOUR_PASSWORD"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PWD;

bool ledState = 0;
const int ledPin = 8;

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(GPIO_NUM_0, INPUT_PULLDOWN);
  pinMode(GPIO_NUM_1, INPUT_PULLDOWN);
  pinMode(GPIO_NUM_2, INPUT_PULLDOWN);
  pinMode(GPIO_NUM_3, INPUT_PULLDOWN);
  pinMode(GPIO_NUM_4, INPUT_PULLDOWN);
  pinMode(GPIO_NUM_5, INPUT_PULLDOWN);
  pinMode(GPIO_NUM_6, INPUT_PULLDOWN);
  pinMode(GPIO_NUM_7, INPUT_PULLDOWN);
  pinMode(GPIO_NUM_8, OUTPUT);  // BuiltIn led
  pinMode(GPIO_NUM_9, INPUT_PULLDOWN);
  pinMode(GPIO_NUM_10, INPUT_PULLDOWN);
  pinMode(GPIO_NUM_20, INPUT_PULLDOWN);
  pinMode(GPIO_NUM_21, INPUT_PULLDOWN);
    
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ledPin,LOW);
    delay(100);
    digitalWrite(ledPin,HIGH);
    delay(900);
    Serial.print("Connecting to WiFi, status: ");
    Serial.print(WiFi.status());
    Serial.print(", RSSI: ");
    Serial.println(WiFi.RSSI());
  }
  digitalWrite(ledPin,HIGH);

  delay(3000);
  // Print ESP Local IP Address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
}

void loop() {
}
