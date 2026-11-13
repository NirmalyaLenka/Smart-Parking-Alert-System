# Smart Parking Alert System

An ESP32 prototype that measures distance with an HC-SR04-compatible ultrasonic sensor, scans for a configured BLE owner beacon, and drives a buzzer/LED alarm when a nearby object is not recognized. It uses timed deep sleep after repeated empty readings.

This is a learning prototype, not a validated security system. BLE name/MAC matching is not authentication and can be spoofed; do not rely on it to protect property.

## Behavior in the current firmware

1. Read ultrasonic distance and print it to Serial.
2. If the reading is above `EMPTY_THRESHOLD_CM` (200 cm), increment an empty counter. After 10 readings at 500 ms intervals, the ESP32 enters deep sleep for 5 minutes.
3. Otherwise, scan BLE for the configured owner name or MAC address.
4. If the beacon is found, wait about 5 seconds and check again. If it is not found, run the buzzer alarm loop, nominally 10 seconds on and 5 seconds off, until the distance indicates the spot is empty or the owner beacon is detected.

**Implementation details to know:** `CAR_THRESHOLD_CM` is defined as 80 cm but is not used in the current main decision branch; the firmware treats every reading at or below 200 cm as a vehicle/object. A sensor timeout is converted to 999 cm and therefore counted as an empty spot. The deep-sleep wake source currently enabled is the 5-minute timer; the optional sensor-trigger wake is commented out, so the device does not wake immediately when a vehicle enters.

## Hardware and pinout

| ESP32 GPIO | Connection |
|---|---|
| GPIO 5 | HC-SR04 TRIG |
| GPIO 18 | HC-SR04 ECHO |
| GPIO 19 | Active buzzer control (use the transistor stage shown in the wiring guide where required) |
| GPIO 2 | Optional/on-board status LED |
| GND | Common reference as required by the circuit |

See [`WIRING.md`](WIRING.md) for the full parts and wiring notes, and [`SETUP.md`](SETUP.md) for the beacon setup guide.

**Critical electrical note:** a common HC-SR04 powered from 5 V can output a 5 V ECHO signal. ESP32 GPIOs are not 5 V tolerant. Use a correctly sized resistor divider or logic-level shifter on ECHO before connecting it to GPIO 18. Do not connect the 5 V ECHO signal directly. Verify the actual sensor and board pinout before applying power.

## Software prerequisites

- [PlatformIO Core](https://platformio.org/) or VS Code with the PlatformIO extension; alternatively Arduino IDE 2.x with the Espressif ESP32 board package.
- A USB data cable and the correct serial port.
- For owner detection, a phone or BLE beacon that advertises the configured name. The guide in [`BLE_BEACON_SETUP.h`](BLE_BEACON_SETUP.h) describes a sample setup.

## Build and upload

### PlatformIO (configuration is at repository root)

From the repository root:

```bash
pio run -e esp32dev
pio run -e esp32dev --target upload
pio device monitor -b 115200
```

The optional `esp32_lipo` environment is also defined in `platformio.ini`. Select the actual serial port if PlatformIO cannot identify it.

### Arduino IDE

1. Install Arduino IDE and the **ESP32 by Espressif Systems** board package.
2. Open the root-level `parking_alert.ino`.
3. Select the correct ESP32 board and serial port.
4. Compile and upload, then open Serial Monitor at **115200 baud**.

Before the first powered test, verify the ECHO level shifter and buzzer driver, test the sensor with the buzzer disconnected, and keep the alarm output low-volume/brief during bench checks. Do not mount it outdoors until the enclosure, sensor, power supply, and wiring have been checked.

## Configuration

The active settings are currently defined directly in `parking_alert.ino`.

| Constant | Current value | Role |
|---|---:|---|
| `TRIG_PIN` / `ECHO_PIN` | GPIO 5 / 18 | Ultrasonic sensor |
| `BUZZER_PIN` | GPIO 19 | Alarm output |
| `CAR_THRESHOLD_CM` | 80 cm | Defined but not used in the current main decision branch |
| `EMPTY_THRESHOLD_CM` | 200 cm | Distance above which the spot is treated as empty |
| `MEASURE_INTERVAL_MS` | 500 ms | Delay between empty-state readings |
| `EMPTY_COUNT_FOR_SLEEP` | 10 | Empty readings before sleep (about 5 seconds) |
| `DEEP_SLEEP_DURATION_US` | 300,000,000 µs | Timer wake after 5 minutes |
| `OWNER_BLE_SCAN_WINDOW_MS` | 2000 ms | BLE scan window |
| `OWNER_POLL_INTERVAL_MS` | 5000 ms | Delay after owner detection |
| `BEEP_ON_MS` / `BEEP_OFF_MS` | 10000 / 5000 ms | Nominal alarm pattern |
| `OWNER_BLE_NAME` | `Owner-Beacon` | Primary advertised-name match |
| `OWNER_BLE_MAC` | Placeholder MAC | Fallback address match |

Set `OWNER_BLE_NAME` to the exact advertised name from your beacon app. Some phones randomize BLE addresses, and an advertised name can be copied by another device; neither check proves identity.

## Repository map

| Path | Purpose |
|---|---|
| `parking_alert.ino` | Main ESP32 firmware |
| `platformio.ini` | PlatformIO environments and serial monitor speed |
| `BLE_BEACON_SETUP.h` | BLE beacon setup notes |
| `SETUP.md` | Step-by-step build, beacon, and test guide |
| `WIRING.md` | Parts list, wiring, and power notes |
| `index.html` | Interactive browser simulation only; it does not connect to the ESP32 |
| `gitignore (2)` | Ignore rules under the filename currently present |

## Troubleshooting

- **Distance reads 999 cm:** the sketch uses that value when `pulseIn` times out. Check sensor power, TRIG/ECHO wiring, target angle, and the ECHO level shifter.
- **False empty readings or premature sleep:** inspect sensor alignment and the timeout path; a timeout is currently treated as an empty bay.
- **Owner never detected:** check exact beacon name, phone advertising permissions/background behavior, and Serial output. BLE identity is not secure authentication.
- **Buzzer does not sound:** check transistor orientation, base resistor, supply, and buzzer polarity.
- **ESP32 resets:** check supply capacity and the buzzer driver wiring.
- **Alarm timing differs from nominal values:** BLE scans are blocking operations inside the alarm loop, so scans can extend the actual on/off intervals.

## Limitations and verification

- No measured battery-life or current-consumption data is supplied by the current source; power estimates from older documentation should not be treated as verified.
- The sensor interrupt wake option is not enabled; deep sleep wakes by timer only.
- The 80 cm vehicle threshold is currently unused by the decision branch.
- BLE name/MAC checks are spoofable and should not be treated as access control.
- No firmware build, range test, sleep-current measurement, or physical alarm test was performed during this documentation pass.
- No `LICENSE` file appears in the current repository tree, so the previous README's MIT claim is not backed by a license file in the checked-in root.

## Maintenance & audit

- **Audit date:** 13 November 2026
- **Scope:** documentation-only review; firmware, wiring, and hardware behavior were not changed.
- **Verification:** build and physical behavior remain unverified.
