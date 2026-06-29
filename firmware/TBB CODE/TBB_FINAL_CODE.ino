#include <Bluepad32.h>
#include <ESP32Servo.h>

// -------------------- Pins --------------------
const int motor1_enable_pin   = 13;  // PWM (left motor EN)
const int motor2_enable_pin   = 25;  // PWM (right motor EN)
const int motor1_forward_pin  = 12;  // IN1 (left dir A)
const int motor1_backward_pin = 14;  // IN2 (left dir B)
const int motor2_forward_pin  = 27;  // IN3 (right dir A)
const int motor2_backward_pin = 26;  // IN4 (right dir B)

const int servo1_pin = 2;
const int servo2_pin = 4;
const int servo3_pin = 5;
const int servo4_pin = 18;
const int servo5_pin = 19;

const int led1 = 21;
const int led2 = 22;
const int buzzer = 15;

ControllerPtr activeController = nullptr;

// -------------------- Bluepad / State --------------------

Servo servo1, servo2, servo3, servo4, servo5;

unsigned long comboStart = 0;
bool comboActive = false;

// -------------------- Record / Playback (PS button) --------------------
enum Mode { MODE_IDLE, MODE_RECORD, MODE_PLAYBACK };
Mode mode = MODE_IDLE;

const uint16_t SAMPLE_MS = 20;        // 50 Hz
const uint16_t MAX_FRAMES = 1500;     // ~30s @ 50Hz
struct Frame {
  uint8_t a1, a2, a3, a4, a5;   // servo angles
  int16_t mL, mR;               // left/right motor speeds -255..255
};
Frame frames[MAX_FRAMES];
uint16_t frameCount = 0;
uint16_t playIndex = 0;
uint32_t lastSampleMs = 0;
bool prevPS = false;

// -------------------- Cruise Control (Select button) --------------------
enum CruiseState { CRUISE_OFF, CRUISE_DRIVE, CRUISE_REPEAT_X, CRUISE_REPEAT_Y, CRUISE_REPEAT_B, CRUISE_REPEAT_A };
CruiseState cruise = CRUISE_OFF;
bool prevSelect = false;
bool cruiseAwaitRelease = false;   // avoid instant off after on

enum LastAction { ACT_NONE, ACT_LIVE, ACT_X, ACT_Y, ACT_B, ACT_A };
LastAction lastAction = ACT_NONE;

int16_t baseSpeed = 0;            // -255..255 speed for cruise-drive
const int SPEED_STEP = 32;        // cruise UP/DOWN increment
const int TURN_MIN  = 180;        // minimum pivot strength during cruise

// -------------------- Utils --------------------
static inline uint8_t clamp180(int v){ return (uint8_t)(v < 0 ? 0 : (v > 180 ? 180 : v)); }
static inline int16_t clamp255(int v){ return (int16_t)(v < -255 ? -255 : (v > 255 ? 255 : v)); }

// -------------------- Motor helpers (PWM on EN pins) --------------------
// Each motor: EN pin gets PWM, IN pins set direction
void driveM1(int speed) {
  speed = clamp255(speed);
  if (speed > 0) {
    digitalWrite(motor1_forward_pin, HIGH);
    digitalWrite(motor1_backward_pin, LOW);
    analogWrite(motor1_enable_pin, speed);
  } else if (speed < 0) {
    digitalWrite(motor1_forward_pin, LOW);
    digitalWrite(motor1_backward_pin, HIGH);
    analogWrite(motor1_enable_pin, -speed);
  } else {
    digitalWrite(motor1_forward_pin, LOW);
    digitalWrite(motor1_backward_pin, LOW);
    analogWrite(motor1_enable_pin, 0);
  }
}

void driveM2(int speed) {
  speed = clamp255(speed);
  if (speed > 0) {
    digitalWrite(motor2_forward_pin, HIGH);
    digitalWrite(motor2_backward_pin, LOW);
    analogWrite(motor2_enable_pin, speed);
  } else if (speed < 0) {
    digitalWrite(motor2_forward_pin, LOW);
    digitalWrite(motor2_backward_pin, HIGH);
    analogWrite(motor2_enable_pin, -speed);
  } else {
    digitalWrite(motor2_forward_pin, LOW);
    digitalWrite(motor2_backward_pin, LOW);
    analogWrite(motor2_enable_pin, 0);
  }
}

void driveMotorsFromSpeeds(int16_t mL, int16_t mR) {
  driveM1(mL);
  driveM2(mR);
}

