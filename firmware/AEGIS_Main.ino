
/***************************************************************
 * PROJECT AEGIS FINAL - PROFILE OLED + ACCESS COUNTER
 * Ambient Environmental Guard and Intelligence System
 *
 * ESP32 DevKit V1
 * DHT11
 * HC-SR501 PIR
 * SH1106 OLED
 * BC547 + 5V Active Buzzer
 * Red and Green LEDs
 *
 * Blynk:
 * V0 = Temperature
 * V1 = Humidity
 * V2 = Motion Event
 * V3 = Operating Profile
 * V4 = Persistent FAMILY Room Access Count
 *
 * USB Serial = Python / YOLO26n communication
 *
 * Profiles:
 * 0 = ME
 * 1 = FAMILY
 * 2 = VULNERABLE
 ***************************************************************/

// =============================================================
// 1. BLYNK CONFIGURATION
// =============================================================
// Create Blynk Datastream V4 manually: Integer, Room Access Count.


char ssid[] = "********";
char pass[] = "********";
#define BLYNK_TEMPLATE_ID "******"
#define BLYNK_TEMPLATE_NAME "****"
#define BLYNK_AUTH_TOKEN    "********"

// =============================================================
// 2. LIBRARIES
// =============================================================

#include <WiFi.h>
#include <BlynkSimpleEsp32_SSL.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <Preferences.h>

// =============================================================
// 3. GPIO CONFIGURATION
// =============================================================

constexpr uint8_t DHT_PIN       = 4;
constexpr uint8_t PIR_PIN       = 27;
constexpr uint8_t BUZZER_PIN    = 26;

constexpr uint8_t RED_LED_PIN   = 18;
constexpr uint8_t GREEN_LED_PIN = 19;

constexpr uint8_t I2C_SDA_PIN   = 21;
constexpr uint8_t I2C_SCL_PIN   = 22;

// =============================================================
// 4. TIMING CONFIGURATION
// =============================================================

constexpr uint32_t PIR_WARMUP_MS        = 60000UL;
constexpr uint32_t DHT_READ_INTERVAL    = 2000UL;
constexpr uint32_t ENV_PUBLISH_INTERVAL = 60000UL;

constexpr uint32_t MOTION_EVENT_MS      = 2000UL;
constexpr uint32_t ALARM_DURATION_MS    = 6000UL;
constexpr uint32_t BUZZER_TOGGLE_MS     = 200UL;

constexpr uint32_t DISPLAY_REFRESH_MS   = 250UL;

constexpr uint32_t WIFI_RETRY_MS        = 10000UL;
constexpr uint32_t BLYNK_RETRY_MS       = 5000UL;

// Computer vision communication
constexpr uint32_t CAMERA_HEARTBEAT_TIMEOUT_MS = 8000UL;
constexpr uint32_t CV_VERIFICATION_TIMEOUT_MS  = 7500UL;

// =============================================================
// 5. SENSOR AND DISPLAY OBJECTS
// =============================================================

#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

Preferences aegisPreferences;
bool accessCounterStorageReady = false;

U8G2_SH1106_128X64_NONAME_1_HW_I2C
u8g2(U8G2_R0, U8X8_PIN_NONE);

// =============================================================
// 6. OPERATING PROFILES
// =============================================================

enum OperatingMode : uint8_t {
  MODE_ME = 0,
  MODE_FAMILY = 1,
  MODE_VULNERABLE = 2
};

OperatingMode currentMode = MODE_ME;

const char* profileName(OperatingMode mode) {
  switch (mode) {
    case MODE_ME: return "ME";
    case MODE_FAMILY: return "FAMILY";
    case MODE_VULNERABLE: return "VULNERABLE";
    default: return "UNKNOWN";
  }
}

// =============================================================
// 7. RUNTIME VARIABLES
// =============================================================

// Environmental data
float temperatureC = NAN;
float humidityPct = NAN;

bool dhtReadingValid = false;

uint32_t lastDhtReadMs = 0;
uint32_t lastEnvPublishMs = 0;

bool environmentPublished = false;

// Persistent count of FAMILY-mode PIR motion events.
uint32_t familyAccessCount = 0;
constexpr uint32_t MAX_ACCESS_COUNT = 2147483647UL;

// PIR
bool pirReady = false;
bool pirOutputHigh = false;
bool previousPirState = false;

uint32_t pirWarmupStartMs = 0;

// Motion event
bool motionEventActive = false;
uint32_t motionEventStartMs = 0;

