// Include necessary libraries for display, communication, lighting, audio, and standard I/O
#include <Wire.h>
#include <Adafruit_GFX.h>               // Core graphics library
#include <Adafruit_SH110X.h>            // SH1106 OLED display library
#include <WiFi.h>                       // WiFi (required for ESP-NOW)
#include <esp_now.h>                    // ESP-NOW communication
#include <Adafruit_NeoPixel.h>          // NeoPixel LED strip library
#include "DFRobotDFPlayerMini.h"        // DFPlayer Mini audio library
#include <iostream>                     // Standard I/O library

// === OLED Display Configuration ===
#define i2c_Address 0x3c                // I2C address for SH1106 OLED
#define SCREEN_WIDTH 128                // OLED display width in pixels
#define SCREEN_HEIGHT 64                // OLED display height in pixels
#define OLED_RESET -1                   // OLED reset pin (-1 if not used)

// === Motor Driver Pins ===
#define Motor_B1 12                     // Backward pin for motor 1
#define Motor_F1 13                     // Forward pin for motor 1
#define Motor_B2 4                      // Backward pin for motor 2
#define Motor_F2 25                     // Forward pin for motor 2

// === PWM Configuration ===
#define Resolution 8                    // PWM resolution (8-bit)
#define Freq 1000                       // PWM frequency (1 kHz)
#define PWM_CHANNEL_B1 0                // PWM channel for Motor_B1
#define PWM_CHANNEL_F1 1                // PWM channel for Motor_F1
#define PWM_CHANNEL_B2 2                // PWM channel for Motor_B2
#define PWM_CHANNEL_F2 3                // PWM channel for Motor_F2

// === NeoPixel LED Configuration ===
#define NEOPIXEL_PIN 5                  // Pin connected to NeoPixels
#define NUM_PIXELS 6                    // Number of NeoPixel LEDs

// === Line Following Sensor Pins (Strip Sensors) ===
#define STRIP_SENSOR_1 32              // Left-most line sensor
#define STRIP_SENSOR_2 39              // Mid-left line sensor
#define STRIP_SENSOR_3 15              // Mid-right line sensor
#define STRIP_SENSOR_4 33              // Right-most line sensor

// === Obstacle IR Sensor Pins ===
#define IR_SENSOR1_PIN 35              // IR obstacle sensor 1
#define IR_SENSOR2_PIN 36              // IR obstacle sensor 2

// === LDR (Light Dependent Resistor) Pin ===
#define LDR 34                         // Light sensor for day/night detection

// === Global Speed Control ===
float speed = 0.7;                      // Default motor speed (range 0.0 to 1.0)

// === ESP-NOW Communication Variables ===
String receivedMsg = " ";              // Received message string
char lastReceivedMsg[20] = "";         // Last received message stored for repeat use
uint8_t masterAddress[] = {0xFC, 0xE8, 0xC0, 0x91, 0x6B, 0x70};         // MAC address of the ESP32 slave

//fc:e8:c0:91:6b:70
// 0x08, 0xA6, 0xF7, 0x08, 0x3E, 0x98


// === DFPlayer Mini Audio ===
HardwareSerial mp3Serial(1);           // Use UART1 for DFPlayer Mini (TX1/RX1)
DFRobotDFPlayerMini mp3;               // MP3 player object

// === LDR Flag ===
bool flagLDR = true;                   // Flag for day/night mode toggle

// === Peripheral Objects ===
Adafruit_NeoPixel NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);  // NeoPixel object
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // OLED object

// === Status Flags and Timing ===
bool isConnected = false;              // ESP-NOW connection status
bool isMasterStopped = false;          // Flag to indicate master has stopped
bool isIRtracking = false;             // Line tracking active flag
unsigned long lastReceivedTime = 0;    // Timestamp of last received message
const unsigned long TIMEOUT = 8000;    // Timeout duration for communication

