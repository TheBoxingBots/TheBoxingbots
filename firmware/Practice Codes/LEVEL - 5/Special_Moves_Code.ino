#include <Bluepad32.h>
#include <ESP32Servo.h>

// Define motor pins
const int motor1_enable_pin = 13;
const int motor2_enable_pin = 25;
const int motor1_forward_pin = 12;
const int motor1_backward_pin = 14;
const int motor2_forward_pin = 27;
const int motor2_backward_pin = 26;

// Define servo pins
const int servo1_pin = 2;
const int servo2_pin = 4;
const int servo3_pin = 5;
const int servo4_pin = 18;
const int servo5_pin = 19;

const int led1 = 21;  // First LED
const int led2 = 22;  // Second LED

// Define buzzer pin
const int buzzer = 15;

// Initialize the controller array
ControllerPtr myControllers[BP32_MAX_GAMEPADS] = { nullptr };

// Initialize the Servo objects
Servo servo1, servo2, servo3, servo4, servo5;

// Flag to track whether a controller is connected
bool controllerConnected = false;

// Function to control motors
void controlMotor(int forwardPin, int backwardPin, int speed) {
    Serial.printf("Controlling motor: forwardPin=%d, backwardPin=%d, speed=%d\n", forwardPin, backwardPin, speed);

    if (speed > 0) {
        // Move the motor forward
        analogWrite(forwardPin, speed);
        analogWrite(backwardPin, 0);
        Serial.println("Motor moving forward.");
    } else if (speed < 0) {
        // Move the motor backward
        analogWrite(forwardPin, 0);
        analogWrite(backwardPin, -speed);
        Serial.println("Motor moving backward.");
    } else {
        // Stop the motor
        analogWrite(forwardPin, 0);
        analogWrite(backwardPin, 0);
        Serial.println("Motor stopped.");
    }
}

// Function to stop all motors and servos
void stopAllMotorsAndServos() {
    // Stop all motors
    controlMotor(motor1_forward_pin, motor1_backward_pin, 0);
    controlMotor(motor2_forward_pin, motor2_backward_pin, 0);
    digitalWrite(motor1_enable_pin, LOW);
    digitalWrite(motor2_enable_pin, LOW);
    Serial.println("All motors stopped.");

    // Stop all servos
    servo1.write(90);  // Neutral position
    servo2.write(90);  // Neutral position
    servo3.write(90);  // Neutral position
    servo4.write(90);  // Neutral position
    servo5.write(90);  // Neutral position
    Serial.println("All servos stopped.");
}

// Function to move the bot forward by 8 cm and set servo angles simultaneously
void moveBotAndSetServos() {
    // Define the servo target angles
    int targetServo1Angle = 170; // hand left
    int targetServo2Angle = 60;  // shoulder
    int targetServo3Angle = 10;  // right hand
    int targetServo4Angle = 120; // right shoulder

    // Define the initial servo angles
    int currentServo1Angle = servo1.read();
    int currentServo2Angle = servo2.read();
    int currentServo3Angle = servo3.read();
    int currentServo4Angle = servo4.read();

    // Define the movement duration and step size
    const int moveDuration = 400; // Adjust this duration to move 8 cm (depends on your bot's speed)
    const int stepDuration = 20;  // Time for each step in milliseconds
    const int steps = moveDuration / stepDuration;  // Number of steps for the movement

    // Calculate the angle increment for each step
    float servo1Increment = (targetServo1Angle - currentServo1Angle) / (float)steps;
    float servo2Increment = (targetServo2Angle - currentServo2Angle) / (float)steps;
    float servo3Increment = (targetServo3Angle - currentServo3Angle) / (float)steps;
    float servo4Increment = (targetServo4Angle - currentServo4Angle) / (float)steps;

    // Enable the motors and set speed
    const int motorSpeed = 255; // Set speed
    digitalWrite(motor1_enable_pin, HIGH);
    digitalWrite(motor2_enable_pin, HIGH);
    controlMotor(motor1_forward_pin, motor1_backward_pin, motorSpeed);
    controlMotor(motor2_forward_pin, motor2_backward_pin, motorSpeed);

    // Move the servos and motors simultaneously
    for (int i = 0; i < steps; i++) {
        // Increment servo angles
        currentServo1Angle += servo1Increment;
        currentServo2Angle += servo2Increment;
        currentServo3Angle += servo3Increment;
        currentServo4Angle += servo4Increment;

        // Write new servo angles
        servo1.write(currentServo1Angle);
        servo2.write(currentServo2Angle);
        servo3.write(currentServo3Angle);
        servo4.write(currentServo4Angle);

        // Delay for the step duration
        delay(stepDuration);
    }

    // Stop the motors after moving forward
    controlMotor(motor1_forward_pin, motor1_backward_pin, 0);
    controlMotor(motor2_forward_pin, motor2_backward_pin, 0);
    digitalWrite(motor1_enable_pin, LOW);
    digitalWrite(motor2_enable_pin, LOW);
}

