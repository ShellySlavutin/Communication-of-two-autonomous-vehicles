#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_NeoPixel.h>
#include "DFRobotDFPlayerMini.h"

// OLED pins
#define i2c_Address 0x3c // OLED screen I2C address
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Creating an object for communication with the OLED screen

// IR Sensor Pins
#define STRIP_SENSOR_1 32
#define STRIP_SENSOR_2 39
#define STRIP_SENSOR_3 15
#define STRIP_SENSOR_4 33

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

#define NEOPIXEL_PIN 5 
#define NUM_PIXELS 6

#define LDR 34 //LDR pins

Adafruit_NeoPixel NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800); // Creating an object for the neoPixel


void setup() {
  // Initialize the display
  display.begin(i2c_Address,true); 
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  pinMode(STRIP_SENSOR_1, INPUT);
  pinMode(STRIP_SENSOR_2, INPUT);
  pinMode(STRIP_SENSOR_3, INPUT);
  pinMode(STRIP_SENSOR_4, INPUT);

  // Set up PWM channels for motors
  ledcAttach(Motor_F1, Freq, Resolution);
  ledcAttach(Motor_F2, Freq, Resolution);
  ledcAttach(Motor_B1, Freq, Resolution);
  ledcAttach(Motor_B2, Freq, Resolution);


  delay(1000);
}

void loop() 
{
  if (digitalRead(STRIP_SENSOR_2) && digitalRead(STRIP_SENSOR_3))
  {
    motorsWrite(0.2, 0.2, 0, 0);
    displayMessage("state:", "foward");

  // Extreme correcting to the left at the start, when the middle sensors see the line
   if(digitalRead(STRIP_SENSOR_4))
   {
    motorsWrite(0.5,0,0,0);
    displayMessage("state:", "extreme left");  
   }

   // Extreme correcting to the right at the start, when the middle sensors see the line
   else if(digitalRead(STRIP_SENSOR_1))
   {
    motorsWrite(0,0.5,0,0);
    displayMessage("state:", "extreme right");   
   }
  } 

  // Extreme correcting to the left in the end, when only STRIP_SENSOR_4 is able to see the line
  else if(digitalRead(STRIP_SENSOR_4))
  {
    motorsWrite(0.5,0,0,0);
    displayMessage("state:", "extreme left");    
    if (!digitalRead(STRIP_SENSOR_4)) // A case in which the turn is wide and no sensor can see the line
    {  
      while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)); // keep turning until it sees the line
    }

  }

  // Extreme correcting to the right in the end, when only STRIP_SENSOR_1 is able to see the line
  else if(digitalRead(STRIP_SENSOR_1))
  {
    motorsWrite(0,0.5,0,0);
    displayMessage("state:", "extreme right");  
    if (!digitalRead(STRIP_SENSOR_1)) // A case in which the turn is wide and no sensor can see the line
    {  
      while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)); // keep turning until it sees the line
    }  
  }

  // Minor correcting to the left, when the car moves a bit, when there are twists.
  else if (digitalRead(STRIP_SENSOR_3))
  {
    displayMessage("state:", "left");    
    motorsWrite(0.5, 0.1, 0, 0);
  } 

  // Minor correcting to the right, when the car moves a bit, when there are twists.
  else if (digitalRead(STRIP_SENSOR_2))
  {
    displayMessage("state:", "right");    
    motorsWrite(0.1, 0.5, 0, 0);
  } 

  // Stop
  else
  {
    displayMessage("state:", "stop");    
    motorsWrite(0, 0, 0, 0);

    // Light up brake lights manually
    NeoPixel.setPixelColor(4, NeoPixel.Color(255, 0, 0));  
    NeoPixel.setPixelColor(5, NeoPixel.Color(255, 0, 0));  
    NeoPixel.show();
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

