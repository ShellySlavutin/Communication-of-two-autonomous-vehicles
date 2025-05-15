#include <Wire.h> //library which facilitates I2C communication used for the OLED
#include <Adafruit_GFX.h> //library for graphics functions 
#include <Adafruit_SH110X.h>
#include <ESP32Servo.h>

#define i2c_Address 0x3c // Screen address
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Creating an object for communication with the screen 

// Servo
Servo servo;

void setup()
{
  display.setTextColor(SH110X_WHITE); // Setting font color
  display.setTextSize(1.5);  // Setting font size
  display.begin(i2c_Address,true); // Initializing 
  display.clearDisplay(); 

  servo.attach(18);

}


void displayAngle(int angle)
{
  display.clearDisplay(); 
  display.setCursor(1, 10); 
  display.print("Angle: ");
  display.print(angle); 
  display.display(); 
}

void loop()
{
  servo.write(90);
  displayAngle(90);
  delay(500);

  servo.write(180);
  displayAngle(180);
  delay(500);
  
  servo.write(0);
  displayAngle(0);
  delay(500);
}
