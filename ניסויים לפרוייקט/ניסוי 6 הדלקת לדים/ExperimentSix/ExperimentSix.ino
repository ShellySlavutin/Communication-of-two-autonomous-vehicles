
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 5 
#define NUM_PIXELS 6

#define LDR 34

Adafruit_NeoPixel NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  NeoPixel.begin(); // initialize NeoPixel strip object (REQUIRED)

  delay (200);

  NeoPixel.clear(); // Start the program with lights off
  NeoPixel.show(); // update to the NeoPixel Led Strip

  pinMode(LDR, INPUT); // Init the LDR
}

void loop() {
  if (digitalRead(LDR) == LOW)
  {
    // Turn on the lights

    // The front leds will be white (because of the blue tint we put it on 150)
    NeoPixel.setPixelColor(0, NeoPixel.Color(255, 255, 150));  
    NeoPixel.setPixelColor(1, NeoPixel.Color(255, 255, 150));  
    NeoPixel.setPixelColor(2, NeoPixel.Color(255, 255, 150));  
    NeoPixel.setPixelColor(3, NeoPixel.Color(255, 255, 150)); 

    // The back leds will be red
    NeoPixel.setPixelColor(4, NeoPixel.Color(255, 0, 0));  
    NeoPixel.setPixelColor(5, NeoPixel.Color(255, 0, 0));  

    // Winkers will be yellow 
    //NeoPixel.setPixelColor(2, NeoPixel.Color(255, 100, 0));  
    //NeoPixel.setPixelColor(3, NeoPixel.Color(255, 100, 0));  

    NeoPixel.show(); // update to the NeoPixel Led Strip

  }
  else
  {
    // Turn off the lights
    NeoPixel.clear(); // Start the program with lights off
    NeoPixel.show(); // update to the NeoPixel Led Strip

  }
}
