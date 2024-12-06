#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_AHT10.h>

// Constants for OLED display
#define SCREEN_WIDTH 128  // OLED display width in pixels
#define SCREEN_HEIGHT 64  // OLED display height in pixels
#define OLED_RESET -1     // Reset pin (-1 if using Arduino reset pin)
#define I2C_ADDRESS 0x3C  // Default I2C address for OLED display

// Constants for AHT10
#define AHT10_ADDRESS 0x38
#define AHT_RESET_CMD 0xBA
#define AHT_START_MEASUREMENT 0xAC

// Constants for LED pins
#define RED_PIN 26    // Pin for the red LED
#define GREEN_PIN 27  // Pin for the green LED
#define BLUE_PIN 2    // Pin for the blue LED (not used in this code)

// Create objects for OLED display and AHT10 sensor
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_AHT10 aht;

void setup()
{
  // Configure LED pins as outputs
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);

  // Initialize the AHT10 sensor
  Wire.begin();
  aht.begin();

  // Initialize the OLED display
  display.begin(I2C_ADDRESS, true);  // true -> use internal buffer
  display.clearDisplay();           // Clear the display
  display.setTextColor(SH110X_WHITE); // Set text color to white
  display.setTextSize(1);            // Set default text size
}

void loop()
{
  // Read data from the sensor
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  // Control LED indicators based on temperature
  if (temp.temperature > 29) {
    digitalWrite(GREEN_PIN, LOW);  // Turn off green LED
    digitalWrite(RED_PIN, HIGH);  // Turn on red LED
  } else if (temp.temperature < 28) {
    digitalWrite(GREEN_PIN, HIGH); // Turn on green LED
    digitalWrite(RED_PIN, LOW);    // Turn off red LED
  } else {
    digitalWrite(GREEN_PIN, LOW);  // Turn off both LEDs
    digitalWrite(RED_PIN, LOW);
  }

  // Display temperature and humidity data on the OLED
  display.setCursor(1, 1); 
  display.print("Temp: ");
  display.print(temp.temperature);
  display.print("C");

  display.setCursor(1, 10); 
  display.print("Humidity: ");
  display.print(humidity.relative_humidity);
  display.print("%");

  display.display();  // Update the display
  delay(100);         // Small delay before the next update
  display.clearDisplay(); // Clear the display for new data
}


