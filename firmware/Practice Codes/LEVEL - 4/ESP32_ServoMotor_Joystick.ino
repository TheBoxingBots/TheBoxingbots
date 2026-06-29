#include <Bluepad32.h>
#include <ESP32Servo.h>  // Use ESP32 Servo library

// Define the buzzer pin
const int buzzer = 15;

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

// Callback function for connected controller
void onConnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      myControllers[i] = ctl;
      break;
    }
  }
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(500);
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(500);
}

// Callback function for disconnected controller
void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      myControllers[i] = nullptr;
      stopAllServos();
      break;
    }
  }
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(500);
}

// Function to stop all servos
void stopAllServos() {
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  servo5.write(90);
}

// Process gamepad input to control servos
void processGamepad(ControllerPtr ctl) {
  // Read joystick axes
  int left_axis_x = ctl->axisX();
  int left_axis_y = ctl->axisY();
  int right_axis_x = ctl->axisRX();
  int right_axis_y = ctl->axisRY();

  // Map to servo angles
  int servo1_angle = map(left_axis_y, 511, -512, 0, 180);
  int servo2_angle = map(left_axis_x, 511, -512, 0, 180);
  int servo3_angle = map(right_axis_y, -511, 512, 0, 180);
  int servo4_angle = map(right_axis_x, 511, -512, 0, 180);

  // Control servos
  servo1.write(servo1_angle);
  servo2.write(servo2_angle);
  servo3.write(servo3_angle);
  servo4.write(servo4_angle);

  // Control Servo 5
  if (ctl->brake()) {
    servo5.write(180);
  } else if (ctl->throttle()) {
    servo5.write(0);
  } else {
    servo5.write(90);
  }
}

// Process all controllers
void processControllers() {
  for (auto myController : myControllers) {
    if (myController && myController->isConnected() && myController->hasData()) {
      if (myController->isGamepad()) {
        processGamepad(myController);
      }
    }
  }
}

// Setup
void setup() {
  Serial.begin(115200);

  pinMode(buzzer, OUTPUT);

  servo1.attach(servo1_pin);
  servo2.attach(servo2_pin);
  servo3.attach(servo3_pin);
  servo4.attach(servo4_pin);
  servo5.attach(servo5_pin);

  BP32.setup(&onConnectedController, &onDisconnectedController);
}

// Loop
void loop() {
  BP32.update();
  processControllers();
}
