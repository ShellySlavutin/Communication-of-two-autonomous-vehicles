#define PASSWORD "1234"  
#define MAX_ATTEMPTS 3   
#define LOCK_TIME 5000   

int attempts = 0;        

void setup() {
  Serial.begin(9600);
  delay(1000); // small delay is needed to wait that the serial is up 
  Serial.println("Welcome! Please enter your password:");
}

void loop() {
  if (attempts >= MAX_ATTEMPTS) {
    Serial.println("System is locked for 5 seconds. Please wait...");
    delay(LOCK_TIME);
    attempts = 0;
    Serial.println("You can try again. Please enter your password:");
  }

  if (Serial.available() > 0) {
    String inputPassword = Serial.readStringUntil('\n');  
    inputPassword.trim(); // remove stuff like backspaces, tabs etc from the end of the pass

    if (inputPassword.equals(PASSWORD)) {
      Serial.println("Typed pass is " + inputPassword);
      Serial.println("Access Granted. Welcome!");
      attempts = 0;  // change the numbers of attemps
    } else {
      attempts++;
      Serial.println("Incorrect password.");

      if (attempts >= MAX_ATTEMPTS) {
        Serial.println("Too many failed attempts. System locked for 5 seconds.");
      } else {
        Serial.println("Attempt " + String(attempts) + " of " + String(MAX_ATTEMPTS));
        Serial.println("Typed pass is " + inputPassword);
      }
    }
  }
}
