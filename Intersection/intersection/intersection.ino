#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_NeoPixel.h>
#include "DFRobotDFPlayerMini.h" // library
#include <ESP32Servo.h>

// Servo Settings
Servo servo;

// OLED pins
#define i2c_Address 0x3c // OLED screen I2C address
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

//Ultrasonic Pins
#define triger_Pin 32
#define echo_Pin 33  

#define MIN_DISTANCE 10 // Minimum distance for stop function

//Motors pins
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

// IR Sensor Pins
#define STRIP_SENSOR_1 36
#define STRIP_SENSOR_2 39
#define STRIP_SENSOR_3 15
#define STRIP_SENSOR_4 5

//RGB pins
#define BLUE_RGB_PIN 2
#define RED_RGB_PIN 26
#define GREEN_RGB_PIN 27

#define NEOPIXEL_PIN 5 
#define NUM_PIXELS 6

#define LDR 34 //LDR pins

float speed = 0.5; //speed for DC motors

bool flagLDR = true; 

char lastReceivedMsg[20] = ""; // Stores the last received message

uint8_t slaveAddress[] = {0xFC, 0xE8, 0xC0, 0x91, 0x6D, 0x54};

Adafruit_NeoPixel NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800); // Creating an object for the neoPixel

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Creating an object for communication with the OLED screen

HardwareSerial mp3Serial(1);  //  Defines UART1 for communicating with DFPlayer Mini
DFRobotDFPlayerMini mp3;      // Create an object to control mp3


void displayDistance(String title, float distance)
{
  display.clearDisplay();               // Clear the OLED display
  display.setTextSize(1);              // Set text size
  display.setCursor(0, 20);             // Set cursor position
  display.println(title);              // Print title
  display.setCursor(0, 30);           // Move cursor to next line
  display.println(distance); 
  display.setCursor(20, 30); 
  display.print(" cm");           // Print the message
  display.display();                   // Update the display
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



float calculateDistance()
{
  digitalWrite(triger_Pin, LOW);
  delayMicroseconds(2);
  digitalWrite(triger_Pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triger_Pin, LOW);
  float Duration = pulseIn(echo_Pin, HIGH);
  float distance = Duration / 58.0;
  return distance;
}


void moveForward()
{
  speed = 0.5;
  ledcWriteChannel(PWM_CHANNEL_F1, speed*255);
  ledcWriteChannel(PWM_CHANNEL_F2, speed*255);

  ledcWriteChannel(PWM_CHANNEL_B1, 0);
  ledcWriteChannel(PWM_CHANNEL_B2, 0);
}

void motorsWrite(float m1, float m2, float m3, float m4) {
  ledcWrite(Motor_F1, m1 * 255);
  ledcWrite(Motor_F2, m2 * 255);
  ledcWrite(Motor_B1, m3 * 255);
  ledcWrite(Motor_B2, m4 * 255);
}

void moveAccordingToStrip()
{
  // Drive forward
   if (digitalRead(STRIP_SENSOR_2) && digitalRead(STRIP_SENSOR_3))
  {
    motorsWrite(0.2, 0.2, 0, 0);

  // Intersection
  if(digitalRead(STRIP_SENSOR_1) && digitalRead(STRIP_SENSOR_2) && digitalRead(STRIP_SENSOR_3) && digitalRead(STRIP_SENSOR_4)){
    displayMessage("intersection", "in");
    delay(200);
    motorsWrite(0, 0, 0, 0);

    // Measure the distance at every angle
    moveServo(0);
    float disR = calculateDistance();

    moveServo(90);
    float disF = calculateDistance();

    moveServo(180);
    float disL = calculateDistance();

    moveServo(90);

    // Determine the course
    if((disF > disR) && (disF > disL)){
      motorsWrite(0.2, 0.2, 0, 0);
      delay(1000);
    }
    else if((disR > disF) && (disR > disL)){
      motorsWrite(0,0.5,0,0);
      delay(1000);
      while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3));
    }
    else if((disL > disF) && (disL > disR)){
      motorsWrite(0.5,0,0,0);
      delay(1000);
      while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3));
    }
  }


  // Extreme correcting to the left at the start, when the middle sensors see the line
   if(digitalRead(STRIP_SENSOR_4))
   {
    motorsWrite(0.5,0,0,0);
   }

   // Extreme correcting to the right at the start, when the middle sensors see the line
   else if(digitalRead(STRIP_SENSOR_1))
   {
    motorsWrite(0,0.5,0,0);
   }
  } 

  // Extreme correcting to the left in the end, when only STRIP_SENSOR_4 is able to see the line
  else if(digitalRead(STRIP_SENSOR_4))
  {
    motorsWrite(0.5,0,0,0);
    if (!digitalRead(STRIP_SENSOR_4)) // A case in which the turn is wide and no sensor can see the line
    {  
      while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)); // keep turning until it sees the line
    }

  }

  // Extreme correcting to the right in the end, when only STRIP_SENSOR_1 is able to see the line
  else if(digitalRead(STRIP_SENSOR_1))
  {
    motorsWrite(0,0.5,0,0);
    if (!digitalRead(STRIP_SENSOR_1)) // A case in which the turn is wide and no sensor can see the line
    {  
      while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)); // keep turning until it sees the line
    }  
  }

  // Minor correcting to the left, when the car moves a bit, when there are twists.
  else if (digitalRead(STRIP_SENSOR_3))
  {
    motorsWrite(0.5, 0.1, 0, 0);
  } 

  // Minor correcting to the right, when the car moves a bit, when there are twists.
  else if (digitalRead(STRIP_SENSOR_2))
  {
    motorsWrite(0.1, 0.5, 0, 0);
  } 

  // Stop
  else
  {
    motorsWrite(0, 0, 0, 0);
  }
}

void moveServo(int angle) {
  servo.attach(18,500,2500);
  servo.write(angle);
  delay(500);
  servo.detach();
}

void setup()
{
  // Initialize the display
  display.begin(i2c_Address,true); 
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  
  pinMode(triger_Pin, OUTPUT);
  pinMode(echo_Pin, INPUT);

  ledcAttachChannel(Motor_B1 , Freq, Resolution, PWM_CHANNEL_B1);
  ledcAttachChannel(Motor_F1 , Freq, Resolution, PWM_CHANNEL_F1);
  ledcAttachChannel(Motor_B2 , Freq, Resolution, PWM_CHANNEL_B2);
  ledcAttachChannel(Motor_F2 , Freq, Resolution, PWM_CHANNEL_F2);

  pinMode(STRIP_SENSOR_1, INPUT);
  pinMode(STRIP_SENSOR_2, INPUT);
  pinMode(STRIP_SENSOR_3, INPUT);
  pinMode(STRIP_SENSOR_4, INPUT);


}


void loop()
{

  float distance = calculateDistance();
  
  if (distance < MIN_DISTANCE)
  {
    const char *message = "Stop";
    esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));
    stopMotors();

  }

  else
  {
    const char *message = "Start";
    esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));
    moveAccordingToStrip();
    displayDistance("Distance:" , distance);
  }

  delay(100);
}