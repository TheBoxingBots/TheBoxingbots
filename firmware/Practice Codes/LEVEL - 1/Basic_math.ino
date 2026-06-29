// 🧮 Basic Math Operations – Your bot’s calculator brain!  
// 🤖 It performs addition, subtraction, multiplication, division, and modulo.  
// 🎓 Students can change num1 and num2 to explore different calculations.  

void setup() {  
    Serial.begin(115200); // 🖥️ Start Serial Communication  
    delay(500); // ⏳ Ensure Serial Monitor is ready  

    // 🌟 Define numbers  
    int num1 = 10;  // ✏️ Change this number  
    int num2 = 5;   // ✏️ Change this number  

    Serial.println("🔢 Basic Math Operations:");  
    Serial.print("➕ Addition: "); Serial.println(num1 + num2);  
    Serial.print("➖ Subtraction: "); Serial.println(num1 - num2);  
    Serial.print("✖ Multiplication: "); Serial.println(num1 * num2);  
    Serial.print("➗ Division: "); Serial.println(num1 / num2);  
    Serial.print("🔄 Modulo: "); Serial.println(num1 % num2); 
    Serial.println("\n🎯 Try changing 'num1' and 'num2' in the code for new results!");  
}  

void loop() {  
    // 🔁 Empty loop – Code runs once in setup  
}
