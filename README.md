**# Pro Mini Liftsteuerung mit HC-SR04 & BTS7960**

## Beschreibung

Steuerung eines Liftmoduls mit einem Arduino Pro Mini (ATmega328P @ 16 MHz), Ultraschallsensor (HC-SR04T) und BTS7960 H‑Brücke. Über zwei entprellte Fußpedale fährt der Lift automatisch bis zu definierten Endanschlägen und nutzt sanfte Soft-Start/Stop sowie langsame Zonen an den Enden.

## Features

* Intervallgetaktete Abstandsmessung per Ultraschall (60 ms, 0‑Filter)
* Soft-Start & Soft-Stop (sanfte Beschleunigung/Abbremsung via Ramp)
* Halbierte Geschwindigkeit in Slow-Zonen (letzte 10 cm vor Endanschlägen)
* Entprellte Fußpedale für Hoch-/Runter-Fahrbefehl
* Automatisches Stoppen an oberem und unterem Anschlag
* Reduziertes Logging (nur jede 5. Iteration) für höhere Loop-Rate

## Verdrahtung

| Komponente                   | Pin am Pro Mini | Beschreibung        |
| ---------------------------- | --------------- | ------------------- |
| 5 V / VCC                    | 5 V             | Versorgungsspannung |
| GND                          | GND             | Masse               |
| Ultraschall TRIG             | D8              | Trigger-Eingang     |
| Ultraschall ECHO             | D9              | Echo-Ausgang        |
| Pedal Hoch (INPUT\_PULLUP)   | D7              | NC → GND drückt     |
| Pedal Runter (INPUT\_PULLUP) | D4              | NC → GND drückt     |
| BTS7960 IN1 (RPWM)           | D5 (PWM)        | Motor Richtung UP   |
| BTS7960 IN2 (LPWM)           | D6 (PWM)        | Motor Richtung DOWN |
| BTS7960 ENABLE               | D10             | Treiber ein = HIGH  |

## Setup & Kompilierung

1. Arduino IDE öffnen.
2. Pro Mini 16 MHz (ATmega328P) auswählen.
3. Sketch (`lift.ino`) und `README.md` im selben Ordner ablegen.
4. Baudrate im Sketch: `Serial.begin(115200);` (oder bis 1 000 000 Bd).
5. COM‑Port wählen und hochladen.

## Konfiguration

* **END-Positionen:** `UPPER_LIMIT` (cm), `LOWER_LIMIT` (cm), `SLOW_ZONE` (cm)
* **Geschwindigkeiten:** `FULL_SPEED` (0–255), `SLOW_SPEED` (z. B. 128)
* **Ramp-Parameter:** `ACCEL_STEP` (z. B. 10 für schnellere Ramp)
* **Sensor:** `MEASUREMENT_INTERVAL` (ms), `TRIG_PULSE_WIDTH` (µs)
* **Debounce:** `DEBOUNCE_DELAY` (ms)
* **Logging:** Anpassen der Iterations-Häufigkeit im Code
* **Loop-Timing:** `delay(2)` im Loop für feine Taktung

*Dieses README kann direkt in dein Projekt übernommen oder nach Bedarf weiter angepasst werden.*
