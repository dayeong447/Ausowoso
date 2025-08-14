#ifndef TRAFFICLIGHT_HANDLER_H
#define TRAFFICLIGHT_HANDLER_H

// Traffic light phases
enum TrafficLightPhase {
  PHASE_RED = 1,
  PHASE_RED_YELLOW = 2,
  PHASE_GREEN = 3,
  PHASE_YELLOW = 4
};

class TrafficLight {
private:
    // Pin configuration
    uint8_t redPin;
    uint8_t yellowPin;
    uint8_t greenPin;

    // Timing constants
    uint16_t redTime;
    uint16_t redYellowTime;
    uint16_t greenTime;
    uint16_t yellowTime;

    // Current state
    TrafficLightPhase currentPhase;
    unsigned long phaseStartTime;
    bool manualMode;

    void setLights(uint8_t red, uint8_t yellow, uint8_t green) {
        digitalWrite(redPin, red);
        digitalWrite(yellowPin, yellow);
        digitalWrite(greenPin, green);
    }

public:
    // Static mock variables for testing
    static bool mockRedState;
    static bool mockYellowState;
    static bool mockGreenState;

    TrafficLight(uint8_t redPin, uint8_t yellowPin, uint8_t greenPin) 
        : redPin(redPin), yellowPin(yellowPin), greenPin(greenPin),
          redTime(5000), redYellowTime(1000), greenTime(5000), yellowTime(5000),
          currentPhase(PHASE_RED), manualMode(false) {}

    void initialize() {
        pinMode(redPin, OUTPUT);
        pinMode(yellowPin, OUTPUT);
        pinMode(greenPin, OUTPUT);
        
        currentPhase = PHASE_RED;
        phaseStartTime = millis();
        manualMode = false;
        
        setLights(HIGH, LOW, LOW);
    }

    void setPhase(TrafficLightPhase newPhase) {
        if (newPhase == currentPhase) return;
        
        currentPhase = newPhase;
        phaseStartTime = millis();
        
        switch(currentPhase) {
            case PHASE_RED:
                setLights(HIGH, LOW, LOW);
                break;
            case PHASE_RED_YELLOW:
                setLights(HIGH, HIGH, LOW);
                break;
            case PHASE_GREEN:
                setLights(LOW, LOW, HIGH);
                break;
            case PHASE_YELLOW:
                setLights(LOW, HIGH, LOW);
                break;
        }
    }

    void update() {
        if (manualMode) return;
        
        unsigned long currentTime = millis();
        unsigned long elapsed = currentTime - phaseStartTime;
        
        switch(currentPhase) {
            case PHASE_RED:
                if (elapsed >= redTime) {
                    setPhase(PHASE_RED_YELLOW);
                }
                break;
            case PHASE_RED_YELLOW:
                if (elapsed >= redYellowTime) {
                    setPhase(PHASE_GREEN);
                }
                break;
            case PHASE_GREEN:
                if (elapsed >= greenTime) {
                    setPhase(PHASE_YELLOW);
                }
                break;
            case PHASE_YELLOW:
                if (elapsed >= yellowTime) {
                    setPhase(PHASE_RED);
                }
                break;
        }
    }

    void toggleMode() {
        manualMode = !manualMode;
    }

    bool isManual() const {
        return manualMode;
    }

    TrafficLightPhase getPhase() const {
        return currentPhase;
    }

    void nextPhase() {
        TrafficLightPhase nextPhase = static_cast<TrafficLightPhase>(currentPhase + 1);
        if (nextPhase > PHASE_YELLOW) nextPhase = PHASE_RED;
        setPhase(nextPhase);
    }

    void prevPhase() {
        TrafficLightPhase nextPhase = static_cast<TrafficLightPhase>(currentPhase - 1);
        if (nextPhase < PHASE_RED) nextPhase = PHASE_YELLOW;
        setPhase(nextPhase);
    }

    void setTiming(uint16_t red, uint16_t redYellow, uint16_t green, uint16_t yellow) {
        redTime = red;
        redYellowTime = redYellow;
        greenTime = green;
        yellowTime = yellow;
    }
};

// Initialize static members (should be in a .cpp file, but for Arduino we can put here)
bool TrafficLight::mockRedState = false;
bool TrafficLight::mockYellowState = false;
bool TrafficLight::mockGreenState = false;

#endif
