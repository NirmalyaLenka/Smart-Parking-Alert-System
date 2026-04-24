/*
 * ============================================================
 *  SMART PARKING ALERT SYSTEM
 *  Platform : ESP32 (38-pin or 30-pin DevKit)
 *  Sensor   : HC-SR04 Ultrasonic (chosen over IR for range,
 *             directionality, and immunity to ambient light)
 *  Owner ID : BLE Beacon (phone / keyfob)
 *             — chosen over NFC because it works from inside
 *               the car without opening a door / window
 *  Author   : Your Name
 *  License  : MIT
 * ============================================================
 *
 *  WHY ULTRASONIC over IR?
 *  - Range up to 4 m  (IR passive ~1 m)
 *  - Not fooled by sunlight, heat, or object colour
 *  - Single sensor covers the full width of a parking bay
 *  - Easily waterproofed (JSN-SR04T variant)
 *
 *  WHY BLE BEACON over NFC?
 *  - Works from inside the car (phone in pocket/dashboard)
 *  - Range ~5-10 m; ESP32 detects owner BEFORE car fully parks
 *  - No extra hardware tag needed — just the owner's phone
 *  - Ultra-low power scanning with ESP32's built-in BLE radio
 *
 *  LOGIC OVERVIEW
 *  ─────────────────────────────────────────────────────────
 *  1. Wake from deep-sleep (or cold boot).
 *  2. Quick BLE scan (OWNER_BLE_SCAN_WINDOW_MS).
 *     If owner beacon found → owner is parking → go back to
 *     light-sleep; poll every OWNER_POLL_INTERVAL_MS.
 *  3. Fire ultrasonic sensor.
 *     If distance > EMPTY_THRESHOLD_CM → spot empty.
 *       Increment empty counter; if ≥ EMPTY_COUNT_FOR_SLEEP
 *       → deep-sleep for DEEP_SLEEP_DURATION_US.
 *     If distance ≤ CAR_THRESHOLD_CM → vehicle detected.
 *       BLE scan again (double-check owner).
 *       If owner NOT present → INTRUDER!
 *         → beep pattern: 10 s ON / 5 s OFF (continuous)
 *         → repeat until spot clears OR owner detected
 * ============================================================
 */

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"

// ── PIN DEFINITIONS ──────────────────────────────────────────
#define TRIG_PIN        5   // HC-SR04 Trigger
#define ECHO_PIN        18  // HC-SR04 Echo
#define BUZZER_PIN      19  // Active buzzer (or piezo via transistor)
#define STATUS_LED_PIN  2   // Onboard LED (blue)

// ── DISTANCE THRESHOLDS (cm) ─────────────────────────────────
#define CAR_THRESHOLD_CM    80   // Object closer than this = vehicle in bay
#define EMPTY_THRESHOLD_CM  200  // Object farther than this = bay clear

// ── TIMING ───────────────────────────────────────────────────
#define MEASURE_INTERVAL_MS       500   // Poll sensor every 500 ms
#define EMPTY_COUNT_FOR_SLEEP     10    // 10 × 500 ms = 5 s of empty → sleep
#define DEEP_SLEEP_DURATION_US    (5ULL * 60 * 1000000)  // 5 min deep sleep
#define OWNER_BLE_SCAN_WINDOW_MS  2000  // BLE scan duration
#define OWNER_POLL_INTERVAL_MS    5000  // Re-check when owner is here

// ── BUZZER ALARM PATTERN ─────────────────────────────────────
#define BEEP_ON_MS   10000  // 10 s buzzer on
#define BEEP_OFF_MS   5000  // 5 s pause

// ── OWNER BLE BEACON ─────────────────────────────────────────
// Set this to your phone's BLE MAC address (lower-case, colon-separated)
// OR set OWNER_BLE_NAME to the advertised name of your BLE device.
// To find your phone's BLE MAC: Android → Developer Options → BLE address
//                               iOS    → Settings → General → About (BT address)
#define OWNER_BLE_NAME  "Owner-Beacon"   // BLE advertised name  ← primary match
#define OWNER_BLE_MAC   "aa:bb:cc:dd:ee:ff"  // fallback MAC match (optional)

// ─────────────────────────────────────────────────────────────

BLEScan* pBLEScan = nullptr;
bool     ownerFound   = false;
uint8_t  emptyCounter = 0;

// ── BLE SCAN CALLBACK ────────────────────────────────────────
class OwnerBLECallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    String name = advertisedDevice.getName().c_str();
    String mac  = advertisedDevice.getAddress().toString().c_str();
    if (name == OWNER_BLE_NAME || mac == OWNER_BLE_MAC) {
      ownerFound = true;
      pBLEScan->stop();   // Stop early — owner found
      Serial.println("[BLE] Owner beacon detected.");
    }
  }
};

// ── ULTRASONIC MEASURE ───────────────────────────────────────
float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30 ms timeout ≈ 5 m
  if (duration == 0) return 999.0f;               // timeout → treat as empty
  return (duration * 0.0343f) / 2.0f;
}

