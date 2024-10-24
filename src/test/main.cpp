/*
  Test de Periféricos  
*/

#include <Arduino.h>

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

  prev_time = millis();

}

void loop() 
{ 


  if(millis() - prev_time > 1000){
    Serial.println(analogRead(BATTERY));
    Serial.println(analogReadMilliVolts(BATTERY));
    prev_time = millis();
  }

    // Serial.print("X: ");
    // Serial.print(RemoteXY.joystick_x);
    // Serial.print(" Y: ");
    // Serial.print(RemoteXY.joystick_y);
    // Serial.println("\r");
    // RemoteXY_delay(500);

  
}