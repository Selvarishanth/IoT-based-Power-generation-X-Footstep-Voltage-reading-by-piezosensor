# Contributing to IoT Piezoelectric Energy Harvesting

Thank you for considering a contribution! This is an open academic project and improvements are welcome.

## How to Contribute

### Reporting Bugs or Suggesting Improvements

1. Open a **GitHub Issue** describing:
   - What you observed vs. what you expected
   - Your hardware (Arduino board, LCD model, piezo disc specs)
   - Your Arduino IDE version
   - Any error messages from Serial Monitor or the compiler

### Submitting Code Changes

1. **Fork** the repository
2. Create a feature branch:
   ```bash
   git checkout -b feature/your-descriptive-name
   ```
3. Make your changes — see the code style guidelines below
4. Test on real hardware (or clearly document that it is untested)
5. Commit with a meaningful message:
   ```
   feat: add EEPROM persistence for step count across resets
   fix: correct voltage divider ratio comment (was 10x, should be 11x)
   docs: add wiring diagram for ESP32 variant
   ```
6. Open a **Pull Request** against `main` with a description of the change

## Code Style (C++/Arduino)

- Use the `#define` constants at the top for all magic numbers — no inline literals
- Document every function with a brief comment block explaining parameters and return value
- Use `unsigned long` for `millis()` comparisons to avoid rollover bugs
- Prefix global variables descriptively: `stepCount`, `lastStepTime` — not `s`, `t`
- Keep `loop()` non-blocking: no `delay()` in the main loop
- Serial output should remain valid CSV — don't add extra text mid-session

## Branches

| Branch | Purpose |
|--------|---------|
| `main` | Stable, tested code only |
| `dev` | Integration branch for new features |
| `feature/*` | Individual feature work |
| `fix/*` | Bug fixes |

## Ideas Open for Contribution

- [ ] ESP32 WiFi variant with MQTT or HTTP POST to a dashboard
- [ ] Python script to plot serial CSV data automatically
- [ ] Wiring diagram as a proper KiCad schematic
- [ ] Power consumption analysis (sleep modes, duty cycling)
- [ ] FFT analysis of piezo frequency response

## Questions?

Open a GitHub Discussion or contact the maintainer via the profile linked in the README.
