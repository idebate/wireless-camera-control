
#include <RFM69.h>
#include <Wire.h>
#include <AccelStepper.h>

// Communication delays and serial configuration.
#define DELAY_STABILIZE 10
#define DELAY_COMMSTOP  1
#define COMM_COUNT      25
#define SERIAL_BAUD     9600
#define MOTOR_STEPS     50
#define MOTOR_SLP_TMR   2000

// Stepper motor configuration.
#define STP_ZOOM_MAXSPD 2000 
#define STP_ZOOM_MINSPD 0
#define STP_ZOOM_STEP   15
#define STP_ZOOM_DIR    14
#define STP_ZOOM_SLP    18
long timerZoom = 0;
AccelStepper stepperZoom(1, STP_ZOOM_STEP, STP_ZOOM_DIR);

// Stepper motor configuration.
#define STP_FOCUS_MINSPD 0 
#define STP_FOCUS_MAXSPD 2000 
#define STP_FOCUS_STEP   17
#define STP_FOCUS_DIR    16
#define STP_FOCUS_SLP    19
long timerFocus = 0;
AccelStepper stepperFocus(1, STP_FOCUS_STEP, STP_FOCUS_DIR);

// Motor driver settings.
#define MP_SPEED        3
#define MP_DIR1         8
#define MP_DIR2         9
#define MT_SPEED        5
#define MT_DIR1         7
#define MT_DIR2         6
#define MR_STBY        10

// Communication network settings.
#define DEFNODEID       1                         // ID of the receiver node. Unique to every receiver. Default value.
#define NETWORKID       100                       // ID of the network. Same across all nodes.
#define FREQUENCY       RF69_915MHZ
#define KEY             "wirelessCameraMV"        // Exactly 16 characters, needs to be exactly the same across all nodes.

RFM69 radio;                                      // Initialize radio.

// PTZPacket data structure.
typedef struct
{
  int   DirPan;
  int   DirTilt;
  int   DirZoom;
  int   DirFocus;
  int   SpeedPan;
  int   SpeedTilt;
  int   SpeedZoom;
  int   SpeedFocus;
  int   PositionFocus;
  int   PTZMovement;
} PTZPacket;

PTZPacket PTZData;

int commCount = 1;

int currentPos = 0;

int NodeID = 0;

// Node ID settings.
#define NODE_ID_1       4
#define NODE_ID_2       A6
#define NODE_ID_3       A7

void setup()
{
  // Set Node ID.
  getNodeID();    
  delay(DELAY_STABILIZE);

  // Set output pins.
  for (int i = 3; i <= 10; i++) {
    if (i != 4) {
      pinMode(i, OUTPUT);
    }
  }
  for (int i = 14; i <= 15 ; i++) {
    pinMode(i, OUTPUT);
  }

  pinMode(STP_ZOOM_SLP, OUTPUT);
  digitalWrite(STP_ZOOM_SLP, LOW);
  // Set zoom stepper.
  stepperZoom.setMaxSpeed(STP_ZOOM_MAXSPD);
  stepperZoom.setAcceleration(STP_ZOOM_MAXSPD / 2);
  stepperZoom.setSpeed(0);

  pinMode(STP_FOCUS_SLP, OUTPUT);
  digitalWrite(STP_FOCUS_SLP, LOW);
  // Set zoom stepper.
  stepperFocus.setMaxSpeed(STP_FOCUS_MAXSPD);
  stepperZoom.setAcceleration(STP_FOCUS_MAXSPD / 2);
  stepperFocus.setSpeed(0);

  // Initialize radio receiver.
  radio.initialize(FREQUENCY, NodeID, NETWORKID);
  radio.setHighPower();
  radio.encrypt(KEY);
  radio.promiscuous(false);

  delay(DELAY_STABILIZE);
}

