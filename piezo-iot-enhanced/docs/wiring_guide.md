# Wiring Guide — Piezoelectric Energy Harvesting

## Overview

This guide walks through connecting every component step-by-step. Complete each section before moving to the next.

---

## Step 1 — Prepare the Piezo Discs

1. Take 6 piezoelectric brass discs (27 mm).
2. Identify the **positive lead** (usually the top ceramic face, silver wire) and **negative lead** (brass backing, red or bare wire).
3. Twist all 6 **positive leads together** → one junction (+P).
4. Twist all 6 **negative leads together** → one junction (–P).

> **Parallel wiring** keeps voltage the same as a single disc but 6× the current output.

---

## Step 2 — Build the Bridge Rectifier

Using 4× 1N4007 diodes on the breadboard:

```
          D1 (→ anode to AC+, cathode to DC+)
+P ──────┤►├──────────────────────── DC+ rail
          D2 (→ anode to DC–, cathode to AC+)  ← (actually anode to AC–)

Let me draw it more clearly:

       D1         D3
+P ──►|──┬──────┬──|◄── –P
         │      │
       DC+    DC–
         │      │
–P ──►|──┴──────┴──|◄── +P
       D2         D4
```

Simpler breadboard layout:

| Diode | Anode connects to | Cathode connects to |
|-------|-------------------|---------------------|
| D1 | +P (piezo positive) | DC+ |
| D2 | –P (piezo negative) | DC+ |
| D3 | GND (DC–) | +P (piezo positive) |
| D4 | GND (DC–) | –P (piezo negative) |

After this step you should see positive DC voltage on DC+ whenever a piezo disc is pressed.

---

## Step 3 — Smoothing Capacitor

1. Connect a 470 µF / 25 V electrolytic capacitor between **DC+** and **GND**.
2. **Polarity matters!** The capacitor **positive leg (longer)** goes to DC+. Negative leg to GND.

---

## Step 4 — Voltage Divider

The piezo can generate up to ~20 V peak; Arduino ADC maximum is 5 V.

```
DC+ ──── R1 (100 kΩ) ──┬──── R2 (10 kΩ) ──── GND
                        │
                     A0 pin (Arduino)
```

1. Place R1 (100 kΩ) between DC+ and a mid-point node.
2. Place R2 (10 kΩ) between that mid-point node and GND.
3. Connect mid-point node to Arduino **A0**.

---

## Step 5 — Step Detection Tap

1. Take one **direct** wire from any single piezo disc's positive lead.
2. Connect it to Arduino **A1**.

> This pin detects the sharp voltage spike of a step (used for counting) while A0 reads the smoother rectified voltage.

---

## Step 6 — LCD I2C

| LCD Backpack Pin | Arduino Pin |
|-----------------|-------------|
| VCC | 5V |
| GND | GND |
| SDA | A4 |
| SCL | A5 |

---

## Step 7 — Power Supply

1. Connect 2× 18650 batteries in series (positive of B1 → negative of B2).
2. Total = ~7.4 V. Connect to Arduino **VIN** and **GND**.

---

## Completed Wiring Checklist

- [ ] 6 piezo discs in parallel (+P and –P junctions)
- [ ] Bridge rectifier (4× 1N4007) with DC+ and DC– rails
- [ ] 470 µF cap across DC+ and GND (correct polarity)
- [ ] Voltage divider (100 kΩ + 10 kΩ) — mid-point → A0
- [ ] Direct piezo tap → A1
- [ ] LCD VCC→5V, GND→GND, SDA→A4, SCL→A5
- [ ] Battery pack → Arduino VIN + GND

---

## Safety Notes

- Piezo discs can generate transient voltages up to 50–100 V under hard impact. The 1N4007 diodes (1000 V PIV) and the voltage divider safely handle this.
- Do not connect piezo output directly to Arduino pins without the voltage divider — this will damage the MCU.
- Use 25 V-rated capacitor (minimum) to handle rectified piezo peaks safely.
