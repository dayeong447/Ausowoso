#ifndef UNITTEST_H
#define UNITTEST_H

#include "EmergencyTraffic.h"
#include "TrafficLightHandler.h"
#include "BarrierServoHandler.h"
#include "DistanceSensorHandler.h"
#include "HeatSensorHandler.h"
#include "LcdDisplayHandler.h"

namespace TestUtils {
    unsigned long mockMillis = 0;
    
    unsigned long testMillis() {
        return mockMillis;
    }
    
    void advanceTime(unsigned long ms) {
        mockMillis += ms;
    }
}

class CompactTest {
public:
    uint16_t passed = 0;
    uint16_t failed = 0;
    
    void test(bool condition, const String &message) {
        if (condition) {
            passed++;
            Serial.print(F("✓ "));
        } else {
            failed++;
            Serial.print(F("✗ "));
        }
        Serial.println(message);
    }
    
    void summary() {
        Serial.println(F("\n=== TEST RESULTS ==="));
        Serial.print(F("Passed: ")); Serial.println(passed);
        Serial.print(F("Failed: ")); Serial.println(failed);
        Serial.print(F("Success Rate: ")); 
        Serial.print((passed * 100) / (passed + failed)); 
        Serial.println(F("%"));
        
        if (failed == 0) {
            Serial.println(F("🎉 ALL TESTS PASSED!"));
        } else {
            Serial.println(F("⚠️ SOME TESTS FAILED"));
        }
    }
};

class SmartCityTestSuite {
private:
    CompactTest test;
    unsigned long emergencyStartTime = 0;
    bool emergencyActive = false;
    
    DHTSensor dht;
    DistanceSensorHandler distance;
    BarrierControl barrier;
    LCDDisplay lcd;
    TrafficLight trafficLight;
    EmergencyTraffic emergencyTraffic;

    void activateEmergencyMode() {
        emergencyActive = true;
        emergencyStartTime = TestUtils::mockMillis;
        trafficLight.setPhase(PHASE_RED);
        emergencyTraffic.allStop();
        lcd.displayDisasterWarning(1, FIRE, dht.getTemperature(), "EVACUATE NOW!");
    }

    void deactivateEmergencyMode() {
        emergencyActive = false;
        barrier.raise();
        trafficLight.initialize();
        emergencyTraffic.allStop();
        lcd.displayStatic("SYSTEM RECOVERED", "Normal Operation");
    }

public:
    SmartCityTestSuite(uint8_t dhtPin, uint8_t trigPin, uint8_t echoPin, uint8_t servoPin, 
                      uint8_t redPin, uint8_t yellowPin, uint8_t greenPin,
                      uint8_t rightR, uint8_t rightG, uint8_t leftR, uint8_t leftG,
                      uint8_t centerR, uint8_t centerG) :
        dht(dhtPin), distance(trigPin, echoPin), barrier(servoPin),
        trafficLight(redPin, yellowPin, greenPin),
        emergencyTraffic(rightR, rightG, leftR, leftG, centerR, centerG) {}

    void begin() {
        Serial.println(F("\n=== SMART CITY TEST SUITE ==="));
        
        dht.setTestMode(true);
        distance.setTestMode(true);
        
        lcd.initialize();
        barrier.initialize();
        trafficLight.initialize();
        emergencyTraffic.initialize();
        
        dht.setTestValues(22.0, 55.0);
        distance.setTestDistance(300.0);
        barrier.raise();
        
        Serial.println(F("✅ Test environment ready"));
    }

    void advanceTime(unsigned long ms) {
        TestUtils::advanceTime(ms);
    }

