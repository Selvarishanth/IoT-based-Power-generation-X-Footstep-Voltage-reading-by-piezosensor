/*
 * ============================================================
 *  IoT Piezoelectric Footstep Energy Harvesting System
 *  Version: 2.0.0
 *  Author:  Selverishanth
 *  Course:  21EIC101J — Sensors and Actuators
 *           SRM Institute of Science and Technology
 *
 *  Description:
 *    Harvests ambient mechanical energy from human footsteps
 *    using 6 piezoelectric discs wired in parallel. The AC
 *    signal is bridge-rectified, smoothed by a capacitor, and
 *    read by Arduino's ADC. A 16×2 I2C LCD displays live step
 *    count, output voltage, and raw ADC signal in real time.
 *    Serial output provides CSV data for PC logging/analysis.
 *
 *  Hardware Connections:
 *    Piezo discs (6× parallel) → Bridge rectifier (1N4007×4)
 *      → 470µF capacitor → Voltage divider (R1=100kΩ, R2=10kΩ) → A0
 *    Piezo direct tap (1 disc)  → A1  (step-pulse detection)
 *    LCD I2C:  VCC=5V  GND=GND  SDA=A4  SCL=A5
 *
 *  Required Libraries (Arduino IDE → Manage Libraries):
 *    LiquidCrystal_I2C  by Frank de Brabander  v1.1.2+
 *
 *  Serial Monitor: 9600 baud
 *    Output format: Steps, Voltage(V), A0_Raw, Energy_uJ
 *
 *  Voltage Divider Maths:
 *    R1 = 100 kΩ,  R2 = 10 kΩ
 *    Ratio = (R1 + R2) / R2 = 11.0
 *    V_actual = V_adc × 11.0
 *    Max measurable = 5V × 11 = 55V  (safe for peak piezo output)
 *
 *  Changelog:
 *    v2.0.0 - Added energy estimation, moving-average filter,
 *             session statistics, improved LCD layout, and
 *             structured serial CSV with header
 *    v1.0.0 - Initial release (basic step + voltage display)
 * ============================================================
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ─────────────────────────────────────────────────────────────
// HARDWARE CONFIGURATION — edit these if you change the wiring
// ─────────────────────────────────────────────────────────────

/** I2C address of the LCD backpack (try 0x3F if screen stays blank). */
#define LCD_I2C_ADDR    0x27
#define LCD_COLS        16
#define LCD_ROWS         2

/** Arduino analog pin connected to the voltage-divider output (A0). */
#define VOLTAGE_PIN     A0

/** Arduino analog pin wired directly to one raw piezo disc (A1). */
#define STEP_DETECT_PIN A1

// ─────────────────────────────────────────────────────────────
// CALIBRATION CONSTANTS
// ─────────────────────────────────────────────────────────────

/** Voltage divider scaling: V_actual = V_adc × DIVIDER_RATIO. */
#define DIVIDER_RATIO   11.0f

/** Arduino ADC reference voltage (5.0V for Uno; 3.3V for 3.3V boards). */
#define VREF             5.0f

/** ADC full-scale counts (10-bit ADC → 1023). */
#define ADC_FULL_SCALE 1023.0f

// ─────────────────────────────────────────────────────────────
// STEP DETECTION TUNING
// ─────────────────────────────────────────────────────────────

/**
 * Raw ADC threshold on STEP_DETECT_PIN to register a footstep.
 * Increase (up to ~200) to reject phantom steps from vibration.
 * Decrease (down to ~50) if real steps are not being counted.
 */
#define STEP_THRESHOLD   80

/**
 * Debounce window in milliseconds after a valid step event.
 * Any pulse within this window after a detected step is ignored.
 */
#define DEBOUNCE_MS     300UL

// ─────────────────────────────────────────────────────────────
// DISPLAY / LOGGING INTERVALS
// ─────────────────────────────────────────────────────────────

/** How often the LCD refreshes (milliseconds). */
#define LCD_REFRESH_MS  500UL

