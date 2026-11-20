# AEGIS — Ambient Environmental Guard and Intelligence System

**An IoT-based intelligent room monitoring and security system designed for privacy-conscious room protection.**

AEGIS is an ESP32-based project developed to improve room security and environmental awareness, particularly in hostel rooms and shared accommodation. It combines motion detection, environmental sensing, local alarm activation, an OLED interface, and remote monitoring through the Blynk IoT platform.

## 1. Project Overview

Students frequently leave their rooms unattended while attending classes, visiting libraries, or travelling. AEGIS addresses this situation through configurable security profiles that adapt the system's response to the selected room-occupancy state.

The system provides:
- PIR-based motion detection.
- Temperature and humidity monitoring.
- Three configurable operating profiles.
- Local buzzer-based security alerts.
- OLED-based status and environmental information.
- Remote monitoring through the Blynk IoT dashboard.
- Persistent room-access event counting in FAMILY mode.
- Serial communication interface for external computer-vision verification.

## 2. Operating Profiles

| Profile | Function |
|---|---|
| ME | Displays room temperature and humidity while monitoring the room. |
| FAMILY | Monitors motion events and maintains a room-access counter without activating the security alarm. |
| VULNERABLE | Treats detected motion as a potential security breach and activates the alarm response. |

In VULNERABLE mode, the system uses a six-second local alarm response. When the external computer-vision service is available, it can request person verification. If the service is unavailable or verification times out, the firmware supports a PIR-only fallback response.

## 3. System Architecture

The ESP32 acts as the central controller, coordinating sensor readings, operating profiles, alarm logic, display updates, and cloud communication.

**Main components:**
- ESP32 DevKit V1
- PIR motion sensor (HC-SR501)
- DHT11 temperature and humidity sensor
- SH1106 128×64 OLED display
- BC547 NPN transistor-based buzzer driver
- 5V active buzzer
- Red and green status LEDs
- Blynk IoT platform

## 4. Hardware Connections

The current firmware defines the following GPIO assignments:

| Component | ESP32 GPIO |
|---|---:|
| DHT11 Data | GPIO 4 |
| PIR Output | GPIO 27 |
| Buzzer Driver | GPIO 26 |
| Red LED | GPIO 18 |
| Green LED | GPIO 19 |
| I²C SDA | GPIO 21 |
| I²C SCL | GPIO 22 |

Refer to the hardware directory for the circuit and wiring diagrams.

## 5. Software and Technologies

- Arduino IDE
- ESP32 Arduino Core
- C/C++
- Blynk IoT
- U8g2 graphics library
- DHT sensor library
- ESP32 Preferences / Non-Volatile Storage
- Serial communication for external computer-vision integration

## 6. Blynk Dashboard

The Blynk dashboard provides remote access to:
- Temperature readings
- Humidity readings
- Motion-event status
- Operating-profile selection
- Room-access count

## 7. Project Gallery

Photographs of the physical prototype, OLED interface, operating profiles, breach alert, and Blynk dashboard are available in the `images/` directory.

## 8. Repository Structure

```text
AEGIS-Smart-Room-Security-System/
├── firmware/
├── hardware/
├── images/
├── documentation/
├── README.md
├── LICENSE
└── .gitignore
```

## 9. Current Development Status

AEGIS has been developed and tested as a functional hardware prototype. Its core motion-detection, environmental-monitoring, OLED-display, operating-profile, buzzer-alarm, and Blynk communication functions have been demonstrated.

## 10. Future Scope

- Improved multi-room deployment.
- More advanced occupancy verification.
- Enhanced user authentication and access management.
- Improved enclosure and PCB integration.
- Further optimisation of power consumption.

## 11. Author

**Kartikey Bhardwaj**

Student Developer | AEGIS Project

---

*AEGIS — Ambient Environmental Guard and Intelligence System.*
