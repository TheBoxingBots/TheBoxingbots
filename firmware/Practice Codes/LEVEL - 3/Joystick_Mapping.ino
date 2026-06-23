#include <Bluepad32.h>

GamepadPtr myGamepad = nullptr;  // 🎮 Pointer to connected gamepad

// 🔗 Callback function when a controller connects
// This function will be triggered when the gamepad connects to the ESP32.
void onConnected(GamepadPtr gp) {
    Serial.println("🎮 Controller Connected!");  // Notify that the controller is connected
    myGamepad = gp;  // Store the pointer to the connected gamepad
}

// 🔌 Callback function when a controller disconnects
// This function will be triggered when the gamepad disconnects from the ESP32.
void onDisconnected(GamepadPtr gp) {
    Serial.println("⚠️ Controller Disconnected!");  // Notify that the controller is disconnected
    myGamepad = nullptr;  // Clear the pointer as no gamepad is connected
}

void setup() {
    Serial.begin(115200);  // Initialize serial communication at 115200 baud rate
    BP32.setup(&onConnected, &onDisconnected);  // Setup Bluepad32 with the connected and disconnected callbacks
    Serial.println("🚀 Waiting for PS4 controller connection...");  // Notify that the system is waiting for controller connection
}

void loop() {
    BP32.update();  // Continuously update the state of the gamepad to check for input

    // 🎮 Check if the controller is connected
    if (myGamepad && myGamepad->isConnected()) {
        
        // Get the values of the Left Stick axis
        int left_axis_x = myGamepad->axisX();  // Left Stick X-axis (horizontal movement)
        int left_axis_y = myGamepad->axisY();  // Left Stick Y-axis (vertical movement)

        // Left Stick Diagonal Movements (check if both axes are moved simultaneously)
        if (left_axis_x < -100 && left_axis_y < -100) {
            Serial.println("⬅️ Left Stick Left + ⬆️ Left Stick Up");  // Left Stick moved Left and Up simultaneously
        }
        else if (left_axis_x > 100 && left_axis_y < -100) {
            Serial.println("➡️ Left Stick Right + ⬆️ Left Stick Up");  // Left Stick moved Right and Up simultaneously
        }
        else if (left_axis_x < -100 && left_axis_y > 100) {
            Serial.println("⬅️ Left Stick Left + ⬇️ Left Stick Down");  // Left Stick moved Left and Down simultaneously
        }
        else if (left_axis_x > 100 && left_axis_y > 100) {
            Serial.println("➡️ Left Stick Right + ⬇️ Left Stick Down");  // Left Stick moved Right and Down simultaneously
        }
        // Pure Left Stick Movements (check for individual axis movement)
        else {
            if (left_axis_x < -100) Serial.println("⬅️ Left Stick Left");  // Left Stick moved Left
            if (left_axis_x > 100) Serial.println("➡️ Left Stick Right");  // Left Stick moved Right
            if (left_axis_y < -100) Serial.println("⬆️ Left Stick Up");  // Left Stick moved Up
            if (left_axis_y > 100) Serial.println("⬇️ Left Stick Down");  // Left Stick moved Down
        }

        // Get the values of the Right Stick axis
        int right_axis_x = myGamepad->axisRX();  // Right Stick X-axis (horizontal movement)
        int right_axis_y = myGamepad->axisRY();  // Right Stick Y-axis (vertical movement)

        // Right Stick Diagonal Movements (check if both axes are moved simultaneously)
        if (right_axis_x < -100 && right_axis_y < -100) {
            Serial.println("⬅️ Right Stick Left + ⬆️ Right Stick Up");  // Right Stick moved Left and Up simultaneously
        }
        else if (right_axis_x > 100 && right_axis_y < -100) {
            Serial.println("➡️ Right Stick Right + ⬆️ Right Stick Up");  // Right Stick moved Right and Up simultaneously
        }
        else if (right_axis_x < -100 && right_axis_y > 100) {
            Serial.println("⬅️ Right Stick Left + ⬇️ Right Stick Down");  // Right Stick moved Left and Down simultaneously
        }
        else if (right_axis_x > 100 && right_axis_y > 100) {
            Serial.println("➡️ Right Stick Right + ⬇️ Right Stick Down");  // Right Stick moved Right and Down simultaneously
        }
        // Pure Right Stick Movements (check for individual axis movement)
        else {
            if (right_axis_x < -100) Serial.println("⬅️ Right Stick Left");  // Right Stick moved Left
            if (right_axis_x > 100) Serial.println("➡️ Right Stick Right");  // Right Stick moved Right
            if (right_axis_y < -100) Serial.println("⬆️ Right Stick Up");  // Right Stick moved Up
            if (right_axis_y > 100) Serial.println("⬇️ Right Stick Down");  // Right Stick moved Down
        }

        delay(100);  // Delay to avoid flooding the Serial Monitor with too many messages at once
    }
}
