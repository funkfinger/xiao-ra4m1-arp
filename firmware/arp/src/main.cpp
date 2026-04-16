#include <Arduino.h>

// DAC ramp — Story 003 bench bring-up.
// Writes a 0→4095 linear ramp to DAC0 (pin D0/A0) at ~10 Hz,
// producing a 0–3.3 V sawtooth visible on a scope.
// LED toggles every 5 ramp cycles (~1 Hz visible blink).

static constexpr int DAC_PIN = DAC;
static constexpr int DAC_RESOLUTION = 12;
static constexpr int DAC_MAX = (1 << DAC_RESOLUTION) - 1;
static constexpr int RAMP_STEPS = 256;
static constexpr unsigned long RAMP_PERIOD_US = 100000;
static constexpr unsigned long STEP_US = RAMP_PERIOD_US / RAMP_STEPS;
static constexpr int LED_TOGGLE_CYCLES = 5;

static int cycleCount = 0;
static bool ledState = true;

void setup() {
    analogWriteResolution(DAC_RESOLUTION);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
    for (int i = 0; i < RAMP_STEPS; ++i) {
        int dacValue = static_cast<int>((static_cast<long>(i) * DAC_MAX) / (RAMP_STEPS - 1));
        analogWrite(DAC_PIN, dacValue);
        delayMicroseconds(STEP_US);
    }

    if (++cycleCount >= LED_TOGGLE_CYCLES) {
        cycleCount = 0;
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
    }
}
