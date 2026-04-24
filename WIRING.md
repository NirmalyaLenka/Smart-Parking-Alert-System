# Hardware Guide — Smart Parking Alert System

## Component List

| Component | Qty | Notes |
|-----------|-----|-------|
| ESP32 DevKit v1 (38-pin) | 1 | Any 38- or 30-pin variant works |
| HC-SR04 Ultrasonic Sensor | 1 | Or JSN-SR04T for weatherproof outdoor use |
| Active Buzzer 5 V | 1 | "Active" = built-in oscillator, needs only ON/OFF signal |
| NPN Transistor (2N2222 or BC547) | 1 | Drives buzzer from 3.3 V GPIO |
| 1 kΩ Resistor | 1 | Base resistor for transistor |
| 1N4007 Diode | 1 | Flyback protection across buzzer |
| LED (blue or red) | 1 | Optional status indicator |
| 470 Ω Resistor | 1 | LED current limit |
| 18650 LiPo cell + TP4056 charger | 1 | Battery power option |
| 5 V voltage regulator (AMS1117-5V) | 1 | If powering from car 12 V |
| Waterproof project box | 1 | IP65 recommended for outdoor use |

---

## Wiring Diagram (Text)

```
ESP32 Pin   →  Component
──────────────────────────────────────────────────
GPIO 5      →  HC-SR04 TRIG
GPIO 18     →  HC-SR04 ECHO
3.3V        →  HC-SR04 VCC  (HC-SR04 tolerates 3.3 V; 5 V is also fine via VIN)
GND         →  HC-SR04 GND

GPIO 19     →  1kΩ Resistor → NPN Base (2N2222)
              NPN Collector → Buzzer (-) terminal
              Buzzer (+)    → 5V
              NPN Emitter   → GND
              1N4007        → across Buzzer (cathode to +)

GPIO 2      →  470Ω → LED (+)
              LED (-) → GND

VIN (5V)    →  HC-SR04 VCC (if using 5V supply)
3.3V        →  HC-SR04 VCC (if running at 3.3V logic)
```

---

## Schematic ASCII

```
         ┌──────────────────────────────────┐
         │          ESP32 DevKit            │
         │                                  │
  TRIG ──┤ GPIO5                  3.3V ─────┼─── VCC (HC-SR04)
  ECHO ──┤ GPIO18                           │
         │                        GND ──────┼─── GND (HC-SR04)
  LED  ──┤ GPIO2 ──[470Ω]──LED──┐           │
         │                      GND         │
  BUZZ ──┤ GPIO19                           │
         │   │                              │
         │  [1kΩ]                           │
         │   │                              │
         │  NPN(B)                          │
         │   NPN(C)──[Buzzer+]──5V          │
         │   NPN(E)──GND                    │
         └──────────────────────────────────┘
```

---

## Why HC-SR04 over IR?

| Feature | HC-SR04 (Ultrasonic) | PIR / IR Sensor |
|---------|----------------------|-----------------|
| Max range | ~4 m | ~1 m (IR) / 7 m (PIR motion) |
| Sunlight immunity | ✅ Excellent | ⚠ IR affected by sunlight |
| Object colour independent | ✅ Yes | ⚠ IR reflectance varies |
| Point measurement | ✅ Yes (exact distance) | ❌ No (only presence) |
| Waterproof variant | ✅ JSN-SR04T | Limited |
| Gives distance reading | ✅ Yes (needed for threshold) | ❌ No |

---

## Why BLE over NFC?

| Feature | BLE Beacon (Phone) | NFC Tag |
|---------|-------------------|---------|
| Works from inside car | ✅ Yes, ~5–10 m range | ❌ Tap required (<4 cm) |
| Owner detected before parking | ✅ Pre-entry detection | ❌ Must exit car |
| No extra hardware tag | ✅ Phone app only | ❌ Separate NFC tag needed |
| Battery on ESP32 | ✅ BLE scan uses ~20 mA for 2 s | ✅ NFC reader is lower power |
| Rain / weather proof use | ✅ Works through metal/glass | ⚠ NFC affected by metal |

**Winner: BLE** — detects the owner's phone before the car is fully parked, preventing any false alarms.

---

## Power Options

### Option 1 — USB Power Bank
- Connect 5 V USB to ESP32 VIN
- Deep sleep at 5 mA × 5 min cycle → ~4 mA average
- 10,000 mAh bank ≈ **100 days** of operation

### Option 2 — 18650 LiPo + TP4056
- 3.7 V → 5 V boost converter → ESP32 VIN
- Add solar panel (5 V 1 W) to TP4056 for indefinite runtime

### Option 3 — Car 12 V (if near parking spot)
- 12 V → AMS1117-5V → ESP32 VIN

---

## Mounting Suggestions

- Mount the HC-SR04 at bumper height (40–60 cm from ground)
- Face sensor horizontally toward the parking bay entrance
- Place ESP32 + buzzer unit in a waterproof IP65 box
- Drill two holes for sensor "eyes" and seal with silicone
- Use outdoor double-sided tape or screws for wall mounting