// === Intersection Handling ===
char intersectionTurn = 'z';           // Command to send at intersections (L, R, F, or z if none)
#define COMMAND_QUEUE_SIZE 5           // Max size of command queue
std::vector<char> q;                   // Queue for storing intersection turn commands

// === Function Declarations ===
bool isEmpty();                             
void enqueue(char x);                       
void dequeue();                           
char getFront();                          
void headLights(bool isStopped);            
void dayNightMode();               
void displayMessage(String title, String message); 
void motorsWrite(float m1, float m2, float m3, float m4);
void moveAccordingToStrip();                
void onDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incomingData, int len); 
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);               


/**
 * @brief Checks if the queue is empty.
 * @return true if the queue is empty, false otherwise
 */
bool isEmpty()
{ 
  return q.empty(); 
}

/**
 * @brief Adds a new element to the end of the queue.
 * @param x the character (direction) to enqueue
 */
void enqueue(char x) 
{
  q.push_back(x);
}

/**
 * @brief Removes the element at the front of the queue, if it is not empty.
 */
void dequeue() 
{
  if (!isEmpty()) q.erase(q.begin());
}

/**
 * @brief Retrieves the element at the front of the queue without removing it.
 * @return the front character if queue is not empty; 'z' otherwise
 */
char getFront() 
{
  return isEmpty() ? 'z' : q.front();
}


/**
 * @brief Controls the NeoPixel headlights based on LDR (light sensor) readings and robot motion state.
 * 
 * @param isStopped Indicates whether the robot is currently stopped.
 */
void headLights(bool isStopped)
{
  NeoPixel.clear();

  // If it's dark and either the robot is stopped or not
  if ((digitalRead(LDR) == LOW && isStopped == true) || (digitalRead(LDR) == LOW))
  {
    // Turn on white headlights and red rear lights
    NeoPixel.setPixelColor(0, NeoPixel.Color(255, 255, 150));  // Front left
    NeoPixel.setPixelColor(1, NeoPixel.Color(255, 255, 150));  // Front middle left
    NeoPixel.setPixelColor(2, NeoPixel.Color(255, 255, 150));  // Front middle right
    NeoPixel.setPixelColor(3, NeoPixel.Color(255, 255, 150));  // Front right
    NeoPixel.setPixelColor(4, NeoPixel.Color(255, 0, 0));      // Rear left
    NeoPixel.setPixelColor(5, NeoPixel.Color(255, 0, 0));      // Rear right
    NeoPixel.show();
  }
  // If it's bright and robot is stopped, only rear red lights turn on
  else if (digitalRead(LDR) == HIGH && isStopped == true)
  {
    NeoPixel.setPixelColor(4, NeoPixel.Color(255, 0, 0));
    NeoPixel.setPixelColor(5, NeoPixel.Color(255, 0, 0));
    NeoPixel.show();
  }
  // Otherwise, turn off all lights
  else
  {
    NeoPixel.clear();
    NeoPixel.show();
  }
}

/**
 * @brief Activates rear brake lights using NeoPixels.
 * 
 * This function is called when the robot is stopping, idle, or at an intersection.
 */
void brakeLights()
{
  NeoPixel.setPixelColor(4, NeoPixel.Color(255, 0, 0));  
  NeoPixel.setPixelColor(5, NeoPixel.Color(255, 0, 0));  
  
  NeoPixel.show(); // update to the NeoPixel Led Strip
}

/**
 * @brief Detects transition between day and night using the LDR sensor and plays corresponding audio.
 * 
 * Plays a night mode audio cue if transitioning to night,
 * or a day mode cue if transitioning to day. Uses a flag to prevent repeated playback.
 */
