#ifndef GAS_SENSOR_HANDLER_H
#define GAS_SENSOR_HANDLER_H

#include <Arduino.h>

class GasSensor {
private:
    uint8_t analogPin;
    
    // Thresholds (adjust based on your needs)
    float warningThreshold = 1000.0;  // ppm for warning
    float dangerThreshold = 2000.0;   // ppm for danger
    
    // Filter variables
    float filteredPPM = 0;
    const float filterFactor = 0.2; // Smoothing factor
    
    unsigned long lastReadTime = 0;
    const unsigned long readInterval = 2000; // Read every 2 seconds

    // Test mode support
    bool testMode = false;
    float testPPM = 400.0; // Default to normal air in test mode
    bool testWarning = false;
    bool testDanger = false;

public:
    GasSensor(uint8_t pin) : analogPin(pin) {}
    
    void initialize() {
        pinMode(analogPin, INPUT);
    }
    
    void update() {
        if (testMode) return; // Skip hardware reads in test mode
        
        if (millis() - lastReadTime < readInterval) return;
        
        int rawValue = analogRead(analogPin);
        // Simple conversion from raw analog value to ppm (adjust scaling as needed)
        float ppm = map(rawValue, 0, 1023, 300, 5000);
        
        // Apply low-pass filter
        filteredPPM = (filterFactor * ppm) + ((1 - filterFactor) * filteredPPM);
        lastReadTime = millis();
    }
    
    // Test mode functions
    void setTestMode(bool enabled, float ppm = 400.0) {
        testMode = enabled;
        if (enabled) {
            testPPM = ppm;
            testWarning = (ppm > warningThreshold);
            testDanger = (ppm > dangerThreshold);
        }
    }

    void setTestPPM(float ppm) {
        if (testMode) {
            testPPM = ppm;
            testWarning = (ppm > warningThreshold);
            testDanger = (ppm > dangerThreshold);
        }
    }

    // Get current CO2 level in ppm
    float getCO2() const {
        return testMode ? testPPM : filteredPPM;
    }
    
    // Threshold checks
    bool isWarning() const {
        return testMode ? testWarning : (getCO2() > warningThreshold);
    }
    
    bool isDanger() const {
        return testMode ? testDanger : (getCO2() > dangerThreshold);
    }
    
    // Configuration methods
    void setThresholds(float warning, float danger) {
        warningThreshold = warning;
        dangerThreshold = danger;
        if (testMode) {
            testWarning = (testPPM > warningThreshold);
            testDanger = (testPPM > dangerThreshold);
        }
    }
};

#endif
