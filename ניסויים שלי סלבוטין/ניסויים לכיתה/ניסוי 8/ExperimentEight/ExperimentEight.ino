#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define MOTOR_B1 12
#define MOTOR_F1 13

#define MOTOR_B2 4
#define MOTOR_F2 25

// Constants for OLED display
#define SCREEN_WIDTH 128  // OLED display width in pixels
#define SCREEN_HEIGHT 64  // OLED display height in pixels
#define OLED_RESET -1     // Reset pin (-1 if using Arduino reset pin)
#define I2C_ADDRESS 0x3C  // Default I2C address for OLED display

// Create object for OLED display 
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);

  // Initialize the motors
  pinMode(MOTOR_F1, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT);

  pinMode(MOTOR_F2, OUTPUT);
  pinMode(MOTOR_B2, OUTPUT);

  // Initialize the OLED display
  display.begin(I2C_ADDRESS, true);  
  display.clearDisplay();           // Clear the display
  display.setTextColor(SH110X_WHITE); // Set text color to white
  display.setTextSize(2);            // Set default text size
}

char command = ' '; // Read the received character

void turnOnMotorWithCommand()
{
  switch (command) {
      case 'F': // Forward
        Serial.println("Moving Forward");
        // Adjusted for forward direction on both drivers
        // First set : 
        digitalWrite(MOTOR_F1, HIGH);
        digitalWrite(MOTOR_B1, LOW); 
        // Second set : 
        digitalWrite(MOTOR_F2, HIGH);
        digitalWrite(MOTOR_B2, LOW); 
        // display
        display.setCursor(1, 1); 
        display.print("Forward");
        display.display();  // Update the display
        delay(100);         // Small delay before the next update
        display.clearDisplay(); // Clear the display for new data
        break;

      case 'B': // Backward
        Serial.println("Moving Backward");
        // Adjusted for backward direction
        // First set : 
        digitalWrite(MOTOR_F1, LOW);
        digitalWrite(MOTOR_B1, HIGH); 
        // Second set : 
        digitalWrite(MOTOR_F2, LOW);
        digitalWrite(MOTOR_B2, HIGH);    
        // display
        display.setCursor(1, 1); 
        display.print("Backwards");
        display.display();  // Update the display
        delay(100);         // Small delay before the next update
        display.clearDisplay(); // Clear the display for new data
     
        break;

      case 'L': // Stop
        Serial.println("Moving Left");
        // First set : 
        digitalWrite(MOTOR_F1, HIGH);
        digitalWrite(MOTOR_B1, LOW); 
        // Second set : 
        digitalWrite(MOTOR_F2, LOW);
        digitalWrite(MOTOR_B2, HIGH); 
        // display
        display.setCursor(1, 1); 
        display.print("Left");
        display.display();  // Update the display
        delay(100);         // Small delay before the next update
        display.clearDisplay(); // Clear the display for new data

        break;

      case 'R': // Stop
        Serial.println("Moving Right");
        // First set : 
        digitalWrite(MOTOR_F1, LOW);
        digitalWrite(MOTOR_B1, HIGH); 
        // Second set : 
        digitalWrite(MOTOR_F2, HIGH);
        digitalWrite(MOTOR_B2, LOW); 
        // display
        display.setCursor(1, 1); 
        display.print("Right");
        display.display();  // Update the display
        delay(100);         // Small delay before the next update
        display.clearDisplay(); // Clear the display for new data

        break;

      case 'S': // Stop
        Serial.println("Stopping");
        digitalWrite(MOTOR_F1, LOW);
        digitalWrite(MOTOR_B1, LOW); 
        digitalWrite(MOTOR_F2, LOW);
        digitalWrite(MOTOR_B2, LOW);  
        // display
        display.setCursor(1, 1); 
        display.print("Stopping");
        display.display();  // Update the display
        delay(100);         // Small delay before the next update
        display.clearDisplay(); // Clear the display for new data

        break;
    }
}

void loop() {
  if (Serial.available()) {
    command = Serial.read(); // Read the received character
  }
  turnOnMotorWithCommand();
}

