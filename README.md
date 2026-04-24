# 🅿️ Smart Parking Alert System

A battery-friendly ESP32-based system that guards your parking spot.  
It detects intruders using an **ultrasonic sensor**, identifies your car via a **BLE beacon** (your phone), sounds a buzzer alarm in a 10 s ON / 5 s OFF pattern, and **deep-sleeps for 5 minutes** when the bay is empty.

---

## ✨ Features

| Feature | Detail |
|---------|--------|
| 🚗 Vehicle detection | HC-SR04 ultrasonic sensor (up to 4 m range) |
| 📱 Owner recognition | BLE beacon from your phone — works from inside the car |
| 🔔 Alarm pattern | 10 s buzzer ON → 5 s pause → repeat until car leaves |
| 😴 Deep sleep | ESP32 sleeps 5 min when bay is empty (battery saving) |
| ⚡ Low power | ~4 mA average current in monitoring mode |
| 🌧️ Weatherproof option | Use JSN-SR04T sensor for outdoor mounting |

---

## 📁 Repository Structure

```
parking-alert-system/
├── firmware/
│   ├── parking_alert.ino      # Main ESP32 sketch (Arduino IDE)
│   ├── platformio.ini         # PlatformIO config
│   └── BLE_BEACON_SETUP.h     # Guide to configure your BLE beacon
├── hardware/
│   └── WIRING.md              # Full wiring diagram & component list
├── demo/
│   └── index.html             # Interactive browser demo of the system
├── docs/
│   └── SETUP.md               # Step-by-step setup guide
└── README.md
```

---

## 🔧 Hardware Required

- ESP32 DevKit v1 (any 30/38-pin)
- HC-SR04 Ultrasonic Sensor (or JSN-SR04T for weatherproofing)
- Active 5 V Buzzer
- NPN Transistor (2N2222/BC547) + 1 kΩ resistor
- 1N4007 diode (flyback protection)
- Optional: 18650 LiPo + TP4056 + 5V boost converter

See [`hardware/WIRING.md`](hardware/WIRING.md) for full diagram.

---

## 🚀 Quick Start

### 1. Flash the Firmware

**Arduino IDE:**
1. Install [Arduino IDE 2.x](https://www.arduino.cc/en/software)
2. Add ESP32 board support: `File → Preferences → Additional Boards URL`:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. `Tools → Board → ESP32 Dev Module`
4. Open `firmware/parking_alert.ino`
5. Upload

**PlatformIO (VS Code):**
```bash
cd firmware
pio run --target upload
```

### 2. Configure Your BLE Beacon

See [`firmware/BLE_BEACON_SETUP.h`](firmware/BLE_BEACON_SETUP.h) for full instructions.

**Quick version:**
1. Install **nRF Connect** on your phone
2. Go to Advertiser → Add new advertisement
3. Set device name to `Owner-Beacon`
4. Start advertising & keep running in background

### 3. Update Firmware Constants

In `parking_alert.ino`, update:
```cpp
#define OWNER_BLE_NAME  "Owner-Beacon"   // must match your beacon name
#define OWNER_BLE_MAC   "aa:bb:cc:dd:ee:ff"  // your phone's BT MAC (optional)
```

### 4. Wire & Mount

Follow `hardware/WIRING.md`, mount sensor at bumper height facing your bay.

---

## 🎮 Try the Demo

Open [`demo/index.html`](demo/index.html) in any browser — no hardware needed.  
Simulates the full system: sensor readings, BLE detection, alarm pattern, and sleep mode.

---

## ⚙️ Configuration Reference

| Constant | Default | Description |
|----------|---------|-------------|
| `CAR_THRESHOLD_CM` | 80 | Distance (cm) below which = vehicle present |
| `EMPTY_THRESHOLD_CM` | 200 | Distance above which = bay empty |
| `EMPTY_COUNT_FOR_SLEEP` | 10 | Consecutive empty readings before deep sleep |
| `DEEP_SLEEP_DURATION_US` | 300,000,000 | Sleep duration (5 minutes) |
| `OWNER_BLE_SCAN_WINDOW_MS` | 2000 | BLE scan window per cycle (ms) |
| `BEEP_ON_MS` | 10000 | Buzzer on time per alarm cycle (ms) |
| `BEEP_OFF_MS` | 5000 | Buzzer off pause per alarm cycle (ms) |

---

## 🔋 Power Consumption

| State | Current | Notes |
|-------|---------|-------|
| Active (BLE scan) | ~120 mA | 2 s every poll cycle |
| Active (sensor only) | ~80 mA | WiFi/BT radio off |
| Deep sleep | ~10 µA | ESP32 deep sleep |
| **Average (5 min cycle)** | **~4 mA** | Estimated |

A 10,000 mAh USB bank ≈ **100+ days** of runtime.

---

## 📄 License

MIT  free to use, modify, and distribute.

---

## 🤝 Contributing

PRs welcome! Ideas for improvement:
- WiFi push notification when intruder detected
- OLED display showing distance + status
- MQTT integration for home automation
- Multiple bay support
- A battery-friendly ESP32-based system that guards your parking spot.
It detects intruders using an ultrasonic sensor, identifies your car via a BLE beacon (your phone), sounds a buzzer alarm in a 10 s ON / 5 s OFF pattern, and deep-sleeps for 5 minutes when the bay is empty.