// DPAD → desired speeds. If your LEFT/RIGHT are still swapped, flip signs here.
void motorsFromDpad(uint8_t dpad, int16_t &mL, int16_t &mR) {
  mL = 0; mR = 0;
  if (dpad & DPAD_UP)        { mL =  255; mR =  255; }           //santhosh
  else if (dpad & DPAD_DOWN) { mL = -255; mR = -255; }
  else if (dpad & DPAD_LEFT) { mL =  -255; mR = 255; }  // <-- pivot LEFT 
  else if (dpad & DPAD_RIGHT){ mL = 255; mR =  -255; }  // <-- pivot RIGHT
}

void computeAndDriveMotorsFromDpad(uint8_t dpad, int16_t &mL, int16_t &mR) {
  motorsFromDpad(dpad, mL, mR);
  driveMotorsFromSpeeds(mL, mR);
}

// -------------------- Safe stop --------------------
void stopAllMotorsAndServos() {
  driveMotorsFromSpeeds(0, 0);
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  servo5.write(90);
}

// -------------------- Your existing custom move --------------------
void moveBotAndSetServos() {
  int targetServo1Angle = 170;  // hand left
  int targetServo2Angle = 60;   // shoulder
  int targetServo3Angle = 10;   // right hand
  int targetServo4Angle = 120;  // right shoulder

  int currentServo1Angle = servo1.read();
  int currentServo2Angle = servo2.read();
  int currentServo3Angle = servo3.read();
  int currentServo4Angle = servo4.read();

  const int moveDuration = 400;
  const int stepDuration = 20;
  const int steps = moveDuration / stepDuration;

  float servo1Increment = (targetServo1Angle - currentServo1Angle) / (float)steps;
  float servo2Increment = (targetServo2Angle - currentServo2Angle) / (float)steps;
  float servo3Increment = (targetServo3Angle - currentServo3Angle) / (float)steps;
  float servo4Increment = (targetServo4Angle - currentServo4Angle) / (float)steps;

  // move forward while animating
  driveMotorsFromSpeeds(255, 255);

  for (int i = 0; i < steps; i++) {
    currentServo1Angle += servo1Increment;
    currentServo2Angle += servo2Increment;
    currentServo3Angle += servo3Increment;
    currentServo4Angle += servo4Increment;

    servo1.write(currentServo1Angle);
    servo2.write(currentServo2Angle);
    servo3.write(currentServo3Angle);
    servo4.write(currentServo4Angle);

    delay(stepDuration);
  }

  driveMotorsFromSpeeds(0, 0);
}

// -------------------- Connect/Disconnect --------------------
void onConnectedController(ControllerPtr ctl) {
  if (activeController != nullptr) {
    Serial.println("❌ Another controller tried to connect. Rejected.");
    ctl->disconnect();
    return;
  }

  activeController = ctl;
  BP32.enableNewBluetoothConnections(false); // 🔒 LOCK Bluetooth
  Serial.println("✅ Controller connected and locked.");
}

void onDisconnectedController(ControllerPtr ctl) {
  if (ctl == activeController) {
    Serial.println("⚠️ Controller disconnected.");
    activeController = nullptr;
    BP32.enableNewBluetoothConnections(true); // 🔓 UNLOCK Bluetooth
    stopAllMotorsAndServos();
  }
}

// -------------------- Record/Playback control (PS) --------------------
void enterRecordMode() {
  cruise = CRUISE_OFF; cruiseAwaitRelease = false;
  mode = MODE_RECORD;
  frameCount = 0;
  lastSampleMs = millis();
  Serial.println("▶️ RECORD (PS): Move servos (sticks) + motors (DPAD). Press PS to stop & save.");
  digitalWrite(buzzer, HIGH); delay(80); digitalWrite(buzzer, LOW); delay(80);
  digitalWrite(buzzer, HIGH); delay(80); digitalWrite(buzzer, LOW);
}

void enterPlaybackMode() {
  cruise = CRUISE_OFF; cruiseAwaitRelease = false;
  if (frameCount == 0) {
    Serial.println("ℹ️ No frames recorded. Ignoring playback.");
    return;
  }
  mode = MODE_PLAYBACK;
  playIndex = 0;
  lastSampleMs = millis();
  Serial.printf("🎬 PLAYBACK (PS): %u frames.\n", frameCount);
  digitalWrite(buzzer, HIGH); delay(250); digitalWrite(buzzer, LOW);
}

