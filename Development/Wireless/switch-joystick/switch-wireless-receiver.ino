/*

    Project:      wireless-camera-control
    Section:      receiver
    Description:  Controls a standard Bescor MP-101 pan/tilt head, using a Moteino + RFM69HW receiver.

    Referenced and using example code, logic and format from:

    https://lowpowerlab.com/forum/index.php/topic,84.0.html

    http://protechy.com/bescor-mp-101-hack/

    The code is licensed under GPL.

*/

  #include <RFM69.h>
  #include <Wire.h>

  #define ACT_LEFT        3                         // Pan left direction. Typically the yellow wire on the Bescor MP-101 connection.
  #define ACT_RIGHT       4                         // Pan right direction. Typically the green wire on the Bescor MP-101 connection.
  #define ACT_UP          7                         // Tilt up direction. Typically the black wire on the Bescor MP-101 connection.
  #define ACT_DOWN        8                         // Tilt down direction. Typically the blue wire on the Bescor MP-101 connection.
  #define DELAY_STABILIZE 100                       // Typical delay for the stabilization of radio initialization.
  #define DELAY_COMMSTOP  10                        // Typical delay for communication count. Important in order to track radio communication end.

  #define NODEID          1                         // ID of the receiver node. Unique to every receiver.
  #define NETWORKID       100                       // ID of the network. Same across all nodes.
  #define FREQUENCY       RF69_915MHZ
  #define KEY             "wirelessCameraMV"        // Exactly 16 characters, needs to be exactly the same across all nodes.
  #define SERIAL_BAUD     9600

  RFM69 radio;                                      // Initialize radio.

  typedef struct 
  {    
    int                   Direction = 0;            // Integer value to hold the direction of pan/tilt - combination of binary values (1L, 2R, 4U, 8D). 
  } PTZPacket;
  
  PTZPacket PTZData;                                // Initialize data packet to hold required values for node control.

  int commCount = 1;

  void setup() 
  {
    Serial.begin(SERIAL_BAUD);
    delay(DELAY_STABILIZE);
    radio.initialize(FREQUENCY,NODEID,NETWORKID);
    radio.setHighPower(); //uncomment only for RFM69HW!
    radio.encrypt(KEY);
    radio.promiscuous(false);
  }

  void loop() 
  {
    if (radio.receiveDone())
    {  
      if (radio.DATALEN == sizeof(PTZPacket))
      {
        PTZData = *(PTZPacket*)radio.DATA;
        movePTH(PTZData.Direction);
      }
    }
    else
    {
      commCount += 1;
      if (commCount == 5) { movePTH(0); }
      if (commCount > 100) { commCount = 6; }
    }
    delay(DELAY_COMMSTOP);
  }

  void movePTH(int PTHPos)
  {
    if (PTHPos != 0) { commCount = 0; }
    switch (PTHPos)
    {
      case 0:
        digitalWrite(ACT_LEFT, LOW); digitalWrite(ACT_RIGHT, LOW); digitalWrite(ACT_UP, LOW); digitalWrite(ACT_DOWN, LOW);
        break;
      case 1:
        digitalWrite(ACT_RIGHT, LOW); digitalWrite(ACT_UP, LOW); digitalWrite(ACT_DOWN, LOW);
        analogWrite(ACT_LEFT, 255);
        break;
      case 2:
        digitalWrite(ACT_LEFT, LOW); digitalWrite(ACT_UP, LOW); digitalWrite(ACT_DOWN, LOW);
        analogWrite(ACT_RIGHT, 255);
        break;
      case 4:
        digitalWrite(ACT_LEFT, LOW); digitalWrite(ACT_RIGHT, LOW); digitalWrite(ACT_DOWN, LOW);
        analogWrite(ACT_UP, 255);
        break;
      case 8:
        digitalWrite(ACT_LEFT, LOW); digitalWrite(ACT_RIGHT, LOW); digitalWrite(ACT_UP, LOW);
        analogWrite(ACT_DOWN, 255);
        break;
      case 5:
        digitalWrite(ACT_RIGHT, LOW); digitalWrite(ACT_DOWN, LOW);
        analogWrite(ACT_LEFT, 255);
        analogWrite(ACT_UP, 255);
        break;
      case 6:
        digitalWrite(ACT_LEFT, LOW); digitalWrite(ACT_DOWN, LOW);
        analogWrite(ACT_RIGHT, 255);
        analogWrite(ACT_UP, 255);
        break;
      case 9:
        digitalWrite(ACT_RIGHT, LOW); digitalWrite(ACT_UP, LOW);
        analogWrite(ACT_LEFT, 255);
        analogWrite(ACT_DOWN, 255);
        break;
      case 10:
        digitalWrite(ACT_LEFT, LOW); digitalWrite(ACT_UP, LOW);
        analogWrite(ACT_RIGHT, 255);
        analogWrite(ACT_DOWN, 255);
        break;
    }
  }
