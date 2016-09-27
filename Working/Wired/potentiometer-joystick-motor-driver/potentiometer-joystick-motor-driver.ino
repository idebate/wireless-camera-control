/*

    Project:      wireless-camera-control
    Section:      wired control via potentiometer based joystick
    Description:  Controls a standard Bescor MP-101 pan/tilt head, using a Moteino. Direction control using potentiometer joystick, along with a TB6612FNG motor driver.

    Referenced and using example code, logic and format from:

    https://lowpowerlab.com/forum/index.php/topic,84.0.html

    http://protechy.com/bescor-mp-101-hack/

    The code is licensed under GPL.

*/

  #include <RFM69.h>
  #include <Wire.h>
   
  #define POT_PAN         14		  // Or A0 on Uno.
  #define POT_TILT        15		  // Or A1 on Uno.
 
  // Communication delays and serial configuration.
  #define DELAY_STABILIZE 10
  #define DELAY_COMMSTOP  1
  #define DELAY_MOTORSTOP 125
  #define SERIAL_BAUD     115200

  // Motor driver settings.
  #define MP_SPEED        3
  #define MP_DIR1         8
  #define MP_DIR2         9
  #define MT_SPEED        5
  #define MT_DIR1         6
  #define MT_DIR2         7
  #define MR_STBY        10

  // PTZPacket data structure.
  typedef struct 
  {    
    int   DirPan;
    int   DirTilt;
    int   SpeedPan;
    int   SpeedTilt;
    int   PTZMovement;
  } PTZPacket;
  
  PTZPacket PTZData;

  void setup()
  {
    // Start serial communication.
    Serial.begin(SERIAL_BAUD);
    delay(DELAY_STABILIZE);

    // Set output pins.
    for (int i = 3; i <= 10; i++) { if (i != 4) { pinMode(i, OUTPUT); } }
    for (int i = 14; i <= 15 ; i++) { pinMode(i, OUTPUT); }
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
	  if (PTZData.DirPan != 0) { PTH_Move(1, PTZData.SpeedPan, PTZData.DirPan); }
	  if (PTZData.DirTilt != 0) { PTH_Move(2, PTZData.SpeedTilt, PTZData.DirTilt); }
    }
    else
    {
      PTH_Stop;
    }
    delay(DELAY_STABILIZE);
  }

  void PTH_Move(int motorNumber, int motorSpeed, int motorDirection)
  {
    // motorNumber: 1 for Pan, and 2 for Tilt.
    // motorSpeed: Between a set minimum (typically 160 for lower voltage threshold of DC motors) and 255.
    // motorDirection: 1 for Left/Down, 2 for Right/Up.

    // Disable standby.
    digitalWrite(MR_STBY, HIGH);

    // Set direction pins.
    boolean inPin1 = LOW;
    boolean inPin2 = HIGH;
    if (motorDirection == 1) { inPin1 = HIGH; inPin2 = LOW; }

    if (motorNumber == 1)
    {
      if (PTZData.DirTilt == 0) { analogWrite(MT_SPEED, 0); }
      digitalWrite(MP_DIR1, inPin1); digitalWrite(MP_DIR2, inPin2); analogWrite(MP_SPEED, motorSpeed);
    }
    else
    {
      if (PTZData.DirPan == 0) { analogWrite(MP_SPEED, 0); }
      digitalWrite(MT_DIR1, inPin1); digitalWrite(MT_DIR2, inPin2); analogWrite(MT_SPEED, motorSpeed);
    }
  }

  void PTH_Stop()
  {
    digitalWrite(MR_STBY, LOW); 
    analogWrite(MP_SPEED, 0);
    analogWrite(MT_SPEED, 0);
  }