#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_NeoPixel.h>

// OLED pins
#define i2c_Address 0x3c // OLED screen I2C address
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Creating an object for communication with the OLED screen

// IR Sensor Pins
#define IR1 32
#define IR2 39
#define IR3 15
#define IR4 5

// OLED pins
#define i2c_Address 0x3c // OLED screen I2C address
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 

// Motor and PWM pins
#define Motor_B1 12 
#define Motor_F1 13 
#define Motor_B2 4  
#define Motor_F2 25  

#define Resolution 8
#define Freq 1000

#define PWM_CHANNEL_B1 0
#define PWM_CHANNEL_F1 1
#define PWM_CHANNEL_B2 2
#define PWM_CHANNEL_F2 3

void setup() {
  // Initialize the display
  display.begin(i2c_Address,true); 
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(IR3, INPUT);
  pinMode(IR4, INPUT);

  // Set up PWM channels for motors
  ledcAttach(Motor_F1, Freq, Resolution);
  ledcAttach(Motor_F2, Freq, Resolution);
  ledcAttach(Motor_B1, Freq, Resolution);
  ledcAttach(Motor_B2, Freq, Resolution);

  delay(1000);
}

void loop() 
{
   // If car is on course
  if (digitalRead(IR2) && digitalRead(IR3)) {
    motorsWrite(1, 1, 0, 0);
  } 
  // Extreme correcting to the left
  else if(digitalRead(IR1)){
    motorsWrite(1,0,0,0);
    while(!digitalRead(IR2) && !digitalRead(IR3));
  }
  // Extreme correcting to the right
  else if(digitalRead(IR4)){
    motorsWrite(0,1,0,0);
    while(!digitalRead(IR2) && !digitalRead(IR3));
  }
  // Correcting to the left
  else if (digitalRead(IR2)) {
    motorsWrite(0.8, 0.1, 0, 0);
  } 
  // Correcting to the right
  else if (digitalRead(IR3)) {
    motorsWrite(0.1, 0.8, 0, 0);
  } 
  // Stop
  else {
    motorsWrite(0,0,0,0);
  }
}



//FUNCTIONS

void motorsWrite(float m1, float m2, float m3, float m4) {
  ledcWrite(Motor_F1, m1 * 255);
  ledcWrite(Motor_F2, m2 * 255);
  ledcWrite(Motor_B1, m3 * 255);
  ledcWrite(Motor_B2, m4 * 255);
}


void displayMessage(String title, String message)
{
  display.clearDisplay();               // Clear the OLED display
  display.setTextSize(1);              // Set text size
  display.setCursor(0, 0);             // Set cursor position
  display.println(title);              // Print title
  display.setCursor(0, 10);           // Move cursor to next line
  display.println(message);           // Print the message
  display.display();                   // Update the display
}