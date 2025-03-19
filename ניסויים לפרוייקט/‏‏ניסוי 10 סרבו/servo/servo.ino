#include <ESP32Servo.h>

#define SERVO 18
#define DELAY_SERVO 2000

Servo ultrasonicServo;

//Ultrasonic Pins
#define triger_Pin 32
#define echo_Pin 33  

#define MIN_DISTANCE 10 // Minimum distance for stop function

// IR Sensor Pins
#define STRIP_SENSOR_1 36
#define STRIP_SENSOR_2 39
#define STRIP_SENSOR_3 15
#define STRIP_SENSOR_4 5

// Motors pins
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

float speed = 0.5;
float distance;

void setup() 
{
  pinMode(triger_Pin, OUTPUT);
  pinMode(echo_Pin, INPUT);

  ledcAttachChannel(Motor_B1 , Freq, Resolution, PWM_CHANNEL_B1);
  ledcAttachChannel(Motor_F1 , Freq, Resolution, PWM_CHANNEL_F1);
  ledcAttachChannel(Motor_B2 , Freq, Resolution, PWM_CHANNEL_B2);
  ledcAttachChannel(Motor_F2 , Freq, Resolution, PWM_CHANNEL_F2);

  pinMode(STRIP_SENSOR_1, INPUT);
  pinMode(STRIP_SENSOR_2, INPUT);
  pinMode(STRIP_SENSOR_3, INPUT);
  pinMode(STRIP_SENSOR_4, INPUT);

  ultrasonicServo.attach(SERVO);
}

void loop() 
{
  moveAccordingToStrip();
  /*ultrasonicServo.write(90);  // Rotate the servo to 90 degrees, forward
  distance = calculateDistance();

  if (distance < MIN_DISTANCE)
  {
    stopMotors();
    // There is an obstecle forward, check other 
    ultrasonicServo.write(145);  // Rotate the servo to the right
    distance = calculateDistance();

    if(distance < MIN_DISTANCE)
    {
       rightTurn();
    }

    else 
    {
      ultrasonicServo.write(35);  // Rotate the servo to the left
      distance = calculateDistance();
    
      if(distance < MIN_DISTANCE)
      {
        leftTurn();
      }

      else
      {
        moveAccordingToStrip();
      }

    }
  }

  else
  {
    moveAccordingToStrip();
  }
*/
  delay(100);

}

void rightTurn()
{
  motorsWrite(0,1,0,0);
  if (!digitalRead(STRIP_SENSOR_1)) // A case in which the turn is wide and no sensor can see the line
  {  
    while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)); // keep turning until it sees the line
  }  
}

void leftTurn()
{
    motorsWrite(1,0,0,0);
    if (!digitalRead(STRIP_SENSOR_4)) // A case in which the turn is wide and no sensor can see the line
    {  
      while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)); // keep turning until it sees the line
    }
}

void stopMotors()
{
  // stooping the motors is the same as setting the pwm to 0 which is what we did here
  speed = 0;
  ledcWriteChannel(PWM_CHANNEL_F1, speed*255);
  ledcWriteChannel(PWM_CHANNEL_F2, speed*255);

  ledcWriteChannel(PWM_CHANNEL_B1, 0);
  ledcWriteChannel(PWM_CHANNEL_B2, 0);
}

float calculateDistance()
{
  digitalWrite(triger_Pin, LOW);
  delayMicroseconds(2);
  digitalWrite(triger_Pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triger_Pin, LOW);
  float Duration = pulseIn(echo_Pin, HIGH);
  float distance = Duration / 58.0;
  return distance;
}

void motorsWrite(float m1, float m2, float m3, float m4) 
{
  ledcWrite(Motor_F1, m1 * 255);
  ledcWrite(Motor_F2, m2 * 255);
  ledcWrite(Motor_B1, m3 * 255);
  ledcWrite(Motor_B2, m4 * 255);
}

void moveAccordingToStrip()
{
  // Drive forward
  if (digitalRead(STRIP_SENSOR_2) && digitalRead(STRIP_SENSOR_3))
  {
    motorsWrite(0.4, 0.4, 0, 0);

  // Extreme correcting to the left at the start, when the middle sensors see the line
   if(digitalRead(STRIP_SENSOR_4))
   {
    motorsWrite(1,0,0,0);
   }

   // Extreme correcting to the right at the start, when the middle sensors see the line
   else if(digitalRead(STRIP_SENSOR_1))
   {
    motorsWrite(0,1,0,0);
   }
  } 

  else if(digitalRead(STRIP_SENSOR_4) && digitalRead(STRIP_SENSOR_2) && digitalRead(STRIP_SENSOR_3))
  {
     stopMotors();
     ultrasonicServo.write(90);  // Rotate the servo to 90 degrees, forward
     distance = calculateDistance();

     if (distance < MIN_DISTANCE)
     {
       motorsWrite(1,0,0,0);
       if(digitalRead(STRIP_SENSOR_4))
       {
          motorsWrite(1,0,0,0);
          if (!digitalRead(STRIP_SENSOR_4)) // A case in which the turn is wide and no sensor can see the line
          {  
            while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)); // keep turning until it sees the line
          }
       }
     }

     else
     {
        motorsWrite(0.4, 0.4, 0, 0);
     }
  }
       
  // Extreme correcting to the left in the end, when only STRIP_SENSOR_4 is able to see the line
  else if(digitalRead(STRIP_SENSOR_4))
  {
       motorsWrite(1,0,0,0);
       if (!digitalRead(STRIP_SENSOR_4)) // A case in which the turn is wide and no sensor can see the line
       {  
         while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)); // keep turning until it sees the line
       }
  }

  // Extreme correcting to the right in the end, when only STRIP_SENSOR_1 is able to see the line
  else if(digitalRead(STRIP_SENSOR_1))
  {
      motorsWrite(0,1,0,0);
      if (!digitalRead(STRIP_SENSOR_1)) // A case in which the turn is wide and no sensor can see the line
      {  
        while(!digitalRead(STRIP_SENSOR_2) && !digitalRead(STRIP_SENSOR_3)); // keep turning until it sees the line
      } 
  }

  // Minor correcting to the left, when the car moves a bit, when there are twists.
  else if (digitalRead(STRIP_SENSOR_3))
  {
    motorsWrite(0.8, 0.1, 0, 0);
  } 

  // Minor correcting to the right, when the car moves a bit, when there are twists.
  else if (digitalRead(STRIP_SENSOR_2))
  {
    motorsWrite(0.1, 0.8, 0, 0);
  } 

  else if (digitalRead(STRIP_SENSOR_2) && digitalRead(STRIP_SENSOR_1) && digitalRead(STRIP_SENSOR_3) && digitalRead(STRIP_SENSOR_4))
  {
    motorsWrite(0.1, 0.8, 0, 0);
  } 

  // Stop
  else
  {
    motorsWrite(0, 0, 0, 0);
  }
}