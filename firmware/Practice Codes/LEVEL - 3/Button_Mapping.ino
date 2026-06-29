#include <Bluepad32.h>

GamepadPtr myGamepad = nullptr;  // Pointer to connected gamepad

// Callback when a controller connects
void onConnected(GamepadPtr gp) {
    Serial.println("🎮 Controller Connected!");  // Notify when the controller is connected
    myGamepad = gp;  // Store the connected gamepad
}

// Callback when a controller disconnects
void onDisconnected(GamepadPtr gp) {
    Serial.println("⚠️ Controller Disconnected!");  // Notify when the controller is disconnected
    myGamepad = nullptr;  // Clear the gamepad pointer when disconnected
}

void setup() {
    Serial.begin(115200);  // Start serial communication at 115200 baud rate
    BP32.setup(&onConnected, &onDisconnected);  // Initialize Bluepad32 with callbacks
    Serial.println("🚀 Waiting for PS4 controller connection...");  // Notify that the system is waiting for a controller
}

void loop() {
    BP32.update();  // Update controller state

    if (myGamepad && myGamepad->isConnected()) {  // Check if the gamepad is connected
        // 🎮 Face Buttons
        if (myGamepad->a()) Serial.println("❌ Cross Button Pressed");  // Cross button (X)
        if (myGamepad->b()) Serial.println("🔴 Circle Button Pressed");  // Circle button (O)
        if (myGamepad->x()) Serial.println("🔵 Square Button Pressed");  // Square button (□)
        if (myGamepad->y()) Serial.println("🟢 Triangle Button Pressed");  // Triangle button (△)

        // 🎮 Bumpers & Triggers
        if (myGamepad->l1()) Serial.println("⬅️ L1 Button Pressed");  // Left Bumper (L1)
        if (myGamepad->r1()) Serial.println("➡️ R1 Button Pressed");  // Right Bumper (R1)
        if (myGamepad->l2()) Serial.println("🔻 L2 Button Pressed");  // Left Trigger (L2)
        if (myGamepad->r2()) Serial.println("🔺 R2 Button Pressed");  // Right Trigger (R2)

        // 🎮 D-Pad Handling
        uint8_t dpad = myGamepad->dpad();  // Get D-Pad state
        if (dpad & DPAD_UP) Serial.println("⬆️ D-Pad Up Pressed");  // D-Pad Up
        if (dpad & DPAD_DOWN) Serial.println("⬇️ D-Pad Down Pressed");  // D-Pad Down
        if (dpad & DPAD_LEFT) Serial.println("⬅️ D-Pad Left Pressed");  // D-Pad Left
        if (dpad & DPAD_RIGHT) Serial.println("➡️ D-Pad Right Pressed");  // D-Pad Right

        // 🎮 Misc Buttons (Select and Start)
        if (myGamepad->miscSelect()) Serial.println("📤 Select Button Pressed");  // Select button
        if (myGamepad->miscStart()) Serial.println("⚙️ Start Button Pressed");  // Start button

        // 🎮 Stick Clicks (L3 and R3)
        if (myGamepad->buttons() & BUTTON_THUMB_L) Serial.println("🕹️ L3 (Left Stick Click) Pressed");  // Left Stick Click (L3)
        if (myGamepad->buttons() & BUTTON_THUMB_R) Serial.println("🕹️ R3 (Right Stick Click) Pressed");  // Right Stick Click (R3)

        // 🎮 PS Home Button & Touchpad
        if (myGamepad->miscHome()) Serial.println("🏠 PS Button Pressed");  // PS Home Button
        if (myGamepad->miscSystem()) Serial.println("🖲️ Touchpad Pressed");  // Touchpad Button

        delay(100);  // Prevent flooding the Serial Monitor with too many messages
    }
}