void dayNightMode()
{
  // Check if it is currently night (LDR reads LOW)
  if ((digitalRead(LDR) == LOW))
  {
      // If this is the first time detecting night, play night mode audio
      if(flagLDR)
      {
          mp3.play(2); // Play the second MP3 file (0002.mp3) indicating night mode
          flagLDR = false; // Clear flag to avoid replaying the audio repeatedly
      }
  }
  else // It is day (LDR reads HIGH)
  {
      // If this is the first time detecting light, play light mode audio
      if(flagLDR)
      {
          mp3.play(3); // Play the third MP3 file (0003.mp3) indicating day mode
          flagLDR = false; // Clear flag to avoid replaying the audio repeatedly
      }
  }
}

/**
 * @brief Displays a title and a message on the OLED screen.
 * 
 * @param title   The title text to display at the top.
 * @param message The message text to display below the title.
 */
void displayMessage(String title, String message)
{
  display.clearDisplay();               // Clear the OLED display buffer
  display.setTextSize(1);               // Set text size to normal
  display.setCursor(0, 0);              // Set cursor to top-left corner
  display.println(title);               // Print the title
  display.setCursor(0, 10);             // Move cursor to next line (y=10)
  display.println(message);             // Print the message
  display.display();                    // Send buffer to the OLED to update screen
}

/**
 * @brief Controls the speed of four motors using PWM signals.
 * 
 * @param m1 Speed for motor 1 (range 0.0 to 1.0).
 * @param m2 Speed for motor 2 (range 0.0 to 1.0).
 * @param m3 Speed for motor 3 (range 0.0 to 1.0).
 * @param m4 Speed for motor 4 (range 0.0 to 1.0).
 */
void motorsWrite(float m1, float m2, float m3, float m4) 
{
  ledcWrite(Motor_F1, m1 * 255); // Write PWM value for motor 1
  ledcWrite(Motor_F2, m2 * 255); // Write PWM value for motor 2
  ledcWrite(Motor_B1, m3 * 255); // Write PWM value for motor 3
  ledcWrite(Motor_B2, m4 * 255); // Write PWM value for motor 4
}

/**
 * @brief Function containing the logic of movinf according the the course
 */
void moveAccordingToStrip()
{
  if(digitalRead(STRIP_SENSOR_1) && digitalRead(STRIP_SENSOR_2) && digitalRead(STRIP_SENSOR_3) && digitalRead(STRIP_SENSOR_4))
  {
    // Get the correct turn from the queue, after, remove it
    intersectionTurn = getFront();
    dequeue();

    if (intersectionTurn == 'F')
    {
      motorsWrite(0.7, 0.7, 0, 0);
    }
    if (intersectionTurn == 'R')
    {
      motorsWrite(0,0.8,0,0);
      delay(1000);
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
      unsigned long startTime = millis();
      while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)) // keep turning until it sees the line
      {
        if (millis() - startTime > 2000) break; // max wait 2s
      }
    }
  }

  // If center sensors (2 & 3) are on the line — continue forward
  else if (digitalRead(STRIP_SENSOR_2) && digitalRead(STRIP_SENSOR_3))
  {
    motorsWrite(0.7, 0.7, 0, 0);  // Move forward
    displayMessage("Driving:", "Foward");  // Show status on OLED
  }

  // Extreme correction to the left (only far-left sensor detects line)
  else if (digitalRead(STRIP_SENSOR_4))
  {
    motorsWrite(0.8, 0, 0, 0);  // Turn left in place
    displayMessage("Driving:", "Hard Left");

    // Keep turning left until center sensors detect the line
    while (!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3))
    {
      motorsWrite(0.8, 0, 0, 0);
    }
  }

  // Extreme correction to the right (only far-right sensor detects line)
  else if (digitalRead(STRIP_SENSOR_1))
  {
    motorsWrite(0, 0.8, 0, 0);  // Turn right in place
    displayMessage("Driving:", "Hard Right");

    // Keep turning right until center sensors detect the line
    while (!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3))
    {
      motorsWrite(0, 0.8, 0, 0);
    }
  }

  // Minor left correction (slight drift detected by sensor 3)
  else if (digitalRead(STRIP_SENSOR_3))
  {
    displayMessage("Driving:", "Slight Left");    
    motorsWrite(0.8, 0.5, 0, 0);  // Adjust left
  }

  // Minor right correction (slight drift detected by sensor 2)
  else if (digitalRead(STRIP_SENSOR_2))
  {
    displayMessage("Driving:", "Slight Right");    
    motorsWrite(0.5, 0.8, 0, 0);  // Adjust right
  }

  // No line detected by any sensor — stop as a fallback safety mechanism
  else
  {
    displayMessage("state:", "stop");    
    motorsWrite(0, 0, 0, 0);

    brakeLights();
  }
}

