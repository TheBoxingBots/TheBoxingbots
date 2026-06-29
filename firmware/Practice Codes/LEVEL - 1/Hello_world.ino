// 🛠️ Hello World - Your Bot Speaks!  
// A simple program to print a message on the Serial Monitor.  
// 💡 Customize your own text: Change the message inside Serial.println()!  

void setup() {  
    Serial.begin(115200);  // 📡 Start serial communication  
    delay(500);  // ⏳ Minimum delay to ensure Serial Monitor works properly  
    Serial.println("🤖 Hello, I'm your Boxing Bot!");  // 🗣️ First message (Customize this!)  
}  

void loop() {  
    // 🔄 Keep the program running (Required for ESP32)  
}  

/* 🔒 Unlock Task: Can you make the bot speak continuously?  
   ➤ Remove the "//" from the loop function below.  
   ➤ Hint: Use delay(5000) inside the loop to repeat the message every 5 seconds!  
*/

// 🔓 LOOP FUNCTION - Remove "//" from the lines below to activate!  
// void loop() {  
//     delay(5000);  // ⏳ Wait 5 seconds before repeating  
//     Serial.println("Hello again! I'm still here, ready to fight!");  // 🥊 Print message again  
// }  
