# System Architecture — Piezoelectric Energy Harvesting

## Signal Flow

```
Physical Force
    │
    ▼
┌─────────────────────────────────┐
│   ENERGY TRANSDUCTION LAYER     │
│                                 │
│  6× Piezoelectric brass discs   │  Each disc: 27mm diameter
│  Wired in parallel              │  Generates ~5–20V AC per step
│  Mounted under footstep pad     │
└──────────────┬──────────────────┘
               │ AC voltage (polarity alternates each half-cycle)
               ▼
┌─────────────────────────────────┐
│   SIGNAL CONDITIONING LAYER     │
│                                 │
│  1. Bridge Rectifier (4×1N4007) │  AC → pulsed positive DC
│  2. 470µF Capacitor             │  Ripple smoothing
│  3. Voltage Divider (100k/10k)  │  0–20V mapped to 0–1.82V ADC-safe
└──────────────┬──────────────────┘
               │ Two signals:
               │  A0 — rectified & divided voltage
               │  A1 — direct piezo tap (raw pulse for step detection)
               ▼
┌─────────────────────────────────┐
│   MEASUREMENT LAYER (MCU)       │
│                                 │
│  Arduino Uno — ATmega328P       │
│  • 10-bit ADC on A0, A1         │
│  • 8-sample moving average      │
│  • Threshold + debounce logic   │
│  • Energy calc: E = ½CV²        │
└──────┬───────────────┬──────────┘
       │ I2C           │ UART/USB
       ▼               ▼
┌────────────┐  ┌───────────────────┐
│ 16×2 LCD   │  │  Serial CSV Log   │
│ (PCF8574)  │  │  9600 baud        │
│ Steps/V/E  │  │  Steps,V,A0,E,Pk  │
└────────────┘  └───────────────────┘
```

## Power Budget

| State | Current Draw | Notes |
|-------|-------------|-------|
| Arduino Uno active | ~50 mA | All peripherals active |
| LCD backlight | ~20 mA | Via PCF8574 I2C expander |
| Piezo harvesting (peak) | source | ~0.3 mJ per step — supplemental only |
| **Total draw from batteries** | ~70 mA | 2× 18650 in series = 7.4V regulated |

The 18650 batteries power the MCU and LCD. The piezo circuit is a separate harvesting branch — the demo measures the harvested energy rather than using it to power the system.

## ADC Timing

```
loop() runs ~every 1–2 ms (no blocking delays)
│
├─ A1 read:      ~0.1 ms  (single sample, step detection)
├─ A0 read:      ~1.8 ms  (8 samples × ~225 µs each)
├─ LCD refresh:  only every 500 ms (guarded by millis())
└─ Serial print: only every 1000 ms (guarded by millis())
```

## I2C LCD Addressing

PCF8574-based LCD backpacks ship with one of two I2C addresses depending on the hardware revision:

| Address | Jumper state |
|---------|-------------|
| `0x27`  | A0=A1=A2=1 (default, most common) |
| `0x3F`  | A0=A1=A2=0 |

Run the I2C scanner sketch (Arduino Examples → Wire → i2c_scanner) to discover your LCD's address if neither works.
