#include <Bluepad32.h>
#include <ESP32Servo.h>  // Use ESP32 Servo library

// Define the enable pins for the motors
const int motor1_enable_pin = 13;
const int motor2_enable_pin = 25;
const int motor1_forward_pin = 12;
const int motor1_backward_pin = 14;
const int motor2_forward_pin = 27;
const int motor2_backward_pin = 26;
const int buzzer = 15;

// Define the motor control range
const int motor_range_max = 255;
const int motor_range_min = -255;

// Define the servo pins
const int servo1_pin = 2;
const int servo2_pin = 4;
const int servo3_pin = 5;
const int servo4_pin = 18;
const int servo5_pin = 19;

// Initialize the controller array
ControllerPtr myControllers[BP32_MAX_GAMEPADS] = { nullptr };

// Initialize the Servo objects
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;

bool controllerConnected = false;

void controlMotor(int forwardPin, int backwardPin, int speed) {
  if (speed > 0) {
    analogWrite(forwardPin, speed);
    analogWrite(backwardPin, 0);
  } else if (speed < 0) {
    analogWrite(forwardPin, 0);
    analogWrite(backwardPin, -speed);
  } else {
    analogWrite(forwardPin, 0);
    analogWrite(backwardPin, 0);
  }
}

void stopAllMotorsAndServos() {
  controlMotor(motor1_forward_pin, motor1_backward_pin, 0);
  controlMotor(motor2_forward_pin, motor2_backward_pin, 0);
  digitalWrite(motor1_enable_pin, LOW);
  digitalWrite(motor2_enable_pin, LOW);

  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  servo5.write(90);
}

void onConnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      myControllers[i] = ctl;
      controllerConnected = true;
      break;
    }
  }
  digitalWrite(buzzer, HIGH); delay(500);
  digitalWrite(buzzer, LOW); delay(500);
  digitalWrite(buzzer, HIGH); delay(500);
  digitalWrite(buzzer, LOW); delay(500);
}

void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      myControllers[i] = nullptr;
      controllerConnected = false;
      stopAllMotorsAndServos();
      break;
    }
  }
  digitalWrite(buzzer, HIGH); delay(500);
  digitalWrite(buzzer, LOW); delay(500);
}

void processGamepad(ControllerPtr ctl) {
  uint8_t dpadState = ctl->dpad();
  if (dpadState & DPAD_UP) {
    digitalWrite(motor1_enable_pin, HIGH);
    digitalWrite(motor2_enable_pin, HIGH);
    controlMotor(motor1_forward_pin, motor1_backward_pin, motor_range_max);
    controlMotor(motor2_forward_pin, motor2_backward_pin, motor_range_max);
  } else if (dpadState & DPAD_DOWN) {
    digitalWrite(motor1_enable_pin, HIGH);
    digitalWrite(motor2_enable_pin, HIGH);
    controlMotor(motor1_forward_pin, motor1_backward_pin, motor_range_min);
    controlMotor(motor2_forward_pin, motor2_backward_pin, motor_range_min);
  } else if (dpadState & DPAD_LEFT) {
    digitalWrite(motor1_enable_pin, HIGH);
    digitalWrite(motor2_enable_pin, HIGH);
    controlMotor(motor1_forward_pin, motor1_backward_pin, motor_range_min);
    controlMotor(motor2_forward_pin, motor2_backward_pin, motor_range_max);
  } else if (dpadState & DPAD_RIGHT) {
    digitalWrite(motor1_enable_pin, HIGH);
    digitalWrite(motor2_enable_pin, HIGH);
    controlMotor(motor1_forward_pin, motor1_backward_pin, motor_range_max);
    controlMotor(motor2_forward_pin, motor2_backward_pin, motor_range_min);
  } else {
    digitalWrite(motor1_enable_pin, LOW);
    digitalWrite(motor2_enable_pin, LOW);
    controlMotor(motor1_forward_pin, motor1_backward_pin, 0);
    controlMotor(motor2_forward_pin, motor2_backward_pin, 0);
  }

  int left_axis_x = ctl->axisX();
  int left_axis_y = ctl->axisY();
  int right_axis_x = ctl->axisRX();
  int right_axis_y = ctl->axisRY();

  int servo1_angle = map(left_axis_y, 511, -512, 0, 180);
  int servo2_angle = map(left_axis_x, 511, -512, 0, 180);
  int servo3_angle = map(right_axis_y, -511, 512, 0, 180);
  int servo4_angle = map(right_axis_x, 511, -512, 0, 180);

  servo1.write(servo1_angle);
  servo2.write(servo2_angle);
  servo3.write(servo3_angle);
  servo4.write(servo4_angle);

  if (ctl->brake()) {
    servo5.write(180);
  } else if (ctl->throttle()) {
    servo5.write(0);
  } else {
    servo5.write(90);
  }
}

void processControllers() {
  for (auto myController : myControllers) {
    if (myController && myController->isConnected() && myController->hasData()) {
      if (myController->isGamepad()) {
        processGamepad(myController);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(motor1_forward_pin, OUTPUT);
  pinMode(motor1_backward_pin, OUTPUT);
  pinMode(motor2_forward_pin, OUTPUT);
  pinMode(motor2_backward_pin, OUTPUT);
  pinMode(motor1_enable_pin, OUTPUT);
  pinMode(motor2_enable_pin, OUTPUT);
  pinMode(buzzer, OUTPUT);

  digitalWrite(motor1_enable_pin, LOW);
  digitalWrite(motor2_enable_pin, LOW);
  analogWrite(motor1_forward_pin, 0);
  analogWrite(motor1_backward_pin, 0);
  analogWrite(motor2_forward_pin, 0);
  analogWrite(motor2_backward_pin, 0);

  servo1.attach(servo1_pin);
  servo2.attach(servo2_pin);
  servo3.attach(servo3_pin);
  servo4.attach(servo4_pin);
  servo5.attach(servo5_pin);

  BP32.setup(&onConnectedController, &onDisconnectedController);
}

void loop() {
  BP32.update();
  processControllers();
}
