#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_NeoPixel.h>
#include "DFRobotDFPlayerMini.h" //
#include <ESP32Servo.h>

// Servo
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
#define STRIP_SENSOR_4 35

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

void headLights(float distance)
{
  NeoPixel.clear();
  if ((digitalRead(LDR) == LOW && distance < MIN_DISTANCE) || (digitalRead(LDR) == LOW)) // Night and stop or night -> all leds
  {
    // Turn on all the light since it is noght and stop the lights

    // The front leds will be white (because of the blue tint we put it on 150)
    NeoPixel.setPixelColor(0, NeoPixel.Color(255, 255, 150));  
    NeoPixel.setPixelColor(1, NeoPixel.Color(255, 255, 150));  
    NeoPixel.setPixelColor(2, NeoPixel.Color(255, 255, 150));  
    NeoPixel.setPixelColor(3, NeoPixel.Color(255, 255, 150)); 

    // The back leds will be red
    NeoPixel.setPixelColor(4, NeoPixel.Color(255, 0, 0));  
    NeoPixel.setPixelColor(5, NeoPixel.Color(255, 0, 0));  

    NeoPixel.show(); // update to the NeoPixel Led Strip

  }
  
  else if (digitalRead(LDR) == HIGH && distance < MIN_DISTANCE) // Light and stop -> only back leds
  {
    // Only the back leds will be red
    NeoPixel.setPixelColor(4, NeoPixel.Color(255, 0, 0));  
    NeoPixel.setPixelColor(5, NeoPixel.Color(255, 0, 0));  
    
    NeoPixel.show(); // update to the NeoPixel Led Strip
  }

  else
  {
    NeoPixel.clear();
    NeoPixel.show(); 
  }

}


void dayNightMode()
{
  if ((digitalRead(LDR) == LOW))
  {
    if(flagLDR)
    {
      mp3.play(2);// Play the second MP3 file (0002.mp3)
      flagLDR = false;
    }
  }

  else
  {
    if(flagLDR)
    {
      mp3.play(3);// Play the third MP3 file (0003.mp3)
      flagLDR = false;
    }
  }
}

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


void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{

}


void onDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incomingData, int len)
{
  char receivedMsg[len + 1];
  memcpy(receivedMsg, incomingData, len);
  receivedMsg[len] = '\0';

  if (strcmp(receivedMsg, "Stop received") == 0)
  {
    // Turn on blue light to indicate stopping
    digitalWrite(RED_RGB_PIN, LOW);
    digitalWrite(GREEN_RGB_PIN, LOW);
    digitalWrite(BLUE_RGB_PIN, HIGH);
  }

  else if (strcmp(receivedMsg, "Start received") == 0)
  {
    // Turn on green light to indicate stopping
    digitalWrite(RED_RGB_PIN, LOW);
    digitalWrite(GREEN_RGB_PIN, HIGH);
    digitalWrite(BLUE_RGB_PIN, LOW);
  }
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

void motorsWrite(float m1, float m2, float m3, float m4) {
  ledcWrite(Motor_F1, m1 * 255);
  ledcWrite(Motor_F2, m2 * 255);
  ledcWrite(Motor_B1, m3 * 255);
  ledcWrite(Motor_B2, m4 * 255);
}

void moveServo(int angle) {
  servo.attach(18,500,2500);
  servo.write(angle);
  delay(500);
  servo.detach();
}

void setup()
{
  // Initialize all LEDs as output
  pinMode(RED_RGB_PIN, OUTPUT);
  pinMode(GREEN_RGB_PIN, OUTPUT);
  pinMode(BLUE_RGB_PIN, OUTPUT);

  // Turn all LEDs off initially
  digitalWrite(RED_RGB_PIN, LOW);
  digitalWrite(GREEN_RGB_PIN, LOW);
  digitalWrite(BLUE_RGB_PIN, LOW);

  // Initialize the display
  display.begin(i2c_Address,true); 
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    displayMessage("Error", "Init ESP-NOW");
    return;
  }

  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, slaveAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    displayMessage("Error", "Add Peer Fail");
    return;
  }
  
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

  NeoPixel.begin(); // initialize NeoPixel strip object (REQUIRED)
  delay (200);

  NeoPixel.clear(); // Start the program with lights off
  NeoPixel.show(); // update to the NeoPixel Led Strip

  pinMode(LDR, INPUT); // Init the LDR

  // Start Serial communication with DFPlayer Mini
  mp3Serial.begin(9600, SERIAL_8N1, 16, 17);  //rx=16, tx=17

  mp3.begin(mp3Serial); // Initialize DFPlayer Mini
  mp3.volume(30); // Set volume to 30
}


void loop()
{
  dayNightMode();

  float distance = calculateDistance();
  headLights(distance);

  /*
  display.setCursor(2,10);
  display.print(digitalRead(STRIP_SENSOR_1));
  display.display();

  display.setCursor(2,20);
  display.print(digitalRead(STRIP_SENSOR_2));
  display.display();

  display.setCursor(2,30);
  display.print(digitalRead(STRIP_SENSOR_3));
  display.display();

  display.setCursor(2,40);
  display.print(digitalRead(STRIP_SENSOR_4));
  display.display();*/


  if(digitalRead(STRIP_SENSOR_1) && digitalRead(STRIP_SENSOR_2) && digitalRead(STRIP_SENSOR_3) && digitalRead(STRIP_SENSOR_4))
  {
    bool Tintersection = true;
    delay(200);

    if (!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3))
    {
      // There is no line, do not enter the forward if
      Tintersection = false;
    }

    motorsWrite(0, 0, 0, 0);

    const char *message = "Stop";
    esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));

    delay(500);

    display.setCursor(2,10);
    display.print("stop");
    display.display();

    // Measure the distance at every angle
    moveServo(0);
    moveServo(0);
    delay(500);

    float disR = calculateDistance();
    display.setCursor(2,20);
    display.print("0 - " + String(disR));
    display.display();
    delay(500);

    moveServo(90);
    moveServo(90);
    delay(500);
    
    float disF = calculateDistance();
    display.setCursor(2,30);
    display.print("90 - " + String(disF));
    display.display();
    delay(500);

    moveServo(180);
    moveServo(180);
    delay(500);

    float disL = calculateDistance();
    display.setCursor(2,40);
    display.print("180 - " + String(disL));
    display.display();
    delay(500);

    moveServo(90);

    // Determine the course
    if((disF > disR) && (disF > disL) && Tintersection){
      const char *message = "F";
      esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));

      motorsWrite(0.2, 0.2, 0, 0);
      delay(1000);
    }
    else if((disR > disF) && (disR > disL)){
      const char *message = "R";
      esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));

      motorsWrite(0,0.5,0,0);
      delay(1000);
      while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)){
      }
    }
    else if((disL > disF) && (disL > disR)){
      const char *message = "L";
      esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));

      motorsWrite(0.5,0,0,0);
      delay(1000);
      while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)){
      }
    }
  }

  // If car is on course
  else if (digitalRead(STRIP_SENSOR_2) && digitalRead(STRIP_SENSOR_3))
  {
    const char *message = "Start";
    esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));

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
    const char *message = "Stop";
    esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));

    displayMessage("state:", "stop");    
    motorsWrite(0, 0, 0, 0);
  }
}