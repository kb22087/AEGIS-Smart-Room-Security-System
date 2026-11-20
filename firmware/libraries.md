# AEGIS – Libraries and Dependencies

This document lists the software libraries and development dependencies required to compile and run the AEGIS ESP32 firmware.

## 1. Development Environment

| Component | Description |
|---|---|
| IDE | Arduino IDE |
| Microcontroller | ESP32 DevKit V1 |
| Board Platform | ESP32 by Espressif Systems |
| Programming Language | C++ (Arduino framework) |
| Communication | Wi-Fi, I²C and UART Serial |

## 2. External Libraries

The following libraries are used in the AEGIS firmware.

### 2.1 Blynk Library

**Header:** `BlynkSimpleEsp32_SSL.h`  
**Library:** Blynk

**Purpose:**
- Establishes a secure connection between the ESP32 and the Blynk IoT Cloud.
- Sends temperature and humidity readings to the dashboard.
- Updates motion detection events and room access counts.
- Receives operating profile commands from the Blynk application.
- Synchronises the selected operating profile after reconnection.

**Blynk Virtual Pin Configuration:**

| Virtual Pin | Function |
|---|---|
| V0 | Temperature |
| V1 | Humidity |
| V2 | Motion Detection Event |
| V3 | Operating Profile |
| V4 | FAMILY Mode Room Access Count |

**Installation:** Arduino IDE → Library Manager → Search for `Blynk` → Install.

### 2.2 U8g2 Graphics Library

**Header:** `U8g2lib.h`  
**Library:** U8g2

**Purpose:**
- Controls the SH1106 OLED display.
- Renders text and system information.
- Displays environmental readings and operating profile information.
- Provides the graphical interface for local system feedback.

**Display Configuration:**
- Controller: SH1106
- Resolution: 128 × 64 pixels
- Communication: I²C
- Buffer mode: Page buffer

**Installation:** Arduino IDE → Library Manager → Search for `U8g2` → Install.

### 2.3 DHT Sensor Library

**Header:** `DHT.h`  
**Library:** DHT sensor library by Adafruit

**Purpose:**
- Interfaces with the DHT11 temperature and humidity sensor.
- Retrieves temperature readings in degrees Celsius.
- Retrieves relative humidity readings in percentage.
- Provides sensor data for local display and Blynk transmission.

**Sensor Configuration:**
- Sensor: DHT11
- Data Pin: GPIO 4
- Reading Interval: 2 seconds
- Environmental data publishing interval: 60 seconds

**Installation:** Arduino IDE → Library Manager → Search for `DHT sensor library` by Adafruit → Install.

**Dependency:** Adafruit Unified Sensor library may be required by the installed DHT library version. Install it through the Arduino Library Manager if prompted or if compilation requires it.

## 3. ESP32 Built-in Libraries

The following libraries are included with the ESP32 Arduino board package. They do not need to be installed separately through the Library Manager.

### 3.1 WiFi.h

**Purpose:**
- Connects the ESP32 to a wireless network.
- Manages Wi-Fi connection status.
- Supports automatic reconnection and network communication.

### 3.2 Wire.h

**Purpose:**
- Provides I²C communication.
- Handles communication between the ESP32 and SH1106 OLED display.

**I²C Pin Configuration:**
- SDA: GPIO 21
- SCL: GPIO 22

### 3.3 Preferences.h

**Purpose:**
- Provides access to the ESP32 Non-Volatile Storage (NVS).
- Stores the FAMILY Mode room access counter.
- Retains the saved access count even after the ESP32 restarts or loses power.

### 3.4 BlynkSimpleEsp32_SSL.h

This header belongs to the external Blynk library listed earlier. It enables secure Blynk communication over the ESP32 platform.

## 4. Standard C/C++ Headers

These headers are part of the standard development environment and do not require separate installation.

| Header | Purpose |
|---|---|
| `math.h` | Mathematical functions |
| `string.h` | String and memory manipulation |
| `stdio.h` | Standard input/output utilities |

## 5. Computer Vision Integration

AEGIS also supports communication with an external computer-vision application through USB Serial.

The computer-vision component operates separately from the ESP32 firmware.

**Communication interface:** USB Serial / UART

**Communication protocol:**
- `HEARTBEAT` – Indicates that the computer-vision application is active.
- `VERIFY,<eventId>` – Requests verification of a motion event.
- `RESULT,<eventId>,PERSON_DETECTED` – Reports successful person detection.
- `RESULT,<eventId>,NO_PERSON` – Reports that no person was detected.
- `CAMERA_ERROR` – Reports a camera-side error.

The ESP32 monitors the computer-vision connection and uses PIR-based motion detection as a fallback when visual verification is unavailable or times out.

The computer-vision software and its Python dependencies are separate from the Arduino firmware dependencies.

## 6. Installation Procedure

1. Install the Arduino IDE.
2. Install the ESP32 board package by Espressif Systems through Boards Manager.
3. Select the appropriate ESP32 Dev Module board configuration for the ESP32 DevKit V1.
4. Open the AEGIS firmware file.
5. Open Library Manager and install:
   - Blynk
   - U8g2
   - DHT sensor library by Adafruit
   - Adafruit Unified Sensor, if required
6. Ensure the required ESP32 built-in libraries are available through the board package.
7. Configure the Wi-Fi and Blynk credentials securely.
8. Compile and upload the firmware to the ESP32.

## 7. Important Configuration Notes

- The firmware is configured for a DHT11 sensor.
- The OLED driver is configured for the SH1106 display controller.
- The Blynk firmware uses secure SSL communication.
- The ESP32 communicates with the external computer-vision application through Serial.
- The FAMILY Mode access counter uses ESP32 NVS for persistent storage.
- Wi-Fi credentials and Blynk authentication tokens must not be committed to a public repository.

## 8. Version Information

The exact library and ESP32 board-package versions used during the original development are not pinned in the firmware source.

For reproducibility, record the installed versions from Arduino IDE → Library Manager and Boards Manager before publishing a release.

---

**Project:** AEGIS – Ambient Environmental Guard and Intelligence System  
**Platform:** ESP32 DevKit V1  
**Repository:** AEGIS Smart Room Security System
