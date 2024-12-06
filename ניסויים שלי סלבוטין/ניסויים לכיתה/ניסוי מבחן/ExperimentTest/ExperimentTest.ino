#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Constants
#define i2c_Address 0x3c
#define SCREEN_WIDTH 128 //OLED display width in pixels
#define SCREEN_HEIGHT 64 // OLED display height in pixels
#define OLED_RESET -1 // QT-PY / XIAO
#define LDR_PIN 34

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup()
{
  // Initialize the display
  display.begin(i2c_Address,true); 
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  // Initialize the LDR
  pinMode(LDR_PIN, INPUT);
  display.clearDisplay();
}

void loop()
{
  // reset display settings
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  // check if it is dark outside -> if it is, write accordingly
  if(digitalRead(LDR_PIN)==HIGH)
  {
      display.clearDisplay(); 

      display.setTextSize(3);    
      display.setCursor(1, 10);    
      display.print("Light"); 

      display.display();
  }

  // check if there it is light outside -> if it is, write accordingly
  else if(digitalRead(LDR_PIN)==LOW)
  {  
      display.clearDisplay(); 

      display.setTextSize(3);    
      display.setCursor(1, 10);     
      display.print("Night"); 

      display.display(); 
  }

}


