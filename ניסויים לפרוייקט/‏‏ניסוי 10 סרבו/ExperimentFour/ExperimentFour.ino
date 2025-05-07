#include <ESP32Servo.h>
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
#define STRIP_SENSOR_1 32
#define STRIP_SENSOR_2 39
#define STRIP_SENSOR_3 15
#define STRIP_SENSOR_4 5

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

const int servoPin = 18;
const int time_for_delay = 2000;

Servo myServo;

void setup() {
  myServo.attach(servoPin);

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
  myServo.write(0);  // Rotate the servo to 0 degrees
  delay(time_for_delay);  // Wait for 2 seconds

  myServo.write(90);  // Rotate the servo to 90 degrees
  delay(time_for_delay);  // Wait for 2 seconds

  myServo.write(180);  // Rotate the servo to 180 degrees
  delay(time_for_delay);  // Wait for 2 seconds
  
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