// Computer vision
bool cameraLinkOnline = false;

uint32_t lastCameraHeartbeatMs = 0;

uint32_t nextVerificationEventId = 1;
uint32_t pendingVerificationEventId = 0;

uint32_t verificationStartMs = 0;

bool verificationPending = false;

char serialCommandBuffer[96];
uint8_t serialCommandLength = 0;

// Alarm
bool alarmActive = false;
bool buzzerOutputOn = false;

uint32_t alarmStartMs = 0;
uint32_t lastBuzzerToggleMs = 0;

// Other
uint32_t lastDisplayUpdateMs = 0;
uint32_t lastWifiRetryMs = 0;
uint32_t lastBlynkRetryMs = 0;

// =============================================================
// 8. FAMILY ACCESS COUNTER (ESP32 NVS / PREFERENCES)
// =============================================================

void loadFamilyAccessCount() {

  accessCounterStorageReady = aegisPreferences.begin("aegis", false);

  if (!accessCounterStorageReady) {
    Serial.println("[NVS] Could not open storage. Access count is RAM-only.");
    familyAccessCount = 0;
    return;
  }

  familyAccessCount = aegisPreferences.getUInt("familyCount", 0);

  Serial.print("[NVS] Restored FAMILY access count: ");
  Serial.println(familyAccessCount);
}

void recordFamilyAccess() {

  if (currentMode != MODE_FAMILY) return;

  if (familyAccessCount < MAX_ACCESS_COUNT) {
    familyAccessCount++;
  }

  if (accessCounterStorageReady) {
    size_t bytesWritten = aegisPreferences.putUInt(
      "familyCount",
      familyAccessCount
    );

    if (bytesWritten != sizeof(familyAccessCount)) {
      Serial.println("[NVS] WARNING: Access count could not be saved.");
    }
  }
  else {
    Serial.println("[NVS] WARNING: Access count is not persistent.");
  }

  Serial.print("[FAMILY] Room access count: ");
  Serial.println(familyAccessCount);

  if (Blynk.connected()) {
    Blynk.virtualWrite(V4, familyAccessCount);
    Serial.println("[BLYNK] V4 access count updated.");
  }
}

// =============================================================
// 9. PROFILE LED CONTROL
// =============================================================

void updateProfileLEDs() {

  switch (currentMode) {

    case MODE_ME:
      digitalWrite(GREEN_LED_PIN, HIGH);
      digitalWrite(RED_LED_PIN, LOW);
      break;

    case MODE_FAMILY:
      digitalWrite(GREEN_LED_PIN, HIGH);
      digitalWrite(RED_LED_PIN, HIGH);
      break;

    case MODE_VULNERABLE:
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(RED_LED_PIN, HIGH);
      break;
  }
}

// =============================================================
// 10. BUZZER CONTROL
// =============================================================

void stopSecurityAlarm(const char* reason) {

  if (alarmActive || buzzerOutputOn) {
    Serial.print("[ALARM] Stopped: ");
    Serial.println(reason);
  }

  alarmActive = false;
  buzzerOutputOn = false;

  digitalWrite(BUZZER_PIN, LOW);
}

void startSecurityAlarm(uint32_t now) {

  if (currentMode != MODE_VULNERABLE) return;

  alarmActive = true;

  alarmStartMs = now;
  lastBuzzerToggleMs = now;

  buzzerOutputOn = true;

  digitalWrite(BUZZER_PIN, HIGH);

  Serial.println();
  Serial.println("[SECURITY] BREACH DETECTED!");
  Serial.println("[ALARM] Active buzzer activated.");
}

void serviceSecurityAlarm(uint32_t now) {

  if (!alarmActive) return;

  if (currentMode != MODE_VULNERABLE) {
    stopSecurityAlarm("Profile changed");
    return;
  }

  if ((uint32_t)(now - alarmStartMs) >= ALARM_DURATION_MS) {
    stopSecurityAlarm("Six-second alarm completed");
    return;
  }

  if ((uint32_t)(now - lastBuzzerToggleMs) >= BUZZER_TOGGLE_MS) {

    lastBuzzerToggleMs = now;

    buzzerOutputOn = !buzzerOutputOn;

    digitalWrite(
      BUZZER_PIN,
      buzzerOutputOn ? HIGH : LOW
    );
  }
}

// =============================================================
// 11. BLYNK MOTION EVENT
// =============================================================

