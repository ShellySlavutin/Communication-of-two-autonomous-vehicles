#define TRIG_PIN 8          
#define ECHO_PIN 9         
#define RED_PIN 26          
#define GREEN_PIN 27       
#define BLUE_PIN 2          

#define MAX_DISTANCE 10     

void setup() {
  // Initialize pins for the RGB LED
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Initialize pins for the ultrasonic sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Ensure LED starts off
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);

  // Start the serial monitor for debugging
  Serial.begin(9600);
}

void loop() {
  long duration, distance;

  // Send a pulse (Trig pin HIGH for 10 microseconds)
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the duration of the pulse
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate the distance in cm
  // *!* for some reason the data on the unltrasonic and the debugging
  // print only matches when dvided by 5.8 and not 58 (which is what is supposed to be)
  distance = duration / 5.8;

  // Print the distance for debugging
  Serial.print("Distance: ");
  Serial.println(distance);

  // Turn the RGB LED red if the distance is 10cm or less
  if (distance <= MAX_DISTANCE) {
    digitalWrite(RED_PIN, HIGH);   // Turn red LED on
    digitalWrite(GREEN_PIN, LOW);  // Turn green LED off
    digitalWrite(BLUE_PIN, LOW);   // Turn blue LED off (not used)
  } else {
    digitalWrite(RED_PIN, LOW);    // Turn red LED off
    digitalWrite(GREEN_PIN, HIGH); // Turn green LED on
    digitalWrite(BLUE_PIN, LOW);   // Turn blue LED off (not used)
  }

  delay(100);
}
