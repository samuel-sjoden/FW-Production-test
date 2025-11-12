# Capacitor Kicker Test Script

### Overview
This firmware is designed for testing and validating the **kicker and chipper capacitor circuits** on an embedded system (ESP32).  
It provides a simple serial interface to **charge capacitors**, **fire kick or chip pulses**, and monitor system feedback.  

### Features

- **Capacitor Charging Control**
  - Charges high-voltage capacitors for kick/chip actuators.
  - Automatically detects when charging is complete using a digital interrupt on the `DONE` pin.
  - Implements timeout protection to prevent indefinite charging.

- **Kick/Chip Pulse Generation**
  - Sends a high-voltage discharge pulse to either the **kick** or **chip** solenoid.
  - Pulse duration is configurable via the serial interface (1–10,000 μs).
  - Uses ESP32 hardware timers for precise microsecond-level timing.
  - Automatically disables pulse output after the target duration.

- **Serial Command Interface**
  - `charge` — begin charging the capacitors  
  - `kick` — fire the kick solenoid (after charging)  
  - `chip` — fire the chip solenoid (after charging)  

---

### System Behavior

| Event | Description |
|-------|--------------|
| `charge` command | Begins charging by setting `PIN_CHARGE` high and enabling the DONE pin interrupt. |
| `DONE` pin interrupt | Fires when capacitors are charged; stops charging automatically. |
| `kick` / `chip` command | Prompts user for pulse width (μs) and triggers solenoid pulse. |
| Timeout | Automatically disables charging if it exceeds `TIMEOUT` (5 seconds). |

---

### Hardware Pin Assignments

| Signal | ESP32 Pin | Description |
|---------|------------|-------------|
| CHARGE | 26 | Enables capacitor charging |
| DONE | 25 | Indicates charge completion (active low) |
| CHIP | 32 | Chipper solenoid control |
| KICK | 33 | Kicker solenoid control |

---

### Configuration Constants

| Constant | Default | Description |
|-----------|----------|-------------|
| `MAX_PULSE_WIDTH` | 10,000 µs | Maximum pulse duration for kick/chip |
| `BAUD_RATE` | 115,200 | Serial communication speed |
| `TIMEOUT` | 5,000 ms | Charge timeout |
| `KICK_CHIP_DELAY` | 5,000 µs | Delay before firing after input |
