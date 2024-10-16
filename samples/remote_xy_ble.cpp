/*
   -- Autito Bluetooth --
  
*/

#include <Arduino.h>

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG    

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP32CORE_BLE

#include <BLEDevice.h>

// RemoteXY connection settings 
#define REMOTEXY_BLUETOOTH_NAME "BT-CAR-ESP32"

#include <RemoteXY.h>

// RC Car Pin Mappings
#define DRIVER_STATUS 0 // DRV8833 Status pin
#define DRIVER_SLEEP  1 // DRV8833 Sleep command pin
#define BATTERY       4 // A/D Input for battery level
#define FRONT_MOTOR_1 5
#define FRONT_MOTOR_2 6
#define SPEAKER       7 // Speaker? output
#define FRONT_LIGHTS  8 // Onboard LED
#define REAR_MOTOR_1  9 //20
#define REAR_MOTOR_2  10//21

// PWM Setup
const int FRONT_MOTOR_1_CH = 0;
const int FRONT_MOTOR_2_CH = 1;
const int REAR_MOTOR_1_CH = 2;
const int REAR_MOTOR_2_CH = 3;
const int frequency = 500;
const int resolution = 8;

unsigned long prev_time = 0;
bool release = false;
unsigned long release_timer = 0;
#define RELEASE_TIME 10

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 125 bytes
  { 255,3,0,2,0,118,0,19,0,0,0,65,117,116,105,116,111,32,87,105,
  70,105,0,31,1,106,200,1,1,5,0,5,9,92,81,81,37,135,26,16,
  73,13,23,19,4,13,128,0,36,26,0,0,0,0,0,0,200,66,0,0,
  0,0,129,8,5,87,12,64,6,65,117,116,105,116,111,32,66,108,117,101,
  116,111,111,116,104,0,73,67,23,19,4,13,128,0,135,26,0,0,0,0,
  0,0,200,66,0,0,0,0,2,32,39,39,17,0,2,26,31,31,79,78,
  0,79,70,70,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  int8_t joystick_x; // from -100 to 100
  int8_t joystick_y; // from -100 to 100
  uint8_t lights; // =1 if switch ON and =0 if OFF

    // output variables
  int8_t battery; // from 0 to 100
  int8_t signal; // from 0 to 100

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   

#pragma pack(pop)

struct commands
{
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool light = false;
}current_command, prev_command;
 
void handleCommand(){
  current_command.left = (RemoteXY.joystick_x < -20)? true : false;
  current_command.right = (RemoteXY.joystick_x > 20)? true : false;
  current_command.up = (RemoteXY.joystick_y > 10)? true : false;
  current_command.down = (RemoteXY.joystick_y < -10)? true : false;
  current_command.light = RemoteXY.lights;

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
    else Serial.println("CENTER");
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
    else Serial.println("CENTER");
  }

  // Decrease PWM signal to front motor after RELEASE_TIME to reduce power consumption and ESP reset due to battery leakage 
  if(release){
    if(millis() - release_timer > RELEASE_TIME){
      Serial.println("RELEASE");
      ledcWrite(FRONT_MOTOR_1_CH,(current_command.left)?100:0);
      ledcWrite(FRONT_MOTOR_2_CH,(current_command.right)?100:0);
      release = false;
    }
  }

  // Forward
  if(current_command.up != prev_command.up){
    Serial.println(current_command.up?"FORWARD":"STOP");
    ledcWrite(REAR_MOTOR_1_CH,(current_command.up)?254:0);
    ledcWrite(REAR_MOTOR_2_CH,0);
    prev_command.up = current_command.up;
  }
  // Reverse
  if(current_command.down != prev_command.down){
    Serial.println(current_command.down?"REVERSE":"STOP");
    ledcWrite(REAR_MOTOR_1_CH,0);
    ledcWrite(REAR_MOTOR_2_CH,(current_command.down)?254:0);
    prev_command.down = current_command.down;
  }

  // Light
  if(current_command.light != prev_command.light){
    Serial.print("LIGHTS ");
    Serial.println(current_command.light?"ON":"OFF");
    digitalWrite(FRONT_LIGHTS,current_command.light);
    prev_command.light = current_command.light;
  }
}

void setup() 
{
  Serial.begin(115200);
  
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

  pinMode(BATTERY, INPUT);
  pinMode(DRIVER_SLEEP, OUTPUT);
  digitalWrite(DRIVER_SLEEP, LOW);  // DRV8833 Disabled
  
  RemoteXY_Init (); 

  prev_time = millis();

}

void loop() 
{ 
  RemoteXY_Handler ();
  
  if(RemoteXY.connect_flag){

    digitalWrite(DRIVER_SLEEP, HIGH);  // DRV8833 enabled

    handleCommand();

    if(millis() - prev_time > 1000){
      map(RemoteXY.battery, 0, analogRead(BATTERY), 0, 100);  // Read Battery ADC pin and map to 0 - 100 %
      RemoteXY.signal = esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_DEFAULT); // Read BLE TX Power
      prev_time = millis();
    }

    // Serial.print("X: ");
    // Serial.print(RemoteXY.joystick_x);
    // Serial.print(" Y: ");
    // Serial.print(RemoteXY.joystick_y);
    // Serial.println("\r");
    // RemoteXY_delay(500);
  }
  else{
    digitalWrite(DRIVER_SLEEP, LOW);  // DRV8833 Disabled
    if(millis() - prev_time > 100){
      digitalWrite(FRONT_LIGHTS,!digitalRead(FRONT_LIGHTS)); // Blink lights until connect
      prev_time = millis();
    }
  }
  
}