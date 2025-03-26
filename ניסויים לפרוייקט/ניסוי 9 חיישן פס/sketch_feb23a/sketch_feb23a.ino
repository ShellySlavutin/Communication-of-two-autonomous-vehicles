#include <Wire.h>
#include <Adafruit_GFX.h> 
#include <Adafruit_SH110X.h>

#define i2c_Address 0x3c // OLED screen I2C address
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define STRIP_SENSOR_1 32
#define STRIP_SENSOR_2 39
#define STRIP_SENSOR_3 15
#define STRIP_SENSOR_4 5

void setup()
{
  pinMode(STRIP_SENSOR_1, INPUT);
  pinMode(STRIP_SENSOR_2, INPUT);
  pinMode(STRIP_SENSOR_3, INPUT);
  pinMode(STRIP_SENSOR_4, INPUT);

  display.begin(i2c_Address,true); 
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1); 
}

void loop()
{
    bool SENSOR1 = digitalRead(STRIP_SENSOR_1);
    bool SENSOR2 = digitalRead(STRIP_SENSOR_2);
    bool SENSOR3 = digitalRead(STRIP_SENSOR_3);
    bool SENSOR4 = digitalRead(STRIP_SENSOR_4);

    bool sensorActive = false;

    display.clearDisplay();  // Clear buffer before printing new content
    display.setCursor(1, 5);
  
    if(SENSOR1)
    {
      display.println("First sensor from the right");
      sensorActive = true;
    }

    if(SENSOR2)
    {
      display.println("Second sensor from the right");
      sensorActive = true;
    }

      
    if(SENSOR4)
    {
      display.println("First sensor from the left");
      sensorActive = true;
    }

      
    if(SENSOR3)
    {
      display.println("Second sensor from the left");
      sensorActive = true;
    }

    else if(!sensorActive)
    {
      display.println("No line");
    }

    display.display(); 
    delay(200); 

}