// Callback function for connected controller
void onConnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller connected, index=%d\n", i);
            myControllers[i] = ctl;
            controllerConnected = true;
            printBatteryStatus(ctl);
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
            controllerConnected = false;
            stopAllMotorsAndServos();
            break;
        }
    }
}

// Function to print battery status of a controller
void printBatteryStatus(ControllerPtr ctl) {
    int batteryLevel = ctl->battery();

    if (batteryLevel >= 0) {
        Serial.printf("Controller index=%d battery level: %d%%\n", ctl->index(), batteryLevel);
    } else {
        Serial.printf("Controller index=%d battery level: Not available\n", ctl->index());
    }
}

// Function to perform the BUTTON_X action
void performButtonXAction() {
    // Move forward by 8 cm (assume 2 cm per unit speed)
    digitalWrite(motor1_enable_pin, HIGH);
    digitalWrite(motor2_enable_pin, HIGH);
    controlMotor(motor1_forward_pin, motor1_backward_pin, 255 / 8);
    controlMotor(motor2_forward_pin, motor2_backward_pin, 255 / 8);
    delay(400); // Adjust the delay to achieve the correct 8 cm movement
    controlMotor(motor1_forward_pin, motor1_backward_pin, 0);
    controlMotor(motor2_forward_pin, motor2_backward_pin, 0);
    digitalWrite(motor1_enable_pin, LOW);
    digitalWrite(motor2_enable_pin, LOW);

    // Set servo angles
    servo1.write(170);  // left hand at 45 degrees
    servo2.write(40);   // right shoulder at 45 degrees
    servo3.write(10);   // left hand at 135 degrees
    servo4.write(140);  // right shoulder at 135 degrees
    delay(1500);
}

// Function to perform the BUTTON_Y action
void performButtonYAction() {
    // Move bot forward by 8 cm and set servo angles simultaneously
    moveBotAndSetServos();
}

// Function to perform the BUTTON_A(circle) action
void performButtonAAction() {
    // Set servo angles for a Mortal Kombat position
    servo1.write(90);  // Right hand at 60 degrees
    servo2.write(90); // Left hand at 120 degrees
    servo3.write(90); // Right shoulder at 180 degrees
    servo4.write(90);  // Left shoulder at 90 degrees
    servo5.write(45);  // Move hip 45 degrees left
    servo2.write(40); // Right hand at 60 degrees
    servo1.write(180); // Left hand at 120 degrees
    delay(400);
    servo1.write(90);  // Right hand at 60 degrees
    servo2.write(90); // Left hand at 120 degrees
    servo3.write(90); // Right shoulder at 180 degrees
    servo4.write(90);  // Left shoulder at 90 degrees
    servo5.write(90);  // Move hip 45 degrees left
    delay(100);
    servo5.write(135);
    servo3.write(0); // Right shoulder at 180 degrees
    servo4.write(130);
    delay(400);
    servo1.write(90);  // Right hand at 60 degrees
    servo2.write(90); // Left hand at 120 degrees
    servo3.write(90); // Right shoulder at 180 degrees
    servo4.write(90);  // Left shoulder at 90 degrees
    servo5.write(90);
    delay(100);
    servo5.write(45);
    servo2.write(40); // Right hand at 60 degrees
    servo1.write(180); // Left hand at 120 degrees
    delay(400);
    servo1.write(90);  // Right hand at 60 degrees
    servo2.write(90); // Left hand at 120 degrees
    servo3.write(90); // Right shoulder at 180 degrees
    servo4.write(90);  // Left shoulder at 90 degrees
    servo5.write(90);  // Move hip 45 degrees left
    delay(100);
    servo5.write(135);
    servo3.write(0); // Right shoulder at 180 degrees
    servo4.write(130);
    delay(400);
}

