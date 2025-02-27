#include <Wire.h>
#include <Adafruit_GFX.h> 
#include <Adafruit_SH110X.h>

#define i2c_Address 0x3c // OLED screen I2C address
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define STRIPE_SENSOR_1 34
#define STRIPE_SENSOR_2 39
#define STRIPE_SENSOR_3 15
#define STRIPE_SENSOR_4 5

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

float speed = 0.5;

void setup()
{
  pinMode(STRIPE_SENSOR_1, INPUT);
  pinMode(STRIPE_SENSOR_2, INPUT);
  pinMode(STRIPE_SENSOR_3, INPUT);
  pinMode(STRIPE_SENSOR_4, INPUT);

  display.begin(i2c_Address,true); 
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1); 

  ledcAttachChannel(Motor_B1 , Freq, Resolution, PWM_CHANNEL_B1);
  ledcAttachChannel(Motor_F1 , Freq, Resolution, PWM_CHANNEL_F1);
  ledcAttachChannel(Motor_B2 , Freq, Resolution, PWM_CHANNEL_B2);
  ledcAttachChannel(Motor_F2 , Freq, Resolution, PWM_CHANNEL_F2);
}

/*void turnsOnTheStripe()
{

}
*/

void loop()
{
    bool SENSOR1 = digitalRead(STRIPE_SENSOR_1);
    bool SENSOR2 = digitalRead(STRIPE_SENSOR_2);
    bool SENSOR3 = digitalRead(STRIPE_SENSOR_3);
    bool SENSOR4 = digitalRead(STRIPE_SENSOR_4);

    //bool sensorActive = false;

    display.clearDisplay();  // Clear buffer before printing new content
    display.setCursor(1, 5);

    if (!SENSOR2 && !SENSOR3) {
      display.println("No line detected - STOP");
      ledcWriteChannel(PWM_CHANNEL_F1, 0);
      ledcWriteChannel(PWM_CHANNEL_F2, 0);
      ledcWriteChannel(PWM_CHANNEL_B1, 0);
      ledcWriteChannel(PWM_CHANNEL_B2, 0);
    } 
    else if (SENSOR2 && SENSOR3) {
      display.println("Moving Forward");
      ledcWriteChannel(PWM_CHANNEL_F1, speed*255);
      ledcWriteChannel(PWM_CHANNEL_F2, speed*255);
      ledcWriteChannel(PWM_CHANNEL_B1, 0);
      ledcWriteChannel(PWM_CHANNEL_B2, 0);
    } 
    else if (SENSOR2) {
      display.println("Adjusting Slight RIGHT");
      // turn right
      ledcWriteChannel(PWM_CHANNEL_F1, 0);
      ledcWriteChannel(PWM_CHANNEL_F2, speed*255);
      ledcWriteChannel(PWM_CHANNEL_B1, speed*255);
      ledcWriteChannel(PWM_CHANNEL_B2, 0);    
    }
    else if (SENSOR3) {
      display.println("Adjusting Slight LEFT");
      // turn left
      ledcWriteChannel(PWM_CHANNEL_F1, speed*255);
      ledcWriteChannel(PWM_CHANNEL_F2, 0);
      ledcWriteChannel(PWM_CHANNEL_B1, 0);
      ledcWriteChannel(PWM_CHANNEL_B2, speed*255);    
    }

    
    /*if(SENSOR2==false)
    {
      // turn left
      ledcWriteChannel(PWM_CHANNEL_F1, speed*255);
      ledcWriteChannel(PWM_CHANNEL_F2, 0);
      ledcWriteChannel(PWM_CHANNEL_B1, 0);
      ledcWriteChannel(PWM_CHANNEL_B2, speed*255);
    }
    

    if(SENSOR3==false)
    {
      // turn right
      ledcWriteChannel(PWM_CHANNEL_F1, 0);
      ledcWriteChannel(PWM_CHANNEL_F2, speed*255);
      ledcWriteChannel(PWM_CHANNEL_B1, speed*255);
      ledcWriteChannel(PWM_CHANNEL_B2, 0);

    }

    if(SENSOR2)
    {
      ledcWriteChannel(PWM_CHANNEL_F1, speed*255);
      ledcWriteChannel(PWM_CHANNEL_F2, speed*255);
      ledcWriteChannel(PWM_CHANNEL_B1, 0);
      ledcWriteChannel(PWM_CHANNEL_B2, 0);

      display.println("Second sensor from the right");
      //sensorActive = true;
    }

    if(SENSOR3)
    {
      ledcWriteChannel(PWM_CHANNEL_F1, speed*255);
      ledcWriteChannel(PWM_CHANNEL_F2, speed*255);
      ledcWriteChannel(PWM_CHANNEL_B1, 0);
      ledcWriteChannel(PWM_CHANNEL_B2, 0);

      display.println("Second sensor from the left");
      //sensorActive = true;
    } */
  
 /*   if(SENSOR1)
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
*/

    display.display(); 
    delay(20); 

}