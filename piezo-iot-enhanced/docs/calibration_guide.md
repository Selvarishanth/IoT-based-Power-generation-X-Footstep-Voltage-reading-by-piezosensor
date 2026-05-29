# Calibration Guide

## 1. LCD I2C Address

If the LCD is blank after uploading the sketch:

1. Upload the I2C scanner sketch:
   `Arduino IDE → File → Examples → Wire → i2c_scanner`
2. Open Serial Monitor at 9600 baud.
3. Note the printed address (typically `0x27` or `0x3F`).
4. In `piezo_energy_harvester.ino`, change `LCD_I2C_ADDR` to match.

---

## 2. Step Detection Threshold

`STEP_THRESHOLD` default = **80** (ADC counts on A1, range 0–1023).

**Symptoms and fixes:**

| Symptom | Cause | Fix |
|---------|-------|-----|
| Steps counted when no one steps | Vibration or electrical noise | Increase threshold by 20–30 |
| Real steps not counted | Threshold too high | Decrease threshold by 20–30 |
| Count jumps by 2–3 per step | Debounce too short | Increase `DEBOUNCE_MS` to 400–500 |

**How to find your ideal threshold:**
1. Open Serial Monitor (9600 baud).
2. Temporarily add `Serial.print(rawA1)` at the top of `loop()`.
3. Observe A1 values during: (a) no activity, (b) light tap, (c) firm step.
4. Set `STEP_THRESHOLD` to roughly halfway between idle max and light-tap min.

---

## 3. Voltage Divider Calibration

If you use resistor values different from 100 kΩ / 10 kΩ, update `DIVIDER_RATIO`:

```
DIVIDER_RATIO = (R1 + R2) / R2
```

To verify calibration with a multimeter:
1. Apply a known voltage (e.g. 5V from a bench supply) to DC+.
2. Read `Voltage_V` in Serial Monitor.
3. If it shows 5.5V instead of 5.0V, adjust `DIVIDER_RATIO` proportionally.

---

## 4. Energy Estimation Accuracy

The formula `E = ½ × C × V²` estimates energy stored in the capacitor at a given voltage. It is an approximation because:
- The capacitor charges from each piezo pulse, not continuously.
- Effective capacitance may differ from the stamped value by ±20%.
- Diode forward-voltage drops (~0.7 V × 2 across the rectifier) are not corrected.

For a more accurate measurement, use an external power analyser (e.g. INA219 current sensor on the rectifier output).
