#include <Wire.h> //library which facilitates I2C communication used for the OLED
#include <Adafruit_GFX.h> //library for graphics functions 
#include <Adafruit_SH110X.h>
#define i2c_Address 0x3c // כתובת המסך
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //יצירת אובייקט לתקשורת עם מסך 

#define Motor_B1 12
#define Motor_F1 13
#define Motor_B2 4
#define Motor_F2 25

#define Resolution 8  // PWM resolution set 
#define Freq 1000  // PWM frequency set to 1000 Hz

//PWM chanels for each motor
#define PWM_CHANNEL_B1 0
#define PWM_CHANNEL_F1 1
#define PWM_CHANNEL_B2 2
#define PWM_CHANNEL_F2 3


void setup()
{
  Serial.begin(9600);

  // Configures PWM channels for each motor pin with specified frequency and resolution
  ledcAttachChannel(Motor_B1 , Freq, Resolution, PWM_CHANNEL_B1);
  ledcAttachChannel(Motor_F1 , Freq, Resolution, PWM_CHANNEL_F1);
  ledcAttachChannel(Motor_B2 , Freq, Resolution, PWM_CHANNEL_B2);
  ledcAttachChannel(Motor_F2 , Freq, Resolution, PWM_CHANNEL_F2);

  display.setTextColor(SH110X_WHITE); // Sets text color to white on the OLED display
  display.setTextSize(1.5);           // Sets the text size to 1.5 times the normal size
  display.begin(i2c_Address,true);    // Initializes the OLED display with the I2C address
  display.clearDisplay();             // Clears any content on the OLED screen
}
 

char character = ' ';
float speed = 1.0 ;

void StartMotorsMovment(char character) 
{

    if (character == '+')
      {
        speed = min(speed + 0.1, 1.0); // Increase speed, max 1.0
        Serial.println(speed);
             // Display speed
        display.clearDisplay();
        display.setCursor(1, 1); 
        display.print("The speed is higher: ");
        display.display();  
        display.print(speed);
        display.display(); 
     }

    else if (character == '-')
    {
        speed = max(speed - 0.1, 0.0); // Decrease speed, min 0.0
        Serial.println(speed);
               // Display speed
        display.clearDisplay();
        display.setCursor(1, 1); 
        display.print("The speed is lower: ");
        display.display();  
        display.print(speed);
        display.display(); 
    }
 

        ledcWriteChannel(PWM_CHANNEL_F1, speed*255);
        ledcWriteChannel(PWM_CHANNEL_F2, speed*255);

        ledcWriteChannel(PWM_CHANNEL_B1, 0);
        ledcWriteChannel(PWM_CHANNEL_B2, 0);
}


void loop()
{
  if(Serial.available())
  {
    Serial.println("Enter + to speed up, or - to slow down");
    character = Serial.read();
    StartMotorsMovment(character);
  }
}



