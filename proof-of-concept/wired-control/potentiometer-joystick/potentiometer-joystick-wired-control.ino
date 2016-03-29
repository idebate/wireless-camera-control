/*

    Project:      wireless-camera-control
    Section:      wired control via potentiometer based joystick
    Description:  Controls a standard Bescor MP-101 pan/tilt head, using a Moteino. Direction control using potentiometer joystick.

    Referenced and using example code, logic and format from:

    https://lowpowerlab.com/forum/index.php/topic,84.0.html

    http://protechy.com/bescor-mp-101-hack/

    The code is licensed under GPL.

*/

#include <Wire.h>
  
#define POT_PAN         14		  // Or A0 on Uno.
#define POT_TILT        15		  // Or A1 on Uno.

typedef struct 
{    
  int   DirPan;                   // Integer value to hold the direction of pan. 0 = stop, 1 = left, 2 = right.
  int   DirTilt;                  // Integer value to hold the direction of tilt. 0 = stop, 1 = down, 2 = up.
  int   SpeedPan;                 // Integer value to hold the speed of panning. Ranges from 0 to 255 to correspond with AnalogWrite().
  int   SpeedTilt;                // Integer value to hold the speed of tilting. Ranges from 0 to 255 to correspond with AnalogWrite().
  int   PTZMovement;              // Integer value to represent a movement detection flag. 0 for no movement, 1 for any movement.
} PTZPacket;
  
PTZPacket PTZData;                // Initialize data packet to hold required values for node control.
  
int joystick_combo = 0;
int valPan = 0;
int valTilt = 0;
int motor_pan[] = {6, 9};
int motor_tilt[] = {3, 5};

void setup()
{
  int i;
  for(i = 0; i <=] 1; i++){
    pinMode(motor_pan[i], OUTPUT);
    pinMode(motor_tilt[i], OUTPUT);
  }
}
  
void handle_joystick()
{
  PTZData.DirPan = 0;
  PTZData.DirTilt = 0;
  PTZData.SpeedPan = 0;
  PTZData.SpeedTilt = 0;
  PTZData.PTZMovement = 0;
  valPan = analogRead(POT_PAN);
  valTilt = analogRead(POT_TILT);
  if (valPan < 500) { PTZData.DirPan = 1; PTZData.SpeedPan = map(valPan, 0, 470, 255, 0); PTZData.PTZMovement = 1; }
  if (valPan > 560) { PTZData.DirPan = 2; PTZData.SpeedPan = map(valPan, 560, 1023, 0, 255); PTZData.PTZMovement = 1; }
  if (valTilt < 470) { PTZData.DirTilt = 1; PTZData.SpeedTilt = map(valTilt, 0, 470, 255, 0); PTZData.PTZMovement = 1; }
  if (valTilt > 560) { PTZData.DirTilt = 2; PTZData.SpeedTilt = map(valTilt, 560, 1023, 0, 255); PTZData.PTZMovement = 1; }
}

void loop()
{
  handle_joystick();
  if (PTZData.PTZMovement != 0)
  {
	movePan(PTZData.DirPan, PTZData.SpeedPan);
	moveTilt(PTZData.DirTilt, PTZData.SpeedTilt);
  }
  else
  {
    movePan(0, 0);
    moveTilt(0, 0);
  }
  delay(DELAY_STABILIZE);
}

void movePan(int dirPan, int speedPan)
{
  if (dirPan == 1)
  {
    analogWrite(motor_pan[0], speedPan);
    digitalWrite(motor_pan[1], LOW);
  }
  elseif (dirPan == 2)
  {
    digitalWrite(motor_pan[0], LOW);
    analogWrite(motor_pan[1], speedPan);
  }
  else
  {
    digitalWrite(motor_pan[0], LOW);
    digitalWrite(motor_pan[1], LOW);
  }
}

void moveTilt(int dirTilt, int speedTilt)
{
  if (dirTilt == 1)
  {
    analogWrite(motor_tilt[0], speedTilt);
    digitalWrite(motor_tilt[1], LOW);
  }
  elseif (dirTilt == 2)
  {
    digitalWrite(motor_tilt[0], LOW);
    analogWrite(motor_tilt[1], speedTilt);
  }
  else
  {
    digitalWrite(motor_tilt[0], LOW);
    digitalWrite(motor_tilt[1], LOW);
  }
}
