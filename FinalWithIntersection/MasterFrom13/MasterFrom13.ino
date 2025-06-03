// === Include necessary libraries ===
#include <Wire.h>                      // For I2C communication (OLED)
#include <Adafruit_GFX.h>              // Core graphics library for OLED
#include <Adafruit_SH110X.h>           // SH1106 OLED driver
#include <WiFi.h>                      // Required for ESP-NOW on ESP32
#include <esp_now.h>                   // ESP-NOW protocol for wireless communication
#include <Adafruit_NeoPixel.h>         // Library for NeoPixel RGB LED strip
#include "DFRobotDFPlayerMini.h"       // DFPlayer Mini MP3 module control
#include <ESP32Servo.h>                // For controlling a servo motor

// === Declare servo object ===
Servo servo;                           // Servo motor object

// === OLED Display Configuration ===
#define i2c_Address 0x3c               // OLED I2C address
#define SCREEN_WIDTH 128               // OLED width in pixels
#define SCREEN_HEIGHT 64               // OLED height in pixels
#define OLED_RESET -1                  // OLED reset pin (-1 = not used)

// === Ultrasonic Sensor Configuration ===
#define triger_Pin 32                  // Ultrasonic trigger pin
#define echo_Pin 33                    // Ultrasonic echo pin
#define MIN_DISTANCE 10                // Minimum safe distance (in cm)

// === Motor Driver Pin Definitions ===
#define Motor_B1 13                    // Motor 1 - backward
#define Motor_F1 12                    // Motor 1 - forward
#define Motor_B2 25                     // Motor 2 - backward
#define Motor_F2 4                    // Motor 2 - forward

/*// === Motor Driver Pin Definitions ===
#define Motor_B1 12                    // Motor 1 - backward
#define Motor_F1 13                    // Motor 1 - forward
#define Motor_B2 4                     // Motor 2 - backward
#define Motor_F2 25                    // Motor 2 - forward
*/

// === PWM Configuration for Motors ===
#define Resolution 8                   // PWM resolution (8-bit)
#define Freq 1000                      // PWM frequency (Hz)
#define PWM_CHANNEL_B1 0               // PWM channel for Motor_B1
#define PWM_CHANNEL_F1 1               // PWM channel for Motor_F1
#define PWM_CHANNEL_B2 2               // PWM channel for Motor_B2
#define PWM_CHANNEL_F2 3               // PWM channel for Motor_F2

// === Line-Following IR Sensor Pins ===
#define STRIP_SENSOR_1 36              // Far left IR sensor
#define STRIP_SENSOR_2 39              // Mid left IR sensor
#define STRIP_SENSOR_3 15              // Mid right IR sensor
#define STRIP_SENSOR_4 35              // Far right IR sensor

// === RGB LED Pin Definitions ===
#define BLUE_RGB_PIN 2                 // Blue LED pin
#define RED_RGB_PIN 26                 // Red LED pin
#define GREEN_RGB_PIN 27               // Green LED pin

// === NeoPixel LED Strip Configuration ===
#define NEOPIXEL_PIN 5                 // NeoPixel data pin
#define NUM_PIXELS 6                   // Number of NeoPixels in the strip

// === LDR (Light Sensor) Pin ===
#define LDR 34                         // Analog pin connected to LDR

// === LDR Day/Night Flag ===
bool flagLDR = true;                  // Used to announce day/night only once per change

// === ESP-NOW Communication ===
char lastReceivedMsg[20] = "";        // Stores the most recent message from slave

// === Slave ESP32 MAC Address ===
uint8_t slaveAddress[] = {0xFC, 0xE8, 0xC0, 0x91, 0x6D, 0x54};  // Slave device MAC
//0xFC, 0xE8, 0xC0, 0x91, 0x6D, 0x54

// === Peripheral Objects Initialization ===
Adafruit_NeoPixel NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);  // NeoPixel object
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // OLED object

// === DFPlayer Mini Audio Setup ===
HardwareSerial mp3Serial(1);          // Use UART1 (TX1/RX1) for DFPlayer Mini
DFRobotDFPlayerMini mp3;              // Create DFPlayer object for MP3 control

// ===== Function Declarations =====
void headLights(float distance);
void dayNightMode();
void displayMessage(String title, String message);
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void onDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incomingData, int len);
float calculateDistance();
void motorsWrite(float m1, float m2, float m3, float m4);
void moveServo(int angle);

