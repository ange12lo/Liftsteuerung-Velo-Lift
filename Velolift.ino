// --------------------------------------------------
//  ESP32-C6 Liftsteuerung mit HC-SR04T + BTS7960
// --------------------------------------------------

#include <Arduino.h>

// --- Pin-Definitionen ---
// Ultraschall-Sensor
const int TRIG_PIN        = 2;
const int ECHO_PIN        = 3;

// Fußpedale (INPUT_PULLUP, LOW = gedrückt)
const int UP_BUTTON_PIN   = 4;
const int DOWN_BUTTON_PIN = 5;

// BTS7960 H-Brücke
const int MOTOR_IN1_PIN   = 18;
const int MOTOR_IN2_PIN   = 19;
const int MOTOR_EN_PIN    = 21;

// Fahrgeschwindigkeiten
const int FULL_SPEED      = 255;          // 0–255
const int SLOW_SPEED      = FULL_SPEED/2; // halbe Geschwindigkeit

// Soft-Ramp
const int ACCEL_STEP      = 5;            // Schrittweite pro Loop

// Endanschläge (cm Abstand Sensor→Decke)
const float UPPER_LIMIT   =  75.0;
const float LOWER_LIMIT   = 140.0;
const float SLOW_ZONE     =  10.0;        // Zone der halben Geschwindigkeit

// Debounce
const unsigned long DEBOUNCE_DELAY = 50;

// Debounce-Variablen
int      upState       = HIGH, lastUpReading       = HIGH;
int      downState     = HIGH, lastDownReading     = HIGH;
unsigned long lastUpDebounce   = 0;
unsigned long lastDownDebounce = 0;

// aktuelle PWM-Speed (geramp-t)
int currentSpeed = 0;

// === Neue Hilfsvariablen für Ultraschall-Ping ===
const unsigned long MEASUREMENT_INTERVAL = 60; // ms zwischen Pings
const unsigned int  TRIG_PULSE_WIDTH     = 20; // µs Trigger-Länge
static unsigned long lastPingTime       = 0;  // Timestamp der letzten Messung
static float         lastValidDistance  = LOWER_LIMIT; // Startwert plausibel

// Motor ansteuern über analogWrite
void applyMotor(int speed, bool up) {
  if (up) {
    analogWrite(MOTOR_IN1_PIN, speed);
    analogWrite(MOTOR_IN2_PIN, 0);
  } else {
    analogWrite(MOTOR_IN1_PIN, 0);
    analogWrite(MOTOR_IN2_PIN, speed);
  }
}

// Abstand in cm messen mit Intervall & 0-Filter
float readDistanceCM() {
  unsigned long now = millis();
  if (now - lastPingTime < MEASUREMENT_INTERVAL) {
    // zu früh: letzten Wert zurückgeben
    return lastValidDistance;
  }
  lastPingTime = now;

  // Trigger-Impuls
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(TRIG_PULSE_WIDTH);
  digitalWrite(TRIG_PIN, LOW);

  // Echo messen (Timeout 30000 µs ≙ 30 ms)
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);

  // Timeout / no-echo → alten Wert behalten
  if (duration == 0) {
    return lastValidDistance;
  }

  // Umrechnen in cm und merken
  float dist = (duration * 0.0343f) / 2.0f;
  lastValidDistance = dist;
  return dist;
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("=== Liftsteuerung gestartet ===");

  // Sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Pedale
  pinMode(UP_BUTTON_PIN,   INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);

  // Motor-Pins
  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);
  pinMode(MOTOR_EN_PIN,  OUTPUT);
  digitalWrite(MOTOR_EN_PIN, HIGH);  // H-Brücke einschalten

  // sicherheitshalber Motor aus
  applyMotor(0, true);
}

void loop() {
  unsigned long now = millis();

  // --- Entprellen UP ---
  int rUp = digitalRead(UP_BUTTON_PIN);
  if (rUp != lastUpReading) lastUpDebounce = now;
  if (now - lastUpDebounce > DEBOUNCE_DELAY) upState = rUp;
  lastUpReading = rUp;

  // --- Entprellen DOWN ---
  int rDown = digitalRead(DOWN_BUTTON_PIN);
  if (rDown != lastDownReading) lastDownDebounce = now;
  if (now - lastDownDebounce > DEBOUNCE_DELAY) downState = rDown;
  lastDownReading = rDown;

  bool upPressed   = (upState   == LOW);
  bool downPressed = (downState == LOW);

  // Abstand messen
  float d = readDistanceCM();
  Serial.printf("Abstand: %.1f cm | UP=%d DOWN=%d | Speed=%d\n",
                d, upPressed, downPressed, currentSpeed);

  // Ziel-Speed & Richtung ermitteln
  int   targetSpeed = 0;
  bool  directionUp = false;

  if (upPressed && !downPressed) {
    // hochfahren
    if (d > UPPER_LIMIT + SLOW_ZONE)      targetSpeed = FULL_SPEED;
    else if (d > UPPER_LIMIT)             targetSpeed = SLOW_SPEED;
    else                                  targetSpeed = 0;  // Anschlag
    directionUp = true;
  }
  else if (downPressed && !upPressed) {
    // runterfahren
    if (d < LOWER_LIMIT - SLOW_ZONE)      targetSpeed = FULL_SPEED;
    else if (d < LOWER_LIMIT)             targetSpeed = SLOW_SPEED;
    else                                  targetSpeed = 0;  // Anschlag
    directionUp = false;
  }
  else {
    targetSpeed = 0;  // weder hoch noch runter
  }

  // Soft-Ramp
  if (currentSpeed < targetSpeed) {
    currentSpeed = min(currentSpeed + ACCEL_STEP, targetSpeed);
  }
  else if (currentSpeed > targetSpeed) {
    currentSpeed = max(currentSpeed - ACCEL_STEP, targetSpeed);
  }

  // Motor ansteuern
  applyMotor(currentSpeed, directionUp);

  delay(10);
}
