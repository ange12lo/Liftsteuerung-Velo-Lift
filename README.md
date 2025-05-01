# ESP32-C6 Liftsteuerung mit HC-SR04 & BTS7960


## Beschreibung
Steuerung eines Liftmoduls mit einem ESP32-C6, Ultraschallsensor (JSN-SR04T / HC-SR04) und BTS7960 H‑Brücke. Über zwei Fußpedale fährt der Lift automatisch bis zu definierten Endanschlägen (60 cm / 140 cm) und nutzt Soft-Start/Stop sowie langsame Zonen an den Enden.

## Features
- Abstandsmessung per Ultraschall (intervallgetaktet, 0‑Filter, Median-/Min-of-N-Filter)
- Soft-Start & Soft-Stop (sanfte Beschleunigung/Abbremsung)
- Halbierte Geschwindigkeit in Slow-Zonen (letzte 10 cm)
- Entprellte Fußpedale für Hoch-/Runter-Fahrbefehl
- Automatisches Stoppen an Ober-/Unteranschlag

## Verdrahtung

| Komponente               | Pin am ESP32-C6 | Pin am Modul         |
|--------------------------|-----------------|----------------------|
| 5 V / 3.3 V           | 5 V / 3.3 V    | VCC                  |
| GND                      | GND             | GND                  |
| Ultraschall TRIG         | GPIO 2          | TRIG                 |
| Ultraschall ECHO         | GPIO 3          | ECHO                 |
| Pedal Hoch (Input_PULLUP) | GPIO 4          | Pedal_Hoch (NC→GND)  |
| Pedal Runter (Input_PULLUP)| GPIO 5         | Pedal_Runter (NC→GND)|
| BTS7960 RPWM (IN1)       | GPIO 18         | RPWM                 |
| BTS7960 LPWM (IN2)       | GPIO 19         | LPWM                 |
| BTS7960 R_EN + L_EN      | 5 V (oder GPIO) | R_EN, L_EN (parallel)|
| (optional) Current Sense | —               | R_IS / L_IS (NC)     |

## Setup & Kompilierung
1. Arduino IDE oder PlatformIO öffnen.
2. ESP32-C6 Board-Package installieren.
3. Projekt öffnen, `README.md` und `lift.ino` im selben Ordner.
4. Baudrate: 115200 
5. COM‑Port wählen und hochladen.

## Konfiguration
- `UPPER_LIMIT`, `LOWER_LIMIT`, `SLOW_ZONE` in cm anpassen
- `FULL_SPEED`, `SLOW_SPEED`, `ACCEL_STEP` für Fahrprofil
- `MEASUREMENT_INTERVAL`, `TRIG_PULSE_WIDTH` bei Bedarf für Sensor-Performance tweakbar


