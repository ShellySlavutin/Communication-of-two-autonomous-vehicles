#include <Wire.h>

#define IR_SENSOR_PIN1 36
#define IR_SENSOR_PIN1 35

#define DISTANCE

void setup() {
  Serial.begin(115200);
  pinMode(IR_SENSOR_PIN, INPUT);
}

void loop() {
  int sensorValue = analogRead(IR_SENSOR_PIN);
  float distance = sensorValue / 9.766; // Convert sensor value to distance
  Serial.print("Distance: ");
  Serial.println(distance);
  delay(1000);
}