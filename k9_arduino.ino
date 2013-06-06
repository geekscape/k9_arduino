/*
 * k9_arduino.ino
 * ~~~~~~~~~~~~~~
 * Version: 2013-03-25
 *
 * To Do
 * ~~~~~
 * - Define analog input pin literals.
 * - Refactor event handling code to be data driven.
 * - Fix problem if tail still finishing (back to centre) and different direction selected.
 * - Mobile phone GUI (bypass 12-channel relay board).
 *   - Adjust limits via GUI and plass in EEPROM.
 * - Random independent ear movements.
 * - Circular tail movements.
 * - Incorporate blaster and probe.
 * - Replace existing "button panel" functionality.
 */

#define LIMIT_EARS_INWARD            115
#define LIMIT_EARS_CENTRE             80
#define LIMIT_EARS_OUTWARD            30

#define LIMIT_TAIL_HORIZONTAL_LEFT    87
#define LIMIT_TAIL_HORIZONTAL_CENTRE 101  //  97
#define LIMIT_TAIL_HORIZONTAL_RIGHT  112  // 105

#define LIMIT_TAIL_VERTICAL_DOWN      70
#define LIMIT_TAIL_VERTICAL_CENTRE    75
#define LIMIT_TAIL_VERTICAL_UP        85

#include <Servo.h>

Servo servoEarLeft;
Servo servoEarRight;
Servo servoTailHorizontal;
Servo servoTailVertical;

static const byte PIN_INPUT_EARS_SLOW       = 0;  // Analog input: Blue   cable from relay 3
static const byte PIN_INPUT_EARS_FAST       = 1;  // Analog input: Black  cable from relay 4
static const byte PIN_INPUT_TAIL_HORIZONTAL = 2;  // Analog input: Yellow cable from relay 2
static const byte PIN_INPUT_EARS_VERTICAL   = 3;  // Analog input: Blue   cable from relay 1

static const byte PIN_SERVO_EARS_LEFT       = 8;  // Digital output: White  cable to head
static const byte PIN_SERVO_EARS_RIGHT      = 9;  // Digital output: Yellow cable to head
static const byte PIN_SERVO_TAIL_HORIZONTAL = 4;  // Digital output: Servo  cable to tail
static const byte PIN_SERVO_TAIL_VERTICAL   = 7;  // Digital output: Servo  cable to tail

static const byte ACTION_EARS_OFF  = 0;
static const byte ACTION_EARS_SLOW = 1;
static const byte ACTION_EARS_FAST = 2;

static const byte ACTION_TAIL_OFF        = 0;
static const byte ACTION_TAIL_HORIZONTAL = 1;
static const byte ACTION_TAIL_VERTICAL   = 2;

static const byte STATE_EARS_STOP   = 0;
static const byte STATE_EARS_ROTATE = 1;
static const byte STATE_EARS_FINISH = 2;

int earsDirection = 1;
int earsPosition  = LIMIT_EARS_CENTRE;
int earsSpeed     = 0;
int earsState     = STATE_EARS_STOP;
 
static const byte STATE_TAIL_STOP              = 0;
static const byte STATE_TAIL_HORIZONTAL        = 1;
static const byte STATE_TAIL_VERTICAL          = 2;
static const byte STATE_TAIL_FINISH_HORIZONTAL = 3;
static const byte STATE_TAIL_FINISH_VERTICAL   = 4;

int tailDirection          = 1;
int tailHorizontalPosition = LIMIT_TAIL_HORIZONTAL_CENTRE;
int tailVerticalPosition   = LIMIT_TAIL_VERTICAL_CENTRE;
int tailSpeed              = 3;
int tailState              = STATE_TAIL_STOP;

void setup(void) {
  Serial.begin(38400);

  pinMode(A0, INPUT);  digitalWrite(A0, HIGH);
  pinMode(A1, INPUT);  digitalWrite(A1, HIGH);
  pinMode(A2, INPUT);  digitalWrite(A2, HIGH);
  pinMode(A3, INPUT);  digitalWrite(A3, HIGH);

  servoEarLeft.attach(PIN_SERVO_EARS_LEFT);
  servoEarRight.attach(PIN_SERVO_EARS_RIGHT);
  positionEars();

  servoTailHorizontal.attach(PIN_SERVO_TAIL_HORIZONTAL);
  servoTailVertical.attach(PIN_SERVO_TAIL_VERTICAL);
  positionTail();
}

