const int redPin = 26;  
const int greenPin = 27;
const int bluePin = 2; 

const int redButton = 19;
const int blueButton = 23;

void setup() {
  // Initialize all LEDs as output
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Initialize buttons with internal pull-up resistors
  pinMode(redButton, INPUT_PULLUP);
  pinMode(blueButton, INPUT_PULLUP);
  
  // Turn all LEDs off initially
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
}

void loop() 
{
  // Check if the red button is pressed
  if (digitalRead(redButton) == LOW)
  {
    // Turn on red LED and turn off the others
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
  }

  // Check if the blue button is pressed
  else if (digitalRead(blueButton) == LOW)
  {
    // Turn on blue LED and turn off the others
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, HIGH);
  }

  else
  {
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);    
  }
}