/** How often a CSV line is printed to Serial (milliseconds). */
#define SERIAL_LOG_MS  1000UL

// ─────────────────────────────────────────────────────────────
// ENERGY ESTIMATION CONSTANTS
// ─────────────────────────────────────────────────────────────

/**
 * Approximate effective capacitance of the smoothing capacitor (Farads).
 * Used for energy estimation: E = ½ × C × V²
 * 470 µF = 0.000470 F
 */
#define SMOOTHING_CAP_F 0.000470f

// ─────────────────────────────────────────────────────────────
// MOVING-AVERAGE FILTER
// ─────────────────────────────────────────────────────────────

/** Number of ADC samples to average for voltage reading. */
#define AVG_SAMPLES     8

// ─────────────────────────────────────────────────────────────
// GLOBAL STATE
// ─────────────────────────────────────────────────────────────

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

unsigned long stepCount       = 0;    // cumulative step count this session
unsigned long lastStepTime    = 0;    // millis() of last valid step
unsigned long lastLCDTime     = 0;    // millis() of last LCD refresh
unsigned long lastSerialTime  = 0;    // millis() of last serial print
float         sessionPeakV    = 0.0f; // highest voltage seen this session
float         totalEnergyUJ   = 0.0f; // accumulated energy estimate in µJ

// ─────────────────────────────────────────────────────────────
// HELPER FUNCTIONS
// ─────────────────────────────────────────────────────────────

/**
 * Read VOLTAGE_PIN multiple times and return the averaged ADC value.
 * A simple moving-average filter reduces ADC noise introduced by
 * electromagnetic interference near the piezo rectifier circuit.
 *
 * @return  Averaged raw ADC reading (0–1023, float for precision).
 */
float readVoltageADC() {
  long sum = 0;
  for (uint8_t i = 0; i < AVG_SAMPLES; i++) {
    sum += analogRead(VOLTAGE_PIN);
    delayMicroseconds(200); // short settling gap between samples
  }
  return (float)sum / AVG_SAMPLES;
}

/**
 * Convert a raw ADC reading to actual voltage (accounting for
 * the voltage divider on the input).
 *
 * @param rawADC  Raw averaged ADC value (0.0 – 1023.0).
 * @return        Actual voltage at the piezo output in Volts.
 */
float adcToVoltage(float rawADC) {
  float vADC = (rawADC / ADC_FULL_SCALE) * VREF;
  return vADC * DIVIDER_RATIO;
}

/**
 * Estimate instantaneous energy stored in the smoothing capacitor.
 * Uses the formula E = ½ × C × V²  (result in Joules → converted to µJ).
 *
 * @param voltage  Voltage across the capacitor in Volts.
 * @return         Stored energy in micro-Joules (µJ).
 */
float estimateEnergyUJ(float voltage) {
  float energyJ = 0.5f * SMOOTHING_CAP_F * (voltage * voltage);
  return energyJ * 1.0e6f; // convert J → µJ
}

/**
 * Print a fixed-width float to the LCD.
 * Pads with spaces to overwrite leftover characters from wider numbers.
 *
 * @param value     Floating-point number to display.
 * @param decimals  Digits after the decimal point.
 * @param width     Minimum field width (pads with trailing spaces if shorter).
 */
void lcdPrintFloat(float value, uint8_t decimals, uint8_t width) {
  char buf[16];
  dtostrf(value, width, decimals, buf);
  lcd.print(buf);
}

