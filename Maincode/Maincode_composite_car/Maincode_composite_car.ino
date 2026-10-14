/* Maincode for composite car use ESP32 */
/* Author:      Trần Lê Nhật Đoan       */
/* Researcher:  Phạm Hùng Khổng
                Nguyễn Quang Hy
                Hồ Đoàn Nhựt Thắng      */
/*--------------------------------------*/
#include <ESP32Servo.h>

// Define physicals for ESP32-S3
#define ENA  1       // Motor DC speed
#define IN1  4       
#define IN2  5       
#define SERVO_PIN 8  // Servo vector

#define RX_PIN 15
#define TX_PIN 16

Servo myservo;
char lenh;

// --- SMOOTH STATE MANAGEMENT VARIABLE---
int servoAngle = 90;          // Servo current Angle
int targetServoAngle = 90;    // Servo aimed Angle

int maxSpeed = 255;           // Max speed
int currentMotorSpeed = 0;    // Motor DC current speed
int targetMotorSpeed = 0;     // Motor DC aimed speed
int motorDirection = 0;       // 0: Stop, 1: Forward, -1: Back

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  myservo.attach(SERVO_PIN);
  myservo.write(servoAngle);
}

void loop() {
  // 1. Reading commands from Bluetooth without lag
  while (Serial1.available() > 0) {
    lenh = Serial1.read(); 
    
    if (lenh == '\n' || lenh == '\r' || lenh == ' ') {
      continue; 
    }

    // --- Update goals for the DC Motor ---
    if (lenh == 'w' || lenh == 'F') { 
      motorDirection = 1;         // Set direction to progress
      targetMotorSpeed = maxSpeed;// Set maxspeed
    } 
    else if (lenh == 's' || lenh == 'B') { 
      motorDirection = -1;        // Set direction to progress
      targetMotorSpeed = maxSpeed;// Set maxspeed
    } 
    else if (lenh == 'z' || lenh == 'x' || lenh == 'S') { 
      targetMotorSpeed = 0;       // Return 0 to stop
    } 

    // --- Update the target for the Servo ---
    if (lenh == 'a' || lenh == 'L') { 
      targetServoAngle = 45;      // Left turn target
    } 
    else if (lenh == 'd' || lenh == 'R') { 
      targetServoAngle = 135;     // Right turn target
    } 
    else if (lenh == 'c' || lenh == 'C' || lenh == 'g') { 
      targetServoAngle = 90;      // Return default target
    }
  }

  // 2. Logic for smoothing DC Motor speed (Increase/decrease evenly)
  if (currentMotorSpeed < targetMotorSpeed) {
    currentMotorSpeed += 5; // Increase speed by 5
    if (currentMotorSpeed > targetMotorSpeed) currentMotorSpeed = targetMotorSpeed;
  } 
  else if (currentMotorSpeed > targetMotorSpeed) {
    currentMotorSpeed -= 5; // Decrease speed by 5
    if (currentMotorSpeed < targetMotorSpeed) currentMotorSpeed = targetMotorSpeed;
  }

  // Output the actual speed signal to the L298N hardware
  if (motorDirection == 1) {
    analogWrite(ENA, currentMotorSpeed);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } 
  else if (motorDirection == -1) {
    analogWrite(ENA, currentMotorSpeed);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  } 
  else {
    analogWrite(ENA, 0);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }

  // 3. Logic to smooth Servo angles (Rotate slowly and steadily)
  if (servoAngle < targetServoAngle) {
    servoAngle++;
  } 
  else if (servoAngle > targetServoAngle) {
    servoAngle--;
  }
  myservo.write(servoAngle);

  // Small delay for system
  delay(3); 
}