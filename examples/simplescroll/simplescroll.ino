/*
    Name:       MsgBoard_ESP8266_GB.ino
    Created:  7/7/2018 8:36:29 AM
    Author:     Qi Sun
*/

#include <ESP8266_LED_64x16_Matrix_GB.h>

// Connections to board
// latchPin must be D0
#define latchPin D0

//The following pin can be changed if needed
#define clockPin D1
#define data_R1 D3
#define data_R2 D2
#define en_74138 D4
#define la_74138 D8
#define lb_74138 D7
#define lc_74138 D6
#define ld_74138 D5 


ESP8266_LED_64x16_Matrix_GB LEDMATRIX;

void setup()
{
  Serial.begin(115200);

  //I do not know why, this 4 seconds delay is necessary on my board to get things to work
  delay(4000);
  Serial.println("begin");
  uint8_t t[8] = { latchPin, clockPin, data_R1, en_74138, la_74138, lb_74138, lc_74138, ld_74138 };
  LEDMATRIX.setPins(t);

  //Matrix type: 0(other type not supported yet)
  // panel number: 1, could be any numer of panels in serial
  LEDMATRIX.setDisplay(0, 1);

  //expected inPut message is GB encoded characters, two types for each character
  //If you want to display ascii character with one column, you still need to use two type,
  //The first byte needs to be 0xaa, the second byte is the the ascii code
  //0xaa is not used in qu code of GB2312
  String message = "";
  message += (char)0xb0;
  message += (char)0xa2;

  message += (char)0xaa;
  message += 'Q';


  LEDMATRIX.setMessage (message);
  Serial.println("message set");

  LEDMATRIX.turnOn();

}

void loop()
{
  //horizontal scroll matrix, delay time is the movement of 1/8 of a character
  LEDMATRIX.scrollTextHorizontal(100);

  //switching display of different part of a string, delay time is show time of each frame
  //LEDMATRIX.BreakTextInFrames(1000);

  //vertical scroll matrix, delay time is the show time  of each frame
  //LEDMATRIX.scrollTextVertical (1000);
}