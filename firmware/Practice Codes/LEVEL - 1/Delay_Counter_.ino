// 🌟 LED Blinking with Counter – Learn Timing & Loops!
// 📌 This program turns an LED ON and OFF every second & counts the cycles.

const int ledPin = 2;  // 💡 LED pin (ESP32: 2, Arduino: 13)
int counter = 0;       // 🔢 Blink counter

void setup() {
    Serial.begin(115200);  // 🖥️ Start Serial Monitor
    pinMode(ledPin, OUTPUT);  // 🔌 Set LED pin as OUTPUT
    Serial.println("🚀 LED Blink Counter Started!");
}

void loop() {
    counter++;  // ➕ Increase blink count

    // 🔴 Turn LED ON
    digitalWrite(ledPin, HIGH);
    Serial.print("✨ Cycle "); Serial.print(counter); Serial.println(": 🔵LED ON");
    delay(1000);  // ⏳ Wait 1 second

    // ⚫ Turn LED OFF
    digitalWrite(ledPin, LOW);
    Serial.print("✨ Cycle "); Serial.print(counter); Serial.println(": ⚪LED OFF");
    delay(1000);  // ⏳ Wait 1 second

    // 🛠️ CUSTOMIZATION: Change the delay to blink faster/slower
    // Example: delay(500); for a faster blink rate
}
