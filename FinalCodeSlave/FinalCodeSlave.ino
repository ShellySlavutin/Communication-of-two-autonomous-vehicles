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

#define LDR 34

float speed = 0.5;
String receivedMsg = " ";

uint8_t masterAddress[] = {0x08, 0xA6, 0xF7, 0x08, 0x3E, 0x98};

Adafruit_NeoPixel NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800); // Creating an object for the neoPixel

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Creating an object for communication with the OLED screen

void headLights(bool isStopped)
{
  NeoPixel.clear();
  if ((digitalRead(LDR) == LOW && isStopped == true) || (digitalRead(LDR) == LOW)) // Night and stop or night -> all leds
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
  else if (digitalRead(LDR) == HIGH && isStopped == true) // Light and stop -> only back leds
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

void onDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incomingData, int len)
{
  char receivedMsg[len + 1];
  memcpy(receivedMsg, incomingData, len);
  receivedMsg[len] = '\0';


  if (strcmp(receivedMsg, "Stop") == 0)
  {
    displayMessage("Recived :" ,receivedMsg);
    stopMotors();

    headLights(true);

    const char *message = "Stop received";
    esp_now_send(masterAddress, (uint8_t *)message, strlen(message));
  }

  else if (strcmp(receivedMsg, "Start") == 0)
  {
    displayMessage("Recived :" ,receivedMsg);
    moveForward();

    headLights(false);

    const char *message = "Start received";
    esp_now_send(masterAddress, (uint8_t *)message, strlen(message));
  }
}

void setup()
{
  display.begin(i2c_Address,true); 
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

  displayMessage("Status", "ESP-NOW Ready");

  ledcAttachChannel(Motor_B1 , Freq, Resolution, PWM_CHANNEL_B1);
  ledcAttachChannel(Motor_F1 , Freq, Resolution, PWM_CHANNEL_F1);
  ledcAttachChannel(Motor_B2 , Freq, Resolution, PWM_CHANNEL_B2);
  ledcAttachChannel(Motor_F2 , Freq, Resolution, PWM_CHANNEL_F2);

  NeoPixel.begin(); // initialize NeoPixel strip object (REQUIRED)

  delay (200);

  NeoPixel.clear(); // Start the program with lights off
  NeoPixel.show(); // update to the NeoPixel Led Strip

  pinMode(LDR, INPUT); // Init the LDR

}

void loop()
{
}