void sendMotionEventOn() {

  motionEventActive = true;
  motionEventStartMs = millis();

  Serial.println("[BLYNK EVENT] Motion detected: V2 = 1");

  if (Blynk.connected()) {
    Blynk.virtualWrite(V2, 1);
  }
}

void serviceMotionEvent(uint32_t now) {

  if (
    motionEventActive &&
    (uint32_t)(now - motionEventStartMs) >= MOTION_EVENT_MS
  ) {

    motionEventActive = false;

    if (Blynk.connected()) {
      Blynk.virtualWrite(V2, 0);
    }

    Serial.println("[BLYNK EVENT] Motion event reset: V2 = 0");
  }
}

// =============================================================
// 12. COMPUTER VISION SERIAL VERIFICATION
// =============================================================

void cancelPendingVerification(const char* reason) {

  if (verificationPending) {
    Serial.print("[CV] Verification cancelled: ");
    Serial.println(reason);
  }

  verificationPending = false;
  pendingVerificationEventId = 0;
}

void beginCameraVerification(uint32_t now) {

  pendingVerificationEventId = nextVerificationEventId++;
  verificationStartMs = now;
  verificationPending = true;

  Serial.print("VERIFY,");
  Serial.println(pendingVerificationEventId);

  Serial.print("[CV] Verification requested for event ");
  Serial.println(pendingVerificationEventId);
}

void processCameraCommand(const char* command) {

  if (strcmp(command, "HEARTBEAT") == 0) {

    lastCameraHeartbeatMs = millis();

    if (!cameraLinkOnline) {
      cameraLinkOnline = true;
      Serial.println("[CV] Camera computer link ONLINE.");
    }

    return;
  }

  if (strncmp(command, "RESULT,", 7) != 0) return;

  unsigned long eventId = 0;
  char result[32] = {0};

  if (
    sscanf(command, "RESULT,%lu,%31[^,]", &eventId, result) != 2
  ) {
    Serial.println("[CV] Malformed result ignored.");
    return;
  }

  if (
    !verificationPending ||
    eventId != pendingVerificationEventId
  ) {
    Serial.println("[CV] Stale or unexpected event result ignored.");
    return;
  }

  verificationPending = false;
  pendingVerificationEventId = 0;

  if (currentMode != MODE_VULNERABLE) return;

  if (strcmp(result, "PERSON_DETECTED") == 0) {

    Serial.println("[CV] PERSON CONFIRMED. Activating security alarm.");

    startSecurityAlarm(millis());
  }

  else if (strcmp(result, "NO_PERSON") == 0) {

    Serial.println("[CV] No person confirmed. No alarm.");
  }

  else if (strcmp(result, "CAMERA_ERROR") == 0) {

    cameraLinkOnline = false;

    Serial.println(
      "[CV] Camera error. Switching to PIR-only fallback."
    );

    startSecurityAlarm(millis());
  }

  else {

    Serial.print("[CV] Unknown result ignored: ");
    Serial.println(result);
  }
}

void serviceCameraSerial(uint32_t now) {

  while (Serial.available() > 0) {

    char incoming = (char)Serial.read();

    if (incoming == '\r') continue;

    if (incoming == '\n') {

      serialCommandBuffer[serialCommandLength] = '\0';

      if (serialCommandLength > 0) {
        processCameraCommand(serialCommandBuffer);
      }

      serialCommandLength = 0;
    }

    else if (
      serialCommandLength < sizeof(serialCommandBuffer) - 1
    ) {

      serialCommandBuffer[serialCommandLength++] = incoming;
    }

    else {

      serialCommandLength = 0;

      Serial.println("[CV] Oversized serial command discarded.");
    }
  }

  // Refresh time after processing incoming serial commands.
  // A heartbeat may have updated lastCameraHeartbeatMs above.
  // Using the older loop timestamp here can cause unsigned
  // subtraction underflow and a false OFFLINE transition.
  now = millis();

  bool linkNowOnline =
    lastCameraHeartbeatMs != 0 &&
    (uint32_t)(now - lastCameraHeartbeatMs)
      <= CAMERA_HEARTBEAT_TIMEOUT_MS;

  // Genuine loss of heartbeat: switch to PIR-only fallback.

  if (cameraLinkOnline && !linkNowOnline) {

    cameraLinkOnline = false;

    Serial.println(
      "[CV] Camera link OFFLINE. PIR-only fallback enabled."
    );

    if (
      verificationPending &&
      currentMode == MODE_VULNERABLE
    ) {

      cancelPendingVerification(
        "Camera link lost; using PIR-only fallback"
      );

      startSecurityAlarm(now);
    }
  }

  // No valid result before deadline: treat verification service
  // as unavailable and activate PIR-only fallback.

  if (
    verificationPending &&
    (uint32_t)(now - verificationStartMs)
      >= CV_VERIFICATION_TIMEOUT_MS
  ) {

    cancelPendingVerification(
      "Verification timeout; using PIR-only fallback"
    );

    if (currentMode == MODE_VULNERABLE) {

      Serial.println(
        "[CV] VERIFICATION TIMEOUT. PIR-only fallback alarm."
      );

      cameraLinkOnline = false;

      startSecurityAlarm(now);
    }
  }
}

