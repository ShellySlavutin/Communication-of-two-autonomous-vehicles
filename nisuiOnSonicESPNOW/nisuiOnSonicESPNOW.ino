#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include <esp_now.h>

// OLED pins
#define i2c_Address 0x3c // OLED screen I2C address
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define triger_Pin 32
#define echo_Pin 33  

#define MIN_DISTANCE 10

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

// Creating an object for communication with the OLED screen
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t slaveAddress[] = {0xFC, 0xE8, 0xC0, 0x91, 0x6D, 0x54};

void displayDistance(String title, float distance)
{
  display.clearDisplay();               // Clear the OLED display
  display.setTextSize(1);              // Set text size
  display.setCursor(0, 30);             // Set cursor position
  display.println(title);              // Print title
  display.setCursor(0, 40);           // Move cursor to next line
  display.println(distance);           // Print the message
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
  String statusMsg = (status == ESP_NOW_SEND_SUCCESS) ? "Success" : "Fail";
  displayMessage("Send Status:", statusMsg);
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

void stopMotors()
{
  // stooping the motors is the same as setting the pwm to 0 which is what we did here
  speed = 0;
  ledcWriteChannel(PWM_CHANNEL_F1, speed*255);
  ledcWriteChannel(PWM_CHANNEL_F2, speed*255);

  ledcWriteChannel(PWM_CHANNEL_B1, 0);
  ledcWriteChannel(PWM_CHANNEL_B2, 0);
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
}


void loop()
{
/*  const char *message = "Hello from Master";
  esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));
  //displayMessage("Sent:", "Hello from Master");
  delay(2000);
*/
  float distance = calculateDistance();
  if (distance < MIN_DISTANCE)
  {
    const char *message = "Stop";
    esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));
    stopMotors();
    displayDistance("Distance:" , distance);
  }

  else
  {
    const char *message = "Start";
    esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));
    moveForward();
    displayDistance("Distance:" , distance);
  }

  delay(100);
}

