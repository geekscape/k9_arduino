/*
 * To Do
 * ~~~~~
 * - None, yet.
 */

#include <Servo.h>

static const byte STATE_EARS_STOP = 0;
static const byte STATE_EARS_SLOW = 1;
static const byte STATE_EARS_FAST = 2;

static const byte SERVO_EARS_LEFT  = 8;
static const byte SERVO_EARS_RIGHT = 9;

Servo servoEarLeft;
Servo servoEarRight;

int pos = 80;
 
void setup(void) {
  servoEarLeft.attach(SERVO_EARS_LEFT);
  servoEarRight.attach(SERVO_EARS_RIGHT);
}

void loop(void) {
  int state = STATE_EARS_STOP;

  if (analogRead(0) < 200) state = STATE_EARS_SLOW;
  if (analogRead(1) < 200) state = STATE_EARS_FAST;

  switch (state) {
    case STATE_EARS_SLOW:
      cycleEars(false);
      break;

    case STATE_EARS_FAST:
      cycleEars(true);
      break;

    default:
      break;
  }
}

void cycleEars(
  boolean mode) {

  int speed = mode  ?  3  :  1;

  moveEars( 80, 130,  speed);
  moveEars(130,  30, -speed);
  moveEars( 30,  80,  speed);
}

void moveEars(
  int  startPos,
  int  endPos,
  int  speed) {

  pos = startPos;

  while (true) {
    servoEarLeft.write(180 - pos);
    servoEarRight.write(pos);
    pos += speed;
    if (speed > 0  &&  pos >= endPos) return;
    if (speed < 0  &&  pos <= endPos) return;
    delay(10);
  }
}
