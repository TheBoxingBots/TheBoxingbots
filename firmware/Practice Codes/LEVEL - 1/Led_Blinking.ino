// 🛠️ LED Blinking - Your Bot Signals Life!  
// This program makes the LED turn ON and OFF continuously.  
// 💡 Change the LED pin if needed. The built-in LED is usually on GPIO 2.  

#define LED_PIN 2  // 🔌 Define the pin for the LED (Built-in LED on ESP32)  

void setup() {  
    pinMode(LED_PIN, OUTPUT);  // ⚙️ Set the LED pin as an OUTPUT  
}  

void loop() {  
    digitalWrite(LED_PIN, HIGH);  // 💡 Turn the LED ON (HIGH = 1)  
    delay(500);  // ⏳ Wait for 500ms  
    digitalWrite(LED_PIN, LOW);  // 🔄 Turn the LED OFF (LOW = 0)  
    delay(500);  // ⏳ Wait for 500ms  
}  

/* 🔒 Unlock Task: Can you change the blinking speed?  
   ➤ Modify the delay() values.  
   ➤ Try delay(1000) for a slower blink or delay(200) for a faster blink!  
*/
