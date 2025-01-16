#include <Wire.h> // Library for I2C communication used for the OLED
#include <Adafruit_GFX.h> // Library for graphics functions
#include <Adafruit_SH110X.h>
#include <esp_now.h>

// Motor pins
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

// OLED pins
#define i2c_Address 0x3c // OLED screen I2C address
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 

// Creating an object for communication with the OLED screen
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t masterAddress[] = {0x08, 0xA6, 0xF7, 0x08, 0x3E, 0x98};

esp_now_peer_info_t peerInfo;

String receivedCommand;


void stopMotors()
{
  speed = 0;
  ledcWriteChannel(PWM_CHANNEL_F1, speed*255);
  ledcWriteChannel(PWM_CHANNEL_F2, speed*255);
  ledcWriteChannel(PWM_CHANNEL_B1, 0);
  ledcWriteChannel(PWM_CHANNEL_B2, 0);
}

void moveForward()
{
  // Move forward for both motor sets
  speed = 0.5;
  ledcWriteChannel(PWM_CHANNEL_F1, speed*255);
  ledcWriteChannel(PWM_CHANNEL_F2, speed*255);
  ledcWriteChannel(PWM_CHANNEL_B1, 0);
  ledcWriteChannel(PWM_CHANNEL_B2, 0);
}

void displayCommand(String command)
{
  display.clearDisplay();               // Clear the OLED display
  display.setCursor(0, 10);             // Set cursor position
  display.setTextSize(1.5);             // Set text size
  display.print("Command Received: ");  // Print label
  display.setCursor(0, 30);             // Move cursor to the next line
  display.print(command);               // Print the command
  display.display();                    // Update the display
}


void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len)
{
  char command[len + 1];
  memcpy(command, incomingData, len);
  command[len] = '\0'; // Null-terminate the string
  String receivedCommandLocal = String(command);
  receivedCommand = receivedCommandLocal;

  // Display the command on the OLED screen
  displayCommand(receivedCommandLocal);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  //Serial.print("Send Status: ");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}


void setup()
{
  // Initialize the OLED screen
  display.begin(i2c_Address, true); 
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  WiFi.mode(WIFI_STA);

  ledcAttachChannel(Motor_B1 , Freq, Resolution, PWM_CHANNEL_B1);
  ledcAttachChannel(Motor_F1 , Freq, Resolution, PWM_CHANNEL_F1);
  ledcAttachChannel(Motor_B2 , Freq, Resolution, PWM_CHANNEL_B2);
  ledcAttachChannel(Motor_F2 , Freq, Resolution, PWM_CHANNEL_F2);

  if (esp_now_init() != ESP_OK)
  {
  return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  memcpy(peerInfo.peer_addr, masterAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    return;
  }


}

void loop()
{
  
  if (receivedCommand == "STOP")
  {
    stopMotors();

    // Send confirmation back to the master
    //const char *reply = "STOP received";
    //esp_now_send(masterAddress, (uint8_t *)reply, strlen(reply));
    receivedCommand = "";
  }
  else if (receivedCommand == "START")
  {
    moveForward();

    // Send confirmation back to the master
    //const char *reply = "START received";
    //esp_now_send(masterAddress, (uint8_t *)reply, strlen(reply));
    receivedCommand = "";
  }
  
}