void returnToIdle() {
  mode = MODE_IDLE;
  stopAllMotorsAndServos();
  Serial.println("⏹️ IDLE.");
  digitalWrite(buzzer, HIGH); delay(60); digitalWrite(buzzer, LOW);
}

void handlePSEdge(ControllerPtr ctl) {
  static bool prev = false;
  bool ps = ctl->miscHome();
  if (ps && !prev) {
    if (mode == MODE_IDLE) {
      enterRecordMode();
    } else if (mode == MODE_RECORD) {
      Serial.printf("💾 Recorded %u frames.\n", frameCount);
      enterPlaybackMode();
    } else if (mode == MODE_PLAYBACK) {
      Serial.println("⏩ Playback interrupted (PS).");
      returnToIdle();
    }
  }
  prev = ps;
}

// -------------------- Battery (optional) --------------------
void printBatteryStatus(ControllerPtr ctl) {
  int batteryLevel = ctl->battery();
  if (batteryLevel >= 0) {
    Serial.printf("Controller index=%d battery level: %d%%\n", ctl->index(), batteryLevel);
  } else {
    Serial.printf("Controller index=%d battery level: Not available\n", ctl->index());
  }
}

// -------------------- Special Moves (UNCHANGED) --------------------
void performButtonXAction() {
  driveMotorsFromSpeeds(255 / 8, 255 / 8);
  delay(400);
  driveMotorsFromSpeeds(0, 0);

  servo1.write(170);
  servo2.write(40);
  servo3.write(10);
  servo4.write(140);
  delay(1500);
}
void performButtonYAction() { moveBotAndSetServos(); }
void performButtonAAction() {
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  servo5.write(45);
  servo2.write(40);
  servo1.write(180);
  delay(400);
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  servo5.write(90);
  delay(100);
  servo5.write(135);
  servo3.write(0);
  servo4.write(130);
  delay(400);
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  servo5.write(90);
  delay(100);
  servo5.write(45);
  servo2.write(40);
  servo1.write(180);
  delay(400);
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  servo5.write(90);
  delay(100);
  servo5.write(135);
  servo3.write(0);
  servo4.write(130);
  delay(400);
}
void performButtonBAction() {
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  servo5.write(45);
  servo2.write(40);
  delay(800);
  servo1.write(180);
  delay(400);
  servo1.write(140);
  delay(400);
  servo1.write(180);
  delay(400);
  servo1.write(140);
  delay(400);
}
void performL1Action() {
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  servo5.write(45);
  servo2.write(40);
  servo1.write(180);
  delay(400);
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  servo5.write(90);
  delay(100);
}
void performR1Action() {
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  servo5.write(90);
  delay(100);
  servo5.write(135);
  servo3.write(0);
  servo4.write(130);
  delay(400);
}

// -------------------- Cruise helpers --------------------
bool tryExitCruiseOnSelect(ControllerPtr ctl) {
  if (cruiseAwaitRelease) {
    if (!ctl->miscSelect()) cruiseAwaitRelease = false;
    return false;
  }
  if (ctl->miscSelect()) {
    // wait release
    while (ctl->miscSelect()) { BP32.update(); delay(5); }
    cruise = CRUISE_OFF;
    digitalWrite(led2, LOW);
    Serial.println("🛞 Cruise OFF");
    return true;
  }
  return false;
}

