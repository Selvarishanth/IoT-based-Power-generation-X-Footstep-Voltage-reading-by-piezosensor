 IoT Piezoelectric Energy Harvesting

Footstep-based energy harvesting using 6 piezo discs,
Arduino Uno, and 16x2 LCD display.

## Hardware
- 6x Piezoelectric discs (parallel)
- Arduino Uno
- 16x2 I2C LCD display
- 2x 18650 Li-ion batteries
- Bridge rectifier + 470µF capacitor

## What it shows on LCD
- Steps counted
- Voltage generated (V)
- Raw A0 signal level

## Code
[text](../../../../../../OneDrive/Documents/Arduino/piezo_final.ino/piezo_final.ino.ino)

## Circuit
<img width="1080" height="1107" alt="Block diagram" src="https://github.com/user-attachments/assets/6c667755-604f-4379-8c42-45296d79bd84" />


## Project Photo
<img width="1080" height="983" alt="Project Image" src="https://github.com/user-attachments/assets/63c1e093-4b9d-4413-a43c-7898d82069f0" />


## Certficate Image
<img width="1080" height="836" alt="Certfiacation" src="https://github.com/user-attachments/assets/5ffe2dfe-3231-45cf-91be-7c06a01de190" />


## Course
Sensors and Actuators — 21EIC101J, SRM Institute
## Overview

This project demonstrates harvesting ambient mechanical energy from human footsteps using **6 piezoelectric discs** wired in parallel. The generated AC signal is rectified, smoothed, and measured by an Arduino Uno. A 16×2 I2C LCD displays live step count, output voltage, and raw ADC signal level in real time.

Energy harvesting from human motion is a growing area in **wearable electronics**, **self-powered IoT nodes**, and **industrial sensing** — making this directly applicable to smart infrastructure and low-power embedded system design.

---

## Demo

| LCD Display | Physical Setup |
|---|---|
| `S:12   V:3.45V` | 6 piezo discs mounted on footstep pad |
| `A0 Raw: 287   ` | Bridge rectifier + capacitor smoothing |

> **Live LCD Output:**
> ```
> S:8         V:2.87V
> A0 Raw:241
> ```

---

## Hardware Components

| Component | Specification | Purpose |
|---|---|---|
| Arduino Uno | ATmega328P, 5V | Main microcontroller |
| Piezoelectric discs | 27mm brass, 6× parallel | Mechanical-to-electrical transducer |
| Bridge rectifier | 4× 1N4007 diodes | AC → DC conversion |
| Smoothing capacitor | 470µF / 25V electrolytic | Voltage ripple reduction |
| 16×2 LCD (I2C) | Address 0x27, 5V | Real-time data display |
| Voltage divider | R1=100kΩ, R2=10kΩ | Scale 0–20V → 0–5V for ADC |
| 18650 Li-ion batteries | 2× 3.7V in series | Power supply for circuit |
| Breadboard + wires | — | Prototyping |

---

## Circuit Design

### Wiring Overview

```
Piezo Discs (6× parallel)
    │
    ├──► Bridge Rectifier (4× 1N4007)
    │         │
    │    DC+ ─┤─── 470µF capacitor ──── Voltage Divider ──► A0 (Arduino)
    │    DC- ─┴─── GND
    │
    └──► One direct wire ──────────────────────────────────► A1 (step detection)

LCD I2C:
    VCC → 5V  |  GND → GND  |  SDA → A4  |  SCL → A5
```

### Voltage Divider Calculation

```
R1 = 100kΩ,  R2 = 10kΩ
Ratio = (R1 + R2) / R2 = 11
Vactual = Vadc × 11
Max measurable = 5V × 11 = 55V  (safe for piezo output)
```

---

## LCD Display Output

The LCD cycles through real-time readings every 500ms:

```
Line 1:  S:<steps>     V:<voltage>V
Line 2:  A0 Raw:<adc_value>
```

| Field | Description |
|---|---|
| `S:` | Cumulative footstep count since power-on |
| `V:` | Actual rectified output voltage in volts |
| `A0 Raw:` | Direct ADC reading (0–1023) — signal/noise level |

---

## Software

### Dependencies

Install via **Arduino IDE → Tools → Manage Libraries:**

```
LiquidCrystal_I2C   by Frank de Brabander   v1.1.2+
```

### Key Parameters (tunable)

```cpp
#define STEP_THRESHOLD   80    // A1 ADC value to register a step (range: 50–200)
#define DEBOUNCE_MS     300    // Minimum ms between valid steps
#define DIVIDER_RATIO  11.0    // Voltage divider scaling factor
```

### Serial Monitor Output

Connect at **9600 baud** to see live CSV data:

```
Steps, Voltage(V), A0_Raw
1, 2.847V, A0=234
2, 3.102V, A0=267
3, 2.991V, A0=251
```

---

## Repository Structure

```
iot-piezoelectric-energy-harvesting/
├── code/
│   └── piezo_final.ino          # Main Arduino sketch
├── images/
│   ├── project_photo.jpg        # Physical build photo
│   └── circuit_diagram.png      # Wiring diagram
└── README.md
```

---

## How to Run

1. **Clone or download** this repository
2. Open `code/piezo_final.ino` in **Arduino IDE**
3. Install `LiquidCrystal_I2C` library (Manage Libraries)
4. Wire components as per the circuit diagram above
5. Select **Board: Arduino Uno** and correct COM port
6. Click **Upload**
7. Step on the piezo pad — LCD updates in real time

> **LCD blank?** Change `0x27` to `0x3F` on line 13 of the sketch and re-upload.

---

## Results

| Condition | Output Voltage | A0 Raw Value |
|---|---|---|
| No pressure (idle) | 0.00 – 0.05V | 0 – 8 |
| Light tap | 0.8 – 1.5V | 70 – 140 |
| Normal footstep | 2.5 – 4.2V | 210 – 380 |
| Hard stamp | 4.5 – 7.0V | 400 – 650 |

Each footstep generates approximately **0.3 mJ** of electrical energy across 6 parallel discs.

---

## Working Principle

1. **Mechanical stress** applied to piezo disc generates AC voltage via the piezoelectric effect
2. **Bridge rectifier** converts AC to pulsed DC
3. **Capacitor** smooths the pulsed DC into stable DC
4. **Voltage divider** scales the output safely for Arduino ADC (0–5V range)
5. **Arduino** reads voltage on A0, detects step pulses on A1, and drives LCD display

---

## Applications

- Self-powered wireless sensor nodes in smart buildings
- Footstep energy harvesting in railway platforms and malls
- Wearable energy harvesting for low-power IoT devices
- Vibration energy recovery in industrial machinery

---

## Future Improvements

- [ ] Replace Arduino Uno with **ESP32** for WiFi data upload to cloud dashboard
- [ ] Add **MPPT circuit** to maximise power extraction efficiency
- [ ] Log energy data to **Google Sheets** via ESP32 API
- [ ] Add **supercapacitor storage** to power a small load from harvested energy
- [ ] Implement **FFT analysis** to characterise frequency response of piezo discs

---

## Author

**Selverishanth**
Department of Electronics and Instrumentation Engineering
SRM Institute of Science and Technology (College of Engineering and Technology)
Course: **21EIC101J — Sensors and Actuators**

---

## License

This project is open-source and available under the MIT License
