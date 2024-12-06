#include <ESP32Servo.h>

const int servoPin = 18;
const int time_for_delay = 2000;

Servo myServo;

void setup() {
  myServo.attach(servoPin);
}
void loop() {
  myServo.write(0);  // Rotate the servo to 0 degrees
  delay(time_for_delay);  // Wait for 2 seconds

  myServo.write(90);  // Rotate the servo to 90 degrees
  delay(time_for_delay);  // Wait for 2 seconds

  myServo.write(180);  // Rotate the servo to 180 degrees
  delay(time_for_delay);  // Wait for 2 seconds
}

