# Step-by-Step Setup Guide

## Prerequisites

- ESP32 DevKit (any variant with BLE)
- Arduino IDE 2.x or PlatformIO
- Android phone (for BLE beacon)
- Components from the hardware list

---

## Step 1 — Install Arduino IDE & ESP32 Support

1. Download [Arduino IDE 2.x](https://www.arduino.cc/en/software)
2. Open Arduino IDE
3. Go to `File → Preferences`
4. In "Additional boards manager URLs" paste:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
5. Go to `Tools → Board → Boards Manager`
6. Search for "esp32" and install **esp32 by Espressif Systems** (v2.x or later)

---

## Step 2 — Wire the Circuit

Follow [hardware/WIRING.md](../hardware/WIRING.md)

**Key connections:**
- HC-SR04 TRIG → GPIO 5
- HC-SR04 ECHO → GPIO 18
- Buzzer (via transistor) → GPIO 19
- Status LED (optional) → GPIO 2

---

## Step 3 — Configure the Firmware

Open `firmware/parking_alert.ino` and update these lines:

```cpp
// Line ~55 — Your BLE beacon name (set this in nRF Connect app)
#define OWNER_BLE_NAME  "Owner-Beacon"

// Line ~56 — Your phone's Bluetooth MAC address (optional backup)
#define OWNER_BLE_MAC   "aa:bb:cc:dd:ee:ff"
```

Optionally adjust thresholds for your bay size:

```cpp
#define CAR_THRESHOLD_CM    80   // Increase if your bay is wider
#define EMPTY_THRESHOLD_CM  200  // Adjust to your ceiling/wall distance
```

---

## Step 4 — Set Up Your BLE Beacon (Android)

1. Install **nRF Connect** from Google Play Store
2. Open the app → tap ≡ menu → **Advertiser**
3. Tap **+** (new advertisement)
4. Set **Display name** = `Owner-Beacon` (must match `OWNER_BLE_NAME`)
5. Set **Advertising interval** = 200 ms
6. Tap the **▶ Start** button
7. Go to phone Settings → Apps → nRF Connect → Battery → **Unrestricted**
   (prevents Android from killing the background BLE advertisement)

---

## Step 5 — Flash the ESP32

1. Connect ESP32 to PC via USB
2. Select board: `Tools → Board → ESP32 Dev Module`
3. Select port: `Tools → Port → COMx` (Windows) or `/dev/ttyUSBx` (Linux)
4. Click **Upload**

Watch the Serial Monitor (115200 baud) for debug output:
```
[BOOT] Smart Parking Alert System v1.0
[BOOT] Wake: power-on / reset
[SENSOR] Distance: 245.3 cm
[STATUS] Spot empty (1/10)
...
```

---

## Step 6 — Test the System

### Test 1 — Empty Bay
- Point sensor at an empty space
- Serial should show "Spot empty (N/10)"
- After 10 readings (~5 s), device enters deep sleep

### Test 2 — Owner Parking
- Start your BLE beacon on your phone
- Walk toward the sensor (simulate parking)
- Serial should show "Owner is parking. Monitoring quietly."
- No alarm should sound

### Test 3 — Intruder
- Stop the BLE beacon
- Put an object within 80 cm of the sensor
- Serial should show "Unknown vehicle! Triggering alarm."
- Buzzer should sound for 10 s, pause 5 s, repeat

### Test 4 — Alarm Stop
- While alarm is running, either:
  - Remove the object → alarm stops immediately
  - Start BLE beacon → alarm stops within 2 s (next scan cycle)

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Sensor always reads 999 cm | Check TRIG/ECHO wiring; ensure 5 V power to sensor |
| BLE never detects owner | Check beacon name matches exactly; phone BT must be on |
| Buzzer not sounding | Check transistor wiring; try direct GPIO connection for test |
| Deep sleep not waking | Check `esp_sleep_enable_timer_wakeup()` call; re-flash |
| False alarms | Increase `CAR_THRESHOLD_CM` or add debounce readings |
| BLE causes boot loop | Update ESP32 Arduino core to v2.0.14+ |

---

## Serial Monitor Output Reference

```
[BOOT]   — Startup / wake reason messages
[SENSOR] — Raw ultrasonic distance readings
[STATUS] — System state (empty, owner, intruder)
[BLE]    — BLE scan results
[ALARM]  — Alarm state changes
[SLEEP]  — Deep sleep entry
```
