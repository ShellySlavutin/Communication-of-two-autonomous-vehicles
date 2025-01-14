/*
#include <Wire.h> // Library for I2C communication used for the OLED
#include <Adafruit_GFX.h> // Library for graphics functions
#include <Adafruit_SH110X.h>

#include <esp_now.h>

#include <WiFi.h>

#define i2c_Address 0x3c // OLED screen I2C address
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 

#define triger_Pin 32 // Trigger pin
#define echo_Pin 33   // Echo pin

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

float speed = 0.5 ;
float space;
char command;
char reply;


// Creating an object for communication with the OLED screen
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t slaveAddress[] = {0xFC, 0xE8, 0xC0, 0x91, 0x6D, 0x54};

// recv msg struct
typdef struct msgStruct
{
  string Data;
} msgStruct;


void sendCommand(char *command)
{
  esp_now_send(slaveAddress, uint8_t *command, strlen(command) + 1);
}
 

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

float calculateDistance()
{
  // Sending the trigger signal
  digitalWrite(triger_Pin, LOW); // Resetting the trigger
  delayMicroseconds(2);          // Waiting for 2 microseconds
  digitalWrite(triger_Pin, HIGH); // Sending the trigger pulse
  delayMicroseconds(10);         // Sending a pulse for 10 microseconds
  digitalWrite(triger_Pin, LOW); // Resetting the trigger again
  
  // Calculating the distance
  float Duration = pulseIn(echo_Pin, 1); // Measuring the pulse duration from the echo pin
  space = Duration / 58.0;         // Calculating distance in cm based on the pulse duration

  return space;
}

void stopMotors()
{
    // Stop both motor sets
        ledcWriteChannel(PWM_CHANNEL_F1, 0);
        ledcWriteChannel(PWM_CHANNEL_F2, 0);

        ledcWriteChannel(PWM_CHANNEL_B1, 0);
        ledcWriteChannel(PWM_CHANNEL_B2, 0);
}

void moveForward()
{
    // Move forward for both motor sets
        ledcWriteChannel(PWM_CHANNEL_F1, speed*255);
        ledcWriteChannel(PWM_CHANNEL_F2, speed*255);

        ledcWriteChannel(PWM_CHANNEL_B1, 0);
        ledcWriteChannel(PWM_CHANNEL_B2, 0);
}

void displayData(String reply, float space)
{
    display.clearDisplay();               // Clear the OLED display
    display.setTextSize(1.5);             // Set text size

    // Display the replay on the first line
    display.setCursor(0, 10);             // Set cursor position
    display.print("Replay received: ");  // Print label
    display.setCursor(0, 20);             // Move cursor to the next line
    display.print(reply);               // Print the command

    // Display the distance on the second line
    display.setCursor(3, 30);             // Setting cursor position
    display.print("Distance: ");
    display.print(space);                 // Printing the distance measured by the sensor on the screen
    display.print("cm");

    display.display();                    // Update the display
}



void setup()
{
  Serial.begin(9600);

  display.setTextColor(SH110X_WHITE); // Setting font color
  display.setTextSize(1.5);  // Setting font size
  display.begin(i2c_Address, true); // Initializing the OLED screen
  display.clearDisplay(); 

  // Initialize the ultrasonic pins
  pinMode(triger_Pin, OUTPUT); 
  pinMode(echo_Pin, INPUT);   

  // Initialize the motors
  ledcAttachChannel(Motor_B1 , Freq, Resolution, PWM_CHANNEL_B1);
  ledcAttachChannel(Motor_F1 , Freq, Resolution, PWM_CHANNEL_F1);
  ledcAttachChannel(Motor_B2 , Freq, Resolution, PWM_CHANNEL_B2);
  ledcAttachChannel(Motor_F2 , Freq, Resolution, PWM_CHANNEL_F2);

   if (esp_now_init() != ESP_OK)
   {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, slaveAddress, 6);

   if (esp_now_add_peer(&peerInfo) != ESP_OK)
   {
    Serial.println("Failed to add peer");
    return;
   }

}

void loop()
{

    // Check if the distance is below the minimum distance if it is send a stop signal
    space = calculateDistance();
    if (space < MIN_DISTANCE)
    { 
        sendCommand("STOP");

        //Stop the motors
        stopMotors();
        reciveReply();
        displayData(reply, space);
    } 
    else
    {
        sendCommand("START");

        moveForward();

        displayData(reply, space);
    }

    delay(100);

}




//שני

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <esp_now.h>
#include <WiFi.h>

#define i2c_Address 0x3c
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

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t slaveAddress[] = {0xFC, 0xE8, 0xC0, 0x91, 0x6D, 0x54};

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  String receivedMsg = String((char*)incomingData);
  Serial.print("Received: ");
  Serial.println(receivedMsg);
}

float calculateDistance() {
  digitalWrite(triger_Pin, LOW);
  delayMicroseconds(2);
  digitalWrite(triger_Pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triger_Pin, LOW);
  float Duration = pulseIn(echo_Pin, HIGH);
  return Duration / 58.0;
}

void stopMotors() {
  ledcWrite(PWM_CHANNEL_F1, 0);
  ledcWrite(PWM_CHANNEL_F2, 0);
  ledcWrite(PWM_CHANNEL_B1, 0);
  ledcWrite(PWM_CHANNEL_B2, 0);
}

void moveForward() {
  ledcWrite(PWM_CHANNEL_F1, speed * 255);
  ledcWrite(PWM_CHANNEL_F2, speed * 255);
  ledcWrite(PWM_CHANNEL_B1, 0);
  ledcWrite(PWM_CHANNEL_B2, 0);
}

void displayData(String message, float distance) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.print("Msg: ");
  display.println(message);
  display.setCursor(0, 30);
  display.print("Distance: ");
  display.print(distance);
  display.print(" cm");
  display.display();
}

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, slaveAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  display.begin(i2c_Address, true);
  display.clearDisplay();

  pinMode(triger_Pin, OUTPUT);
  pinMode(echo_Pin, INPUT);

  ledcAttachPin(Motor_B1, PWM_CHANNEL_B1);
  ledcAttachPin(Motor_F1, PWM_CHANNEL_F1);
  ledcAttachPin(Motor_B2, PWM_CHANNEL_B2);
  ledcAttachPin(Motor_F2, PWM_CHANNEL_F2);
}

void loop() {
  float space = calculateDistance();
  if (space < MIN_DISTANCE) {
    const char *msg = "STOP";
    esp_now_send(slaveAddress, (uint8_t *)msg, strlen(msg));
    stopMotors();
    displayData("STOP", space);
  } else {
    const char *msg = "START";
    esp_now_send(slaveAddress, (uint8_t *)msg, strlen(msg));
    moveForward();
    displayData("START", space);
  }
  delay(100);
}
*/