void loop()
{
  for (int i = 1; i <= MOTOR_STEPS; i++) {
    stepperZoom.runSpeed();
    stepperFocus.runSpeed();
  }
  if (radio.receiveDone())
  {
    if (radio.DATALEN == sizeof(PTZPacket))
    {
      PTZData = *(PTZPacket*)radio.DATA;
      if (PTZData.DirPan != 0) {
        PTH_Move(1, PTZData.SpeedPan, PTZData.DirPan);
      }
      if (PTZData.DirTilt != 0) {
        PTH_Move(2, PTZData.SpeedTilt, PTZData.DirTilt);
      }

      // Only move the zoom motor if pan/tilt is not active. Working precaution, but can be eliminated.
      if (PTZData.DirPan == 0 && PTZData.DirTilt == 0) {
        if (PTZData.DirZoom != 0) { moveZoom(PTZData.DirZoom, PTZData.SpeedZoom); }
        if (PTZData.DirFocus != 0) { moveFocus(PTZData.DirFocus, PTZData.SpeedFocus, PTZData.PositionFocus); }
      }
      commCount = 0;
    }
  }
  else
  {
    if (CommDelayComplete())
    {
      PTH_Stop();
      stopZoom();
      stopFocus();
    }
  }
  delay(DELAY_COMMSTOP);
  checkZoomSleep();
  checkFocusSleep();
}

boolean CommDelayComplete()
{
  commCount += 1;
  if (commCount > (COMM_COUNT + 1)) {
    commCount = (COMM_COUNT + 1);
  }
  return (commCount == COMM_COUNT);
}

void PTH_Move(int motorNumber, int motorSpeed, int motorDirection)
{
  // motorNumber: 1 for Pan, and 2 for Tilt.
  // motorSpeed: Between a set minimum (typically 160 for lower voltage threshold of DC motors) and 255.
  // motorDirection: 1 for Left/Down, 2 for Right/Up.

  // Disable standby, stop Zoom and Focus steppers.
  digitalWrite(MR_STBY, HIGH);
  stopZoom();
  stopFocus();

  // Set direction pins.
  boolean inPin1 = LOW;
  boolean inPin2 = HIGH;
  if (motorDirection == 1) {
    inPin1 = HIGH;
    inPin2 = LOW;
  }

  if (motorNumber == 1)
  {
    if (PTZData.DirTilt == 0) {
      analogWrite(MT_SPEED, 0);
    }
    digitalWrite(MP_DIR1, inPin1); digitalWrite(MP_DIR2, inPin2); analogWrite(MP_SPEED, motorSpeed);
  }
  else
  {
    if (PTZData.DirPan == 0) {
      analogWrite(MP_SPEED, 0);
    }
    digitalWrite(MT_DIR1, inPin1); digitalWrite(MT_DIR2, inPin2); analogWrite(MT_SPEED, motorSpeed);
  }
}

void PTH_Stop()
{
  digitalWrite(MR_STBY, LOW);
  analogWrite(MP_SPEED, 0);
  analogWrite(MT_SPEED, 0);
}

void moveZoom(int dirZoom, int speedZoom)
{
  if (!digitalRead(STP_ZOOM_SLP)) {
    digitalWrite(STP_ZOOM_SLP, HIGH);
  }
  if (dirZoom == 2) {
    speedZoom = speedZoom * -1;
  }
  stepperZoom.setSpeed(map(speedZoom, 0, 100, STP_ZOOM_MINSPD, STP_ZOOM_MAXSPD));
  timerZoom = millis();
}

void stopZoom() {
  stepperZoom.setSpeed(0);
}

void checkZoomSleep() {
  unsigned long timerZoomX = millis();
  if (timerZoomX - timerZoom > MOTOR_SLP_TMR) {
    digitalWrite(STP_ZOOM_SLP, LOW);
  }
}

void moveFocus(int dirFocus, int speedFocus, int positionFocus)
{
  if (!digitalRead(STP_FOCUS_SLP)) {
    digitalWrite(STP_FOCUS_SLP, HIGH);
  }
  if (dirFocus == 2) {
    speedFocus = speedFocus * -1;
  }
  stepperFocus.setSpeed(map(speedFocus, 0, 100, STP_FOCUS_MINSPD, STP_FOCUS_MAXSPD));
  timerFocus = millis();
}

void stopFocus() {
  stepperFocus.setSpeed(0);
}

void checkFocusSleep() {
  unsigned long timerFocusX = millis();
  if (timerFocusX - timerFocus > MOTOR_SLP_TMR) {
    digitalWrite(STP_FOCUS_SLP, LOW);
  }
}

void getNodeID() {
  pinMode(NODE_ID_1, INPUT);
  pinMode(NODE_ID_2, INPUT);
  pinMode(NODE_ID_3, INPUT);
  if (digitalRead(NODE_ID_1) == 1) { NodeID += 1; } // Digital pin.
  if (analogRead(NODE_ID_2) > 900) { NodeID += 2; } // Analog pin.
  if (analogRead(NODE_ID_3) > 900) { NodeID += 4; } // Analog pin.
}

