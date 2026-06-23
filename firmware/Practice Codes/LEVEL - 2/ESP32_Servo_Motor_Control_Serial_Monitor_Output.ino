#include <Bluepad32.h>
#include <ESP32Servo.h>  // Use ESP32 Servo library

// Define the buzzer pin
const int buzzer = 15;

// Define the servo pins
const int servo1_pin = 2;   // Servo 1 pin (left joystick up and down)
const int servo2_pin = 4;   // Servo 2 pin (left joystick left and right)
const int servo3_pin = 5;   // Servo 3 pin (right joystick up and down)
const int servo4_pin = 18;  // Servo 4 pin (right joystick left and right)
const int servo5_pin = 19;  // Servo 5 pin (throttle and brake buttons)

// Initialize the controller array
ControllerPtr myControllers[BP32_MAX_GAMEPADS] = { nullptr };

// Initialize the Servo objects
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;  // Servo 5 object for throttle and brake buttons

// Callback function for connected controller
void onConnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.printf("CALLBACK: Controller connected, index=%d\n", i);
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
      Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
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
  Serial.println("All servos stopped.");
}

// Process gamepad input to control servos
void processGamepad(ControllerPtr ctl) {
  // Read the left joystick axes
  int left_axis_x = ctl->axisX();
  int left_axis_y = ctl->axisY();

  // Read the right joystick axes
  int right_axis_x = ctl->axisRX();
  int right_axis_y = ctl->axisRY();

  // Map the joystick values to servo angle ranges
  int servo1_angle = map(left_axis_y, 511, -512, 0, 180);
  int servo2_angle = map(left_axis_x, 511, -512, 0, 180);
  int servo3_angle = map(right_axis_y, -511, 512, 0, 180);
  int servo4_angle = map(right_axis_x, 511, -512, 0, 180);

  // Control servos based on joystick inputs
  servo1.write(servo1_angle);
  servo2.write(servo2_angle);
  servo3.write(servo3_angle);
  servo4.write(servo4_angle);

  // Print movement directions
  if (left_axis_y < -200) Serial.println("You moved left joystick up");
  else if (left_axis_y > 200) Serial.println("You moved left joystick down");

  if (left_axis_x < -200) Serial.println("You moved left joystick left");
  else if (left_axis_x > 200) Serial.println("You moved left joystick right");

  if (right_axis_y < -200) Serial.println("You moved right joystick up");
  else if (right_axis_y > 200) Serial.println("You moved right joystick down");

  if (right_axis_x < -200) Serial.println("You moved right joystick left");
  else if (right_axis_x > 200) Serial.println("You moved right joystick right");

  // Control Servo 5 using throttle and brake buttons
  if (ctl->brake()) {
    servo5.write(180);
    Serial.println("Brake button pressed: Servo5 set to 180 degrees.");
  } else if (ctl->throttle()) {
    servo5.write(0);
    Serial.println("Throttle button pressed: Servo5 set to 0 degrees.");
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
      } else {
        Serial.println("Controller type not supported");
      }
    }
  }
}

// Setup function
void setup() {
  Serial.begin(115200);
  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());

  pinMode(buzzer, OUTPUT);

  // Attach the servo objects to their respective pins
  servo1.attach(servo1_pin);
  servo2.attach(servo2_pin);
  servo3.attach(servo3_pin);
  servo4.attach(servo4_pin);
  servo5.attach(servo5_pin);

  // Initialize Bluepad32 and set callback functions
  BP32.setup(&onConnectedController, &onDisconnectedController);

  Serial.println("Setup completed. Waiting for controllers...");
}

// Main loop function
void loop() {
  BP32.update();
  processControllers();
}
