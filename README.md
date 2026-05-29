# ⚡ IoT Piezoelectric Footstep Energy Harvesting System

> **Mechanical energy → measurable electricity, logged in real time.**  
> 6 piezo discs harvest footstep vibrations, an Arduino measures and displays output voltage, step count, and energy estimates live on a 16×2 LCD. Serial CSV output feeds into Python/Excel for analysis.



---

## 📋 Table of Contents

- [Overview](#-overview)
- [System Architecture](#-system-architecture)
- [Hardware Requirements](#-hardware-requirements)
- [Circuit Design](#-circuit-design)
- [Software Setup](#-software-setup)
- [How to Run](#-how-to-run)
- [LCD Display Layout](#-lcd-display-layout)
- [Serial Data Logging](#-serial-data-logging)
- [Measured Results](#-measured-results)
- [Working Principle](#-working-principle)
- [Code Architecture](#-code-architecture)
- [Repository Structure](#-repository-structure)
- [Future Improvements](#-future-improvements)
- [Author & Course](#-author--course)
- [License](#-license)

---

## 🔍 Overview

This project demonstrates **ambient mechanical energy harvesting** from human footsteps using the piezoelectric effect. Six piezoelectric brass discs generate AC voltage when compressed underfoot. The signal is:

1. Rectified (AC → DC) via a 4-diode bridge rectifier
2. Smoothed by a 470 µF capacitor
3. Safely scaled by a voltage divider for Arduino's 0–5 V ADC
4. Measured, counted, and displayed on a 16×2 I2C LCD in real time

Energy harvesting from human motion underpins **self-powered IoT sensor nodes**, **smart flooring in transit hubs**, and **wearable electronics** — making this directly applicable to modern embedded system design and smart infrastructure.

---

## 🏗 System Architecture

```
 ┌───────────────────────────────────────────────────────────┐
 │                   PHYSICAL LAYER                          │
 │  Footstep Force → 6× Piezo Discs (parallel) → AC Signal  │
 └──────────────────────────┬────────────────────────────────┘
                            │ ~0–20 V AC
 ┌──────────────────────────▼────────────────────────────────┐
 │                SIGNAL CONDITIONING                         │
 │  Bridge Rectifier (4× 1N4007) → Pulsed DC                 │
 │  470 µF Capacitor             → Smoothed DC               │
 │  Voltage Divider (100kΩ/10kΩ) → 0–5 V scaled for ADC      │
 └──────────────────────────┬────────────────────────────────┘
                            │
                 A0 (voltage) │ A1 (raw step pulse)
 ┌──────────────────────────▼────────────────────────────────┐
 │               MICROCONTROLLER (Arduino Uno)                │
 │  10-bit ADC → 8-sample moving average → voltage compute   │
 │  Step detection with threshold + 300 ms debounce          │
 │  Energy estimation: E = ½ × C × V²                        │
 └────────────────┬──────────────────────┬───────────────────┘
                  │ I2C (A4/A5)           │ UART (USB)
 ┌────────────────▼────────┐  ┌──────────▼──────────────────┐
 │   16×2 I2C LCD Display  │  │  Serial CSV Logger (9600 b) │
 │  Steps · Voltage · Peak │  │  Steps,V,A0_Raw,Energy,Peak │
 └─────────────────────────┘  └─────────────────────────────┘
```

---

## 🛒 Hardware Requirements

| # | Component | Specification | Qty | Purpose |
|---|-----------|--------------|-----|---------|
| 1 | **Arduino Uno** | ATmega328P, 5 V, 16 MHz | 1 | Main MCU |
| 2 | **Piezoelectric disc** | 27 mm brass, ~20–50 V peak | 6 | Energy transducer |
| 3 | **Signal diode** | 1N4007 (1A, 1000V PIV) | 4 | Bridge rectifier |
| 4 | **Electrolytic capacitor** | 470 µF / 25 V | 1 | Voltage smoothing |
| 5 | **Resistor** | 100 kΩ, ¼W, 5% | 1 | Voltage divider R1 |
| 6 | **Resistor** | 10 kΩ, ¼W, 5% | 1 | Voltage divider R2 |
| 7 | **16×2 I2C LCD** | PCF8574 backpack, addr 0x27 | 1 | Real-time display |
| 8 | **18650 Li-ion battery** | 3.7 V, 2000+ mAh | 2 | Circuit power supply |
| 9 | **Battery holder** | 2× 18650 in series | 1 | Power rail |
| 10 | **Breadboard** | Full-size 830-point | 1 | Prototyping |
| 11 | **Jumper wires** | M-M, M-F assorted | ~20 | Connections |

**Total estimated BOM cost:** ₹350–500 (≈ $4–6 USD)

---

## ⚡ Circuit Design

### Schematic Overview

```
Piezo Discs  ──────────────────────────────────────────────────
  [P1] ─┐                                                      │
  [P2] ─┤  (all 6 in parallel,  +ve → D1 anode,               │
  [P3] ─┤   –ve → D4 cathode)   –ve → D3 anode,               │
  [P4] ─┤                                                      │
  [P5] ─┤                                                      │
  [P6] ─┘                                                      │
                                                               │ direct tap
        Bridge Rectifier (1N4007 × 4):                        │ for step detect
        AC+ ──► D1 ──► DC+ ──── 470µF cap (+) ──┬─────────────┘
                                                  │
                                         R1=100kΩ │
                                                  ├──► A0 (Arduino)
                                         R2=10kΩ  │
                                                  │
        AC– ──► D3 ──► DC– ──── 470µF cap (–) ──GND
                                                  │
                                                  └──► A1 (Arduino)

LCD I2C:
  VCC → Arduino 5V
  GND → Arduino GND
  SDA → Arduino A4
  SCL → Arduino A5
```

### Voltage Divider Calculation

```
         R1 + R2       100k + 10k
Ratio = ──────────── = ──────────── = 11.0
           R2               10k

V_actual = V_ADC × 11.0

Max measurable voltage = 5V × 11 = 55V  ✔ (safe for piezo peaks)
```

### Why 6 Discs in Parallel?

Parallel wiring keeps the terminal voltage the same as a single disc but **multiplies current** (and thus power) by 6. Energy per step scales with the number of discs without exceeding the 5 V ADC reference when divided.

---

## 💻 Software Setup

### 1. Install Arduino IDE

Download from [arduino.cc/en/software](https://www.arduino.cc/en/software) (v1.8.19+ or v2.x).

### 2. Install Required Library

In Arduino IDE:
```
Tools → Manage Libraries → Search "LiquidCrystal I2C"
→ Author: Frank de Brabander → Install v1.1.2+
```

### 3. Clone / Download This Repository

```bash
git clone https://github.com/Selverishanth/IoT-Piezoelectric-Energy-Harvesting.git
cd IoT-Piezoelectric-Energy-Harvesting
```

### 4. Adjustable Parameters

Open `src/piezo_energy_harvester.ino` and tune these `#define` values at the top:

| Parameter | Default | Range | Effect |
|-----------|---------|-------|--------|
| `STEP_THRESHOLD` | `80` | 50 – 200 | ADC level on A1 that counts as a step. Raise to filter phantom counts from vibration. |
| `DEBOUNCE_MS` | `300` | 200 – 500 ms | Dead-time after a step is registered. Prevents double-counting. |
| `AVG_SAMPLES` | `8` | 1 – 16 | ADC samples averaged for voltage. Higher = smoother but slightly slower. |
| `LCD_I2C_ADDR` | `0x27` | `0x27` / `0x3F` | **Change to `0x3F` if LCD stays blank after upload.** |
| `DIVIDER_RATIO` | `11.0` | depends on R | Update if you use different resistor values. |

---

## 🚀 How to Run

```
1. Wire components as per the schematic above.
2. Open src/piezo_energy_harvester.ino in Arduino IDE.
3. Select:  Tools → Board → Arduino Uno
            Tools → Port  → (your COM / /dev/ttyUSB port)
4. Click ✅ Verify first — check for any compile errors.
5. Click → Upload.
6. LCD shows splash screen → then live data within 3 seconds.
7. Step firmly on the piezo pad.
   LCD updates every 500 ms.
   Serial Monitor (9600 baud) logs CSV data every 1 second.
```

**LCD blank after upload?** Change `LCD_I2C_ADDR` from `0x27` to `0x3F` (line 37 in the sketch) and re-upload. This is the only other common I2C address for PCF8574 backpacks.

---

## 📺 LCD Display Layout

```
┌────────────────┐
│S:12    V: 3.45V│   ← Line 1: Step count (left), Voltage (right)
│E:  1.2uJ P:4.2V│   ← Line 2: Accumulated energy (left), Session peak V (right)
└────────────────┘
```

| Field | Meaning |
|-------|---------|
| `S:` | Cumulative footstep count since power-on |
| `V:` | Current rectified output voltage (Volts, 2 d.p.) |
| `E:` | Accumulated energy estimate since power-on (µJ) |
| `P:` | Session peak voltage seen so far |

---

## 📊 Serial Data Logging

Connect at **9600 baud** in Arduino Serial Monitor or any terminal.

**CSV header (printed once at boot):**
```
Steps,Voltage_V,A0_Raw,Energy_uJ,PeakV_Session
```

**Sample output:**
```
Steps,Voltage_V,A0_Raw,Energy_uJ,PeakV_Session
1,2.847,234,1.91,2.847
2,3.102,267,2.26,3.102
3,2.991,251,2.11,3.102
4,0.044,4,0.00,3.102
```

### Import into Python for graphing:
```python
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("session_log.csv")
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 6))

ax1.plot(df["Steps"], df["Voltage_V"], marker="o", color="steelblue")
ax1.set_ylabel("Voltage (V)"); ax1.set_title("Voltage per Step")

ax2.plot(df["Steps"], df["Energy_uJ"].cumsum(), color="orange")
ax2.set_xlabel("Steps"); ax2.set_ylabel("Cumulative Energy (µJ)")
ax2.set_title("Cumulative Energy Harvested")

plt.tight_layout(); plt.savefig("results.png", dpi=150)
plt.show()
```

---

## 📈 Measured Results

| Condition | Output Voltage | A0 Raw | Energy/Step (est.) |
|-----------|---------------|--------|--------------------|
| No pressure (idle) | 0.00 – 0.05 V | 0 – 8 | ~0 µJ |
| Light fingertip tap | 0.8 – 1.5 V | 70 – 140 | ~0.3 µJ |
| Normal footstep | 2.5 – 4.2 V | 210 – 380 | ~2.1 µJ |
| Hard stamp | 4.5 – 7.0 V | 400 – 650 | ~11 µJ |

> Each normal footstep across all 6 discs generates approximately **0.3 mJ** of electrical energy — enough to toggle a low-power sensor or transmit a short BLE packet.

---

## ⚙ Working Principle

```
1. TRANSDUCTION
   Piezoelectric crystal lattice deforms under footstep force.
   Charge separation → AC voltage (~5–20 V peak-to-peak).

2. RECTIFICATION
   4-diode full-wave bridge: both half-cycles contribute to DC.
   Output: pulsed positive DC.

3. SMOOTHING
   470 µF electrolytic capacitor charges from pulse peaks.
   Discharges slowly between pulses → quasi-stable DC voltage.

4. SCALING
   Voltage divider (100 kΩ / 10 kΩ) maps 0–20 V → 0–1.82 V
   (safe margin below Arduino's 5 V ADC reference).

5. MEASUREMENT
   Arduino 10-bit ADC: 8-sample average → voltage × 11.0 ratio.

6. STEP DETECTION
   Direct piezo wire on A1: threshold (80 ADC counts) crossing
   outside a 300 ms debounce window → increment step counter.

7. DISPLAY & LOGGING
   LCD refreshes every 500 ms.  Serial CSV every 1 second.
```

---

## 🗂 Code Architecture

```
src/piezo_energy_harvester.ino
│
├── CONFIGURATION SECTION (#define block)
│   └── All tunable constants in one place — no magic numbers
│
├── HELPER FUNCTIONS
│   ├── readVoltageADC()      — 8-sample moving average ADC read
│   ├── adcToVoltage()        — ADC count → real voltage (V)
│   ├── estimateEnergyUJ()    — E = ½CV² energy estimate
│   └── lcdPrintFloat()       — fixed-width float → LCD
│
├── setup()
│   ├── LCD splash screen (project + author)
│   └── Serial CSV header print
│
└── loop()
    ├── Step detection (threshold + debounce)
    ├── Voltage measurement (averaged ADC)
    ├── Energy estimation & session peak tracking
    ├── LCD refresh every 500 ms
    └── Serial CSV log every 1 s
```

---

## 📁 Repository Structure

```
IoT-Piezoelectric-Energy-Harvesting/
├── src/
│   └── piezo_energy_harvester.ino   # Main Arduino sketch (v2.0)
├── docs/
│   ├── architecture.md              # Detailed system architecture
│   ├── wiring_guide.md              # Step-by-step wiring instructions
│   └── calibration_guide.md         # Tuning thresholds & divider ratio
├── data/
│   └── sample_session_log.csv       # Example serial output for analysis
├── images/
│   ├── project_photo.jpg            # Physical build
│   ├── circuit_diagram.png          # Wiring schematic
│   └── lcd_screenshot.jpg           # Live LCD output
├── config/
│   └── .clang-format                # C++ formatting config
├── .gitignore
├── LICENSE                          # MIT License
├── CONTRIBUTING.md
└── README.md
```

---

## 🔮 Future Improvements

| Priority | Improvement | Complexity |
|----------|-------------|------------|
| 🔴 High | Replace Arduino Uno with **ESP32** for WiFi upload to cloud dashboard | Medium |
| 🔴 High | Log energy data to **Google Sheets** via ESP32 + Google Apps Script | Medium |
| 🟡 Medium | Add **MPPT circuit** to maximise power extraction efficiency | Hard |
| 🟡 Medium | Add **supercapacitor storage** to power a small load from harvested energy | Medium |
| 🟡 Medium | Implement **MQTT** protocol for real-time IoT dashboard (Node-RED / Grafana) | Medium |
| 🟠 Low | FFT analysis to characterise piezo disc frequency response | Hard |
| 🟠 Low | ML anomaly detection to identify surface materials from vibration signature | Hard |

---

## 👤 Author & Course

**Selverishanth**  
Department of Electronics and Instrumentation Engineering  
SRM Institute of Science and Technology (College of Engineering and Technology)  
Course: **21EIC101J — Sensors and Actuators**

---

## 📜 License

This project is open-source and available under the **MIT License**.  
See [LICENSE](LICENSE) for full terms.

---

## 🙏 Acknowledgements

- **Frank de Brabander** — LiquidCrystal_I2C library
- **Arduino community** — reference documentation and forums
- SRM IoT Lab — hardware and workspace

---

*If this project helped you, consider giving it a ⭐ on GitHub!*
