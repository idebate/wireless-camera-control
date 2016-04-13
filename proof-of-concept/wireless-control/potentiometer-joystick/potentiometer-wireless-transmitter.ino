/*

    Project:      wireless-camera-control
    Section:      transmitter
    Description:  Controls a standard Bescor MP-101 pan/tilt head, using a Moteino + RFM69HW receiver. Direction control using potentiometer joystick.

    Referenced and using example code, logic and format from:

    https://lowpowerlab.com/forum/index.php/topic,84.0.html

    http://protechy.com/bescor-mp-101-hack/

    The code is licensed under GPL.

*/

  
  #include <RFM69.h>
  #include <Wire.h>
  
  #define POT_PAN         14
  #define POT_TILT        15
  #define POT_ZOOM        16
  #define DELAY_STABILIZE 100
  
  #define NODEID          99
  #define NETWORKID       100
  #define GATEWAYID       1
  #define FREQUENCY       RF69_915MHZ
  #define KEY             "wirelessCameraMV"    //has to be same 16 characters/bytes on all nodes, not more not less!
  #define LED             9
  #define SERIAL_BAUD     9600
  
  RFM69 radio;                                  // Initialize radio.
  
  typedef struct 
  {    
  int                     Direction;            // Integer value to hold the direction of pan/tilt - combination of binary values (1L, 2R, 4U, 8D).
  } PTZPacket;
  
  PTZPacket PTZData;                            // Initialize data packet to hold required values for node control.
  
  int joystick_combo = 0;
  int valPan = 0;
  int valTilt = 0;
  int valZoom = 0;

  void setup()
  {
    Serial.begin(SERIAL_BAUD);
    radio.initialize(FREQUENCY,NODEID,NETWORKID);
    radio.setHighPower(); 
    radio.encrypt(KEY);
  }
  
  int handle_joystick()
  {
    int joystick_moved = 0;
    valPan = analogRead(POT_PAN);
    valTilt = analogRead(POT_TILT);
    valZoom = analogRead(POT_ZOOM);
    if (valPan < 500) { joystick_moved = joystick_moved + 1; }
    if (valPan > 560) { joystick_moved = joystick_moved + 2; }
    if (valTilt > 550) { joystick_moved = joystick_moved + 4; }
    if (valTilt < 480) { joystick_moved = joystick_moved + 8; }
    return joystick_moved;
  }

  void loop()
  {
    int joystick_combo = handle_joystick();
    if (joystick_combo != 0)
    {
      for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
      Serial.println();
      PTZData.Direction = joystick_combo;
      radio.send(GATEWAYID, (const void*)(&PTZData), sizeof(PTZData));   // transmit data to other end
      Serial.print("Sending struct (");
      Serial.print(sizeof(PTZData));
      Serial.println(" bytes) ");
      Serial.println();
    }
    delay(DELAY_STABILIZE);
  }

