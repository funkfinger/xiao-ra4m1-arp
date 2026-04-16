#include <Arduino.h>

static constexpr unsigned long BLINK_PERIOD_MS = 1000;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
    digitalWrite(LED_BUILTIN, LOW);
    delay(BLINK_PERIOD_MS / 2);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(BLINK_PERIOD_MS / 2);
}
