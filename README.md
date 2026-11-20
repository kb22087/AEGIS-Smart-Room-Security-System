# AEGIS – Ambient Environmental Guard and Intelligence System

### Context-Aware IoT-Based Intelligent Room Monitoring and Security System

<p align="center">
  <img src="images/PROJECT%20OVERALL%20PHOTO.jpeg" width="650" alt="AEGIS complete prototype">
</p>

<p align="center">
  <strong>Environmental Monitoring | Motion Detection | Computer Vision | IoT Automation</strong>
</p>

---

## 1. Introduction

In Ancient Greek mythology, the word *Aegis* represented a symbol of protection and divine defence.

Inspired by this idea, **AEGIS (Ambient Environmental Guard and Intelligence System)** is an ESP32-based room monitoring and security system developed to explore the integration of embedded systems, sensor technology, computer vision, and Internet of Things (IoT) automation.

The system combines environmental monitoring, PIR-based motion detection, selectable operating profiles, remote monitoring through Blynk, and external computer-vision-assisted verification.

The primary objective is to develop a room monitoring system that can adapt its behaviour according to different operating conditions rather than relying solely on conventional motion detection.

---

## 2. Project Features

### 🌡️ Environmental Monitoring
- Monitors room temperature and relative humidity using a DHT11 sensor.
- Displays environmental readings on a local OLED display.
- Sends sensor data to the Blynk IoT dashboard.

### 🚶 Motion Detection
- Uses a PIR sensor to detect movement.
- Employs transition-based event handling to avoid repeatedly processing sustained motion signals.
- Records motion activity according to the selected operating profile.

### 👁️ Computer Vision Integration
- Supports an external Python-based computer-vision application.
- Communicates with the ESP32 through USB Serial.
- Uses person-detection results as an additional verification stage for motion events.
- Supports PIR-only fallback when computer-vision verification is unavailable.

### 📱 IoT-Based Remote Monitoring
- Uses the Blynk IoT platform.
- Displays temperature and humidity readings.
- Provides operating profile selection.
- Displays motion activity and room access information.
- Provides security-related breach notifications.

### 🚨 Local Security Alerts
- Uses a 5V active buzzer for audible alerts.
- Uses red and green LEDs for visual status indication.
- Implements a configured six-second alarm interval in VULNERABLE Mode.

### 💾 Persistent Data Storage
- Uses ESP32 Non-Volatile Storage (NVS).
- Preserves the FAMILY Mode room access counter across restarts.

---

## 3. Operating Profiles

AEGIS incorporates three operating profiles, each designed for a different room condition.

| Profile | Purpose | Main Behaviour |
|---|---|---|
| ME Mode | Personal room monitoring | Environmental monitoring and local display |
| FAMILY Mode | Room activity monitoring | Motion event recording and persistent access counting |
| VULNERABLE Mode | Enhanced security monitoring | Security alerts, visual indicators and computer-vision-assisted verification |

### ME Mode

<p align="center">
  <img src="images/ME%20MODE.jpeg" width="500" alt="AEGIS ME Mode">
</p>

Designed for personal room occupancy, ME Mode focuses on environmental monitoring and displaying room conditions without activating the security alarm for ordinary motion events.

### FAMILY Mode

<p align="center">
  <img src="images/FAMILY%20MODE.jpeg" width="500" alt="AEGIS FAMILY Mode">
</p>

FAMILY Mode monitors room activity without triggering the audible alarm for every movement. It maintains an activity/access count and preserves the count using ESP32 NVS.

### VULNERABLE Mode

<p align="center">
  <img src="images/VULNERABLE%20MODE.jpeg" width="500" alt="AEGIS VULNERABLE Mode">
</p>

Designed for enhanced security monitoring, VULNERABLE Mode responds to qualifying motion events with audible and visual alerts and reports security-related events through the IoT dashboard.

---

## 4. System Architecture

The ESP32 DevKit V1 acts as the central processing and control unit.

The architecture consists of four primary subsystems:

1. **Sensor Subsystem:** Collects environmental data and detects movement.
2. **Processing Subsystem:** Processes sensor inputs, manages operating profiles and executes event-handling logic.
3. **Communication Subsystem:** Connects the ESP32 to Blynk through Wi-Fi and to the external computer-vision application through Serial communication.
4. **Alert and Display Subsystem:** Provides local feedback through the OLED, LEDs and buzzer.

### Hardware Configuration

| Component | Function |
|---|---|
| ESP32 DevKit V1 | Main microcontroller |
| DHT11 | Temperature and humidity monitoring |
| PIR Motion Sensor | Movement detection |
| SH1106 OLED | Local information display |
| BC547 NPN Transistor | Buzzer driver |
| 5V Active Buzzer | Audible security alert |
| Red LED | Security status indication |
| Green LED | Operating status indication |
| Resistors | Current limiting and transistor biasing |

An external computer and camera are used for the computer-vision subsystem.

### GPIO Configuration

| ESP32 GPIO | Component |
|---|---|
| GPIO 4 | DHT11 Data |
| GPIO 27 | PIR Sensor Output |
| GPIO 26 | Buzzer Driver |
| GPIO 18 | Red LED |
| GPIO 19 | Green LED |
| GPIO 21 | OLED SDA |
| GPIO 22 | OLED SCL |

---

## 5. Circuit and Hardware

### Circuit Diagram

<p align="center">
  <img src="hardware/Circuit%20Diagram.png" width="750" alt="AEGIS circuit diagram">
</p>

### Wiring Diagram