// Function to perform the BUTTON_B(square) action
void performButtonBAction() {
    // john cena
    servo1.write(90);  // Right hand at 60 degrees
    servo2.write(90); // Left hand at 120 degrees
    servo3.write(90); // Right shoulder at 180 degrees
    servo4.write(90);  // Left shoulder at 90 degrees
    servo5.write(45);  // Move hip 45 degrees left
    servo2.write(40);
    delay(800);  // Right hand at 60 degrees
    servo1.write(180); // Left hand at 120 degrees
    delay(400);
    servo1.write(140); // Right hand at 60 degrees
    delay(400);
    servo1.write(180); // Left hand at 120 degrees
    delay(400);
    servo1.write(140); // Right hand at 60 degrees
    delay(400);
}

///////////////////////////////////////////////////////
void performL1Action() {
    Serial.println("L1 button pressed: Performing John Cena move with forward motion.");

    // Move forward
    digitalWrite(motor1_enable_pin, HIGH);
    digitalWrite(motor2_enable_pin, HIGH);
    controlMotor(motor1_forward_pin, motor1_backward_pin, 200);  // Normal forward speed
    controlMotor(motor2_forward_pin, motor2_backward_pin, 200);

    delay(1500);

        // Stop movement after performing action
    controlMotor(motor1_forward_pin, motor1_backward_pin, 0);
    controlMotor(motor2_forward_pin, motor2_backward_pin, 0);
    digitalWrite(motor1_enable_pin, LOW);
    digitalWrite(motor2_enable_pin, LOW);

    // First sequence: Execute the John Cena move
    servo1.write(140);  // Right hand at 60 degrees - left hand
    servo2.write(90);   // Left hand at 120 degrees - left shoulder
    servo3.write(90);   // Right shoulder at 180 degrees - right hand
    servo4.write(90);   // Left shoulder at 90 degrees - right shoulder
    servo5.write(45);   // Move hip 45 degrees left
    servo2.write(90);
    delay(400);         // Wait for movement

    // Command servos to neutral
    servo1.write(90);
    servo2.write(90);
    servo3.write(90);
    servo4.write(90);
    servo5.write(90);
    
    // Allow time for servos to reach neutral position
    delay(500);         

    // Execute additional actions once neutral is reached
    actionAfterNeutral();

    // Second sequence: Repeat the move (or perform another action)
    servo1.write(180);  // Right hand at 60 degrees - left hand
    servo2.write(90);   // Left hand at 120 degrees - left shoulder
    servo3.write(180);   // Right shoulder at 180 degrees - right hand
    servo4.write(90);   // Left shoulder at 90 degrees - right shoulder
    servo5.write(45);   // Move hip 45 degrees left
    servo2.write(90);
    delay(600);         // Wait for movement



    Serial.println("L1 Action completed.");
}

// Function to perform walking motion with charge action
void performR1Action() {
    Serial.println("R1 button pressed: Performing walking and charge action.");
    
    // Move forward (walking simulation)
    digitalWrite(motor1_enable_pin, HIGH);
    digitalWrite(motor2_enable_pin, HIGH);
    controlMotor(motor1_forward_pin, motor1_backward_pin, 100);
    controlMotor(motor2_forward_pin, motor2_backward_pin, 100);
    
    // Simulate walking with alternating arm movements
    for (int i = 0; i < 3; i++) {  // Repeat motion 4 times
        servo1.write(180);   // Left hand forward
        servo3.write(180);   // Right hand backward
        delay(500);          // Small delay before switching
        
        servo1.write(0);    // Left hand backward
        servo3.write(0);    // Right hand forward
        delay(500);          // Hold position
    }
    
        // Stop movement
    controlMotor(motor1_forward_pin, motor1_backward_pin, 0);
    controlMotor(motor2_forward_pin, motor2_backward_pin, 0);
    digitalWrite(motor1_enable_pin, LOW);
    digitalWrite(motor2_enable_pin, LOW);

    // Charge action (Final Move)
    servo1.write(170);  // Left hand at 45 degrees
    servo2.write(40);   // Right shoulder at 45 degrees
    servo3.write(10);   // Left hand at 135 degrees
    servo4.write(130);  // Right shoulder at 135 degrees
    delay(1000);
    


    Serial.println("Charge action completed.");
}



void actionAfterNeutral() {
    // Place your additional code here.
    // For example, you might move another servo or trigger a sensor reading.
}



