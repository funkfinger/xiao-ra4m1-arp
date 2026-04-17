#include <Arduino.h>

// V/Oct calibration — Story 004.
// Steps through C3–C7 (0V–4V), holding each for 2 seconds.
// Prints note name, DAC count, expected voltage to Serial.
// Measure op-amp output (after R4) with multimeter or scope.

static constexpr int DAC_PIN = DAC;
static constexpr int DAC_RESOLUTION = 12;
static constexpr int DAC_MAX = (1 << DAC_RESOLUTION) - 1;

static constexpr float GAIN = 1.261f;      // measured; nominal 1.27 (2.7k/10k)
static constexpr float DAC_VREF = 3.3f;
static constexpr int MIDI_ROOT = 48;        // C3 = 0V output
static constexpr float VOLT_PER_OCTAVE = 1.0f;
static constexpr float SEMITONES_PER_OCTAVE = 12.0f;
static constexpr unsigned long HOLD_MS = 2000;

struct CalNote {
    const char* name;
    int midi;
};

static constexpr CalNote notes[] = {
    {"C3", 48},
    {"C4", 60},
    {"C5", 72},
    {"C6", 84},
    {"C7", 96},
};
static constexpr int NUM_NOTES = sizeof(notes) / sizeof(notes[0]);

int midiToDac(int midiNote) {
    float targetVolts = (midiNote - MIDI_ROOT) / SEMITONES_PER_OCTAVE * VOLT_PER_OCTAVE;
    float dacVolts = targetVolts / GAIN;
    int dacCount = static_cast<int>((dacVolts / DAC_VREF) * DAC_MAX + 0.5f);
    if (dacCount < 0) dacCount = 0;
    if (dacCount > DAC_MAX) dacCount = DAC_MAX;
    return dacCount;
}

void setup() {
    analogWriteResolution(DAC_RESOLUTION);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== V/Oct Calibration — Story 004 ===");
    Serial.print("Gain: ");
    Serial.println(GAIN, 3);
    Serial.println("Measure op-amp output with multimeter.");
    Serial.println();
}

void loop() {
    for (int i = 0; i < NUM_NOTES; ++i) {
        int dac = midiToDac(notes[i].midi);
        float expectedV = (notes[i].midi - MIDI_ROOT) / SEMITONES_PER_OCTAVE * VOLT_PER_OCTAVE;

        analogWrite(DAC_PIN, dac);

        digitalWrite(LED_BUILTIN, LOW);

        Serial.print(notes[i].name);
        Serial.print("  MIDI=");
        Serial.print(notes[i].midi);
        Serial.print("  DAC=");
        Serial.print(dac);
        Serial.print("  Expected=");
        Serial.print(expectedV, 3);
        Serial.println("V");

        delay(HOLD_MS);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
    }
    Serial.println("--- cycle ---");
}
