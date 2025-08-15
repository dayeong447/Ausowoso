#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <DHT.h>

#define DHT_TYPE DHT11

class DHTSensor {
private:
    DHT dht;
    float temperature;
    float humidity;
    unsigned long lastReadTime = 0;
    const unsigned long readInterval = 2000; // Read every 2 seconds
    
    // Set Thresholds for fire conditions
    static constexpr float FIRE_TEMP_LOWER_THRESHOLD = -40.0;
    static constexpr float FIRE_TEMP_UPPER_THRESHOLD = 400.0;
    static constexpr float FIRE_HUMIDITY_LOWER_THRESHOLD = 0.0;
    static constexpr float FIRE_HUMIDITY_UPPER_THRESHOLD = 100.0;
    static constexpr float CRITICAL_TEMP_THRESHOLD = 25.0; // °C for critical emergency
    static constexpr float CRITICAL_HUMIDITY_THRESHOLD = 30.0; // %

    // Test mode and validation
    bool testMode = false;
    bool validReading = false;
    uint8_t consecutiveFailures = 0;
    const uint8_t maxFailures = 3;

public:
    DHTSensor(uint8_t pin) : dht(pin, DHT_TYPE) {}

    void initialize() {
        dht.begin();
        // Set initial safe values
        validReading = false;
        consecutiveFailures = 0;
    }

    void setTestMode(bool enabled) {
        testMode = enabled;
    }

    void update() {
        if (testMode) return; // Skip hardware reads in test mode
        
        if (millis() - lastReadTime >= readInterval) {
            float newTemp = dht.readTemperature();
            float newHumidity = dht.readHumidity();
            
            // Check if readings are valid
            if (!isnan(newTemp) && !isnan(newHumidity)) {
                // Additional validation - reasonable ranges
                if (newTemp >= FIRE_TEMP_LOWER_THRESHOLD && newTemp <= FIRE_TEMP_UPPER_THRESHOLD && 
                    newHumidity >= FIRE_HUMIDITY_LOWER_THRESHOLD && newHumidity <= FIRE_HUMIDITY_UPPER_THRESHOLD) {
                    temperature = newTemp;
                    humidity = newHumidity;
                    validReading = true;
                    consecutiveFailures = 0;
                } else {
                    // Out of range readings
                    consecutiveFailures++;
                    if (consecutiveFailures >= maxFailures) {
                        validReading = false;
                    }
                }
            } else {
                // NaN readings
                consecutiveFailures++;
                if (consecutiveFailures >= maxFailures) {
                    validReading = false;
                }
            }
            
            lastReadTime = millis();
        }
    }

    float getTemperature() const {
        return temperature;
    }

    float getHumidity() const {
        return humidity;
    }

    bool isCritical() const {
        if (!validReading) return false; // Don't trigger on invalid readings
        return temperature > CRITICAL_TEMP_THRESHOLD || humidity < CRITICAL_HUMIDITY_THRESHOLD;
    }

    bool hasValidReading() const {
        return validReading;
    }

    uint8_t getFailureCount() const {
        return consecutiveFailures;
    }

    // Test mode functions
    void setTestTemperature(float testTemp) {
        if (testMode) {
            temperature = testTemp;
            validReading = true;
            consecutiveFailures = 0;
        }
    }

    void setTestHumidity(float testHumidity) {
        if (testMode) {
            humidity = testHumidity;
            validReading = true;
            consecutiveFailures = 0;
        }
    }

    void setTestValues(float testTemp, float testHumidity) {
        if (testMode) {
            // Validasi nilai test
            if (testTemp >= FIRE_TEMP_LOWER_THRESHOLD && testTemp <= FIRE_TEMP_UPPER_THRESHOLD &&
                testHumidity >= FIRE_HUMIDITY_LOWER_THRESHOLD && testHumidity <= FIRE_HUMIDITY_UPPER_THRESHOLD) {
                temperature = testTemp;
                humidity = testHumidity;
                validReading = true;
            } else {
                validReading = false;
            }
            consecutiveFailures = 0;
        }
    }
    void simulateFailure() {
        if (testMode) {
            validReading = false;
            consecutiveFailures = maxFailures;
        }
    }
};

#endif
