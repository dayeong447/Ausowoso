#ifndef UNITTEST_H
#define UNITTEST_H

// Compact Test Framework - Memory Optimized
class CompactTest {
public:
    uint8_t passed = 0;
    uint8_t failed = 0;
    
    void test(bool condition, const char* msg) {
        if (condition) {
            Serial.print(F("✓ "));
            passed++;
        } else {
            Serial.print(F("✗ "));
            failed++;
        }
        Serial.println(msg);
    }
    
    void summary() {
        Serial.print(F("\nPassed: ")); Serial.print(passed);
        Serial.print(F(" Failed: ")); Serial.println(failed);
        Serial.print(F("Rate: ")); Serial.print((passed * 100) / (passed + failed)); Serial.println(F("%"));
    }
};

// Compact Smart City Test
class SmartCityCompactTest {
private:
    unsigned long testTime = 0;
    DHTSensor dht;
    DistanceSensorHandler distance;
    BarrierControl barrier;
    LCDDisplay lcd;
    TrafficLight trafficLight;
    CompactTest test;
    bool emergencyActive = false;

public:
    SmartCityCompactTest() : dht(7), distance(5, 6), barrier(9) {}

    void init() {
        Serial.println(F("🚀 Smart City Init"));
        lcd.initialize();
        dht.initialize();
        distance.initialize();
        barrier.initialize();
        barrier.raise();
        trafficLight.initialize();
        
        // Set safe initial values
        dht.temperature = 25.0;
        dht.humidity = 60.0;
        distance.distance = 300.0;
        emergencyActive = false;
        
        Serial.println(F("✅ Ready"));
    }

    void advanceTime(unsigned long ms) {
        testTime += ms;
        mockMillis = testTime;
    }

    void setFire() {
        dht.temperature = 135.0;  // Critical fire temp
        dht.humidity = 25.0;      // Low humidity
        emergencyActive = true;
    }

    void setObstacle(float dist) {
        distance.distance = dist;
    }

    void clearObstacle() {
        distance.distance = 300.0;
    }

    void updateSystem() {
        dht.update();
        distance.update();
        
        // Emergency detection
        if (dht.isCritical() && !emergencyActive) {
            emergencyActive = true;
            activateEmergency();
        }
        
        // Barrier safety - stop for obstacles
        if (barrier.isInMotion() && distance.isObjectDetected(50.0)) {
            if (!barrier.isStopped()) {
                barrier.stop();
            }
        }
        // Resume when clear
        else if (barrier.isStopped() && !distance.isObjectDetected(50.0)) {
            barrier.resume();
        }
        
        barrier.update();
        trafficLight.update();
    }

    void activateEmergency() {
        // Set emergency red lights
        trafficLight.setLights(HIGH, LOW, LOW);
        
        // Display emergency on LCD
        lcd.displayEmergency(3, FIRE, dht.temperature, F("Evacuate Now!"), true);
    }

    void runCompleteTest() {
        Serial.println(F("\n=== COMPLETE DISASTER TEST ==="));
        
        // 1. Initial Safe State
        test.test(!dht.isCritical(), "Initial: Safe state");
        test.test(barrier.status(), "Initial: Barrier up");
        test.test(trafficLight.getPhase() == PHASE_RED, "Initial: Red phase");
        
        advanceTime(1000);
        updateSystem();
        
        // 2. Fire Emergency Detection
        Serial.println(F("\n🔥 FIRE DETECTED"));
        setFire();
        advanceTime(2000);
        updateSystem();
        
        test.test(dht.isCritical(), "Fire: Critical detected");
        test.test(emergencyActive, "Fire: Emergency mode");
        test.test(mockRedState && !mockYellowState && !mockGreenState, "Fire: Red lights only");
        
        // 3. Barrier Activation
        Serial.println(F("\n🚧 BARRIER ACTIVATION"));
        barrier.lower();
        advanceTime(500);
        updateSystem();
        
        test.test(barrier.isInMotion(), "Barrier: Moving down");
        test.test(!barrier.status(), "Barrier: Target lowered");
        
        // 4. Obstacle Detection & Safety
        Serial.println(F("\n⚠️ OBSTACLE SAFETY"));
        setObstacle(35.0);  // Obstacle in path
        advanceTime(200);
        updateSystem();
        
        test.test(barrier.isStopped(), "Safety: Stopped for obstacle");
        test.test(distance.isObjectDetected(50.0), "Safety: Obstacle detected");
        
        // 5. Clear Path & Resume
        Serial.println(F("\n✅ PATH CLEAR"));
        clearObstacle();
        advanceTime(200);
        updateSystem();
        
        test.test(!barrier.isStopped(), "Resume: Barrier moving");
        test.test(!distance.isObjectDetected(50.0), "Resume: Path clear");
        
        // 6. Complete Barrier Descent
        Serial.println(F("\n⏳ COMPLETING DESCENT"));
        uint8_t maxLoop = 30;  // Prevent infinite loop
        while(barrier.isInMotion() && maxLoop-- > 0) {
            advanceTime(100);
            updateSystem();
        }
        
        test.test(!barrier.isInMotion(), "Complete: Barrier stopped");
        test.test(!barrier.status(), "Complete: Barrier down");
        
        // 7. System Integration Check
        test.test(emergencyActive, "Integration: Emergency active");
        test.test(dht.isCritical(), "Integration: Fire conditions");
        test.test(mockRedState, "Integration: Emergency lights");
        
        // 8. Optional: Test Recovery
        Serial.println(F("\n🧯 RECOVERY SIM"));
        dht.temperature = 25.0;
        dht.humidity = 60.0;
        advanceTime(2000);
        updateSystem();
        
        test.test(!dht.isCritical(), "Recovery: Normal conditions");
        
        test.summary();
        
        if (test.failed == 0) {
            Serial.println(F("🎉 ALL TESTS PASSED! 🎉"));
        } else {
            Serial.println(F("⚠️ SOME TESTS FAILED ⚠️"));
        }
    }

    // Quick stress test for obstacles
    void obstacleStressTest() {
        Serial.println(F("\n=== OBSTACLE STRESS TEST ==="));
        
        barrier.lower();
        
        for (uint8_t i = 0; i < 3; i++) {
            // Create obstacle
            setObstacle(25.0 + (i * 10));
            advanceTime(100);
            updateSystem();
            test.test(barrier.isStopped(), "Stress: Stop");
            
            // Clear obstacle
            clearObstacle();
            advanceTime(100);
            updateSystem();
            test.test(!barrier.isStopped(), "Stress: Resume");
        }
        
        Serial.println(F("✅ Stress test done"));
    }
};
#endif
