#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin (-1 if sharing Arduino reset pin)
#define i2c_Address 0x3C // Default I2C address for 128x64 OLED display

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup()
{
  // Initialize the display
  display.begin(i2c_Address,true); 
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
    
  // Display first string
  display.setTextSize(1); // Standard font size
  display.setCursor(2, 10); // Horizontal, vertical position
  display.println("Shelly and Sasha");

  // Display second string
  display.setTextSize(2); // Larger font size
  display.setCursor(5, 25); // Adjusted position
  display.println("Students");

  display.display(); // Show on OLED
}

void loop()
{

}

