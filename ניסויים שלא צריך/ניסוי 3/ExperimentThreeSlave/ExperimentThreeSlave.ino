#include <Wire.h> // Library for I2C communication used for the OLED
#include <Adafruit_GFX.h> // Library for graphics functions
#include <Adafruit_SH110X.h>
#include <WiFi.h>

// WiFi credentials
const char* ssid = "ALEXANDRA_IPHONE"; // WiFi name
const char* password = "sashaelarinova";  // WiFi password

// Port for communication
const int port = 23;

// Motor pins
#define MOTOR_B1 12
#define MOTOR_F1 13
#define MOTOR_B2 4
#define MOTOR_F2 25

// OLED pins
#define i2c_Address 0x3c // OLED screen I2C address
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 

// Creating an object for communication with the OLED screen
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiServer server(port);

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

void stopMotors()
{
    // Stop both motor sets
    digitalWrite(MOTOR_F1, LOW);
    digitalWrite(MOTOR_B1, LOW);
    digitalWrite(MOTOR_F2, LOW);
    digitalWrite(MOTOR_B2, LOW);
}

void moveForward()
{
    // Move forward for both motor sets
    digitalWrite(MOTOR_F1, HIGH);
    digitalWrite(MOTOR_B1, LOW);
    digitalWrite(MOTOR_F2, HIGH);
    digitalWrite(MOTOR_B2, LOW);
}

void displayCommand(String command) {
    display.clearDisplay();               // Clear the OLED display
    display.setCursor(0, 10);             // Set cursor position
    display.setTextSize(1.5);             // Set text size
    display.print("Command Received: ");  // Print label
    display.setCursor(0, 30);             // Move cursor to the next line
    display.print(command);               // Print the command
    display.display();                    // Update the display
}



void setup()
{
    Serial.begin(9600);
    connectToWiFi();

    // Initialize the motors
    pinMode(MOTOR_F1, OUTPUT);
    pinMode(MOTOR_B1, OUTPUT);
    pinMode(MOTOR_F2, OUTPUT);
    pinMode(MOTOR_B2, OUTPUT);

    // Initialize the OLED screen
    display.begin(i2c_Address, true); 
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);

    // Start the server
    server.begin();
    Serial.println("Server started, waiting for connections...");
}

void loop()
{
    WiFiClient client = server.available();

    if (client)
    {
        Serial.println("Client connected");

        while (client.connected())
        {
            if (client.available())
            {
                String command = client.readStringUntil('\n');
                command.trim(); // Remove any extra whitespace or newline characters
                Serial.print("Received: ");
                Serial.println(command);

                // Display the command on the OLED screen
                displayCommand(command);
           
                if (command == "STOP")
                {
                    stopMotors();
                    Serial.println("Motors stopped");

                    // Send confirmation back to the master
                    client.println("STOP received");
                    Serial.println("Sent: STOP received");
                }

                if (command == "START")
                {
                    moveForward();
                    Serial.println("Motors are moving");

                    // Send confirmation back to the master
                    client.println("START received");
                    Serial.println("Sent: START received");
                }

            }
        }
        client.stop();
        Serial.println("Client disconnected");
    }
}