// ─────────────────────────────────────────────────────────────
// SETUP
// ─────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(9600);

  // Initialise LCD
  lcd.init();
  lcd.backlight();

  // Splash screen — project identity
  lcd.setCursor(0, 0); lcd.print("  Piezo Energy  ");
  lcd.setCursor(0, 1); lcd.print(" Harvester v2.0 ");
  delay(2000);
  lcd.clear();

  // Secondary splash — author
  lcd.setCursor(0, 0); lcd.print("  Selveri Sha.  ");
  lcd.setCursor(0, 1); lcd.print("   SRM  IoT Lab ");
  delay(1500);
  lcd.clear();

  // Print CSV header to Serial Monitor
  Serial.println(F("# IoT Piezoelectric Energy Harvesting — Session Log"));
  Serial.println(F("# Threshold=" STRINGIFY(STEP_THRESHOLD)
                   "  Debounce=" STRINGIFY(DEBOUNCE_MS) "ms"
                   "  AvgSamples=" STRINGIFY(AVG_SAMPLES)));
  Serial.println(F("Steps,Voltage_V,A0_Raw,Energy_uJ,PeakV_Session"));
}

// Compile-time string-ification helper for the Serial header above
#define STRINGIFY_INNER(x) #x
#define STRINGIFY(x) STRINGIFY_INNER(x)

// ─────────────────────────────────────────────────────────────
// MAIN LOOP
// ─────────────────────────────────────────────────────────────

void loop() {

  // ── 1. STEP DETECTION ────────────────────────────────────────
  //    Read raw piezo pulse on A1. A threshold crossing that is
  //    outside the debounce window counts as one footstep.
  int rawA1 = analogRead(STEP_DETECT_PIN);
  unsigned long now = millis();

  if (rawA1 > STEP_THRESHOLD) {
    if ((now - lastStepTime) > DEBOUNCE_MS) {
      stepCount++;
      lastStepTime = now;
    }
  }

  // ── 2. VOLTAGE MEASUREMENT ──────────────────────────────────
  //    Average ADC readings to reduce noise, then scale to
  //    actual output voltage using the voltage divider ratio.
  float rawA0    = readVoltageADC();          // averaged ADC count
  float vActual  = adcToVoltage(rawA0);       // real voltage in V
  float energyUJ = estimateEnergyUJ(vActual); // instantaneous energy in µJ

  // Track session peak voltage
  if (vActual > sessionPeakV) {
    sessionPeakV = vActual;
  }

  // Accumulate total energy estimate (sampled every LCD refresh ≈ 0.5 s)
  // This is a rough proxy — a proper integration needs more samples.

  // ── 3. LCD REFRESH ────────────────────────────────────────────
  //    Update display every LCD_REFRESH_MS milliseconds.
  //    LCD layout:
  //      Line 1:  S:<steps>   V:<voltage>V
  //      Line 2:  E:<energy>uJ  Pk:<peak>V
  if ((now - lastLCDTime) >= LCD_REFRESH_MS) {
    lastLCDTime = now;
    totalEnergyUJ += energyUJ * (LCD_REFRESH_MS / 1000.0f);

    lcd.clear();

    // ── LINE 1: Step count (left) + Voltage (right) ──────────
    lcd.setCursor(0, 0);
    lcd.print("S:");
    lcd.print(stepCount);

    lcd.setCursor(7, 0);
    lcd.print("V:");
    lcdPrintFloat(vActual, 2, 4); // e.g. "3.45"
    lcd.print("V");

    // ── LINE 2: Accumulated energy + Session peak ─────────────
    lcd.setCursor(0, 1);
    lcd.print("E:");
    lcdPrintFloat(totalEnergyUJ, 1, 5); // e.g. "  1.2"
    lcd.print("uJ");

    lcd.setCursor(9, 1);
    lcd.print("P:");
    lcdPrintFloat(sessionPeakV, 1, 3); // e.g. "4.2"
    lcd.print("V");
  }

  // ── 4. SERIAL CSV LOGGING ─────────────────────────────────────
  //    Print one CSV row per second.  Import into Excel/Python
  //    for graphing voltage vs. step count over a session.
  if ((now - lastSerialTime) >= SERIAL_LOG_MS) {
    lastSerialTime = now;

    Serial.print(stepCount);         Serial.print(",");
    Serial.print(vActual,    3);     Serial.print(",");
    Serial.print((int)rawA0);        Serial.print(",");
    Serial.print(energyUJ,   2);     Serial.print(",");
    Serial.println(sessionPeakV, 3);
  }
}