// =============================================================
// 13. PIR MOTION PROCESSING
// =============================================================

void handleMotionRisingEdge(uint32_t now) {

  Serial.println();
  Serial.println("==============================");

  Serial.println("[PIR] MOTION DETECTED");

  Serial.print("[PIR] Active profile: ");
  Serial.println(profileName(currentMode));

  sendMotionEventOn();

  // One count per new PIR rising-edge event, only in FAMILY mode.
  recordFamilyAccess();

  if (currentMode == MODE_VULNERABLE) {

    if (cameraLinkOnline) {

      Serial.println(
        "[SECURITY] Camera online. Requesting verification."
      );

      beginCameraVerification(now);
    }

    else {

      Serial.println(
        "[SECURITY] Camera offline. PIR-only fallback alarm."
      );

      startSecurityAlarm(now);
    }
  }

  else {

    Serial.println("[SECURITY] Monitoring only. No alarm.");
  }

  Serial.println("==============================");
}

void servicePIR(uint32_t now) {

  pirOutputHigh = (digitalRead(PIR_PIN) == HIGH);

  if (!pirReady) {

    if (
      (uint32_t)(now - pirWarmupStartMs) >= PIR_WARMUP_MS
    ) {

      previousPirState = pirOutputHigh;
      pirReady = true;

      Serial.println("[PIR] Warm-up completed.");
    }

    return;
  }

  if (pirOutputHigh && !previousPirState) {
    handleMotionRisingEdge(now);
  }

  previousPirState = pirOutputHigh;
}

// =============================================================
// 14. DHT11 SENSOR PROCESSING
// =============================================================

void readDHT11() {

  float newHumidity = dht.readHumidity();
  float newTemperature = dht.readTemperature();

  if (isnan(newHumidity) || isnan(newTemperature)) {

    dhtReadingValid = false;

    Serial.println("[DHT11] Reading failed.");

    return;
  }

  temperatureC = newTemperature;
  humidityPct = newHumidity;

  dhtReadingValid = true;

  Serial.print("[DHT11] Temperature: ");
  Serial.print(temperatureC, 1);

  Serial.print(" C | Humidity: ");
  Serial.print(humidityPct, 1);

  Serial.println(" %");
}

void serviceDHT11(uint32_t now) {

  if (
    (uint32_t)(now - lastDhtReadMs) >= DHT_READ_INTERVAL
  ) {

    lastDhtReadMs = now;

    readDHT11();
  }
}

// =============================================================
// 15. BLYNK ENVIRONMENTAL DATA TRANSMISSION
// =============================================================

void publishEnvironmentIfDue(uint32_t now) {

  if (!dhtReadingValid || !Blynk.connected()) return;

  if (
    !environmentPublished ||
    (uint32_t)(now - lastEnvPublishMs) >= ENV_PUBLISH_INTERVAL
  ) {

    Blynk.virtualWrite(V0, temperatureC);
    Blynk.virtualWrite(V1, humidityPct);

    lastEnvPublishMs = now;
    environmentPublished = true;

    Serial.println("[BLYNK] Environmental data updated.");
  }
}

// =============================================================
// 16. OLED BREACH DISPLAY
// =============================================================

void drawBreachDisplay() {

  u8g2.setFont(u8g2_font_ncenB14_tr);

  const char* message = "BREACH";

  int16_t x =
    (128 - (int16_t)u8g2.getStrWidth(message)) / 2;

  u8g2.drawStr(x, 16, message);
  u8g2.drawStr(x, 38, message);
  u8g2.drawStr(x, 60, message);
}

// =============================================================
// 17. PROFILE-SPECIFIC OLED DISPLAYS
// =============================================================

void drawScreenHeader(const char* rightLabel) {

  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(3, 10, "AEGIS");

  int16_t labelWidth = u8g2.getStrWidth(rightLabel);
  u8g2.drawStr(125 - labelWidth, 10, rightLabel);

  u8g2.drawHLine(0, 14, 128);
}