#define HANDLER_PERIOD_ACTION 100  // 10 ?
#define HANDLER_PERIOD_EARS    10
#define HANDLER_PERIOD_TAIL    36

long timeNextHandlerAction = 0;
long timeNextHandlerEars   = 0;
long timeNextHandlerTail   = 0;

void loop(void) {
  long timeNow = millis();

  if (timeNextHandlerAction < timeNow) {
    timeNextHandlerAction = timeNow + HANDLER_PERIOD_ACTION;
    actionHandler();
  }

  if (timeNextHandlerEars < timeNow) {
    timeNextHandlerEars = timeNow + HANDLER_PERIOD_EARS;
    earsHandler();
  }

  if (timeNextHandlerTail < timeNow) {
    timeNextHandlerTail = timeNow + HANDLER_PERIOD_TAIL;
    tailHandler();
  }
}

#define MINIMUM   87 // 85
#define MAXIMUM  105 // 85
#define SPEED    25

void loop1(void) {
  earsPosition = MINIMUM;
  positionEars();
  delay(1000);

for (int i = 0;  i < 20;  i ++) {
  for (earsPosition = MINIMUM; earsPosition <= MAXIMUM; earsPosition ++) {
    positionEars();
    delay(SPEED);
  }

  for (earsPosition = MAXIMUM; earsPosition >= MINIMUM; earsPosition --) {
    positionEars();
    delay(SPEED);
  }
}
}

void loop2(void) {
  tailHorizontalPosition = MINIMUM;
  tailVerticalPosition = 75;  // tailHorizontalPosition;
  positionTail();
  delay(1000);

for (int i = 0;  i < 20;  i ++) {
  for (tailHorizontalPosition = MINIMUM; tailHorizontalPosition <= MAXIMUM; tailHorizontalPosition ++) {
  tailVerticalPosition = 75;  // tailHorizontalPosition;
    positionTail();
    delay(SPEED);
  }

  for (tailHorizontalPosition = MAXIMUM; tailHorizontalPosition >= MINIMUM; tailHorizontalPosition --) {
  tailVerticalPosition = 75;  // tailHorizontalPosition;
    positionTail();
    delay(SPEED);
  }
}
}

void actionHandler(void) {
  int earsAction = ACTION_EARS_OFF;
  int tailAction = ACTION_TAIL_OFF;

  if (analogRead(0) < 200) earsAction = ACTION_EARS_SLOW;
  if (analogRead(1) < 200) earsAction = ACTION_EARS_FAST;
  if (analogRead(2) < 200) tailAction = ACTION_TAIL_HORIZONTAL;
  if (analogRead(3) < 200) tailAction = ACTION_TAIL_VERTICAL;
/*
  while (Serial.available()) {
    char ch = Serial.read();
    switch (ch) {
      case 'e': earsAction = ACTION_EARS_OFF;         break;
      case 's': earsAction = ACTION_EARS_SLOW;        break;
      case 'f': earsAction = ACTION_EARS_FAST;        break;
      case 't': tailAction = ACTION_TAIL_OFF;         break;
      case 'h': tailAction = ACTION_TAIL_HORIZONTAL;  break;
      case 'v': tailAction = ACTION_TAIL_VERTICAL;    break;
    }
  }
 */
  switch (earsAction) {
    case ACTION_EARS_OFF:
      if (earsState != STATE_EARS_STOP) earsState = STATE_EARS_FINISH;
      break;

    case ACTION_EARS_SLOW:
    case ACTION_EARS_FAST:
      earsState = STATE_EARS_ROTATE;
      earsSpeed = (earsAction == ACTION_EARS_FAST)  ?  3  :  1;
      break;
  }

  switch (tailAction) {
    case ACTION_TAIL_OFF:
      if (tailState == STATE_TAIL_HORIZONTAL) tailState = STATE_TAIL_FINISH_HORIZONTAL;
      if (tailState == STATE_TAIL_VERTICAL)   tailState = STATE_TAIL_FINISH_VERTICAL;
      break;

    case ACTION_TAIL_HORIZONTAL:
      tailState = STATE_TAIL_HORIZONTAL;
      break;

    case ACTION_TAIL_VERTICAL:
      tailState = STATE_TAIL_VERTICAL;
      break;
  }
/*
  Serial.println("--------------");
  Serial.print("Ears state: ");
  Serial.println(earsState);
  Serial.print("Ears speed: ");
  Serial.println(earsSpeed);
  Serial.print("Tail state: ");
  Serial.println(tailState);
  delay(250);
 */
}

