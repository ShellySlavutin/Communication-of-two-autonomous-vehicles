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



void setup()
{
    Serial.begin(9600);

     ledcAttachChannel(Motor_B1 , Freq, Resolution, PWM_CHANNEL_B1);
     ledcAttachChannel(Motor_F1 , Freq, Resolution, PWM_CHANNEL_F1);
     ledcAttachChannel(Motor_B2 , Freq, Resolution, PWM_CHANNEL_B2);
     ledcAttachChannel(Motor_F2 , Freq, Resolution, PWM_CHANNEL_F2);

    // Initialize the OLED screen
    display.begin(i2c_Address, true); 
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);

    if (esp_now_init() != ESP_OK)
    {
    Serial.println("Error initializing ESP-NOW");
    return;
    }

    
  }

}

void loop()
{

    if (client)
    {
        //Serial.println("Client connected");

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
                    Serial.println("Sent: Reply: STOP received");
                }

                if (command == "START")
                {
                    moveForward();
                    Serial.println("Motors are moving");

                    //Send confirmation back to the master
                    client.println("START received");
                    Serial.println("Sent: Reply: START received");
                }

            }
        }
    }
}

