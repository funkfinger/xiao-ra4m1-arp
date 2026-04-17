#include <Arduino.h>
#include "scales.h"
#include "arp.h"

// ─── Pin assignments (spec §2.3) ───────────────────────────────────
#define PIN_DAC DAC                             // D0/A0 — V/Oct output
static constexpr int PIN_GATE     = 6;         // D6 — gate output via NPN
static constexpr int DAC_BITS     = 12;
static constexpr int DAC_MAX      = (1 << DAC_BITS) - 1;

// ─── Calibration (from Story 004 bench measurement) ────────────────
static constexpr float GAIN       = 1.261f;    // measured op-amp gain
static constexpr float DAC_VREF   = 3.3f;
static constexpr int   MIDI_ROOT  = 48;        // C3 = 0V output

// ─── Clock ─────────────────────────────────────────────────────────
static constexpr float BPM        = 120.0f;
static constexpr unsigned long STEP_MS = static_cast<unsigned long>(60000.0f / BPM);
static constexpr float GATE_DUTY  = 0.5f;
static constexpr unsigned long GATE_ON_MS = static_cast<unsigned long>(STEP_MS * GATE_DUTY);

// ─── Engine ────────────────────────────────────────────────────────
static arp::Arp arpeggiator(MIDI_ROOT);

int midiToDac(uint8_t midiNote) {
    float targetV = static_cast<float>(midiNote - MIDI_ROOT) / 12.0f;
    float dacV = targetV / GAIN;
    int count = static_cast<int>((dacV / DAC_VREF) * DAC_MAX + 0.5f);
    if (count < 0) count = 0;
    if (count > DAC_MAX) count = DAC_MAX;
    return count;
}

void setup() {
    analogWriteResolution(DAC_BITS);
    pinMode(PIN_GATE, OUTPUT);
    digitalWrite(PIN_GATE, LOW);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
    uint8_t rawNote = arpeggiator.current();
    uint8_t note = arp::quantize(rawNote, arp::Scale::Major);
    int dac = midiToDac(note);

    // CV: set pitch
    analogWrite(PIN_DAC, dac);

    // Gate on — NPN common-emitter inverts: LOW on base = transistor off = collector HIGH (5V)
    digitalWrite(PIN_GATE, LOW);
    digitalWrite(LED_BUILTIN, LOW);

    delay(GATE_ON_MS);

    // Gate off — HIGH on base = transistor on = collector LOW (0V)
    digitalWrite(PIN_GATE, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);

    delay(STEP_MS - GATE_ON_MS);

    // Advance to next note
    arpeggiator.advance();
}