// ===== Function Definitions Start =====

/**
 * @brief Controls NeoPixel headlights and brake lights based on ambient light and distance to an obstacle.
 * 
 * @param distance Distance to the nearest obstacle. Used to determine if the robot should stop.
 */
void headLights(float distance)
{
  NeoPixel.clear();
  if ((digitalRead(LDR) == LOW && distance < MIN_DISTANCE) || (digitalRead(LDR) == LOW)) // Night and stop or night -> all leds
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
  
  else if (digitalRead(LDR) == HIGH && distance < MIN_DISTANCE) // Light and stop -> only back leds
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


/**
 * @brief Activates rear brake lights using NeoPixels.
 * 
 * This function is typically called when the robot is stopping, idle, or at an intersection.
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
  else
  {
      // It is day (LDR reads HIGH)
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
 * @brief Callback function called when data is sent via ESP-NOW.
 * 
 * @param mac_addr MAC address of the receiver.
 */
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
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
  char receivedMsg[len + 1];
  memcpy(receivedMsg, incomingData, len); // Copy received bytes to local buffer
  receivedMsg[len] = '\0';                 // Null-terminate string

  if (strcmp(receivedMsg, "Stop received") == 0)
  {
    // Indicate stopping by turning on blue RGB LED and turning off others
    digitalWrite(RED_RGB_PIN, LOW);
    digitalWrite(GREEN_RGB_PIN, LOW);
    digitalWrite(BLUE_RGB_PIN, HIGH);
  }
  else if (strcmp(receivedMsg, "Start received") == 0)
  {
    // Indicate starting by turning on green RGB LED and turning off others
    digitalWrite(RED_RGB_PIN, LOW);
    digitalWrite(GREEN_RGB_PIN, HIGH);
    digitalWrite(BLUE_RGB_PIN, LOW);
  }
}

/**
 * @brief Measures distance using an ultrasonic sensor.
 * 
 * @return Distance measured in centimeters.
 */
float calculateDistance()
{
  digitalWrite(triger_Pin, LOW);           // Ensure trigger pin is low
  delayMicroseconds(2);                     // Short delay to settle sensor
  digitalWrite(triger_Pin, HIGH);          // Send 10µs trigger pulse
  delayMicroseconds(10);
  digitalWrite(triger_Pin, LOW);
  float Duration = pulseIn(echo_Pin, HIGH); // Measure the echo pulse duration
  float distance = Duration / 58.0;         // Convert duration to distance in cm
  return distance;
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
 * @brief Moves the servo to the specified angle.
 * 
 * @param angle Angle to move the servo to (degrees).
 */
void moveServo(int angle) 
{
  servo.attach(18, 500, 2500);  // Attach servo to pin 18 with min/max pulse widths
  servo.write(angle);           // Move servo to the desired angle
  delay(500);                  // Wait for servo to reach the position
  servo.detach();              // Detach to save power and free pin
}


void setup()
{
  // ===== LED Declarations =====

  // Initialize all LEDs as output
  pinMode(RED_RGB_PIN, OUTPUT);
  pinMode(GREEN_RGB_PIN, OUTPUT);
  pinMode(BLUE_RGB_PIN, OUTPUT);

  // Turn all LEDs off initially
  digitalWrite(RED_RGB_PIN, LOW);
  digitalWrite(GREEN_RGB_PIN, LOW);
  digitalWrite(BLUE_RGB_PIN, LOW);

  // ===== OLED Display Declaration =====

  // Initialize the display
  display.begin(i2c_Address,true); 
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  // ===== Communication between ESP32 Declarations =====

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    displayMessage("Error", "Init ESP-NOW");
    return;
  }

  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, slaveAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    displayMessage("Error", "Add Peer Fail");
    return;
  }

  // ===== Ultrasonic Declaration =====
  
  pinMode(triger_Pin, OUTPUT);
  pinMode(echo_Pin, INPUT);

  // ===== PWM for Motors Declarations =====

  ledcAttachChannel(Motor_B1 , Freq, Resolution, PWM_CHANNEL_B1);
  ledcAttachChannel(Motor_F1 , Freq, Resolution, PWM_CHANNEL_F1);
  ledcAttachChannel(Motor_B2 , Freq, Resolution, PWM_CHANNEL_B2);
  ledcAttachChannel(Motor_F2 , Freq, Resolution, PWM_CHANNEL_F2);

  // ===== Strip Sensors Declarations =====

  pinMode(STRIP_SENSOR_1, INPUT);
  pinMode(STRIP_SENSOR_2, INPUT);
  pinMode(STRIP_SENSOR_3, INPUT);
  pinMode(STRIP_SENSOR_4, INPUT);

  // ===== NeoPixel Declarations =====

  NeoPixel.begin(); // initialize NeoPixel strip object (REQUIRED)
  delay (200);

  NeoPixel.clear(); // Start the program with lights off
  NeoPixel.show(); // update to the NeoPixel Led Strip

  // ===== LDR Declaration =====

  pinMode(LDR, INPUT); // Init the LDR

  // ===== mp3 Declaration =====

  // Start Serial communication with DFPlayer Mini
  mp3Serial.begin(9600, SERIAL_8N1, 16, 17);  //rx=16, tx=17

  mp3.begin(mp3Serial); // Initialize DFPlayer Mini
  mp3.volume(50); // Set volume to 50
}

