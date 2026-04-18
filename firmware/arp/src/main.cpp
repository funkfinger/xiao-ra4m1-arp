#include <Arduino.h>
#include "scales.h"
#include "arp.h"
#include "tempo.h"

// ─── Pin assignments (spec §2.3) ───────────────────────────────────
#define PIN_DAC DAC                             // D0/A0 — V/Oct output
static constexpr int PIN_GATE     = 6;          // D6 — gate output via NPN
static constexpr int PIN_TEMPO    = 5;          // D5 — tempo pot (RV3) — spec §2.3 said D8 but D8 lacks ADC; remapped to D5
static constexpr int PIN_SCALE    = 2;          // D2/A2 — scale pot (RV1)
static constexpr int DAC_BITS     = 12;
static constexpr int DAC_MAX      = (1 << DAC_BITS) - 1;
static constexpr int ADC_BITS     = 14;
static constexpr int ADC_MAX      = (1 << ADC_BITS) - 1;

// ─── Calibration (from Story 004 bench measurement) ────────────────
static constexpr float GAIN       = 1.261f;     // measured op-amp gain
static constexpr float DAC_VREF   = 3.3f;
static constexpr int   MIDI_ROOT  = 48;         // C3 = 0V output

static constexpr float GATE_DUTY  = 0.5f;
static constexpr int   STEPS_PER_BEAT = 4;      // 16th-note arp steps at BPM

// ─── Engine ────────────────────────────────────────────────────────
static arp::Arp arpeggiator(MIDI_ROOT);
static arp::Scale currentScale = arp::Scale::Major;

int midiToDac(uint8_t midiNote) {
    float targetV = static_cast<float>(midiNote - MIDI_ROOT) / 12.0f;
    float dacV = targetV / GAIN;
    int count = static_cast<int>((dacV / DAC_VREF) * DAC_MAX + 0.5f);
    if (count < 0) count = 0;
    if (count > DAC_MAX) count = DAC_MAX;
    return count;
}

int readBpm() {
    int raw = analogRead(PIN_TEMPO);
    float pot = static_cast<float>(raw) / static_cast<float>(ADC_MAX);
    return arp::tempo::bpmFromPot(pot);
}

arp::Scale readScale(arp::Scale current) {
    int raw = analogRead(PIN_SCALE);
    float pot = static_cast<float>(raw) / static_cast<float>(ADC_MAX);
    return arp::scaleFromPot(pot, current);
}

void setup() {
    analogWriteResolution(DAC_BITS);
    analogReadResolution(ADC_BITS);
    pinMode(PIN_GATE, OUTPUT);
    digitalWrite(PIN_GATE, HIGH);  // gate off (NPN inverts)
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
    int bpm = readBpm();
    unsigned long beatMs = arp::tempo::stepMsFromBpm(bpm);
    unsigned long stepMs = beatMs / STEPS_PER_BEAT;
    unsigned long gateOnMs = static_cast<unsigned long>(stepMs * GATE_DUTY);

    currentScale = readScale(currentScale);

    uint8_t rawNote = arpeggiator.current();
    uint8_t note = arp::quantize(rawNote, currentScale);
    int dac = midiToDac(note);

    analogWrite(PIN_DAC, dac);

    // Gate on — NPN common-emitter inverts
    digitalWrite(PIN_GATE, LOW);
    digitalWrite(LED_BUILTIN, LOW);

    delay(gateOnMs);

    // Gate off
    digitalWrite(PIN_GATE, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);

    delay(stepMs - gateOnMs);

    arpeggiator.advance();
}