    // NEW: Emergency Traffic Tests
    void testEmergencyTraffic() {
        Serial.println(F("\n[TEST] Emergency Traffic Control"));
        
        // Activate emergency mode
        dht.setTestValues(45.0, 20.0);
        advanceTime(2000);
        updateSystem();
        
        // Verify emergency mode
        test.test(emergencyActive, "Emergency mode activated");
        test.test(trafficLight.getPhase() == PHASE_RED, "Main traffic light red");
        
        // Test emergency traffic patterns
        emergencyTraffic.goRight();
        test.test(emergencyTraffic.isRightGo(), "Right direction green");
        test.test(!emergencyTraffic.isLeftGo() && !emergencyTraffic.isCenterGo(), 
                 "Other directions red when Right is green");
        
        emergencyTraffic.goLeft();
        test.test(emergencyTraffic.isLeftGo(), "Left direction green");
        test.test(!emergencyTraffic.isRightGo() && !emergencyTraffic.isCenterGo(),
                 "Other directions red when Left is green");
        
        emergencyTraffic.goCenter();
        test.test(emergencyTraffic.isCenterGo(), "Center direction green");
        test.test(!emergencyTraffic.isRightGo() && !emergencyTraffic.isLeftGo(),
                 "Other directions red when Center is green");
        
        emergencyTraffic.allStop();
        test.test(!emergencyTraffic.isRightGo() && 
                 !emergencyTraffic.isLeftGo() && 
                 !emergencyTraffic.isCenterGo(),
                 "All directions red in allStop()");
    }

    void testNormalOperation() {
        Serial.println(F("\n[TEST] Normal Operation"));
        
        test.test(!dht.isCritical(), "Initial safe conditions");
        test.test(barrier.status(), "Barrier initially raised");
        test.test(trafficLight.getPhase() == PHASE_RED, "Traffic light red phase");
        
        for(int i = 0; i < 10; i++) {
            advanceTime(1000);
            updateSystem();
        }
        
        test.test(trafficLight.getPhase() != PHASE_RED, "Traffic light changed from red");
    }

    void testFireEmergency() {
        Serial.println(F("\n[TEST] Fire Emergency"));
        
        dht.setTestValues(45.0, 20.0);
        emergencyStartTime = TestUtils::mockMillis;
        advanceTime(2000);
        updateSystem();
        
        test.test(dht.isCritical(), "Fire conditions detected");
        test.test(emergencyActive, "Emergency mode activated");
        
        barrier.lower();
        for(int i = 0; i < 20; i++) {
            advanceTime(100);
            updateSystem();
        }
        test.test(!barrier.status(), "Barrier deployed");
        test.test(trafficLight.getPhase() == PHASE_RED, "Emergency red lights");
    }

    void testObstacleDetection() {
        Serial.println(F("\n[TEST] Obstacle Detection"));
        
        barrier.lower();
        advanceTime(100);
        updateSystem();
        
        distance.setTestDistance(15.0);
        advanceTime(100);
        updateSystem();
        
        test.test(barrier.isStopped(), "Barrier stopped for obstacle");
        
        distance.setTestDistance(300.0);
        advanceTime(100);
        updateSystem();
        
        test.test(!barrier.isStopped(), "Barrier resumed after clear");
    }

    void testSystemRecovery() {
        Serial.println(F("\n[TEST] System Recovery"));
        
        dht.setTestValues(22.0, 55.0);
        advanceTime(30000);
        updateSystem();
        
        test.test(!emergencyActive, "Emergency mode deactivated");
        test.test(barrier.status(), "Barrier raised during recovery");
    }

    void runAllTests() {
        begin();
        testNormalOperation();
        testFireEmergency();
        testEmergencyTraffic();  // NEW TEST
        testObstacleDetection();
        testSystemRecovery();
        
        Serial.println(F("\n=== TEST SUMMARY ==="));
        test.summary();
    }

private:
    void updateSystem() {
        dht.update();
        distance.update();
        barrier.update();
        trafficLight.update();
        
        if (dht.isCritical() && !emergencyActive) {
            emergencyActive = true;
            emergencyStartTime = TestUtils::mockMillis;
            activateEmergencyMode();
        }
        
        if (emergencyActive && !dht.isCritical() && 
            (TestUtils::mockMillis - emergencyStartTime > 30000)) {
            emergencyActive = false;
            deactivateEmergencyMode();
        }
        
        updateTestDisplay();
    }

    void updateTestDisplay() {
        if (emergencyActive) {
            lcd.displayDisasterWarning(1, FIRE, dht.getTemperature(), "TEST EMERGENCY");
        } else {
            String line1 = "Test T:" + String(dht.getTemperature(), 1) + "C";
            String line2 = "Dist:" + String(distance.getDistance(), 0) + "cm";
            lcd.displayStatic(line1, line2);
        }
    }
};

#endif
