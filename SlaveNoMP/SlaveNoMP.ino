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

// IR Sensor Pins
#define STRIP_SENSOR_1 32
#define STRIP_SENSOR_2 39
#define STRIP_SENSOR_3 15
#define STRIP_SENSOR_4 5

// Define the analog pin connected to the IR sensor
#define IR_SENSOR1_PIN 35 
#define IR_SENSOR2_PIN 36

#define LDR 34

float speed = 0.5;

String receivedMsg = " ";
char lastReceivedMsg[20] = ""; // Stores the last received message
uint8_t masterAddress[] = {0x08, 0xA6, 0xF7, 0x08, 0x3E, 0x98};

// NeoPixel and display objects
Adafruit_NeoPixel NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Connection tracking
bool isConnected = false;
unsigned long lastReceivedTime = 0;  // Time when the last message was received
const unsigned long TIMEOUT = 1000; // Timeout period in milliseconds

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


void backupStop() 
{
  int rawValue1 = digitalRead(IR_SENSOR1_PIN);
  int rawValue2 = digitalRead(IR_SENSOR2_PIN);

  if (rawValue1 == HIGH && rawValue2 == HIGH) 
  {
    moveAccordingToStrip();
  } 
  else 
  {
    stopMotors();
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

void stopMotors()
{
  speed = 0;
  ledcWriteChannel(PWM_CHANNEL_F1, speed * 255);
  ledcWriteChannel(PWM_CHANNEL_F2, speed * 255);
  ledcWriteChannel(PWM_CHANNEL_B1, 0);
  ledcWriteChannel(PWM_CHANNEL_B2, 0);

}

void moveForward()
{
  speed = 0.5;
  ledcWriteChannel(PWM_CHANNEL_F1, speed * 255);
  ledcWriteChannel(PWM_CHANNEL_F2, speed * 255);
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
    motorsWrite(0.4, 0.4, 0, 0);

    // Extreme correcting to the left at the start, when the middle sensors see the line
    if(digitalRead(STRIP_SENSOR_4))
    {
      motorsWrite(1,0,0,0);
      if (!digitalRead(STRIP_SENSOR_4)) // A case in which the turn is wide and no sensor can see the line
      {  
        while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)); // keep turning until it sees the line
      }
    }

    // Extreme correcting to the right at the start, when the middle sensors see the line
    if(digitalRead(STRIP_SENSOR_1))
    {
      motorsWrite(0,1,0,0);
      if (!digitalRead(STRIP_SENSOR_1)) // A case in which the turn is wide and no sensor can see the line
      {  
        while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)); // keep turning until it sees the line
      } 
    }
  } 

  // Extreme correcting to the left in the end, when only STRIP_SENSOR_4 is able to see the line
  else if(digitalRead(STRIP_SENSOR_4))
  {
    motorsWrite(1,0,0,0);
    if (!digitalRead(STRIP_SENSOR_4)) // A case in which the turn is wide and no sensor can see the line
    {  
      while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)); // keep turning until it sees the line
    }

  }

  // Extreme correcting to the right in the end, when only STRIP_SENSOR_1 is able to see the line
  else if(digitalRead(STRIP_SENSOR_1))
  {
    motorsWrite(0,1,0,0);
    if (!digitalRead(STRIP_SENSOR_1)) // A case in which the turn is wide and no sensor can see the line
    {  
      while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)); // keep turning until it sees the line
    }  
  }

  // Minor correcting to the left, when the car moves a bit, when there are twists.
  else if (digitalRead(STRIP_SENSOR_3))
  {
    motorsWrite(0.8, 0.1, 0, 0);
  } 

  // Minor correcting to the right, when the car moves a bit, when there are twists.
  else if (digitalRead(STRIP_SENSOR_2))
  {
    motorsWrite(0.1, 0.8, 0, 0);
  } 

  // Stop
  else
  {
    motorsWrite(0, 0, 0, 0);
  }
}

void onDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incomingData, int len)
{
  //flagBS = false; // As long as there is connection keep the BS flag off
  isConnected = true;
  lastReceivedTime = millis();

  char receivedMsg[len + 1];
  memcpy(receivedMsg, incomingData, len);
  receivedMsg[len] = '\0';

  if (strcmp(receivedMsg, "Stop") == 0)
  {
    displayMessage("Received:", receivedMsg);
    stopMotors();
    headLights(true);
    const char *message = "Stop received";
    esp_now_send(masterAddress, (uint8_t *)message, strlen(message));

  }

  else if (strcmp(receivedMsg, "Start") == 0)
  {
    displayMessage("Received:", receivedMsg);
    moveAccordingToStrip();
    headLights(false);
    const char *message = "Start received";
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
  delay(1000);

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
}

void loop()
{  
  // Check for connection timeout
  if (millis() - lastReceivedTime > TIMEOUT)
  {
    // No communiction so turn on the backup stop option
    isConnected = false;
  }

  if (!isConnected)
  {
    backupStop();
  }
}
