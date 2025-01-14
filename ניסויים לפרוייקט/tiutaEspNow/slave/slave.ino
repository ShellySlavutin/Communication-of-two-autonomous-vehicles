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

void onDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incomingData, int len)
{
  char receivedMsg[len + 1];
  memcpy(receivedMsg, incomingData, len);
  receivedMsg[len] = '\0';

  // Display recived msg
  displayMessage("Recived :" ,receivedMsg);

  // Reply back
  const char *reply = "Hello from Slave";
  esp_now_send(mac->src_addr, (uint8_t *)reply, strlen(reply));
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

}

void loop()
{
  delay(1000);
}
