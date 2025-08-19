#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class Buzzer {
private:
    uint8_t pin;
    unsigned long lastBeepTime = 0;
    bool isActive = false;
    
    // Melody configuration
    const int melody[2] = {262, 294};  // 도(D4), 레(D4#) frequencies (changed from your example for better distinction)
    const int noteDuration = 250;      // Base duration (ms)
    const int pauseDuration = 50;      // Pause between notes (ms)

public:
    Buzzer(uint8_t buzzerPin) : pin(buzzerPin) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    void playNote(int note, int duration) {
        tone(pin, note);
        delay(duration);
        noTone(pin);
        delay(pauseDuration);
    }

    void playMelody() {
        for (int i = 0; i < 4; i++) {  // Play 4 notes (alternating between the two)
            playNote(melody[i % 2], noteDuration);
        }
    }

    void alertWarning() {
        // Play melody once every 2 seconds for warning
        if (millis() - lastBeepTime >= 2000) {
            lastBeepTime = millis();
            playMelody();
        }
    }

    void alertDanger() {
        // Play melody continuously for danger
        if (millis() - lastBeepTime >= 1000) {
            lastBeepTime = millis();
            playMelody();
        }
    }

    void stop() {
        noTone(pin);
        lastBeepTime = 0;
    }
};

#endif