//שלישי

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <esp_now.h>
#include <WiFi.h>

#define i2c_Address 0x3c
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

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t slaveAddress[] = {0xFC, 0xE8, 0xC0, 0x91, 0x6D, 0x54};

String lastReceivedMsg = "";

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void OnDataRecv(const esp_now_recv_info *recv_info, const uint8_t *incomingData, int len)
{
  lastReceivedMsg = String((char*)incomingData);
  Serial.print("Received: ");
  Serial.println(lastReceivedMsg);
  displayData(lastReceivedMsg, calculateDistance());
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
  ledcWrite(PWM_CHANNEL_F1, 0);
  ledcWrite(PWM_CHANNEL_F2, 0);
  ledcWrite(PWM_CHANNEL_B1, 0);
  ledcWrite(PWM_CHANNEL_B2, 0);
}

void moveForward()
{
  ledcWrite(PWM_CHANNEL_F1, speed * 255);
  ledcWrite(PWM_CHANNEL_F2, speed * 255);
  ledcWrite(PWM_CHANNEL_B1, 0);
  ledcWrite(PWM_CHANNEL_B2, 0);
}

void displayData(String message, float distance)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Received Msg: ");
  display.println(message);
  display.setCursor(0, 20);
  display.print("Distance: ");
  display.print(distance);
  display.print(" cm");
  display.display();
}

void setup()
{
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, slaveAddress, 6);
  /*peerInfo.channel = 0;
  peerInfo.encrypt = false;*/

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

  display.begin(i2c_Address, true);
  display.clearDisplay();

  pinMode(triger_Pin, OUTPUT);
  pinMode(echo_Pin, INPUT);

  ledcAttachChannel(Motor_B1 , Freq, Resolution, PWM_CHANNEL_B1);
  ledcAttachChannel(Motor_F1 , Freq, Resolution, PWM_CHANNEL_F1);
  ledcAttachChannel(Motor_B2 , Freq, Resolution, PWM_CHANNEL_B2);
  ledcAttachChannel(Motor_F2 , Freq, Resolution, PWM_CHANNEL_F2);
}

void loop()
{
  float distance = calculateDistance();
  if (distance < MIN_DISTANCE)
  {
    const char *msg = "STOP";
    esp_now_send(slaveAddress, (uint8_t *)msg, strlen(msg));
    stopMotors();
  }
  else
  {
    const char *msg = "START";
    esp_now_send(slaveAddress, (uint8_t *)msg, strlen(msg));
    moveForward();
  }

  delay(100);
}
