const int redPin = 26;  
const int greenPin = 27;
const int bluePin = 2; 

const int LDRpin = 34;

void setup() {
  // Initialize all LEDs as output
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Init LDR pin
  pinMode(LDRpin, INPUT);

  // Turn all LEDs off initially
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);

  // serial init
  Serial.begin(9600);
}

void loop() 
{
  // check if there is light (high signal)
  if (digitalRead(LDRpin) == HIGH) 
  {
    // Turn off all leds
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);  

    Serial.println("it is light outside, turn off the light!");
  } 
  else 
  {
    // Turn on blue LED and turn off the others
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, HIGH);  

    Serial.println("it is dark outside, turn on the light!");
  }
}
