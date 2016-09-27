#include <RFM69.h>
#include <Wire.h>

#define POT_PAN         A5
#define POT_TILT        A6
#define POT_ZOOM        A7
#define LOWER_LIMIT     440
#define UPPER_LIMIT     590

#define ENC_A     10
#define ENC_B     11
#define ENC_PPR   800
#define ENC_TPR   40

volatile int lastEncoded = 0;
volatile long encoderValue = 0;
long lastencoderValue = 0;
long runningStopped = 0;
int lastMSB = 0;
int lastLSB = 0;

#define CAMERA_START    24                    // First pin in a bank of pins for camera selection buttons.
#define CAMERA_END      28                    // Last pin in a bank of pins for camera selection buttons.

#define NODEID          99
#define NETWORKID       100
#define GATEWAYID       1
#define FREQUENCY       RF69_915MHZ
#define KEY             "wirelessCameraMV"    //has to be same 16 characters/bytes on all nodes, not more not less!

RFM69 radio;                                  // Initialize radio.

#define SERIAL_BAUD     9600

typedef struct
{
  int   DirPan;                   // Integer value to hold the direction of pan. 0 = stop, 1 = left, 2 = right.
  int   DirTilt;                  // Integer value to hold the direction of tilt. 0 = stop, 1 = down, 2 = up.
  int   DirZoom;                  // Integer value to hold the direction of zoom. 0 = stop, 1 = down, 2 = up.
  int   DirFocus;                 // Integer value to hold the direction of focus. 0 = stop, 1 = down, 2 = up.
  int   SpeedPan;                 // Integer value to hold the speed of panning. Ranges from 0 to 255 to correspond with AnalogWrite(). Corrected to 160 to 255 to accomodate for voltage threshold.
  int   SpeedTilt;                // Integer value to hold the speed of tilting. Ranges from 0 to 255 to correspond with AnalogWrite(). Corrected to 160 to 255 to accomodate for voltage threshold.
  int   SpeedZoom;                // Integer value to hold the speed of zooming. Ranges from 0 to 100.
  int   SpeedFocus;               // Integer value to hold the speed of focusing.
  int   PositionFocus;            // Integer value to hold the position of the focus.
  int   PTZMovement;              // Integer value to represent a movement detection flag. 0 for no movement, 1 for any movement.
} PTZPacket;

PTZPacket PTZData;                // Initialize data packet to hold required values for node control.

int valPan = 0;
int valTilt = 0;
int valZoom = 0;
int cameraActive = 1;
int cameraLight[6] = {0, 18, 19, 20, 21, 22};

void setup()
{
  Serial.begin(SERIAL_BAUD);

  /* Setup encoder. */
  pinMode(ENC_A, INPUT);
  pinMode(ENC_B, INPUT);
  digitalWrite(ENC_A, HIGH); //turn pullup resistor on
  digitalWrite(ENC_B, HIGH); //turn pullup resistor on
  attachInterrupt(0, updateEncoder, CHANGE);
  attachInterrupt(1, updateEncoder, CHANGE);

  /* Initialize radio. */
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
  radio.setHighPower();
  radio.encrypt(KEY);

  /* Initialize camera button lights. */
  for (int i = CAMERA_START; i <= CAMERA_END; i++) {
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);
  }
  for (int i = 1; i <= 5; i++) {
    pinMode(cameraLight[i], OUTPUT);
    digitalWrite(cameraLight[i], LOW);
  }
  analogWrite(cameraLight[cameraActive], 255);
}

void handle_joystick()
{
  PTZData.DirPan = 0; PTZData.DirTilt = 0; PTZData.DirZoom = 0; PTZData.DirFocus = 0; PTZData.SpeedPan = 0; PTZData.SpeedTilt = 0; PTZData.SpeedZoom = 0; PTZData.SpeedFocus = 0; PTZData.PositionFocus = 0; PTZData.PTZMovement = 0;
  valPan = analogRead(POT_PAN);
  valTilt = analogRead(POT_TILT);
  valZoom = analogRead(POT_ZOOM);
  if (valPan < LOWER_LIMIT) {
    PTZData.DirPan = 1;
    PTZData.SpeedPan = map(valPan, 0, LOWER_LIMIT, 255, 160);
    PTZData.PTZMovement = 1;
  }
  if (valPan > UPPER_LIMIT) {
    PTZData.DirPan = 2;
    PTZData.SpeedPan = map(valPan, UPPER_LIMIT, 1023, 160, 255);
    PTZData.PTZMovement = 1;
  }
  if (valTilt < LOWER_LIMIT) {
    PTZData.DirTilt = 1;
    PTZData.SpeedTilt = map(valTilt, 0, LOWER_LIMIT, 255, 160);
    PTZData.PTZMovement = 1;
  }
  if (valTilt > UPPER_LIMIT) {
    PTZData.DirTilt = 2;
    PTZData.SpeedTilt = map(valTilt, UPPER_LIMIT, 1023, 160, 255);
    PTZData.PTZMovement = 1;
  }
  if (valZoom < LOWER_LIMIT) {
    PTZData.DirZoom = 1;
    PTZData.SpeedZoom = map(valZoom, 0, LOWER_LIMIT, 100, 1);
    PTZData.PTZMovement = 1;
  }
  if (valZoom > UPPER_LIMIT) {
    PTZData.DirZoom = 2;
    PTZData.SpeedZoom = map(valZoom, UPPER_LIMIT, 1023, 1, 100);
    PTZData.PTZMovement = 1;
  }
  handle_focus();
}

void handle_focus()
{
  int KeepPosition = lastencoderValue;
  if (lastencoderValue != encoderValue)
  {
    PTZData.PTZMovement = 1;
    if (encoderValue > lastencoderValue) {
      PTZData.DirFocus = 2;
    }
    else {
      PTZData.DirFocus = 1;
    }
    PTZData.PositionFocus = encoderValue;
    lastencoderValue = encoderValue;
    runningStopped = 0;
  }
  else
  {
    if (runningStopped == 0) {
      PTZData.DirFocus = 0;
      PTZData.PositionFocus = encoderValue;
      runningStopped = 1;
    }
  }
  if (runningStopped == 0) {
    PTZData.SpeedFocus = map(abs(PTZData.PositionFocus - KeepPosition), 0, 5, 0, 100);
  }
}

void handle_camera_selection()
{
  int cameraSelect = 0;
  for (int i = CAMERA_START; i <= CAMERA_END; i++) {
    if (!digitalRead(i)) {
      cameraSelect = i - CAMERA_START + 1;
    }
  }
  if ((cameraSelect != cameraActive) && (cameraSelect != 0))
  {
    cameraActive = cameraSelect;
    for (int i = 1; i <= 5; i++) {
      digitalWrite(cameraLight[i], LOW);
    }
    analogWrite(cameraLight[cameraActive], 255);
  }
}

void updateEncoder() {
  int MSB = digitalRead(ENC_A);
  int LSB = digitalRead(ENC_B);
  int encoded = (MSB << 1) | LSB;
  int sum  = (lastEncoded << 2) | encoded;
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;
  lastEncoded = encoded;
}

void loop()
{
  handle_joystick();
  handle_camera_selection();
  if (PTZData.PTZMovement != 0)
  {
    radio.send(cameraActive, (const void*)(&PTZData), sizeof(PTZData));   // transmit data to other end
  }
}

