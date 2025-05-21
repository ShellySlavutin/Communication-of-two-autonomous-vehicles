#include <Wire.h>
#include <Adafruit_GFX.h> 
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_NeoPixel.h>
#include "DFRobotDFPlayerMini.h" 

// OLED pins
#define i2c_Address 0x3c // OLED screen I2C address
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 

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

#define NEOPIXEL_PIN 5 
#define NUM_PIXELS 6

// Blinker Settings
#define interval 300
unsigned long previousT = 0;
bool blinkState = 0;

// IR Sensor Pins
#define STRIP_SENSOR_1 32
#define STRIP_SENSOR_2 39
#define STRIP_SENSOR_3 15
#define STRIP_SENSOR_4 33


// Define the analog pin connected to the IR sensor
#define IR_SENSOR1_PIN 35 
#define IR_SENSOR2_PIN 36

#define LDR 34

float speed = 0.7;

String receivedMsg = " ";
char lastReceivedMsg[20] = ""; // Stores the last received message
uint8_t masterAddress[] = {0x08, 0xA6, 0xF7, 0x08, 0x3E, 0x98};

HardwareSerial mp3Serial(1);  //  Defines UART1 for communicating with DFPlayer Mini
DFRobotDFPlayerMini mp3;      // Create an object to control mp3

bool flagLDR = true; 

// NeoPixel and display objects
Adafruit_NeoPixel NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Connection tracking
bool isConnected = false;
bool isMasterStopped = false;
bool isIRtracking = false;
unsigned long lastReceivedTime = 0;  // Time when the last message was received

const unsigned long TIMEOUT = 8000; // Timeout period in milliseconds
char intersectionTurn = 'z'; // The direction to turn to while in an itersection

void headLights(bool isStopped)
{
  NeoPixel.clear();
  if ((digitalRead(LDR) == LOW && isStopped == true) || (digitalRead(LDR) == LOW))
  {
    NeoPixel.setPixelColor(0, NeoPixel.Color(255, 255, 150));  
    NeoPixel.setPixelColor(1, NeoPixel.Color(255, 255, 150));  
    NeoPixel.setPixelColor(2, NeoPixel.Color(255, 255, 150));  
    NeoPixel.setPixelColor(3, NeoPixel.Color(255, 255, 150)); 
    NeoPixel.setPixelColor(4, NeoPixel.Color(255, 0, 0));  
    NeoPixel.setPixelColor(5, NeoPixel.Color(255, 0, 0));  
    NeoPixel.show();
  }
  else if (digitalRead(LDR) == HIGH && isStopped == true)
  {
    NeoPixel.setPixelColor(4, NeoPixel.Color(255, 0, 0));  
    NeoPixel.setPixelColor(5, NeoPixel.Color(255, 0, 0));  
    NeoPixel.show();
  }
  else
  {
    NeoPixel.clear();
    NeoPixel.show(); 
  }
}


void leds(int i, int f, int r, int g, int b){
  for (i; i <= f; i++) {
    NeoPixel.setPixelColor(i, r, g, b);
  }
  NeoPixel.show();
}

void blinker(char dir){
  unsigned long currentT = millis();
  if(currentT - previousT >= interval){
    previousT = currentT;
    if(!blinkState){
      blinkState = 1;
      switch(dir){
        case 'R':
          NeoPixel.setPixelColor(3,100,50,0);
          break;
        case 'L':
          NeoPixel.setPixelColor(2,100,50,0);
          break;
      }
      NeoPixel.show();
    }else{
      leds(2,3,0,0,0);
      blinkState = 0;
    }
  }
}

void dayNightMode()
{
  if ((digitalRead(LDR) == LOW))
  {
    if(flagLDR)
    {
      mp3.play(2);// Play the second MP3 file (0002.mp3)
      delay(2000);
      flagLDR = false;
    }
  }

  else
  {
    if(flagLDR)
    {
      mp3.play(3);// Play the third MP3 file (0003.mp3)
      delay(2000);
      flagLDR = false;
    }
  }
}


void displayMessage(String title, String message)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(title);
  display.setCursor(0, 10);
  display.println(message);
  display.display();
}

void motorsWrite(float m1, float m2, float m3, float m4) {
  ledcWrite(Motor_F1, m1 * 255);
  ledcWrite(Motor_F2, m2 * 255);
  ledcWrite(Motor_B1, m3 * 255);
  ledcWrite(Motor_B2, m4 * 255);
}