/**
 * @brief Callback function called when data is received via ESP-NOW.
 * 
 * @param mac         MAC address of the sender.
 * @param incomingData Pointer to the incoming data buffer.
 * @param len         Length of the incoming data.
 */
void onDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incomingData, int len)
{
  isConnected = true;                          // Mark that the slave is connected to the master
  lastReceivedTime = millis();                 // Update the last time a message was received

  char receivedMsg[len + 1];                   // Create a character array to store the received message
  memcpy(receivedMsg, incomingData, len);      // Copy the received bytes into the message buffer
  receivedMsg[len] = '\0';                     // Null-terminate the string

  // Handle "Start" command
  if (strcmp(receivedMsg, "Start") == 0)
  {
    displayMessage("Received:", receivedMsg);  // Show message on OLED
    isMasterStopped = false;                   // Resume movement
    const char *message = "Start received";    // Prepare acknowledgment
    esp_now_send(masterAddress, (uint8_t *)message, strlen(message)); // Send acknowledgment
  }

  // Handle "Stop course" command (stops movement and enables IR tracking)
  else if (strcmp(receivedMsg, "Stop course") == 0)
  {
    displayMessage("Received:", receivedMsg);  // Show message on OLED
    isMasterStopped = true;                    // Stop movement
    isIRtracking = true;                       // Enable IR line tracking
    const char *message = "Stop received";     // Prepare acknowledgment
    esp_now_send(masterAddress, (uint8_t *)message, strlen(message)); // Send acknowledgment
  }

  // Handle general "Stop" command (just stops the robot)
  else if (strcmp(receivedMsg, "Stop") == 0)
  {
    displayMessage("Received:", receivedMsg);  // Show message on OLED
    isMasterStopped = true;                    // Stop movement
    const char *message = "Stop received";     // Prepare acknowledgment
    esp_now_send(masterAddress, (uint8_t *)message, strlen(message)); // Send acknowledgment
  }

  // Handle direction commands: Forward (F), Left (L), Right (R)
  else if (strcmp(receivedMsg, "F") == 0 || strcmp(receivedMsg, "R") == 0 || strcmp(receivedMsg, "L") == 0)
  {
    displayMessage("Queued:", receivedMsg);    // Show queued direction on OLED
    enqueue(receivedMsg[0]);                   // Add direction command to the turn queue

    char message[10];
    snprintf(message, sizeof(message), "%c received", receivedMsg[0]); // Prepare acknowledgment
    esp_now_send(masterAddress, (uint8_t *)message, strlen(message)); // Send acknowledgment
  }
}


/**
 * @brief Callback function called when data is sent via ESP-NOW.
 * 
 * @param mac_addr MAC address of the receiver.
 */
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
}

