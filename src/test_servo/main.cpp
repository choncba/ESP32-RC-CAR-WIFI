/*
   Test Servo SG90
*/

#include <Arduino.h>
#include <ESP32Servo.h>

#define SERVO_SIGNAL 1

Servo FrontServo;

// Published values for SG90 servos; adjust if needed
// int minUs = 1000;
// int maxUs = 2000;
int minUs = 500;
int maxUs = 2500;
int pos = 0;      // position in degrees

ESP32PWM pwm;

void setup() 
{
    ESP32PWM::allocateTimer(0);

    Serial.begin(115200);

    FrontServo.setPeriodHertz(50);
    FrontServo.attach(SERVO_SIGNAL,minUs, maxUs);
}

void loop() 
{ 
    // Serial.println("0 a 180ª...");
  	// for (pos = 0; pos <= 180; pos += 1) { // sweep from 0 degrees to 180 degrees
	// 	// in steps of 1 degree
	// 	FrontServo.write(pos);
	// 	delay(1);             // waits 20ms for the servo to reach the position
	// }
    // delay(1000);
    // Serial.println("180 a 0ª...");
	// for (pos = 180; pos >= 0; pos -= 1) { // sweep from 180 degrees to 0 degrees
	// 	FrontServo.write(pos);
	// 	delay(1);
	// }
    // delay(1000);

    Serial.println("Center");
	FrontServo.write(90);
    delay(500);
    Serial.println("Left");
	FrontServo.write(0);
    delay(500);
    Serial.println("Center");
	FrontServo.write(90);
    delay(500);
    Serial.println("Right");
	FrontServo.write(180);
    delay(500);
}