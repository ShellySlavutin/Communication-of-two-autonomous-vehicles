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
#define IR1 34
#define IR2 39
#define IR3 15
#define IR4 5

// Motor Pins
#define RF 13
#define RB 12
#define LF 25
#define LB 4

// PWM Settings
#define Resolution 8
#define Freq 1000

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
  ledcAttach(RF, Freq, Resolution);
  ledcAttach(LF, Freq, Resolution);
  ledcAttach(RB, Freq, Resolution);
  ledcAttach(LB, Freq, Resolution);

  delay(1000);
}

void loop() {

  if (digitalRead(IR2) && digitalRead(IR3)) {
    motorsWrite(0.4, 0.4, 0, 0);
    displayMessage("state:", "foward");
  } 

  // Extreme correcting to the left
  else if(digitalRead(IR4) && digitalRead(IR2) && digitalRead(IR3)){
    motorsWrite(1,0,0,0);
    displayMessage("state:", "extreme left");    
    while(digitalRead(IR2) && digitalRead(IR3));
  }
  // Extreme correcting to the right
  else if(digitalRead(IR1) && digitalRead(IR2) && digitalRead(IR3)){
    motorsWrite(0,1,0,0);
    displayMessage("state:", "extreme right");    
    while(digitalRead(IR2) && digitalRead(IR3));
  }
  // Correcting to the left
  else if (digitalRead(IR3)) {
    displayMessage("state:", "left");    
    motorsWrite(0.8, 0.1, 0, 0);
  } 
  // Correcting to the right
  else if (digitalRead(IR2)) {
    displayMessage("state:", "right");    
    motorsWrite(0.1, 0.8, 0, 0);
  } 
  // Stop
  else {
    stopMotors();
    displayMessage("state:", "stop");    
  }
}


//FUNCTIONS

void motorsWrite(float m1, float m2, float m3, float m4) {
  ledcWrite(RF, m1 * 255);
  ledcWrite(LF, m2 * 255);
  ledcWrite(RB, m3 * 255);
  ledcWrite(LB, m4 * 255);
}

void stopMotors(){
  ledcWrite(RF, 0);
  ledcWrite(LF, 0);
  ledcWrite(RB, 0);
  ledcWrite(LB, 0);
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