void earsHandler(void) {
  if (earsState != STATE_EARS_STOP) {
    earsPosition += earsDirection * earsSpeed;

    if (earsPosition <= LIMIT_EARS_OUTWARD) {
      earsDirection = 1;
      earsPosition  = LIMIT_EARS_OUTWARD;
    }

    if (earsPosition >= LIMIT_EARS_INWARD) {
      earsDirection = -1;
      earsPosition  = LIMIT_EARS_INWARD;
    }

    if (earsState == STATE_EARS_FINISH) {
      if (earsPosition >= (LIMIT_EARS_CENTRE - 2)  &&
          earsPosition <= (LIMIT_EARS_CENTRE + 2)) {

        earsPosition = LIMIT_EARS_CENTRE;
        earsState = STATE_EARS_STOP;
      }
    }

    positionEars();
  }
}

void positionEars() {
/*
  Serial.print("Ears: ");
  Serial.print(earsPosition);
  Serial.print(", dir: ");
  Serial.print(earsDirection);
  Serial.print(", spe: ");
  Serial.print(earsSpeed);
  Serial.print(", sta: ");
  Serial.println(earsState);
 */
  servoEarLeft.write(180 - earsPosition);
  servoEarRight.write(earsPosition);
}

void tailHandler(void) {
  if (tailState == STATE_TAIL_HORIZONTAL  ||
      tailState == STATE_TAIL_FINISH_HORIZONTAL) {

    tailHorizontalHandler();
  }

  if (tailState == STATE_TAIL_VERTICAL  ||
      tailState == STATE_TAIL_FINISH_VERTICAL) {

    tailVerticalHandler();
  }

  positionTail();
}

void tailHorizontalHandler() {
  tailHorizontalPosition += tailDirection * tailSpeed;

  if (tailHorizontalPosition <= LIMIT_TAIL_HORIZONTAL_LEFT) {
    tailDirection          = 1;
    tailHorizontalPosition = LIMIT_TAIL_HORIZONTAL_LEFT;
  }

  if (tailHorizontalPosition >= LIMIT_TAIL_HORIZONTAL_RIGHT) {
    tailDirection          = -1;
    tailHorizontalPosition = LIMIT_TAIL_HORIZONTAL_RIGHT;
  }

  if (tailState == STATE_TAIL_FINISH_HORIZONTAL) {
    if (tailHorizontalPosition >= (LIMIT_TAIL_HORIZONTAL_CENTRE - 2)  &&
        tailHorizontalPosition <= (LIMIT_TAIL_HORIZONTAL_CENTRE + 2)) {

      tailHorizontalPosition = LIMIT_TAIL_HORIZONTAL_CENTRE;
      tailState = STATE_TAIL_STOP;
    }
  }
}

void tailVerticalHandler() {
  tailVerticalPosition += tailDirection * tailSpeed;

  if (tailVerticalPosition <= LIMIT_TAIL_VERTICAL_DOWN) {
    tailDirection        = 1;
    tailVerticalPosition = LIMIT_TAIL_VERTICAL_DOWN;
  }

  if (tailVerticalPosition >= LIMIT_TAIL_VERTICAL_UP) {
    tailDirection        = -1;
    tailVerticalPosition = LIMIT_TAIL_VERTICAL_UP;
  }

  if (tailState == STATE_TAIL_FINISH_VERTICAL) {
    if (tailVerticalPosition >= (LIMIT_TAIL_VERTICAL_CENTRE - 2)  &&
        tailVerticalPosition <= (LIMIT_TAIL_VERTICAL_CENTRE + 2)) {

      tailVerticalPosition = LIMIT_TAIL_VERTICAL_CENTRE;
      tailState = STATE_TAIL_STOP;
    }
  }
}

void positionTail(void) {
/*
  Serial.print("Tail: ");
  Serial.print(tailHorizontalPosition);
  Serial.print(", ");
  Serial.print(tailVerticalPosition);
  Serial.print(", dir: ");
  Serial.print(tailDirection);
  Serial.print(", spe: ");
  Serial.print(tailSpeed);
  Serial.print(", sta: ");
  Serial.println(tailState);
 */
  servoTailHorizontal.write(tailHorizontalPosition);
  servoTailVertical.write(tailVerticalPosition);
}
