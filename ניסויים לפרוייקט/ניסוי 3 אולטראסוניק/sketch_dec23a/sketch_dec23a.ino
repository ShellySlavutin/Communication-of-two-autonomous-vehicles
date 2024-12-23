#include <Wire.h> //library which facilitates I2C communication used for the OLED
#include <Adafruit_GFX.h> //library for graphics functions 
#include <Adafruit_SH110X.h>
#define i2c_Address 0x3c // Screen address
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Creating an object for communication with the screen 

#define triger_Pin 32
#define echo_Pin 33

void setup()
{
  display.setTextColor(SH110X_WHITE); // Setting font color
  display.setTextSize(1.5);  // Setting font size
  display.begin(i2c_Address,true); // Initializing 
  display.clearDisplay(); 

  pinMode(triger_Pin, OUTPUT); 
  pinMode(echo_Pin, INPUT); 

}

void loop()
{
  // Activating the trigger signal
  digitalWrite(triger_Pin, LOW); // Resetting the trigger
  delayMicroseconds(2); 
  digitalWrite(triger_Pin, HIGH); // Activating the trigger
  delayMicroseconds(10); 
  digitalWrite(triger_Pin, LOW); // Resetting the trigger
  
  // Calculating the distance
  float Duration = pulseIn(echo_Pin, 1); 
  float space = Duration/58.0; 

  // Displaying data on screen
  display.clearDisplay(); 
  display.setCursor(1, 10); 
  display.print("Distance: ");
  display.print(space); 
  display.print("cm");
  display.display(); 

}

