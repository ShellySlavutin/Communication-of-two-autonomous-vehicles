
// Define the analog pin connected to the IR sensor
#define IR_SENSOR1_PIN 35 
#define IR_SENSOR2_PIN 36

void setup()
{
  // Initialize serial communication for debugging
  Serial.begin(115200);

  // Configure the IR sensor pin as an input
  pinMode(IR_SENSOR1_PIN, INPUT);
  pinMode(IR_SENSOR2_PIN, INPUT);

}

void loop()
{
  // Read the raw digital value from the IR sensor
  int rawValue1 = digitalRead(IR_SENSOR1_PIN);
  int rawValue2 = digitalRead(IR_SENSOR2_PIN);
  float avgRawValue = (rawValue1+rawValue2) / 2;

  if (avgRawValue == HIGH)
  {
    Serial.print("Raw Value: ");
    Serial.print(avgRawValue);  
    Serial.print("\tThere is no object\n");
  }

  else if (avgRawValue == LOW)
  {
    Serial.print("Raw Value: ");
    Serial.print(avgRawValue);  
    Serial.print("\tThere is an object\n");
  }

  // Delay to reduce noise (adjust as needed)
  delay(100);
}
