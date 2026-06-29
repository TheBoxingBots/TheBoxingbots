#include <Bluepad32.h>

// Define motor direction and enable pins
const int motor1Pin1 = 12;  // Motor 1 direction pin 1
const int motor1Pin2 = 14;  // Motor 1 direction pin 2
const int motor2Pin1 = 27;  // Motor 2 direction pin 1
const int motor2Pin2 = 26;  // Motor 2 direction pin 2
const int ena = 13;         // Motor 1 enable pin
const int enb = 25;         // Motor 2 enable pin

GamepadPtr myGamepad;
Bluepad32 bp32; // Declare Bluepad32 as an object

void onConnect(GamepadPtr gp) {
  myGamepad = gp;
}

void onDisconnect(GamepadPtr gp) {
  myGamepad = nullptr;
}

void setup() {
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(ena, OUTPUT);
  pinMode(enb, OUTPUT);

  bp32.setup(&onConnect, &onDisconnect);
}

void loop() {
  bp32.update();

  if (myGamepad && myGamepad->isConnected()) {
    int dpadState = myGamepad->dpad();

    if (dpadState & DPAD_UP) {
      moveForward();
    } else if (dpadState & DPAD_DOWN) {
      moveBackward();
    } else if (dpadState & DPAD_LEFT) {
      turnLeft();
    } else if (dpadState & DPAD_RIGHT) {
      turnRight();
    } else {
      stopMotors();
    }
  } else {
    stopMotors();
  }
}

void moveForward() {
  analogWrite(ena, 255);
  analogWrite(enb, 255);
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
}

void moveBackward() {
  analogWrite(ena, 255);
  analogWrite(enb, 255);
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
}

void turnLeft() {
  analogWrite(ena, 255);
  analogWrite(enb, 255);
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
}

void turnRight() {
  analogWrite(ena, 255);
  analogWrite(enb, 255);
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
}

void stopMotors() {
  analogWrite(ena, 0);
  analogWrite(enb, 0);
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
}