// ── BLE SCAN (blocking for OWNER_BLE_SCAN_WINDOW_MS) ─────────
bool scanForOwner() {
  ownerFound = false;
  if (pBLEScan == nullptr) {
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new OwnerBLECallbacks(), true);
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
  }
  pBLEScan->start(OWNER_BLE_SCAN_WINDOW_MS / 1000, false);
  pBLEScan->clearResults();
  return ownerFound;
}

// ── BUZZER HELPERS ───────────────────────────────────────────
void buzzerOn()  { digitalWrite(BUZZER_PIN, HIGH); }
void buzzerOff() { digitalWrite(BUZZER_PIN, LOW);  }
void shortBeep(int ms = 100) {
  buzzerOn(); delay(ms); buzzerOff();
}

// ── ALARM LOOP (runs until car leaves or owner arrives) ───────
void runAlarm() {
  Serial.println("[ALARM] Intruder detected! Starting alarm.");
  while (true) {
    // 10 s on
    buzzerOn();
    digitalWrite(STATUS_LED_PIN, HIGH);
    unsigned long t = millis();
    while (millis() - t < BEEP_ON_MS) {
      float d = measureDistance();
      Serial.printf("[ALARM] Distance: %.1f cm\n", d);
      // Car left?
      if (d > EMPTY_THRESHOLD_CM) { buzzerOff(); digitalWrite(STATUS_LED_PIN, LOW); Serial.println("[ALARM] Car left. Alarm stopped."); return; }
      // Owner arrived with car?
      if (scanForOwner()) { buzzerOff(); digitalWrite(STATUS_LED_PIN, LOW); Serial.println("[ALARM] Owner detected. Alarm stopped."); return; }
      delay(500);
    }
    // 5 s off
    buzzerOff();
    digitalWrite(STATUS_LED_PIN, LOW);
    t = millis();
    while (millis() - t < BEEP_OFF_MS) {
      float d = measureDistance();
      if (d > EMPTY_THRESHOLD_CM) { Serial.println("[ALARM] Car left during pause. Alarm stopped."); return; }
      if (scanForOwner()) { Serial.println("[ALARM] Owner detected during pause. Alarm stopped."); return; }
      delay(500);
    }
  }
}

// ── DEEP SLEEP ───────────────────────────────────────────────
void goToDeepSleep() {
  Serial.println("[SLEEP] No vehicle for 5 min. Entering deep sleep...");
  shortBeep(50); delay(100); shortBeep(50); // two chirps before sleep
  Serial.flush();

  // Wake on ECHO_PIN going LOW→HIGH (vehicle entering triggers echo change)
  // Simpler: wake by timer every DEEP_SLEEP_DURATION_US and re-measure
  esp_sleep_enable_timer_wakeup(DEEP_SLEEP_DURATION_US);
  // Optional: also wake if something breaks the sensor beam
  // esp_sleep_enable_ext0_wakeup((gpio_num_t)ECHO_PIN, 1);
  esp_deep_sleep_start();
}

// ── SETUP ─────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("\n[BOOT] Smart Parking Alert System v1.0");

  pinMode(TRIG_PIN,       OUTPUT);
  pinMode(ECHO_PIN,       INPUT);
  pinMode(BUZZER_PIN,     OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);

  buzzerOff();
  digitalWrite(STATUS_LED_PIN, LOW);

  // Startup beep
  shortBeep(200);
  delay(100);
  shortBeep(200);

  // Print wake reason
  esp_sleep_wakeup_cause_t wakeup = esp_sleep_get_wakeup_cause();
  switch (wakeup) {
    case ESP_SLEEP_WAKEUP_TIMER:    Serial.println("[BOOT] Wake: timer"); break;
    case ESP_SLEEP_WAKEUP_EXT0:     Serial.println("[BOOT] Wake: sensor trigger"); break;
    default:                        Serial.println("[BOOT] Wake: power-on / reset"); break;
  }
}

// ── LOOP ──────────────────────────────────────────────────────
void loop() {
  float distance = measureDistance();
  Serial.printf("[SENSOR] Distance: %.1f cm\n", distance);

  // ── SPOT EMPTY ──────────────────────────────────────────────
  if (distance > EMPTY_THRESHOLD_CM) {
    emptyCounter++;
    Serial.printf("[STATUS] Spot empty (%d/%d)\n", emptyCounter, EMPTY_COUNT_FOR_SLEEP);
    digitalWrite(STATUS_LED_PIN, LOW);
    if (emptyCounter >= EMPTY_COUNT_FOR_SLEEP) {
      goToDeepSleep();
    }
    delay(MEASURE_INTERVAL_MS);
    return;
  }

  // ── VEHICLE DETECTED ────────────────────────────────────────
  emptyCounter = 0;
  Serial.printf("[STATUS] Vehicle detected at %.1f cm. Scanning BLE...\n", distance);
  digitalWrite(STATUS_LED_PIN, HIGH);

  bool owner = scanForOwner();
  if (owner) {
    Serial.println("[STATUS] Owner is parking. Monitoring quietly...");
    delay(OWNER_POLL_INTERVAL_MS);
  } else {
    Serial.println("[STATUS] Unknown vehicle! Triggering alarm.");
    runAlarm();
    emptyCounter = 0;
  }
}