// Process gamepad input to control motors and servos
void processGamepad(ControllerPtr ctl) {
    // Check if BUTTON_X is pressed
    if (ctl->a()) {
        Serial.println("BUTTON_X pressed: Performing action.");
        performButtonXAction();
        return;
    }

    // ....................................
        if (ctl->l1()) {
        Serial.println("L1 button pressed: Performing John Cena move.");
        performL1Action();
        return;
    }

    // Check if BUTTON_B is pressed (Perform John Cena Move)
    // if (ctl->x()) {
    //     Serial.println("BUTTON_B pressed: Performing John Cena move.");
    //     performButtonBAction();
    //     return;
    // }

    // Check if BUTTON_Y is pressed
    if (ctl->y()) {
        Serial.println("BUTTON_Y pressed: Performing action.");
        performButtonYAction();
        return;
    }

    // Check if BUTTON_A is pressed
    if (ctl->b()) {
        Serial.println("BUTTON_A pressed: Performing action.");
        performButtonAAction();
        return;
    }

    // Check if BUTTON_B is pressed
    if (ctl->x()) {
        Serial.println("BUTTON_B pressed: Performing action.");
        performButtonBAction();
        return;
    }
    if (ctl->r1()) {
        performR1Action();
        return;
    }

    // Check the state of the D-pad buttons
    uint8_t dpadState = ctl->dpad();

    // Handle D-pad input for movement control
    if (dpadState & DPAD_UP) {
        digitalWrite(motor1_enable_pin, HIGH);
        digitalWrite(motor2_enable_pin, HIGH);
        Serial.println("DPAD UP pressed: Moving motors forward.");
        controlMotor(motor1_forward_pin, motor1_backward_pin, 255);
        controlMotor(motor2_forward_pin, motor2_backward_pin, 255);
    } else if (dpadState & DPAD_DOWN) {
        digitalWrite(motor1_enable_pin, HIGH);
        digitalWrite(motor2_enable_pin, HIGH);
        Serial.println("DPAD DOWN pressed: Moving motors backward.");
        controlMotor(motor1_forward_pin, motor1_backward_pin, -255);
        controlMotor(motor2_forward_pin, motor2_backward_pin, -255);
    } else if (dpadState & DPAD_LEFT) {
        digitalWrite(motor1_enable_pin, HIGH);
        digitalWrite(motor2_enable_pin, HIGH);
        Serial.println("DPAD LEFT pressed: Turning left.");
        controlMotor(motor1_forward_pin, motor1_backward_pin, -255);
        controlMotor(motor2_forward_pin, motor2_backward_pin, 255);
    } else if (dpadState & DPAD_RIGHT) {
        digitalWrite(motor1_enable_pin, HIGH);
        digitalWrite(motor2_enable_pin, HIGH);
        Serial.println("DPAD RIGHT pressed: Turning right.");
        controlMotor(motor1_forward_pin, motor1_backward_pin, 255);
        controlMotor(motor2_forward_pin, motor2_backward_pin, -255);
    } else {
        digitalWrite(motor1_enable_pin, LOW);
        digitalWrite(motor2_enable_pin, LOW);
        Serial.println("No DPAD button pressed: Stopping both motors.");
        controlMotor(motor1_forward_pin, motor1_backward_pin, 0);
        controlMotor(motor2_forward_pin, motor2_backward_pin, 0);
    }

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

    // Control Servo 5 using throttle and brake buttons
    if (ctl->brake()) {
        servo5.write(180);
        Serial.println("Brake button pressed: Servo5 set to 0 degrees.");
    } else if (ctl->throttle()) {
        servo5.write(0);
        Serial.println("Throttle button pressed: Servo5 set to 180 degrees.");
    } else {
        servo5.write(90);
    }

    // Print battery status
    printBatteryStatus(ctl);
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
// Setup function
void setup() {
    Serial.begin(115200);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());

    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);

    digitalWrite(led1, HIGH);  // Turn LED1 ON
    digitalWrite(led2, HIGH);  // Turn LED2 ON

    // Set up the motor control pins as outputs
    pinMode(motor1_forward_pin, OUTPUT);
    pinMode(motor1_backward_pin, OUTPUT);
    pinMode(motor2_forward_pin, OUTPUT);
    pinMode(motor2_backward_pin, OUTPUT);
    pinMode(motor1_enable_pin, OUTPUT);
    pinMode(motor2_enable_pin, OUTPUT);
    pinMode(buzzer, OUTPUT);

    // Ensure motors are stopped initially
    digitalWrite(motor1_enable_pin, LOW);
    digitalWrite(motor2_enable_pin, LOW);
    analogWrite(motor1_forward_pin, 0);
    analogWrite(motor1_backward_pin, 0);
    analogWrite(motor2_forward_pin, 0);
    analogWrite(motor2_backward_pin, 0);

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
    // Update the gamepad data
    BP32.update();

    // Process connected controllers
    processControllers();
}
