/*

    Project:      wireless-camera-control
    Section:      wired control via switch based joystick
    Description:  Controls a standard Bescor MP-101 pan/tilt head, using a Moteino. Direction control using switch joystick.

    Referenced and using example code, logic and format from:

    https://lowpowerlab.com/forum/index.php/topic,84.0.html

    http://protechy.com/bescor-mp-101-hack/

    The code is licensed under GPL.

*/

#include "Wire.h"

#define BUTTON_LEFT   14
#define BUTTON_RIGHT  15
#define BUTTON_UP     16
#define BUTTON_DOWN   17
#define ACT_LEFT      3
#define ACT_RIGHT     4
#define ACT_UP        7
#define ACT_DOWN      8
#define DELAY         100

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
  Serial.begin(9600);
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
  if (button_combo == 0) { digitalWrite(ACT_LEFT, LOW); digitalWrite(ACT_RIGHT, LOW); digitalWrite(ACT_UP, LOW); digitalWrite(ACT_DOWN, LOW); }
  switch (button_combo) {
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
  if (button_combo != 0) {
      Serial.println();
  }
  delay(DELAY);
}

