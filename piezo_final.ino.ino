/*
 * ============================================================
 *  IoT Piezoelectric Footstep Energy Harvesting
 *  LCD Display Shows:
 *    Line 1: Steps count + Voltage generated
 *    Line 2: Raw A0 value (noise/signal level)
 *
 *  Hardware:
 *    - 6x Piezo discs (parallel) → Bridge rectifier → A0
 *    - One direct piezo wire     → A1  (step pulse detection)
 *    - 16x2 LCD via I2C          → A4(SDA), A5(SCL)
 *    - Voltage divider on A0     → R1=100kΩ, R2=10kΩ
 *
 *  Library needed:
 *    Arduino IDE → Manage Libraries → "LiquidCrystal I2C"
 *    by Frank de Brabander → Install
 * ============================================================
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ── LCD: address 0x27 (if blank screen, try 0x3F) ──────────
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ── Pins ────────────────────────────────────────────────────
#define VOLTAGE_PIN   A0   // Rectified piezo output via voltage divider
#define PIEZO_RAW_PIN A1   // Direct piezo wire for step detection

// ── Voltage divider: R1=100kΩ, R2=10kΩ ─────────────────────
//    Scales 0–20V down to 0–5V safe for Arduino ADC
//    Formula: Vactual = Vadc × (R1+R2)/R2 = Vadc × 11
#define DIVIDER_RATIO  11.0
#define VREF            5.0
#define ADC_MAX       1023.0

// ── Step detection tuning ───────────────────────────────────
//    Raise if phantom steps appear, lower if real steps missed
#define STEP_THRESHOLD   80    // A1 raw ADC value to count as a step
#define DEBOUNCE_MS     300    // ignore re-trigger for 300ms after step

// ── Globals ─────────────────────────────────────────────────
unsigned long stepCount    = 0;
unsigned long lastStepTime = 0;
unsigned long lastLCDTime  = 0;

// ════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  // Splash screen
  lcd.setCursor(0, 0); lcd.print("  Piezo Energy  ");
  lcd.setCursor(0, 1); lcd.print("  Harvesting... ");
  delay(2000);
  lcd.clear();

  Serial.println("Steps, Voltage(V), A0_Raw");
}

// ════════════════════════════════════════════════════════════
void loop() {

  // ── 1. Detect footstep via raw piezo pulse on A1 ─────────
  int rawA1 = analogRead(PIEZO_RAW_PIN);

  if (rawA1 > STEP_THRESHOLD) {
    unsigned long now = millis();
    if ((now - lastStepTime) > DEBOUNCE_MS) {
      stepCount++;
      lastStepTime = now;
    }
  }

  // ── 2. Read voltage on A0 (through voltage divider) ──────
  int   rawA0     = analogRead(VOLTAGE_PIN);
  float vADC      = (rawA0 / ADC_MAX) * VREF;
  float vActual   = vADC * DIVIDER_RATIO;   // real voltage in volts

  // ── 3. Update LCD every 500ms ─────────────────────────────
  unsigned long now = millis();
  if ((now - lastLCDTime) >= 500) {
    lastLCDTime = now;

    lcd.clear();

    // ── LINE 1: S:xx  V:xx.xxV ────────────────────────────
    // "S:" = Steps,  "V:" = Voltage
    lcd.setCursor(0, 0);
    lcd.print("S:");
    lcd.print(stepCount);

    // Right-align voltage on same line
    // Voltage string: "V:xx.xxV"
    lcd.setCursor(8, 0);
    lcd.print("V:");
    lcd.print(vActual, 2);   // 2 decimal places
    lcd.print("V");

    // ── LINE 2: A0 raw value (noise / signal strength) ────
    // This is the direct ADC reading on A0, 0–1023
    // Useful to see if piezo is outputting anything
    lcd.setCursor(0, 1);
    lcd.print("A0 Raw:");
    lcd.setCursor(7, 1);
    lcd.print(rawA0);
    lcd.print("  ");   // clear leftover digits if number shrinks

  }

  // ── 4. Serial output for monitoring on PC ─────────────────
  // Open Serial Monitor (9600 baud) to watch live values
  static unsigned long lastSerial = 0;
  if ((millis() - lastSerial) >= 1000) {
    lastSerial = millis();
    Serial.print(stepCount);
    Serial.print(", ");
    Serial.print(vActual, 3);
    Serial.print("V, A0=");
    Serial.println(rawA0);
  }

}