// -------------------- Main processing --------------------
void processGamepad(ControllerPtr ctl) {
  // Emergency combo: Touchpad + PS to disconnect
  if (ctl->miscSystem() && ctl->miscHome()) {
    if (!comboActive) {
      comboActive = true;
      comboStart = millis();
    } else if (millis() - comboStart >= 3000) {
      ctl->disconnect();
      comboActive = false;
    }
    return;
  } else {
    comboActive = false;
  }

  handlePSEdge(ctl);

  // Sticks → Servos
  int left_axis_x  = ctl->axisX();
  int left_axis_y  = ctl->axisY();
  int right_axis_x = ctl->axisRX();
  int right_axis_y = ctl->axisRY();

  // ---- FIXED TRIGGER READ (L2 / R2) ----
  int l2 = ctl->brake();      // L2 analog
  int r2 = ctl->throttle();   // R2 analog

  // Noise filter
  if (l2 < 50) l2 = 0;
  if (r2 < 50) r2 = 0;

  // FIX: keep only the stronger trigger
  if (l2 > r2) {
    r2 = 0;
  } else {
    l2 = 0;
  }


  int a1 = map(left_axis_y,  511, -512, 0, 180);
  int a2 = map(left_axis_x,  511, -512, 0, 180);
  int a3 = map(right_axis_y, -511,  512, 0, 180);
  int a4 = map(right_axis_x,  511, -512, 0, 180);
  int a5 = 90;
  if (l2 > 0)      a5 = 180;
  else if (r2 > 0) a5 = 0;


  uint32_t now = millis();

  // ----- PLAYBACK -----
  if (mode == MODE_PLAYBACK) {
    if (frameCount == 0) { returnToIdle(); return; }
    if (now - lastSampleMs >= SAMPLE_MS) {
      lastSampleMs = now;
      const Frame& f = frames[playIndex];
      driveMotorsFromSpeeds(f.mL, f.mR);
      servo1.write(f.a1);
      servo2.write(f.a2);
      servo3.write(f.a3);
      servo4.write(f.a4);
      servo5.write(f.a5);

      playIndex++;
      if (playIndex >= frameCount) {
        Serial.println("✅ Playback finished.");
        returnToIdle();
      }
    }
    return;
  }

  // ----- RECORD -----
  if (mode == MODE_RECORD) {
    uint8_t dpadState = ctl->dpad();
    int16_t mL, mR;
    computeAndDriveMotorsFromDpad(dpadState, mL, mR);

    servo1.write(a1);
    servo2.write(a2);
    servo3.write(a3);
    servo4.write(a4);
    servo5.write(a5);

    if (now - lastSampleMs >= SAMPLE_MS) {
      lastSampleMs = now;
      if (frameCount < MAX_FRAMES) {
        frames[frameCount++] = Frame{
          (uint8_t)clamp180(a1),
          (uint8_t)clamp180(a2),
          (uint8_t)clamp180(a3),
          (uint8_t)clamp180(a4),
          (uint8_t)clamp180(a5),
          (int16_t)mL,
          (int16_t)mR
        };
      } else {
        Serial.println("📦 Buffer full → auto PLAYBACK.");
        enterPlaybackMode();
      }
    }
    return;
  }

  // ----- MODE_IDLE -----

  // SELECT toggles cruise
  bool sel = ctl->miscSelect();
  if (sel && !prevSelect) {
    if (cruise == CRUISE_OFF) {
      if (lastAction == ACT_X)      { cruise = CRUISE_REPEAT_X; Serial.println("🛞 Cruise ON: repeat X"); }
      else if (lastAction == ACT_Y) { cruise = CRUISE_REPEAT_Y; Serial.println("🛞 Cruise ON: repeat Triangle"); }
      else if (lastAction == ACT_B) { cruise = CRUISE_REPEAT_B; Serial.println("🛞 Cruise ON: repeat Square"); }
      else if (lastAction == ACT_A) { cruise = CRUISE_REPEAT_A; Serial.println("🛞 Cruise ON: repeat Circle"); }
      else {
        int16_t mL, mR; motorsFromDpad(ctl->dpad(), mL, mR);
        baseSpeed = clamp255((mL + mR) / 2);  // correct average
        cruise = CRUISE_DRIVE;
        Serial.printf("🛞 Cruise ON: DRIVE base=%d\n", baseSpeed);
      }
      cruiseAwaitRelease = true;
      digitalWrite(led2, HIGH);
    } else {
      cruise = CRUISE_OFF; cruiseAwaitRelease = false;
      digitalWrite(led2, LOW);
      Serial.println("🛞 Cruise OFF (edge)");
    }
  }
  prevSelect = sel;

  // If cruise active
  if (cruise != CRUISE_OFF) {
    if (cruise == CRUISE_REPEAT_X || cruise == CRUISE_REPEAT_Y ||
        cruise == CRUISE_REPEAT_B || cruise == CRUISE_REPEAT_A) {

      if (tryExitCruiseOnSelect(ctl)) return;

      switch (cruise) {
        case CRUISE_REPEAT_X: performButtonXAction(); break;
        case CRUISE_REPEAT_Y: performButtonYAction(); break;
        case CRUISE_REPEAT_B: performButtonBAction(); break;
        case CRUISE_REPEAT_A: performButtonAAction(); break;
        default: break;
      }
      // small breathing window to detect Select press
      unsigned long t0 = millis();
      while (millis() - t0 < 150) { BP32.update(); if (tryExitCruiseOnSelect(ctl)) return; delay(5); }
      return;
    }

    // ---- CRUISE_DRIVE with strong pivot steering ----
    if (cruise == CRUISE_DRIVE) {
      if (tryExitCruiseOnSelect(ctl)) return;

      uint8_t dpad = ctl->dpad();

      // UP/DOWN adjust base speed
      if (dpad & DPAD_UP)        baseSpeed = clamp255(baseSpeed + SPEED_STEP);
      else if (dpad & DPAD_DOWN) baseSpeed = clamp255(baseSpeed - SPEED_STEP);

      // Strong pivot for LEFT/RIGHT: both sides spin opposite
      int turnMag = max(TURN_MIN, min(255, abs(baseSpeed)));
      int16_t mL, mR;
      if (dpad & DPAD_LEFT) {                               //santhosh
        mL = -turnMag; mR = turnMag;
      } else if (dpad & DPAD_RIGHT) {
        mL = turnMag; mR = -turnMag;
      } else {
        mL = baseSpeed; mR = baseSpeed;
      }

      driveMotorsFromSpeeds(mL, mR);

      // Servos stay live during cruise
      servo1.write(a1); servo2.write(a2); servo3.write(a3); servo4.write(a4);
      if (l2 > 0)      servo5.write(180);
      else if (r2 > 0) servo5.write(0);
      else             servo5.write(90);


      return;
    }
  }

  // ---- Normal (non-cruise) controls ----
  if (ctl->a()) { lastAction = ACT_X; Serial.println("BUTTON_X pressed: Performing action."); performButtonXAction(); return; }
  if (ctl->l1()) { Serial.println("L1 button pressed: Performing John Cena move."); performL1Action(); return; }
  if (ctl->y()) { lastAction = ACT_Y; Serial.println("BUTTON_Y pressed: Performing action."); performButtonYAction(); return; }
  if (ctl->b()) { lastAction = ACT_A; Serial.println("BUTTON_A pressed: Performing action."); performButtonAAction(); return; }
  if (ctl->x()) { lastAction = ACT_B; Serial.println("BUTTON_B pressed: Performing action."); performButtonBAction(); return; }
  if (ctl->r1()) { performR1Action(); return; }

  // DPAD driving (normal)
  {
    uint8_t dpadState = ctl->dpad();
    int16_t mL, mR; motorsFromDpad(dpadState, mL, mR);
    if (mL || mR) lastAction = ACT_LIVE;
    computeAndDriveMotorsFromDpad(dpadState, mL, mR);
  }

  // Live servos (normal)
  servo1.write(a1);
  servo2.write(a2);
  servo3.write(a3);
  servo4.write(a4);
  if (l2 > 0)      servo5.write(180);
  else if (r2 > 0) servo5.write(0);
  else             servo5.write(90);


  // If sticks or triggers were active, remember last was live driving
  if (abs(left_axis_x) > 50 || abs(left_axis_y) > 50 ||
      abs(right_axis_x)> 50 || abs(right_axis_y)> 50 ||
      l2 > 0 || r2 > 0) {
    lastAction = ACT_LIVE;
  }


  // Optional: printBatteryStatus(ctl);
}