void loop()
{
  // Handle day/night mode changes, possibly playing sounds or adjusting lights
  dayNightMode();

  // Measure the distance from an obstacle using ultrasonic sensor
  float distance = calculateDistance();

  // Update headlights based on distance and ambient light
  headLights(distance);

  // If obstacle is too close, stop the robot
  if (distance < MIN_DISTANCE)
  {
    const char *message = "Stop"; // Prepare stop message
    esp_now_send(slaveAddress, (uint8_t *)message, strlen(message)); // Send stop command to slave

    motorsWrite(0, 0, 0, 0);          // Stop all motors
  }

  else
  {
    // Check if all 4 strip sensors detect the line (possible intersection or T-junction)
    if (digitalRead(STRIP_SENSOR_1) && digitalRead(STRIP_SENSOR_2) && digitalRead(STRIP_SENSOR_3) && digitalRead(STRIP_SENSOR_4))
    {
      bool Tintersection = true;
      delay(200);  // Wait briefly to stabilize sensor readings

      // If either center sensor does NOT see the line, it's not a proper T-junction
      if (!digitalRead(STRIP_SENSOR_2) || !digitalRead(STRIP_SENSOR_3))
      {
        Tintersection = false;
      }

      motorsWrite(0, 0, 0, 0);  // Stop the motors
      brakeLights();            // Turn on brake lights

      const char *message = "Stop";  // Send stop command
      esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));

      delay(500);  // Pause before scanning environment

      // Scan right
      moveServo(0);  // Rotate servo to 0°
      moveServo(0);
      delay(500);
      float disR = calculateDistance();  // Measure right-side distance
      delay(500);

      // Scan forward
      moveServo(90);  // Rotate servo to 90°
      moveServo(90);
      delay(500);
      float disF = calculateDistance();  // Measure forward distance
      delay(500);

      // Scan left
      moveServo(180);  // Rotate servo to 180°
      moveServo(180);
      delay(500);
      float disL = calculateDistance();  // Measure left-side distance
      delay(500);

      // Reset servo to center
      moveServo(90);

      // Decision-making: go in the direction with most space
      if ((disF > disR) && (disF > disL) && Tintersection)
      {
        const char *message = "F";  // Go Forward
        esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));

        motorsWrite(0.7, 0.7, 0, 0);  // Drive forward
        delay(1000);
      }
      else if ((disR > disF) && (disR > disL))
      {
        const char *message = "R";  // Turn Right
        esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));

        motorsWrite(0, 0.8, 0, 0);  // Rotate right
        delay(1000);
        // Keep turning until center sensors detect line again
        while (!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3));
      }
      else if ((disL > disF) && (disL > disR))
      {
        const char *message = "L";  // Turn Left
        esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));

        motorsWrite(0.8, 0, 0, 0);  // Rotate left
        delay(1000);
        // Keep turning until center sensors detect line again
        while (!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3));
      }
    }

    // If center sensors (2 & 3) are on the line — continue forward
    else if (digitalRead(STRIP_SENSOR_2) && digitalRead(STRIP_SENSOR_3))
    {
      const char *message = "Start";  // Notify slave to start
      esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));

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
      const char *message = "Stop course";  // Notify slave
      esp_now_send(slaveAddress, (uint8_t *)message, strlen(message));

      motorsWrite(0, 0, 0, 0);  // Stop all motors
      brakeLights();           // Turn on brake lights
    }
  }

}