<p align="center">
  <img src="hardware/Circuit%20CHART.png" width="750" alt="AEGIS wiring diagram">
</p>

The hardware is assembled around the ESP32, with the sensors, OLED display, transistor-driven buzzer and status LEDs connected through the designated GPIO pins.

---

## 6. IoT Dashboard

AEGIS uses the Blynk IoT platform for remote monitoring and operating profile selection.

<p align="center">
  <img src="images/BLYNK%20DASHBOARD.png" width="750" alt="AEGIS Blynk dashboard">
</p>

### Blynk Virtual Pin Configuration

| Virtual Pin | Function |
|---|---|
| V0 | Temperature |
| V1 | Humidity |
| V2 | Motion Event |
| V3 | Operating Profile |
| V4 | FAMILY Mode Access Count |

### Breach Notification

<p align="center">
  <img src="images/BREACH%20ALERT.jpeg" width="500" alt="AEGIS breach alert">
</p>

The dashboard provides visibility into security-related events, allowing the user to monitor the system remotely.

---

## 7. Computer Vision Communication

AEGIS supports integration with an external Python-based computer-vision application.

The ESP32 communicates with this application through USB Serial using a simple command-response protocol.

| Serial Command | Description |
|---|---|
| `HEARTBEAT` | Indicates application availability |
| `VERIFY,<eventId>` | Requests motion event verification |
| `RESULT,<eventId>,PERSON_DETECTED` | Reports person detection |
| `RESULT,<eventId>,NO_PERSON` | Reports no person detected |
| `CAMERA_ERROR` | Reports a camera-side error |

If the computer-vision application becomes unavailable or verification times out, the ESP32 can fall back to PIR-only operation.

**Note:** Computer vision runs externally on a computer; it is not executed directly on the ESP32.

---

## 8. Software and Technologies

| Technology | Application |
|---|---|
| Arduino IDE | Firmware development |
| C++ | ESP32 programming |
| ESP32 Arduino Core | Microcontroller platform |
| Blynk IoT | Remote monitoring and control |
| U8g2 | OLED display interface |
| DHT Sensor Library | Environmental sensor interface |
| Wi-Fi | Wireless cloud communication |
| I²C | OLED communication |
| UART / USB Serial | Computer-vision communication |
| Python | External computer-vision application |
| OpenCV | Image processing and visual verification |

For the complete library and dependency information, refer to [`firmware/libraries.md`](firmware/libraries.md).

---

## 9. Repository Structure

```text
AEGIS-Smart-Room-Security-System/
│
├── README.md
│
├── documentation/
│   └── system-overview.md
│
├── firmware/
│   ├── AEGIS_Main.ino
│   └── libraries.md
│
├── hardware/
│   ├── Circuit Diagram.png
│   └── Circuit CHART.png
│
└── images/
    ├── .gitkeep
    ├── BLYNK DASHBOARD.png
    ├── BREACH ALERT.jpeg
    ├── FAMILY MODE.jpeg
    ├── ME MODE.jpeg
    ├── PROJECT OVERALL PHOTO.jpeg
    └── VULNERABLE MODE.jpeg
```

---

## 10. Setup and Installation

### Requirements

- Arduino IDE
- ESP32 DevKit V1
- ESP32 board package by Espressif Systems
- Blynk account and configured IoT template
- Required Arduino libraries
- External computer with Python and OpenCV for computer-vision integration
- Compatible camera for visual verification

### Firmware Setup

1. Clone or download this repository.
2. Open `firmware/AEGIS_Main.ino` in Arduino IDE.
3. Install the required libraries listed in [`firmware/libraries.md`](firmware/libraries.md).
4. Configure your Wi-Fi and Blynk credentials securely.
5. Select the appropriate ESP32 board and COM port.
6. Compile and upload the firmware.
7. Configure the Blynk dashboard using the documented virtual pin mapping.

**Security:** Do not publish Wi-Fi passwords, Blynk authentication tokens or other private credentials. Keep local credential files excluded through `.gitignore`.

---

## 11. Current Development Status

<details>
<summary><strong>Completed implementation</strong></summary>

- ESP32-based environmental monitoring.
- PIR-based motion detection.
- Three selectable operating profiles.
- OLED-based local display.
- Blynk IoT integration.
- Local audible and visual alerts.
- Persistent FAMILY Mode access counting.
- Serial communication support for external computer-vision integration.
- PIR-only fallback mechanism.

</details>

AEGIS has been developed and tested as a functional prototype integrating embedded electronics, sensor monitoring, IoT communication and security-oriented automation.

---

## 12. Future Scope

Potential areas of further development include:

- Improving computer-vision verification reliability.
- Adding additional environmental sensors.
- Implementing more advanced event logging and historical analysis.
- Exploring adaptive monitoring based on room activity.
- Developing a dedicated mobile application interface.
- Improving enclosure design and hardware integration.

These are proposed extensions rather than completed features.

---

## 13. Acknowledgements

AEGIS was developed as an engineering project exploring the practical integration of embedded systems, IoT, sensor technology and computer vision.

The project reflects an interest in building systems that move beyond isolated sensors towards context-aware automation.

---

## Author

**Kartikey Bhardwaj**  
Undergraduate Engineering Student  
KIET

---

<p align="center">
  <strong>AEGIS – Protection, Reimagined.</strong>
</p>

<p align="center">
  <em>Where environmental awareness meets intelligent room security.</em>
</p>

---

**Repository:** [AEGIS-Smart-Room-Security-System](https://github.com/kb22087/AEGIS-Smart-Room-Security-System)