// -------------------- Process all --------------------
void processControllers() {
  if (activeController &&
      activeController->isConnected() &&
      activeController->hasData() &&
      activeController->isGamepad()) {

    processGamepad(activeController);
  }
}


// -------------------- Setup / Loop --------------------
void setup() {
  Serial.begin(115200);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(led1, HIGH);
  digitalWrite(led2, LOW);

  pinMode(motor1_forward_pin, OUTPUT);
  pinMode(motor1_backward_pin, OUTPUT);
  pinMode(motor2_forward_pin, OUTPUT);
  pinMode(motor2_backward_pin, OUTPUT);
  pinMode(motor1_enable_pin, OUTPUT);  // PWM
  pinMode(motor2_enable_pin, OUTPUT);  // PWM
  pinMode(buzzer, OUTPUT);

  analogWrite(motor1_enable_pin, 0);
  analogWrite(motor2_enable_pin, 0);
  digitalWrite(motor1_forward_pin, LOW);
  digitalWrite(motor1_backward_pin, LOW);
  digitalWrite(motor2_forward_pin, LOW);
  digitalWrite(motor2_backward_pin, LOW);

  servo1.attach(servo1_pin);
  servo2.attach(servo2_pin);
  servo3.attach(servo3_pin);
  servo4.attach(servo4_pin);
  servo5.attach(servo5_pin);

  BP32.setup(&onConnectedController, &onDisconnectedController);
  Serial.println("Setup completed. Waiting for controllers...");
}

void loop() {
  BP32.update();
  processControllers();
}
