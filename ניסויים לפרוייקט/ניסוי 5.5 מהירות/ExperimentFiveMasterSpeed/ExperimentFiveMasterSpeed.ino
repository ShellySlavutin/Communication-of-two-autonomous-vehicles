#include <Wire.h> // Library for I2C communication used for the OLED
#include <Adafruit_GFX.h> // Library for graphics functions
#include <Adafruit_SH110X.h>
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

// WiFi credentials
const char* ssid = "ALEXANDRA_IPHONE"; // WiFi name
const char* password = "sashaelarinova";  // WiFi password

// Slave IP and port 
const char* slaveIP = "172.20.10.9"; // Replace with the slave ESP32's IP address changes with every new WIFI connected
const int port = 23;

// Creating an object for communication with the OLED screen
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiClient client;

void connectToWiFi()
{
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi network");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
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
  float space = Duration / 58.0;         // Calculating distance in cm based on the pulse duration

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

void displayData(String replay, float space)
{
    display.clearDisplay();               // Clear the OLED display
    display.setTextSize(1.5);             // Set text size

    // Display the replay on the first line
    display.setCursor(0, 10);             // Set cursor position
    display.print("Replay received: ");  // Print label
    display.setCursor(0, 20);             // Move cursor to the next line
    display.print(replay);               // Print the command

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
  connectToWiFi();

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

}

void loop()
{
  // Attempt to reconnect to the slave if not connected
    if (!client.connected())
    {
        client.connect(slaveIP, port);
    }

    // As long as there is a dafe distance continue driving
    //moveForward();

    // Check if the distance is below the minimum distance if it is send a stop signal
    float space = calculateDistance();
    if (space < MIN_DISTANCE)
    { 
        client.println("STOP"); // Send "STOP" command
        Serial.println("Sent: STOP");

        // Stop the motors
        //stopMotors();

        String reply = client.readStringUntil('\n');
        reply.trim(); // Remove any extra whitespace or newline characters

        displayData(reply, space);
    } 

    else
    {
        client.println("START"); // Send "STOP" command
        Serial.println("Sent: START");

        //moveForward();

        String reply = client.readStringUntil('\n');
        reply.trim(); // Remove any extra whitespace or newline characters

        displayData(reply, space);
    }

    delay(50);  
}

