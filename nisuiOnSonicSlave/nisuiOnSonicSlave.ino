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
String receivedMsg = " ";

// Creating an object for communication with the OLED screen
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void displayMessage(String title, String message)
{
  display.clearDisplay();               // Clear the OLED display
  display.setTextSize(1);              // Set text size
  display.setCursor(0, 0);             // Set cursor position
  display.println(title);              // Print title
  display.setCursor(0, 20);           // Move cursor to next line
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

    // Send confirmation back to the master
    //const char *reply = "Stop received";
    //esp_now_send(masterAddress, (uint8_t *)reply, strlen(reply));


  }

  else if (strcmp(receivedMsg, "Start") == 0)
  {
    displayMessage("Recived :" ,receivedMsg);
    moveForward();

    // Send confirmation back to the master
    //const char *reply = "Start received";
    //esp_now_send(masterAddress, (uint8_t *)reply, strlen(reply));
  }

  // Reply back
  //const char *reply = "Hello from Slave";
  //esp_now_send(mac->src_addr, (uint8_t *)reply, strlen(reply));
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

  displayMessage("Status", "ESP-NOW Ready");

  ledcAttachChannel(Motor_B1 , Freq, Resolution, PWM_CHANNEL_B1);
  ledcAttachChannel(Motor_F1 , Freq, Resolution, PWM_CHANNEL_F1);
  ledcAttachChannel(Motor_B2 , Freq, Resolution, PWM_CHANNEL_B2);
  ledcAttachChannel(Motor_F2 , Freq, Resolution, PWM_CHANNEL_F2);

}

void loop()
{

}

