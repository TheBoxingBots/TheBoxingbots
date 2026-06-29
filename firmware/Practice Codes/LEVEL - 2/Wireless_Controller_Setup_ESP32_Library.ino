#include <Bluepad32.h>

// Define LED pin
const int ledPin = 2;  // Change pin as needed

// Initialize the controller array
ControllerPtr myControllers[BP32_MAX_GAMEPADS] = { nullptr };

// Callback function for connected controller
void onConnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.printf("CALLBACK: Controller connected, index=%d\n", i);
      myControllers[i] = ctl;
      digitalWrite(ledPin, HIGH); // Turn LED on when connected
      break;
    }
  }
}

// Callback function for disconnected controller
void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
      myControllers[i] = nullptr;
      digitalWrite(ledPin, LOW); // Turn LED off when disconnected
      break;
    }
  }
}

// Process gamepad input
void processGamepad(ControllerPtr ctl) {
  // Joysticks
  int left_axis_x = ctl->axisX();
  int left_axis_y = ctl->axisY();
  int right_axis_x = ctl->axisRX();
  int right_axis_y = ctl->axisRY();

  if (left_axis_y < -200) Serial.println("You moved left joystick up");
  else if (left_axis_y > 200) Serial.println("You moved left joystick down");

  if (left_axis_x < -200) Serial.println("You moved left joystick left");
  else if (left_axis_x > 200) Serial.println("You moved left joystick right");

  if (right_axis_y < -200) Serial.println("You moved right joystick up");
  else if (right_axis_y > 200) Serial.println("You moved right joystick down");

  if (right_axis_x < -200) Serial.println("You moved right joystick left");
  else if (right_axis_x > 200) Serial.println("You moved right joystick right");

  // PS-style button presses
  if (ctl->x()) Serial.println("Square button pressed");
  if (ctl->a()) Serial.println("Cross button pressed");
  if (ctl->b()) Serial.println("Circle button pressed");
  if (ctl->y()) Serial.println("Triangle button pressed");

  // Shoulder buttons
  if (ctl->l1()) Serial.println("L1 button pressed");
  if (ctl->r1()) Serial.println("R1 button pressed");

  // Trigger & brake
  if (ctl->brake()) Serial.println("Brake button pressed");
  if (ctl->throttle()) Serial.println("Throttle button pressed");

  // D-Pad (arrow buttons)
  uint8_t dpad = ctl->dpad();
  if (dpad & DPAD_UP) Serial.println("D-Pad UP pressed");
  if (dpad & DPAD_DOWN) Serial.println("D-Pad DOWN pressed");
  if (dpad & DPAD_LEFT) Serial.println("D-Pad LEFT pressed");
  if (dpad & DPAD_RIGHT) Serial.println("D-Pad RIGHT pressed");
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

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // LED off initially

  BP32.setup(&onConnectedController, &onDisconnectedController);

  Serial.println("Setup completed. Waiting for controllers...");
}

// Main loop function
void loop() {
  BP32.update();
  processControllers();
}
