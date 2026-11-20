# AEGIS – System Overview

**AEGIS (Ambient Environmental Guard and Intelligence System)** is an ESP32-based IoT room monitoring and security system designed to combine environmental monitoring, motion detection, computer-vision-assisted verification, and remote security notifications.

The project explores how embedded systems, sensor integration, computer vision, and cloud-based IoT platforms can work together to create a context-aware room monitoring solution.

---

## 1. Project Objectives

The primary objective of AEGIS is to develop a room monitoring system that can adapt its behaviour according to the room's operating condition.

The system aims to:

- Monitor indoor temperature and humidity.
- Detect movement using a PIR motion sensor.
- Provide different monitoring behaviours through selectable operating profiles.
- Display system information locally using an OLED display.
- Transmit sensor readings and security events to a remote dashboard.
- Integrate an external computer-vision application for person-detection-based motion verification.
- Provide local audible and visual alerts during security events.
- Maintain selected system information across power cycles.

---

## 2. System Architecture

AEGIS is built around an ESP32 DevKit V1, which acts as the central controller.

The system consists of four primary subsystems:

### 2.1 Sensor Subsystem

The sensor subsystem collects information about the room environment and detects physical movement.

**DHT11 Temperature and Humidity Sensor**
- Measures ambient temperature.
- Measures relative humidity.
- Supplies environmental data to the ESP32.

**PIR Motion Sensor**
- Detects changes in infrared radiation associated with movement.
- Provides a digital output to the ESP32.
- Acts as the primary motion detection input.

### 2.2 Processing and Control Subsystem

The ESP32 processes incoming sensor data and manages the system's operating logic.

Its responsibilities include:

- Reading environmental sensor data.
- Monitoring PIR sensor output.
- Managing operating profiles.
- Communicating with the Blynk IoT platform.
- Updating the OLED display.
- Controlling the buzzer and indicator LEDs.
- Managing serial communication with the external computer-vision application.
- Maintaining the persistent FAMILY Mode access counter.

### 2.3 Communication Subsystem

AEGIS uses multiple communication interfaces:

| Interface | Purpose |
|---|---|
| Wi-Fi | Connects the ESP32 to the Blynk IoT Cloud |
| I²C | Communicates with the OLED display |
| USB Serial / UART | Exchanges messages with the computer-vision application |

### 2.4 Alert and Display Subsystem

The system provides both local and remote feedback.

- **OLED Display:** Displays environmental readings and system information.
- **Green LED:** Indicates the relevant operating state.
- **Red LED:** Indicates security-related status.
- **Active Buzzer:** Provides an audible alert during a security event.
- **Blynk Dashboard:** Displays sensor readings, activity information, and operating profile controls.

---

## 3. Operating Profiles

One of the defining features of AEGIS is its three operating profiles.

Each profile changes how the system responds to detected activity.

### 3.1 ME Mode

ME Mode is intended for personal room occupancy.

Its primary functions include:

- Monitoring room temperature and humidity.
- Displaying environmental readings on the OLED.
- Maintaining normal system operation without activating the security alarm for ordinary motion events.

### 3.2 FAMILY Mode

FAMILY Mode is designed to monitor room activity without triggering the audible security alarm for every detected movement.

Its functions include:

- Detecting movement through the PIR sensor.
- Recording motion events.
- Maintaining a room access/activity count.
- Displaying activity information through the Blynk dashboard.
- Preserving the access counter using the ESP32's Non-Volatile Storage (NVS).

### 3.3 VULNERABLE Mode

VULNERABLE Mode is intended for situations in which the room requires enhanced security monitoring.

Its functions include:

- Monitoring PIR-based motion events.
- Initiating an audible alarm for a qualifying security event.
- Activating visual security indicators.
- Sending breach-related information to the Blynk dashboard.
- Using computer-vision verification when the external application is available.

The alarm operates for a configured six-second interval.

---

## 4. Computer Vision Integration

AEGIS supports integration with an external Python-based computer-vision application.

The purpose of this integration is to introduce an additional verification stage after PIR-based motion detection.

The general process is:

1. The PIR sensor detects movement.
2. The ESP32 identifies the motion event.
3. The ESP32 sends a verification request to the external application through Serial communication.
4. The computer-vision application processes the camera input.
5. The application returns a result indicating whether a person was detected.
6. The ESP32 processes the response according to its security logic.

### Communication Commands

| Command | Description |
|---|---|
| `HEARTBEAT` | Indicates that the computer-vision application is active |
| `VERIFY,<eventId>` | Requests verification of a motion event |
| `RESULT,<eventId>,PERSON_DETECTED` | Reports successful person detection |
| `RESULT,<eventId>,NO_PERSON` | Reports that no person was detected |
| `CAMERA_ERROR` | Indicates a camera-side error |

### Fallback Mechanism

The firmware monitors communication with the external computer-vision application.

If the application becomes unavailable or verification times out, the ESP32 can fall back to PIR-only operation.

This allows the core monitoring and security functionality to continue without depending entirely on the external computer-vision subsystem.

**Note:** Computer vision operates as a separate application and is not executed directly on the ESP32.

---

## 5. IoT Integration – Blynk

AEGIS uses the Blynk IoT platform to provide remote monitoring and control.

The ESP32 communicates with the Blynk Cloud over Wi-Fi.

The dashboard provides:

- Temperature readings.
- Humidity readings.
- Motion event information.
- Operating profile selection.
- FAMILY Mode room activity count.
- Security-related breach notifications.

### Blynk Virtual Pin Mapping

| Virtual Pin | Function |
|---|---|
| V0 | Temperature |
| V1 | Humidity |
| V2 | Motion Event |
| V3 | Operating Profile |
| V4 | FAMILY Mode Access Count |

The dashboard provides a remote interface for interacting with the system without requiring direct physical access to the ESP32.

---

## 6. Hardware Configuration

The primary hardware components used in AEGIS are:

| Component | Function |
|---|---|
| ESP32 DevKit V1 | Main microcontroller and processing unit |
| DHT11 | Temperature and humidity monitoring |
| PIR Motion Sensor | Movement detection |
| SH1106 OLED Display | Local information display |
| BC547 NPN Transistor | Buzzer driver |
| 5V Active Buzzer | Audible security alert |
| Red LED | Security status indication |
| Green LED | Operating status indication |
| Resistors | Current limiting and transistor biasing |
| Breadboard and Jumper Wires | Circuit assembly and interconnection |

An external computer running the Python-based computer-vision application and a camera are used for the visual verification subsystem.

---

## 7. ESP32 GPIO Configuration

The following GPIO assignments are used by the firmware:

| ESP32 GPIO | Connected Component |
|---|---|
| GPIO 4 | DHT11 Data |
| GPIO 27 | PIR Sensor Output |
| GPIO 26 | Buzzer Driver |
| GPIO 18 | Red LED |
| GPIO 19 | Green LED |
| GPIO 21 | OLED SDA |
| GPIO 22 | OLED SCL |

These assignments correspond to the firmware configuration.

---

## 8. System Logic and Event Handling

AEGIS uses event-based logic to avoid repeatedly processing the same sustained motion detection signal.

The PIR sensor can remain HIGH for a period after movement stops. Therefore, the firmware uses a transition-based approach to identify a new motion event rather than treating every execution cycle with a HIGH signal as a separate event.

The system also uses timed operations for sensor reading, cloud communication, display updates, and alarm control.

### Important Timing Parameters

| Operation | Configured Interval |
|---|---:|
| PIR Sensor Warm-up | 60 seconds |
| DHT11 Reading | 2 seconds |
| Environmental Data Publishing | 60 seconds |
| Motion Event Handling | 2 seconds |
| Security Alarm Duration | 6 seconds |
| Buzzer Toggle Interval | 200 milliseconds |
| OLED Refresh | 250 milliseconds |
| Wi-Fi Reconnection Attempt | 10 seconds |
| Blynk Reconnection Attempt | 5 seconds |
| Computer-Vision Heartbeat Timeout | 8 seconds |
| Computer-Vision Verification Timeout | 7.5 seconds |

These are firmware configuration values, not guaranteed physical response times under every operating condition.

---

## 9. Data Persistence

AEGIS uses the ESP32 Preferences library to access Non-Volatile Storage (NVS).

The FAMILY Mode access counter is stored in NVS so that the recorded count can persist across ESP32 restarts and power interruptions.

This reduces the need to maintain the count only in volatile RAM.

---

## 10. Design Considerations

Several engineering considerations influenced the system architecture:

- **Modularity:** Environmental sensing, motion detection, cloud communication, and computer vision are treated as separate functional subsystems.
- **Event-Based Processing:** Motion events are handled using state transitions to reduce duplicate event processing.
- **Fault Tolerance:** PIR-only fallback allows core monitoring to continue if computer-vision verification becomes unavailable.
- **Local and Remote Feedback:** The OLED, LEDs, and buzzer provide local feedback, while Blynk provides remote visibility.
- **Data Persistence:** NVS retains the FAMILY Mode access counter across restarts.
- **Context-Aware Behaviour:** Operating profiles allow the system to respond differently to the same motion event depending on the selected room state.

---

## 11. Current Project Status

AEGIS has been developed and tested as a working prototype.

The completed implementation includes:

- ESP32-based sensor monitoring.
- Temperature and humidity readings.
- PIR-based motion detection.
- Three selectable operating profiles.
- OLED-based local display.
- Blynk IoT dashboard integration.
- Audible and visual security alerts.
- Persistent FAMILY Mode access counting.
- Serial communication support for external computer-vision integration.
- PIR-only fallback when computer-vision verification is unavailable.

AEGIS demonstrates the integration of embedded electronics, IoT communication, sensor-based automation, and external computer-vision processing into a unified room monitoring and security prototype.

---

## 12. Future Scope

Potential areas for further development include:

- Improving the reliability of person-detection verification.
- Adding additional environmental sensors.
- Introducing more advanced event logging and historical data analysis.
- Developing a dedicated mobile application interface.
- Exploring adaptive monitoring logic based on room activity patterns.
- Improving enclosure design, circuit integration, and deployment readiness.

These are proposed extensions and are not claimed as completed features.

---

## 13. Repository Documentation

For additional implementation details, refer to:

- `README.md` – Project introduction and repository guide.
- `firmware/` – ESP32 firmware source code.
- `firmware/libraries.md` – Software libraries and dependencies.
- `hardware/` – Circuit diagrams, wiring information, and component documentation.
- `images/` – Prototype photographs and dashboard screenshots.

---

**AEGIS – Ambient Environmental Guard and Intelligence System**

*Bringing environmental awareness, embedded intelligence, and room security together.*
