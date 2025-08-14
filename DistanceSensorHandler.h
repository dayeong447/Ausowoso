#ifndef DISTANCE_SENSOR_HANDLER_H
#define DISTANCE_SENSOR_HANDLER_H

class DistanceSensorHandler {
private:
    uint8_t triggerPin;
    uint8_t echoPin;
    float distance;
    unsigned long lastMeasurement;
    const unsigned long measurementInterval = 100; // ms between measurements
    const float speedOfSound = 0.0343; // cm/µs at 20°C

    // Filter variables
    float filteredDistance = 0;
    const float filterFactor = 0.2; // Smoothing factor (0.1-0.5)
    
    // Test mode support
    bool testMode = false;
    bool validReading = false;

public:
    DistanceSensorHandler(uint8_t trigPin, uint8_t echoPin) 
      : triggerPin(trigPin), echoPin(echoPin), distance(0), lastMeasurement(0) {}

    void initialize() {
        pinMode(triggerPin, OUTPUT);
        pinMode(echoPin, INPUT);
        digitalWrite(triggerPin, LOW);
        filteredDistance = 300.0; // Initialize with safe distance
        distance = 300.0;
    }

    void setTestMode(bool enabled) {
        testMode = enabled;
    }

    void update() {
        if (testMode) return; // Skip hardware reads in test mode
        
        if (millis() - lastMeasurement >= measurementInterval) {
            // Send 10µs pulse
            digitalWrite(triggerPin, HIGH);
            delayMicroseconds(10);
            digitalWrite(triggerPin, LOW);

            // Measure echo duration
            long duration = pulseIn(echoPin, HIGH, 30000); // Timeout after 30ms (~5m)

            if (duration > 0) {
                // Calculate distance (cm)
                float newDistance = duration * speedOfSound / 2;

                // Apply simple low-pass filter
                if (newDistance > 2.0 && newDistance < 400) { // Valid range 2cm-4m
                    filteredDistance = (filterFactor * newDistance) + ((1 - filterFactor) * filteredDistance);
                    distance = filteredDistance;
                    validReading = true;
                } else {
                    // Invalid reading, keep previous value
                    validReading = false;
                }
            } else {
                // Timeout - no echo received
                validReading = false;
                // Keep previous distance value
            }

            lastMeasurement = millis();
        }
    }

    float getDistance() const {
        return distance;
    }

    bool isObjectDetected(float thresholdDistance = 50.0) const {
        return validReading && distance > 2.0 && distance < thresholdDistance;
    }

    bool hasValidReading() const {
        return validReading;
    }

    // Test mode functions
    void setTestDistance(float testDistance) {
        if (testMode) {
            distance = testDistance;
            validReading = true;
        }
    }

    void clearTestReading() {
        if (testMode) {
            validReading = false;
        }
    }
};

#endif
