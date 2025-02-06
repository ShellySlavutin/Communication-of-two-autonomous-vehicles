#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_NeoPixel.h>
#include "DFRobotDFPlayerMini.h" // library

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

//RGB pins
#define BLUE_RGB_PIN 2
#define RED_RGB_PIN 26
#define GREEN_RGB_PIN 27

#define NEOPIXEL_PIN 5 
#define NUM_PIXELS 6

#define LDR 34 //LDR pins

#define RECORDING_TIME 2000

HardwareSerial mp3Serial(1);  //  Defines UART1 for communicating with DFPlayer Mini
DFRobotDFPlayerMini mp3;      // Create an object to control mp3

float speed = 0.5; //speed for DC motors

bool flagOB = true; 
bool flagLDR = true; 
bool flagMSG = true;

char lastReceivedMsg[20] = ""; // Stores the last received message

uint8_t slaveAddress[] = {0xFC, 0xE8, 0xC0, 0x91, 0x6D, 0x54};

Adafruit_NeoPixel NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800); // Creating an object for the neoPixel

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Creating an object for communication with the OLED screen

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

  if (strcmp(receivedMsg, lastReceivedMsg) != 0) // Check if the message changed
    {
        strcpy(lastReceivedMsg, receivedMsg); // Update last received message
        flagMSG = true; // Reset the flag since the message has changed
    }

  if (strcmp(receivedMsg, "Stop received") == 0)
  {
    // Turn on blue light to indicate stopping
    digitalWrite(RED_RGB_PIN, LOW);
    digitalWrite(GREEN_RGB_PIN, LOW);
    digitalWrite(BLUE_RGB_PIN, HIGH);
    
    if(flagMSG)
    {
       delay (2000);
       mp3.play(4);// Play the third MP3 file (0003.mp3)
       flagMSG = false;
    }
  }

  else if (strcmp(receivedMsg, "Start received") == 0)
  {
    // Turn on green light to indicate stopping
    digitalWrite(RED_RGB_PIN, LOW);
    digitalWrite(GREEN_RGB_PIN, HIGH);
    digitalWrite(BLUE_RGB_PIN, LOW);
    
    if(flagMSG)
    {
      mp3.play(4);// Play the third MP3 file (0003.mp3)
      flagMSG = false;
    }

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

void stopMotors(bool flagOB)
{
  // stooping the motors is the same as setting the pwm to 0 which is what we did here
  speed = 0;
  ledcWriteChannel(PWM_CHANNEL_F1, speed*255);
  ledcWriteChannel(PWM_CHANNEL_F2, speed*255);

  ledcWriteChannel(PWM_CHANNEL_B1, 0);
  ledcWriteChannel(PWM_CHANNEL_B2, 0);
  if(flagOB)
  {
    mp3.play(1);// Play the first MP3 file (0001.mp3)
  }

}

void moveForward()
{
  speed = 0.5;
  ledcWriteChannel(PWM_CHANNEL_F1, speed*255);
  ledcWriteChannel(PWM_CHANNEL_F2, speed*255);

  ledcWriteChannel(PWM_CHANNEL_B1, 0);
  ledcWriteChannel(PWM_CHANNEL_B2, 0);
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
  
  if (distance < MIN_DISTANCE)
  {
    const char *message = "Stop";
    esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));
    stopMotors(flagOB);
    flagOB = false;
    displayDistance("Distance:" , distance);

  }

  else
  {
    const char *message = "Start";
    esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));
    moveForward();
    displayDistance("Distance:" , distance);
    flagOB = true;
  }

  delay(100);
}