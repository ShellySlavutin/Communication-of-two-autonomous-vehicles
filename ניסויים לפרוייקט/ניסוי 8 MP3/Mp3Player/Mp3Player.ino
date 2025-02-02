#include "DFRobotDFPlayerMini.h" // library

HardwareSerial mp3Serial(1);  //  Defines UART1 for communicating with DFPlayer Mini
DFRobotDFPlayerMini mp3;      // Create an object to control mp3

void setup()
{
  // Start Serial communication with DFPlayer Mini
  mp3Serial.begin(9600, SERIAL_8N1, 16, 17);  //rx=16, tx=17

  mp3.begin(mp3Serial); // Initialize DFPlayer Mini
  mp3.volume(30); // Set volume to 30

  mp3.play(1);// Play the first MP3 file (0001.mp3)
  delay(2000);
  mp3.play(2);// Play the first MP3 file (0002.mp3)
  delay(2000);
  mp3.play(3);// Play the first MP3 file (0003.mp3)
  delay(2000);
}

void loop()
{

}