

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


void setup()
{

  ledcAttachChannel(Motor_B1 , Freq, Resolution, PWM_CHANNEL_B1);
  ledcAttachChannel(Motor_F1 , Freq, Resolution, PWM_CHANNEL_F1);
  ledcAttachChannel(Motor_B2 , Freq, Resolution, PWM_CHANNEL_B2);
  ledcAttachChannel(Motor_F2 , Freq, Resolution, PWM_CHANNEL_F2);
}
 

float speed = 0.5 ;


void loop()
{

    ledcWriteChannel(PWM_CHANNEL_F1, speed*255);
    ledcWriteChannel(PWM_CHANNEL_F2, speed*255);

    ledcWriteChannel(PWM_CHANNEL_B1, 0);
    ledcWriteChannel(PWM_CHANNEL_B2, 0);
}



