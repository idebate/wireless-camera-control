/*

    Project:      wireless-camera-control
    Section:      transmitter
    Description:  Controls a standard Bescor MP-101 pan/tilt head, using a Moteino + RFM69HW receiver.

    Referenced and using example code, logic and format from:

    https://lowpowerlab.com/forum/index.php/topic,84.0.html

    http://protechy.com/bescor-mp-101-hack/

    The code is licensed under GPL.

*/

  #include <RFM69.h>
  #include <Wire.h>
  
  #define BUTTON_LEFT     14                        // Pan left direction.
  #define BUTTON_RIGHT    15                        // Pan right direction.
  #define BUTTON_UP       16                        // Tilt up direction.
  #define BUTTON_DOWN     17                        // Tilt down direction.
  #define DELAY_STABILIZE 100                       // Typical delay for the stabilization of radio initialization.
  
  #define NODEID          99                        // ID of the transmitter node. Unique to every transmitter. 
  #define NETWORKID       100                       // ID of the network. Same across all nodes.
  #define GATEWAYID       1                         // ID of the sender node. Unique to every receiver, changes based on which camera to control.
  #define FREQUENCY       RF69_915MHZ
  #define KEY             "wirelessCameraMV"        // Exactly 16 characters, needs to be exactly the same across all nodes.
  #define LED             9
  #define SERIAL_BAUD     9600
  
  RFM69 radio;                                      // Initialize radio.
  
  typedef struct 
  {    
  int                     Direction;                // Integer value to hold the direction of pan/tilt - combination of binary values (1L, 2R, 4U, 8D).
  } PTZPacket;
  
  PTZPacket PTZData;                                // Initialize data packet to hold required values for node control.
  
  int button_combo = 0;

  void setup()
  {
    pinMode(BUTTON_LEFT, INPUT);
    pinMode(BUTTON_RIGHT, INPUT);
    pinMode(BUTTON_UP, INPUT);
    pinMode(BUTTON_DOWN, INPUT);
    digitalWrite(BUTTON_LEFT, HIGH);
    digitalWrite(BUTTON_RIGHT, HIGH);
    digitalWrite(BUTTON_UP, HIGH);
    digitalWrite(BUTTON_DOWN, HIGH);
    Serial.begin(SERIAL_BAUD);
    radio.initialize(FREQUENCY,NODEID,NETWORKID);
    radio.setHighPower(); 
    radio.encrypt(KEY);
  }
  
  int handle_button()
  {
    int button_pressed = 0;
    if (!digitalRead(BUTTON_LEFT)) { button_pressed = button_pressed + 1; }
    if (!digitalRead(BUTTON_RIGHT)) { button_pressed = button_pressed + 2; }
    if (!digitalRead(BUTTON_UP)) { button_pressed = button_pressed + 4; }
    if (!digitalRead(BUTTON_DOWN)) { button_pressed = button_pressed + 8; }
    return button_pressed;
  }

  void loop()
  {
    int button_combo = handle_button();
    if (button_combo != 0)
    {
      for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
      Serial.println();
      PTZData.Direction = button_combo;
      radio.send(GATEWAYID, (const void*)(&PTZData), sizeof(PTZData));   // transmit data to other end
      Serial.print("Sending struct (");
      Serial.print(sizeof(PTZData));
      Serial.println(" bytes) ");
      Serial.println();
    }
    delay(DELAY_STABILIZE);
  }
