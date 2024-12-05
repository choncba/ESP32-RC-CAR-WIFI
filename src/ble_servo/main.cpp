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
#define FRONT_SERVO   21
#define FRONT_LIGHTS  8 // Onboard LED
#define REAR_RELAY    1 // Rear motor Relay: 0 Forward - 1 Reverse
#define REAR_PWM      2 // Rear motor PWM: Speed Control
#define REAR_LIGHTS   9

#include <ESP32Servo.h>

Servo FrontServo;
// SG90 Servo parameters
int minUs = 500;
int maxUs = 2500;
// Car Parameters
#define DEFAULT_POS 90    // Center
#define MIN_POS     70    // Right Max measured on car's hardware
#define MAX_POS     120   // Left Max

ESP32PWM RearMotor;

unsigned long prev_time = 0;

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
  uint8_t front_lights = 0;
  uint8_t rear_lights = 0;
  uint8_t pos = 0;      // Front Servo position in degrees
  uint8_t rear_direction = 0; // 0 Forward - 1 Reverse
  uint8_t rear_speed = 0;
}current_command, prev_command;

uint8_t connected = 0; // Connection flag
 
void handleCommand(){
  
  current_command.pos = uint8_t(map(RemoteXY.joystick_x, -100, 100, MIN_POS, MAX_POS));
  if(current_command.pos != prev_command.pos){
    FrontServo.write(current_command.pos);
    prev_command.pos = current_command.pos;
    delay(1);
    Serial.printf("Front Servo Pos (ª): %d\n",current_command.pos);
  }

  current_command.rear_direction = (RemoteXY.joystick_y < -10)? 1 : 0;
  if(current_command.rear_direction != prev_command.rear_direction){
    Serial.printf("Direction: %s\n",(current_command.rear_direction == 0)? "Forward":"Reverse");
    digitalWrite(REAR_RELAY, current_command.rear_direction);
    prev_command.rear_direction = current_command.rear_direction;
  }

  current_command.rear_speed = uint8_t(map(abs(RemoteXY.joystick_y),0,100,0,254));
  if(current_command.rear_speed != prev_command.rear_speed){
    Serial.printf("Rear PWM: %d\n", current_command.rear_speed);
    RearMotor.write(current_command.rear_speed);
    prev_command.rear_speed = current_command.rear_speed;
  }

  current_command.front_lights = RemoteXY.lights;
  if(current_command.front_lights != prev_command.front_lights){
    digitalWrite(FRONT_LIGHTS, current_command.front_lights);
    prev_command.front_lights = current_command.front_lights;
    Serial.printf("Front Lights %s",(current_command.front_lights)?"ON":"OFF");
  }
}

// Set default car position
void SetDefault(){
  current_command.pos = DEFAULT_POS; // Center
  current_command.rear_direction = 0;
  current_command.rear_speed = 0;
  FrontServo.write(current_command.pos);
  digitalWrite(REAR_RELAY, current_command.rear_direction);
  RearMotor.write(current_command.rear_speed);
  digitalWrite(REAR_LIGHTS, LOW);
}

void setup() 
{
  ESP32PWM::allocateTimer(0);

  Serial.begin(115200);
  pinMode(FRONT_LIGHTS, OUTPUT);
  pinMode(REAR_LIGHTS, OUTPUT);
  pinMode(REAR_RELAY,OUTPUT);
  FrontServo.setPeriodHertz(50);
  FrontServo.attach(FRONT_SERVO,minUs, maxUs);
  RearMotor.attachPin(REAR_PWM, 50);
  
  RemoteXY_Init (); 

  SetDefault();

  prev_time = millis();
}

void loop() 
{ 
  RemoteXY_Handler ();
  
  if(RemoteXY.connect_flag){
    handleCommand();
  }
  else{
    if(millis() - prev_time > 100){
      digitalWrite(FRONT_LIGHTS,!digitalRead(FRONT_LIGHTS)); // Blink led until connect
      current_command.front_lights = digitalRead(FRONT_LIGHTS);
      prev_time = millis();
    }
  }

  // Sets default state when disconnected
  if(connected != RemoteXY.connect_flag){
    if(connected) SetDefault();
    connected = RemoteXY.connect_flag;
  }
  
}