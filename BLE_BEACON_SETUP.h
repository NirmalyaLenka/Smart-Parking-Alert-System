/*
 * ============================================================
 *  BLE BEACON CONFIGURATION GUIDE
 *  Smart Parking Alert System
 * ============================================================
 *
 *  The ESP32 identifies YOUR car by looking for a specific
 *  BLE (Bluetooth Low Energy) advertisement from your phone.
 *
 *  OPTION A — Android (Recommended)
 *  ─────────────────────────────────
 *  1. Install "BLE Peripheral Simulator" or "nRF Connect"
 *     from the Play Store.
 *
 *  Using nRF Connect:
 *  a. Open nRF Connect → tap the three-dot menu → "Advertiser"
 *  b. Tap "+" to add a new advertisement packet.
 *  c. Under "Device Name" set it to: Owner-Beacon
 *  d. Set Advertising Interval: 200 ms (balance of speed vs battery)
 *  e. Tap the play ▶ button to start advertising.
 *  f. Keep the app running in the background (battery optimisation OFF
 *     for nRF Connect in phone settings).
 *
 *  OPTION B — iPhone / iOS
 *  ────────────────────────
 *  iOS restricts raw BLE advertising. Use the app "Beacon Simulator"
 *  or "LightBlue" and create an iBeacon with a recognisable UUID.
 *
 *  If using iBeacon UUID, update firmware:
 *    #define OWNER_IBEACON_UUID  "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
 *  (An iBeacon callback variant is in firmware/extras/ibeacon_variant.ino)
 *
 *  OPTION C — Dedicated BLE Keyfob / Tag
 *  ──────────────────────────────────────
 *  Any BLE tag that continuously advertises a fixed name/MAC works.
 *  Popular options:
 *  - Tile Pro  (set name via Tile app — requires modification)
 *  - NUT Mini  (advertises continuously, very cheap)
 *  - Feasycom  FSC-BP119 (purpose-built BLE beacon)
 *
 *  FINDING YOUR PHONE'S BLE MAC
 *  ─────────────────────────────
 *  Android: Settings → About Phone → Status → Bluetooth Address
 *  OR use nRF Connect → Scanner → your own device entry
 *
 *  Note: Some phones randomize BLE MAC. Use the advertised NAME
 *  as primary match (OWNER_BLE_NAME) for best reliability.
 *
 *  FIRMWARE SETTINGS TO UPDATE
 *  ─────────────────────────────
 *  In parking_alert.ino:
 *
 *    #define OWNER_BLE_NAME  "Owner-Beacon"     ← match your beacon name
 *    #define OWNER_BLE_MAC   "aa:bb:cc:dd:ee:ff" ← optional fallback
 * ============================================================
 */
