// --------------------------------------------------
//  Pro Mini Liftsteuerung – schnellere Ramp und Loop
// --------------------------------------------------

#include <Arduino.h>

// --- Pin-Definitionen ---
const uint8_t TRIG_PIN        = 8;
const uint8_t ECHO_PIN        = 9;
const uint8_t UP_BUTTON_PIN   = 7;
const uint8_t DOWN_BUTTON_PIN = 4;
const uint8_t MOTOR_IN1_PIN   = 5;  // PWM
const uint8_t MOTOR_IN2_PIN   = 6;  // PWM
const uint8_t MOTOR_EN_PIN    = 10;

// Fahrparameter
const uint8_t FULL_SPEED      = 255;
const uint8_t SLOW_SPEED      = FULL_SPEED/2;

// **Beschleunigungsschritt verdoppelt** für schnellere Ramp
const uint8_t ACCEL_STEP      = 10;  

// Ultraschall
const unsigned long MEASUREMENT_INTERVAL = 60;  // ms
const unsigned int  TRIG_PULSE_WIDTH     = 20;  // µs

// Debounce
const unsigned long DEBOUNCE_DELAY = 50;

// States
int      upState        = HIGH, lastUpReading    = HIGH;
int      downState      = HIGH, lastDownReading  = HIGH;
unsigned long lastUpDebounce   = 0;
unsigned long lastDownDebounce = 0;
int currentSpeed = 0;

// Timing
static unsigned long lastPingTime      = 0;
static float         lastValidDistance = 140.0; // Anfangswert

void applyMotor(int speed, bool up) {
  if (up) {
    analogWrite(MOTOR_IN1_PIN, speed);
    analogWrite(MOTOR_IN2_PIN, 0);
  } else {
    analogWrite(MOTOR_IN1_PIN, 0);
    analogWrite(MOTOR_IN2_PIN, speed);
  }
}

float readDistanceCM() {
  unsigned long now = millis();
  if (now - lastPingTime < MEASUREMENT_INTERVAL) {
    return lastValidDistance;
  }
  lastPingTime = now;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(TRIG_PULSE_WIDTH);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) {
    return lastValidDistance;
  }
  float dist = (duration * 0.0343f) / 2.0f;
  lastValidDistance = dist;
  return dist;
}

void setup() {
  // **Baudrate hochsetzen auf 115200**
  Serial.begin(115200);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(UP_BUTTON_PIN,   INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);
  pinMode(MOTOR_EN_PIN,  OUTPUT);
  digitalWrite(MOTOR_EN_PIN, HIGH);
  applyMotor(0, true);
}

void loop() {
  unsigned long now = millis();

  // Entprellen UP
  int rUp = digitalRead(UP_BUTTON_PIN);
  if (rUp != lastUpReading) lastUpDebounce = now;
  if (now - lastUpDebounce > DEBOUNCE_DELAY) upState = rUp;
  lastUpReading = rUp;

  // Entprellen DOWN
  int rDown = digitalRead(DOWN_BUTTON_PIN);
  if (rDown != lastDownReading) lastDownDebounce = now;
  if (now - lastDownDebounce > DEBOUNCE_DELAY) downState = rDown;
  lastDownReading = rDown;

  bool upPressed   = (upState   == LOW);
  bool downPressed = (downState == LOW);

  // Abstand (wird nur alle MEASUREMENT_INTERVAL ms neu getriggert)
  float d = readDistanceCM();

  // **Logging nur jede 5. Iteration, um Overhead zu sparen**
  static uint8_t logCnt = 0;
  if (++logCnt >= 5) {
    logCnt = 0;
    Serial.print(F("Abst:"));
    Serial.print(d,1);
    Serial.print(F("cm UP="));
    Serial.print(upPressed);
    Serial.print(F(" DN="));
    Serial.print(downPressed);
    Serial.print(F(" Sp="));
    Serial.println(currentSpeed);
  }

  // Ziel-Speed & Richtung
  int   targetSpeed = 0;
  bool  directionUp = false;
  if (upPressed && !downPressed) {
    targetSpeed = (d > 75.0 + 10.0 ? FULL_SPEED :
                   (d > 75.0 ? SLOW_SPEED : 0));
    directionUp = true;
  }
  else if (downPressed && !upPressed) {
    targetSpeed = (d < 140.0 - 10.0 ? FULL_SPEED :
                   (d < 140.0 ? SLOW_SPEED : 0));
    directionUp = false;
  }

  // **Soft-Ramp mit größerem Schritt**
  if (currentSpeed < targetSpeed) {
    currentSpeed = min(currentSpeed + ACCEL_STEP, targetSpeed);
  }
  else if (currentSpeed > targetSpeed) {
    currentSpeed = max(currentSpeed - ACCEL_STEP, targetSpeed);
  }

  applyMotor(currentSpeed, directionUp);

  // **kleinerer Delay** – wir brauchen hier nur noch minimale Pausen
  delay(2);
}
