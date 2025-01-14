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

uint8_t slaveAddress[] = {0xFC, 0xE8, 0xC0, 0x91, 0x6D, 0x54};

void displayMessage(String title, String message) {
  display.clearDisplay();               // Clear the OLED display
  display.setTextSize(1);              // Set text size
  display.setCursor(0, 0);             // Set cursor position
  display.println(title);              // Print title
  display.setCursor(0, 20);           // Move cursor to next line
  display.println(message);           // Print the message
  display.display();                   // Update the display
}

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  String statusMsg = (status == ESP_NOW_SEND_SUCCESS) ? "Success" : "Fail";
  displayMessage("Send Status:", statusMsg);
}

void setup()
{
  // Initialize the display
  display.begin(i2c_Address,true); 
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
    
  // Display first string
  display.setTextSize(1); // Standard font size
  display.setCursor(2, 10); // Horizontal, vertical position
  display.println("test1");

  display.display(); // Show on OLED


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
    //displayMessage("Error", "Add Peer Fail");
    return;
  }

  // Display second string
  display.setTextSize(1); // Larger font size
  display.setCursor(5, 25); // Adjusted position
  display.println("test2");

  display.display(); // Show on OLED
}


void loop()
{
  const char *message = "Hello from Master";
  esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));
  //displayMessage("Sent:", "Hello from Master");
  delay(2000);
}
