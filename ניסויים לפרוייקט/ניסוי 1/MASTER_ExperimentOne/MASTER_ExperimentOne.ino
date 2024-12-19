#include <WiFi.h>

// WiFi credentials
const char* ssid = "SHELLY_SLAVUTIN_IPHONE"; // WiFi name
const char* password = "shellyslavutin";  // WiFi password

// Button pin
#define BUTTON_PIN 19

// Slave IP and port 
const char* slaveIP = "172.20.10.8"; // Replace with the slave ESP32's IP address changes with every new WIFI connected
const int port = 23;

WiFiClient client;

void connectToWiFi() {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi network");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(9600);
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Configure button pin with pull-up resistor
    connectToWiFi();
}

void loop() {
    // Attempt to reconnect to the slave if not connected
    if (!client.connected()) {
        client.connect(slaveIP, port);
    }

    // Check the button state
    if (digitalRead(BUTTON_PIN) == LOW) { // Button pressed
        client.println("PINK"); // Send "PINK" command
        Serial.println("Sent: PINK");
    } else { // Button released
        client.println("OFF"); // Send "OFF" command
        Serial.println("Sent: OFF");
    }

    delay(100); 
}

