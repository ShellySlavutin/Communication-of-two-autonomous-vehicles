#include <WiFi.h>

// WiFi credentials
const char* ssid = "SHELLY_SLAVUTIN_IPHONE"; // WiFi name
const char* password = "shellyslavutin";  // WiFi password

// RGB LED pins
#define RED_PIN 26
#define BLUE_PIN 2
#define GREEN_PIN 27

WiFiServer server(23);

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

    delay(100);

    pinMode(RED_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);

    connectToWiFi();
    server.begin();
    Serial.println("Server started");
}

void loop() {
    WiFiClient client = server.available();

    if (client) {
        Serial.println("Client connected");

        while (client.connected()) {
            if (client.available()) {
                String command = client.readStringUntil('\n');
                command.trim(); // Remove extra whitespace

                if (command == "PINK") {
                    // Turn on pink light (red + blue)
                    digitalWrite(RED_PIN, 255);
                    digitalWrite(BLUE_PIN, 255);
                    digitalWrite(GREEN_PIN, 0);
                    Serial.println("Pink LED ON");
                } else if (command == "OFF") {
                    // Turn off the light
                    digitalWrite(RED_PIN, 0);
                    digitalWrite(BLUE_PIN, 0);
                    digitalWrite(GREEN_PIN, 0);
                    Serial.println("LED OFF");
                }
            }
        }
        client.stop();
        Serial.println("Client disconnected");

        // Ensure LEDs are off when the client disconnects
        digitalWrite(RED_PIN, 0);
        digitalWrite(BLUE_PIN, 0);
        digitalWrite(GREEN_PIN, 0);
    }
}