void moveAccordingToStrip()
{
  
  if(digitalRead(STRIP_SENSOR_1) && digitalRead(STRIP_SENSOR_2) && digitalRead(STRIP_SENSOR_3) && digitalRead(STRIP_SENSOR_4))
  {
    blinker(intersectionTurn);
    displayMessage("state:", "intersection");

    if (intersectionTurn == 'F')
    {
      motorsWrite(0.7, 0.7, 0, 0);
      displayMessage("state:", "foward1");
    }
    if (intersectionTurn == 'R')
    {
      motorsWrite(0,0.8,0,0);
      delay(1000);
      displayMessage("state:", "extreme right1");
      unsigned long startTime = millis();
      while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)) // keep turning until it sees the line
      {
        if (millis() - startTime > 2000) break; // max wait 2s
      }
    }
    if (intersectionTurn == 'L')
    {
      motorsWrite(0.8,0,0,0);
      delay(1000);
      displayMessage("state:", "extreme left1");
      unsigned long startTime = millis();
      while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)) // keep turning until it sees the line
      {
        if (millis() - startTime > 2000) break; // max wait 2s
      }
    }
  }

  else if (digitalRead(STRIP_SENSOR_2) && digitalRead(STRIP_SENSOR_3))
  {
    motorsWrite(0.7, 0.7, 0, 0);
    displayMessage("state:", "foward");
  } 

  // Extreme correcting to the left in the end, when only STRIP_SENSOR_4 is able to see the line
  else if(digitalRead(STRIP_SENSOR_4))
  {
    motorsWrite(0.8,0,0,0);
    displayMessage("state:", "extreme left");    
    while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)) // keep turning until it sees the line
    {
      displayMessage("state:", "extreme left loop");
      motorsWrite(0.8,0,0,0);
    }

  }

  // Extreme correcting to the right in the end, when only STRIP_SENSOR_1 is able to see the line
  else if(digitalRead(STRIP_SENSOR_1))
  {
    motorsWrite(0,0.8,0,0);
    displayMessage("state:", "extreme right");  
    while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)) // keep turning until it sees the line
    {
      displayMessage("state:", "extreme right loop");
      motorsWrite(0,0.8,0,0);
    }
     
  }

  // Minor correcting to the left, when the car moves a bit, when there are twists.
  else if (digitalRead(STRIP_SENSOR_3))
  {
    displayMessage("state:", "left");    
    motorsWrite(0.8, 0.5, 0, 0);
  } 

  // Minor correcting to the right, when the car moves a bit, when there are twists.
  else if (digitalRead(STRIP_SENSOR_2))
  {
    displayMessage("state:", "right");    
    motorsWrite(0.5, 0.8, 0, 0);
  } 

  // Stop
  else
  {
    displayMessage("state:", "stop");    
    motorsWrite(0, 0, 0, 0);

    headLights(true);
  }

}

void onDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incomingData, int len)
{
  isConnected = true;
  lastReceivedTime = millis();

  char receivedMsg[len + 1];
  memcpy(receivedMsg, incomingData, len);
  receivedMsg[len] = '\0';

  if (strcmp(receivedMsg, "Start") == 0)
  {
    displayMessage("Received:", receivedMsg);
    isMasterStopped = false;
    const char *message = "Start received";
    esp_now_send(masterAddress, (uint8_t *)message, strlen(message));
  }

  else if (strcmp(receivedMsg, "Stop course") == 0)
  {
    displayMessage("Received:", receivedMsg);
    isMasterStopped = true;
    isIRtracking = true;
    const char *message = "Stop received";
    esp_now_send(masterAddress, (uint8_t *)message, strlen(message));
  }

  else if (strcmp(receivedMsg, "Stop") == 0)
  {
    displayMessage("Received:", receivedMsg);
    isMasterStopped = true;
    const char *message = "Stop received";
    esp_now_send(masterAddress, (uint8_t *)message, strlen(message));

  }

  else if (strcmp(receivedMsg, "F") == 0 || strcmp(receivedMsg, "R") == 0 || strcmp(receivedMsg, "L") == 0)
  {
    displayMessage("Received:", receivedMsg);
    intersectionTurn = receivedMsg[0];

    char message[10];
    snprintf(message, sizeof(message), "%c received", receivedMsg[0]);
    esp_now_send(masterAddress, (uint8_t *)message, strlen(message));
  }


}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{

}

void setup()
{
  display.begin(i2c_Address, true); 
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    displayMessage("Error", "Init ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
  
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, masterAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    displayMessage("Error", "Add Peer Fail");
    return;
  }

  ledcAttachChannel(Motor_B1, Freq, Resolution, PWM_CHANNEL_B1);
  ledcAttachChannel(Motor_F1, Freq, Resolution, PWM_CHANNEL_F1);
  ledcAttachChannel(Motor_B2, Freq, Resolution, PWM_CHANNEL_B2);
  ledcAttachChannel(Motor_F2, Freq, Resolution, PWM_CHANNEL_F2);

  NeoPixel.begin();
  NeoPixel.clear();
  NeoPixel.show();

  pinMode(STRIP_SENSOR_1, INPUT);
  pinMode(STRIP_SENSOR_2, INPUT);
  pinMode(STRIP_SENSOR_3, INPUT);
  pinMode(STRIP_SENSOR_4, INPUT);

  pinMode(LDR, INPUT);
  pinMode(IR_SENSOR1_PIN, INPUT);
  pinMode(IR_SENSOR2_PIN, INPUT);

  // Start Serial communication with DFPlayer Mini
  mp3Serial.begin(9600, SERIAL_8N1, 16, 17);  //rx=16, tx=17

  mp3.begin(mp3Serial); // Initialize DFPlayer Mini
  mp3.volume(30); // Set volume to 30  
}

void loop()
{  
  dayNightMode();

  // Check for connection timeout
  if (millis() - lastReceivedTime > TIMEOUT)
  {
    // No communiction so turn on the backup stop option
    isConnected = false;
  }

  if (!isConnected || isIRtracking)
  {
    if (digitalRead(IR_SENSOR1_PIN) == LOW || digitalRead(IR_SENSOR2_PIN) == LOW)
    {
      motorsWrite(0, 0, 0, 0);
      headLights(true);
    }
    else 
    {
      moveAccordingToStrip();
      headLights(false);
    }
  }
  else if (isConnected && isMasterStopped)
  {
    motorsWrite(0, 0, 0, 0);
    headLights(isMasterStopped);
  }
  else if (isConnected && !isMasterStopped)
  {
    moveAccordingToStrip();
    headLights(isMasterStopped);
  }

}