void setup()
{
  // ===== OLED Display Initialization =====

  // Initialize the OLED display
  display.begin(i2c_Address, true); 
  display.clearDisplay();                      // Clear any previous content
  display.setTextColor(SH110X_WHITE);         // Set text color to white

  // ===== ESP-NOW Communication Setup =====

  WiFi.mode(WIFI_STA);                        // Set ESP32 to station mode (required for ESP-NOW)

  // Initialize ESP-NOW and display error if failed
  if (esp_now_init() != ESP_OK)
  {
    displayMessage("Error", "Init ESP-NOW");
    return;
  }

  // Register callback for receiving data
  esp_now_register_recv_cb(onDataRecv);

  // Configure peer (master ESP32) for ESP-NOW communication
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, masterAddress, 6); // Copy master's MAC address
  peerInfo.channel = 0;                         // Default channel
  peerInfo.encrypt = false;                     // No encryption

  // Add master as a peer and display error if failed
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    displayMessage("Error", "Add Peer Fail");
    return;
  }

  // ===== PWM Motor Channels Initialization =====

  // Attach motor pins to their respective PWM channels
  ledcAttachChannel(Motor_B1, Freq, Resolution, PWM_CHANNEL_B1);
  ledcAttachChannel(Motor_F1, Freq, Resolution, PWM_CHANNEL_F1);
  ledcAttachChannel(Motor_B2, Freq, Resolution, PWM_CHANNEL_B2);
  ledcAttachChannel(Motor_F2, Freq, Resolution, PWM_CHANNEL_F2);

  // ===== NeoPixel LED Strip Initialization =====

  NeoPixel.begin();       // Initialize the NeoPixel strip
  NeoPixel.clear();       // Ensure all LEDs are off
  NeoPixel.show();        // Update the strip with current LED state

  // ===== Strip Sensor Pins Declaration =====

  pinMode(STRIP_SENSOR_1, INPUT);  // Front-left line sensor
  pinMode(STRIP_SENSOR_2, INPUT);  // Front-right line sensor
  pinMode(STRIP_SENSOR_3, INPUT);  // Back-left line sensor
  pinMode(STRIP_SENSOR_4, INPUT);  // Back-right line sensor

  // ===== LDR and Obstacle Sensor Initialization =====

  pinMode(LDR, INPUT);             // Light-dependent resistor (for day/night detection)
  pinMode(IR_SENSOR1_PIN, INPUT);  // IR sensor 1 (for obstacle detection)
  pinMode(IR_SENSOR2_PIN, INPUT);  // IR sensor 2 (for obstacle detection)

  // ===== DFPlayer Mini MP3 Module Initialization =====

  // Start Serial communication with DFPlayer Mini on custom RX/TX pins
  mp3Serial.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17

  mp3.begin(mp3Serial);  // Initialize the DFPlayer Mini
  mp3.volume(50);        // Set the audio volume level 50
}

void loop()
{  
  // Check for ambient light change and play audio if it's the first time
  dayNightMode();

  // Check if we haven't received any message from the master within the timeout period
  if (millis() - lastReceivedTime > TIMEOUT)
  {
    // If timeout has occurred, assume connection is lost
    isConnected = false;
  }

  // If the slave is not connected to master OR IR tracking mode is enabled
  if (!isConnected || isIRtracking)
  {
    // If either of the front obstacle IR sensors detect an object (LOW = obstacle detected)
    if (digitalRead(IR_SENSOR1_PIN) == LOW || digitalRead(IR_SENSOR2_PIN) == LOW)
    {
      // Stop all motors and turn on stop headlight indication
      motorsWrite(0, 0, 0, 0);
      headLights(true);
    }
    else 
    {
      // Continue following the line using strip sensors
      moveAccordingToStrip();
      headLights(false);
    }
  }

  // If the robot is connected but the master has issued a stop command
  else if (isConnected && isMasterStopped)
  {
    // Stop all motors and turn on stop headlight indication
    motorsWrite(0, 0, 0, 0);
    headLights(isMasterStopped);
  }

  // If the robot is connected and the master has not issued a stop command
  else if (isConnected && !isMasterStopped)
  {
    // Move forward or perform turns based on line sensors
    moveAccordingToStrip();
    // Set headlights depending on stop status 
    headLights(isMasterStopped);
  }
}