void drawMeDisplay() {

  char temperatureText[12];
  char humidityText[12];

  drawScreenHeader("ME MODE");

  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(4, 28, "TEMPERATURE");
  u8g2.drawStr(73, 28, "HUMIDITY");
  u8g2.drawVLine(65, 20, 38);

  if (dhtReadingValid) {
    snprintf(temperatureText, sizeof(temperatureText), "%.1fC", temperatureC);
    snprintf(humidityText, sizeof(humidityText), "%.0f%%", humidityPct);
  }
  else {
    snprintf(temperatureText, sizeof(temperatureText), "--.-C");
    snprintf(humidityText, sizeof(humidityText), "--%%");
  }

  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(3, 53, temperatureText);
  u8g2.drawStr(73, 53, humidityText);
}

void drawFamilyDisplay() {

  char countText[16];

  drawScreenHeader("FAMILY");

  // Live PIR state: one visual indicator and a concise status message.
  if (!pirReady) {
    u8g2.drawCircle(8, 25, 3);
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(17, 28, "PIR INITIALISING");
  }
  else if (pirOutputHigh) {
    u8g2.drawDisc(8, 25, 3);
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(17, 28, "MOTION DETECTED");
  }
  else {
    u8g2.drawCircle(8, 25, 3);
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(17, 28, "ROOM MONITORING");
  }

  u8g2.drawHLine(0, 36, 128);
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(4, 47, "ROOM ACCESS COUNT");

  snprintf(countText, sizeof(countText), "%lu",
           (unsigned long)familyAccessCount);

  u8g2.setFont(u8g2_font_ncenB14_tr);
  int16_t countWidth = u8g2.getStrWidth(countText);
  u8g2.drawStr((128 - countWidth) / 2, 63, countText);
}

void drawVulnerableIdleDisplay() {

  drawScreenHeader("ARMED");

  u8g2.setFont(u8g2_font_ncenB12_tr);
  const char* armedText = "SECURITY ARMED";
  int16_t armedWidth = u8g2.getStrWidth(armedText);
  u8g2.drawStr((128 - armedWidth) / 2, 32, armedText);

  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(3, 47, pirReady ? "PIR: MONITORING" : "PIR: WARMING UP");
  u8g2.drawStr(3, 60, cameraLinkOnline ? "CV: ONLINE" : "CV: PIR ONLY");
}

// =============================================================
// 18. OLED REFRESH
// =============================================================

void refreshOLED(uint32_t now) {

  uint32_t refreshInterval =
    alarmActive ? 150UL : DISPLAY_REFRESH_MS;

  if (
    (uint32_t)(now - lastDisplayUpdateMs) < refreshInterval
  ) {
    return;
  }

  lastDisplayUpdateMs = now;

  u8g2.firstPage();

  do {

    if (
      alarmActive &&
      currentMode == MODE_VULNERABLE
    ) {

      drawBreachDisplay();
    }

    else {

      switch (currentMode) {
        case MODE_ME:
          drawMeDisplay();
          break;

        case MODE_FAMILY:
          drawFamilyDisplay();
          break;

        case MODE_VULNERABLE:
          drawVulnerableIdleDisplay();
          break;
      }
    }

  } while (u8g2.nextPage());
}

// =============================================================
// 19. BLYNK PROFILE RECEIVER
// =============================================================

BLYNK_WRITE(V3) {

  int requestedMode = param.asInt();

  Serial.println();
  Serial.println("[BLYNK] Profile command received.");

  Serial.print("[BLYNK] V3: ");
  Serial.println(requestedMode);

  if (requestedMode < 0 || requestedMode > 2) {

    Serial.println("[ERROR] Invalid profile value.");

    return;
  }

  OperatingMode newMode =
    (OperatingMode)requestedMode;

  if (newMode != currentMode) {

    currentMode = newMode;

    updateProfileLEDs();

    Serial.print("[AEGIS] Profile changed to: ");
    Serial.println(profileName(currentMode));

    if (currentMode != MODE_VULNERABLE) {

      cancelPendingVerification(
        "Profile changed to ME or FAMILY"
      );

      stopSecurityAlarm(
        "Profile changed to ME or FAMILY"
      );
    }

    else {

      Serial.println("[SECURITY] VULNERABLE profile armed.");
    }
  }

  else {

    Serial.print("[AEGIS] Profile remains: ");
    Serial.println(profileName(currentMode));
  }
}

