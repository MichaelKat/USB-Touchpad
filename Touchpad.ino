// **********************************************************
//   Touchpad.ino
//
//   This sketch is a work in progress to make a standalone USB touchpad from a salvaged laptop.
//
//   Note: It is not super user-friendly yet; the mouse is not dragged, but rather follows a vector 
//         from the center of the touchpad to the point of contact (e.g. touching the touchpad 
//         in the middle would not do anything, but touching the touchpad in the upper-right 
//         corner would rapidly and steadily move the mouse to the right and up, as long as 
//         something is making contact with the touchpad). The further from the center you touch
//         the touchpad, the faster the mouse moves.
//
//   The touchpad in question uses PS2 protocol, so the arduino acts as the translator from PS2 to USB.
//   
//   Author: Michael Katilevsky
//
//   Started: 8 August 2017
//
// **********************************************************


#include <Mouse.h>
#include <ps2.h>

#define PS2_DATA 10
#define PS2_CLK 9

byte mstat1;
byte mstat2;
byte mxy;
byte mx;                  
byte my;      
byte mz;               
int msval[2];
int repeatCnt;

PS2 moose(PS2_CLK, PS2_DATA);     

// My touchpad also had built in mouse buttons
int rightButtonCounter;
int leftButtonCounter;

void setup() {
  pinMode(7, INPUT); //left button, black wire
  pinMode(8, INPUT); //right button, green wire
  rightButtonCounter = 0;
  leftButtonCounter = 0;

   Mouse.begin();
  moose.write(0xff);  
  moose.read();
  moose.read();
  moose.read(); 
//  moose.write(0xf0); 
//  moose.read(); 
  delayMicroseconds(100);
  moose.write(0xe8);
  moose.read(); 
  moose.write(0x03);
  moose.read(); 
  moose.write(0xe8);
  moose.read(); 
  moose.write(0x00); 
  moose.read();  
  moose.write(0xe8);
  moose.read();  
  moose.write(0x01);
  moose.read(); 
  moose.write(0xe8);
  moose.read(); 
  moose.write(0x00); 
  moose.read(); 
  moose.write(0xf3);
  moose.read(); 
  moose.write(0x14);
  moose.read(); 
  Serial.begin(9600);
}

void ms_read()
{moose.write(0xeb);
  moose.read();
  mstat1 = moose.read();
  mxy = moose.read();
  mz = moose.read();
  mstat2 = moose.read();
  mx = moose.read();
  my = moose.read();
  msval[0] = (((mstat2 & 0x10) << 8) | ((mxy & 0x0F) << 8) | mx );
  msval[1] = (((mstat2 & 0x20) << 7) | ((mxy & 0xF0) << 4) | my );
  msval[2] = int(mz);
}

void loop() {
  ms_read();

  if (msval[0] > 0 and msval[2] > 10)
  { repeatCnt++; }
  else
  { repeatCnt = 0; }
 
  if (repeatCnt > 2)
  {
    //stores x coordinate
    msval[0] = map(msval[0], 0, 7000, 1023, -1023);   // 0, 7000 are the x-axis bounds for my touchpad
    
    //stores y coordinate
    msval[1] = map(msval[1], 0, 4000, -1023, 1023);   //0, 4000 are the y-axis bounds for my touchpad
    
    //creates the vector that moves the mouse
    Mouse.move(msval[1]/50,msval[0]/35); //divided by 50 and 35 for lower sensitivity

  //left button has been pressed, record for how long
  while(digitalRead(7) == LOW && leftButtonCounter < 150)
  {
    leftButtonCounter++;
  }
  
  if(leftButtonCounter < 150 && leftButtonCounter > 0)
  {
    Mouse.click(MOUSE_RIGHT);
  }

  //if button has been pressed for longer than 150ms, treat it like a long-press, not a click
  else if(leftButtonCounter != 0)
  {
    Mouse.press(MOUSE_RIGHT);
    while(digitalRead(7) == LOW);
    Mouse.release(MOUSE_RIGHT);
  }
  
  leftButtonCounter = 0; //reset counter

  //right button has been pressed, record for how long
  while(digitalRead(8) == LOW && rightButtonCounter < 150)
  {
    rightButtonCounter++;
  }
  
  if(rightButtonCounter < 150  && rightButtonCounter > 0)
  {
    Mouse.click();
  }

  //if button has been pressed for longer than 150ms, treat it like a long-press, not a click
  else if(rightButtonCounter != 0)
  {
    Mouse.press();
    while(digitalRead(8) == LOW);
    Mouse.release();
  }

  rightButtonCounter = 0; //reset counter
  }
}