// =============================================================
// 20. BLYNK CONNECTION CALLBACK
// =============================================================

BLYNK_CONNECTED() {

  Serial.println("[BLYNK] Secure cloud connected.");

  Serial.println("[BLYNK] Synchronising V3 profile.");

  Blynk.syncVirtual(V3);

  Blynk.virtualWrite(
    V2,
    motionEventActive ? 1 : 0
  );

  // Restore the latest persistent count to the Blynk dashboard.
  Blynk.virtualWrite(V4, familyAccessCount);

  if (dhtReadingValid) {

    Blynk.virtualWrite(V0, temperatureC);
    Blynk.virtualWrite(V1, humidityPct);

    lastEnvPublishMs = millis();
    environmentPublished = true;
  }
}

// =============================================================
// 21. NETWORK RECONNECTION
// =============================================================

void serviceNetwork(uint32_t now) {

  if (WiFi.status() != WL_CONNECTED) {

    if (
      (uint32_t)(now - lastWifiRetryMs) >= WIFI_RETRY_MS
    ) {

      lastWifiRetryMs = now;

      Serial.println("[WIFI] Reconnecting...");

      WiFi.reconnect();
    }

    return;
  }

  if (Blynk.connected()) {

    Blynk.run();

    return;
  }

  if (
    (uint32_t)(now - lastBlynkRetryMs) >= BLYNK_RETRY_MS
  ) {

    lastBlynkRetryMs = now;

    Serial.println("[BLYNK] Reconnecting...");

    Blynk.connect(500);
  }
}

// =============================================================
// 22. SETUP
// =============================================================

void setup() {

  Serial.begin(115200);

  delay(300);

  Serial.println();
  Serial.println("========================================");
  Serial.println("       AEGIS INITIALISATION");
  Serial.println("========================================");

  // Restore access history before the first Blynk connection.
  loadFamilyAccessCount();

  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);

  updateProfileLEDs();

  // OLED and I2C

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  u8g2.begin();

  // Sensors

  dht.begin();

  uint32_t bootTime = millis();

  pirWarmupStartMs = bootTime;
  lastDhtReadMs = bootTime;
  lastWifiRetryMs = bootTime;
  lastBlynkRetryMs = bootTime;

  Serial.println("[SYSTEM] Startup profile: ME");
  Serial.println("[SYSTEM] PIR warm-up: 60 seconds.");

  // Wi-Fi

  WiFi.mode(WIFI_STA);

  WiFi.setAutoReconnect(true);

  WiFi.begin(ssid, pass);

  Serial.print("[WIFI] Connecting");

  uint32_t wifiStart = millis();

  while (
    WiFi.status() != WL_CONNECTED &&
    (uint32_t)(millis() - wifiStart) < 15000UL
  ) {

    delay(250);

    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.print("[WIFI] Connected. IP: ");

    Serial.println(WiFi.localIP());
  }

  else {

    Serial.println("[WIFI] Initial connection timed out.");

    Serial.println("[SYSTEM] Local functions remain active.");
  }

  // Blynk SSL

  Blynk.config(BLYNK_AUTH_TOKEN);

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("[BLYNK] Connecting securely...");

    if (Blynk.connect(8000)) {

      Serial.println("[BLYNK] Initial connection successful.");
    }

    else {

      Serial.println("[BLYNK] Connection failed. Retrying later.");
    }
  }

  lastWifiRetryMs = millis();

  lastBlynkRetryMs = millis();

  Serial.println();

  Serial.println("[SYSTEM] AEGIS IS RUNNING.");
  Serial.println("[FAMILY] Persistent room access counter enabled.");

  Serial.println(
    "[SYSTEM] Environmental sampling starts after 2 seconds."
  );

  Serial.println(
    "[SYSTEM] Local PIR and alarm logic are independent of cloud."
  );

  Serial.println(
    "[CV] Waiting for laptop HEARTBEAT. PIR-only fallback is active."
  );
}

// =============================================================
// 23. MAIN LOOP
// =============================================================

void loop() {

  uint32_t now = millis();

  // USB Serial computer-vision communication

  serviceCameraSerial(now);

  now = millis();

  // Network management

  serviceNetwork(now);

  now = millis();

  // Local subsystems

  serviceDHT11(now);

  servicePIR(now);

  serviceMotionEvent(now);

  serviceSecurityAlarm(now);

  publishEnvironmentIfDue(now);

  refreshOLED(now);
}

