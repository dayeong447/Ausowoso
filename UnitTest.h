#ifndef DISASTER_STRESS_TEST_H
#define DISASTER_STRESS_TEST_H

#include "EmergencyTraffic.h"
#include "TrafficLightHandler.h"
#include "BarrierServoHandler.h"
#include "DistanceSensorHandler.h"
#include "HeatSensorHandler.h"
#include "LcdDisplayHandler.h"
#include "BuzzerHandler.h"
#include "GasSensorHandler.h"

// STRESS TEST SCENARIOS BASED ON BONDOWOSO DISASTER CONDITIONS
namespace StressTestScenarios {
    // Bondowoso Mountain Fire Scenarios
    struct FireScenario {
        float temperature;
        float humidity;
        const char* description;
        bool shouldTriggerEmergency;
    };
    
    // Mountain evacuation direction scenarios (Left/Center/Right)
    struct EvacuationScenario {
        bool leftBlocked;
        bool centerBlocked;
        bool rightBlocked;
        const char* expectedRoute;
    };
    
    // Multi-disaster stress scenarios
    struct MultiDisasterScenario {
        bool fireActive;
        bool gasLeak;
        bool obstacleDetected;
        float distance;
        const char* priority;
    };
}

class SmartCityUnitTest {
private:
    uint16_t testsPassed = 0;
    uint16_t testsFailed = 0;
    uint16_t stressTestIterations = 0;
    
    // System components
    DHTSensor dht;
    DistanceSensorHandler distance;
    BarrierControl barrier;
    LCDDisplay lcd;
    TrafficLight trafficLight;
    EmergencyTraffic emergencyTraffic;
    Buzzer buzzer;
    GasSensor gasSensor;
    
    // Test state tracking
    bool emergencyActive = false;
    unsigned long testStartTime = 0;
    unsigned long maxResponseTime = 0;
    
    void logTest(bool condition, const char* testName) {
        if (condition) {
            testsPassed++;
            Serial.print(F("✓ "));
        } else {
            testsFailed++;
            Serial.print(F("✗ "));
        }
        Serial.println(testName);
        delay(20); // Prevent serial buffer overflow
    }

public:
    SmartCityUnitTest(uint8_t dhtPin, uint8_t trigPin, uint8_t echoPin, uint8_t servoPin, 
                           uint8_t redPin, uint8_t yellowPin, uint8_t greenPin,
                           uint8_t rightR, uint8_t rightG, uint8_t leftR, uint8_t leftG,
                           uint8_t centerR, uint8_t centerG, uint8_t buzzerPin, uint8_t gasPin) :
        dht(dhtPin), distance(trigPin, echoPin), barrier(servoPin),
        trafficLight(redPin, yellowPin, greenPin),
        emergencyTraffic(rightR, rightG, leftR, leftG, centerR, centerG),
        buzzer(buzzerPin), gasSensor(gasPin) {}

    void initialize() {
        Serial.println(F("\n🏔️ BONDOWOSO DISASTER STRESS TEST SUITE 🏔️"));
        Serial.println(F("Testing mountain evacuation scenarios..."));
        
        // Set test mode for all sensors
        dht.setTestMode(true);
        distance.setTestMode(true);
        gasSensor.setTestMode(true);
        
        // Initialize all components
        lcd.initialize();
        barrier.initialize();
        trafficLight.initialize();
        emergencyTraffic.initialize();
        gasSensor.initialize();
        
        // Set initial safe conditions
        dht.setTestValues(22.0, 55.0);
        distance.setTestDistance(300.0);
        gasSensor.setTestPPM(400.0);
        barrier.raise();
        
        testStartTime = millis();
        Serial.println(F("✅ Stress test environment ready\n"));
    }

    // TEST 1: Bondowoso Mountain Fire Scenarios
    void testMountainFireScenarios() {
        Serial.println(F("🔥 [STRESS TEST 1] Mountain Fire Scenarios"));
        
        StressTestScenarios::FireScenario fireTests[] = {
            {45.0, 15.0, "Severe mountain fire (high temp, low humidity)", true},
            {38.0, 25.0, "Moderate fire conditions", true},
            {30.0, 20.0, "Early fire detection", true},
            {55.0, 35.0, "Extreme heat with moderate humidity", true},
            {25.0, 60.0, "Normal mountain conditions", false},
            {20.0, 80.0, "High humidity (fire unlikely)", false}
        };
        
        for (int i = 0; i < 6; i++) {
            auto scenario = fireTests[i];
            dht.setTestValues(scenario.temperature, scenario.humidity);
            
            unsigned long responseStart = millis();
            updateSystemState();
            unsigned long responseTime = millis() - responseStart;
            
            if (responseTime > maxResponseTime) maxResponseTime = responseTime;
            
            bool emergencyDetected = dht.isCritical();
            bool testPassed = (emergencyDetected == scenario.shouldTriggerEmergency);
            
            Serial.print(F("  Fire Test "));
            Serial.print(i + 1);
            Serial.print(F(": "));
            Serial.print(scenario.description);
            Serial.print(F(" ("));
            Serial.print(responseTime);
            Serial.print(F("ms) - "));
            
            logTest(testPassed, testPassed ? "PASS" : "FAIL");
            
            delay(100);
        }
    }

    // TEST 2: Mountain Evacuation Route Testing
    void testEvacuationRoutes() {
        Serial.println(F("\n🗻 [STRESS TEST 2] Mountain Evacuation Routes"));
        
        StressTestScenarios::EvacuationScenario routeTests[] = {
            {false, false, true, "Left route clear"},
            {true, false, false, "Center route clear"},
            {false, true, false, "Right route clear"},
            {true, true, false, "Only right route available"},
            {false, true, true, "Only left route available"},
            {true, false, true, "Only center route available"}
        };
        
        // Activate emergency mode first
        activateEmergencyMode();
        
        for (int i = 0; i < 6; i++) {
            auto scenario = routeTests[i];
            
            // Test route configuration
            emergencyTraffic.allStop(); // Reset all lights
            
            if (!scenario.leftBlocked) {
                emergencyTraffic.setLeft(true);
            }
            if (!scenario.centerBlocked) {
                emergencyTraffic.setCenter(true);
            }
            if (!scenario.rightBlocked) {
                emergencyTraffic.setRight(true);
            }
            
            // Verify route configuration
            bool leftOk = !scenario.leftBlocked == emergencyTraffic.isLeftGo();
            bool centerOk = !scenario.centerBlocked == emergencyTraffic.isCenterGo();
            bool rightOk = !scenario.rightBlocked == emergencyTraffic.isRightGo();
            
            bool routeTestPassed = leftOk && centerOk && rightOk;
            
            Serial.print(F("  Route Test "));
            Serial.print(i + 1);
            Serial.print(F(": "));
            logTest(routeTestPassed, scenario.expectedRoute);
            
            delay(150);
        }
    }

    // TEST 3: Multi-Disaster Stress Scenarios
    void testMultiDisasterScenarios() {
        Serial.println(F("\n⚡ [STRESS TEST 3] Multi-Disaster Scenarios"));
        
        StressTestScenarios::MultiDisasterScenario multiTests[] = {
            {true, true, false, 300.0, "Fire + Gas Leak"},
            {true, false, true, 5.0, "Fire + Obstacle"},
            {false, true, true, 3.0, "Gas Leak + Obstacle"},
            {true, true, true, 2.0, "Triple disaster (Fire+Gas+Obstacle)"},
            {false, false, true, 1.0, "Obstacle only"},
            {true, false, false, 400.0, "Fire only"}
        };
        
        for (int i = 0; i < 6; i++) {
            auto scenario = multiTests[i];
            
            // Setup scenario conditions
            if (scenario.fireActive) {
                dht.setTestValues(45.0, 20.0);
            } else {
                dht.setTestValues(22.0, 55.0);
            }
            
            if (scenario.gasLeak) {
                gasSensor.setTestPPM(2500.0);
            } else {
                gasSensor.setTestPPM(400.0);
            }
            
            distance.setTestDistance(scenario.distance);
            
            unsigned long responseStart = millis();
            updateSystemState();
            unsigned long responseTime = millis() - responseStart;
            
            // Check system response
            bool fireDetected = dht.isCritical();
            bool gasDetected = gasSensor.isDanger();
            bool obstacleDetected = distance.isObjectDetected(7.0);
            bool barrierResponse = checkBarrierResponse(scenario);
            bool trafficResponse = (trafficLight.getPhase() == PHASE_RED);
            
            bool systemResponseOk = (fireDetected == scenario.fireActive) &&
                                   (gasDetected == scenario.gasLeak) &&
                                   (obstacleDetected == scenario.obstacleDetected) &&
                                   barrierResponse && trafficResponse;
            
            Serial.print(F("  Multi-Disaster "));
            Serial.print(i + 1);
            Serial.print(F(": "));
            Serial.print(scenario.priority);
            Serial.print(F(" ("));
            Serial.print(responseTime);
            Serial.print(F("ms) - "));
            
            logTest(systemResponseOk, systemResponseOk ? "SYSTEM RESPONSE OK" : "SYSTEM RESPONSE FAILED");
            
            delay(200);
        }
    }

    // TEST 4: System Performance Under Load
    void testSystemPerformanceStress() {
        Serial.println(F("\n⚡ [STRESS TEST 4] System Performance Under Load"));
        
        unsigned long totalResponseTime = 0;
        int performanceTests = 50;
        int failureCount = 0;
        
        Serial.print(F("Running "));
        Serial.print(performanceTests);
        Serial.println(F(" rapid-fire disaster scenarios..."));
        
        for (int i = 0; i < performanceTests; i++) {
            // Randomize disaster conditions
            float temp = random(200, 600) / 10.0; // 20.0 to 60.0
            float humidity = random(100, 900) / 10.0; // 10.0 to 90.0
            float gasLevel = random(3000, 50000) / 10.0; // 300 to 5000 ppm
            float dist = random(10, 4000) / 10.0; // 1.0 to 400.0 cm
            
            dht.setTestValues(temp, humidity);
            gasSensor.setTestPPM(gasLevel);
            distance.setTestDistance(dist);
            
            unsigned long start = millis();
            updateSystemState();
            unsigned long responseTime = millis() - start;
            
            totalResponseTime += responseTime;
            
            // Check for system failures (response time > 100ms is concerning)
            if (responseTime > 100) {
                failureCount++;
            }
            
            if (i % 10 == 0) {
                Serial.print(F("."));
            }
            
            delay(50); // Rapid testing
        }
        
        Serial.println();
        float avgResponseTime = totalResponseTime / (float)performanceTests;
        float failureRate = (failureCount * 100.0) / performanceTests;
        
        Serial.print(F("  Average Response Time: "));
        Serial.print(avgResponseTime, 1);
        Serial.println(F("ms"));
        
        Serial.print(F("  Failure Rate: "));
        Serial.print(failureRate, 1);
        Serial.println(F("%"));
        
        Serial.print(F("  Max Response Time: "));
        Serial.print(maxResponseTime);
        Serial.println(F("ms"));
        
        logTest(avgResponseTime < 50.0, "Average response time acceptable");
        logTest(failureRate < 10.0, "Failure rate acceptable");
        logTest(maxResponseTime < 200, "Max response time acceptable");
    }

    // TEST 5: Memory Stress Test
    void testMemoryStress() {
        Serial.println(F("\n🧠 [STRESS TEST 5] Memory Stress Test"));
        
        // Monitor free memory during intensive operations
        extern int __heap_start, *__brkval; 
        int v; 
        int initialMemory = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
        
        Serial.print(F("  Initial Free Memory: "));
        Serial.print(initialMemory);
        Serial.println(F(" bytes"));
        
        // Stress test with rapid LCD updates and String operations
        for (int i = 0; i < 20; i++) {
            // Simulate heavy LCD usage
            char regionStr[20];
            sprintf(regionStr, "Region %02d", i + 1);
            
            lcd.displayDisasterWarning(i + 1, FIRE, 45.5 + i, "EVACUATE NOW!");
            delay(100);
            
            lcd.displayDisasterWarning(i + 1, GAS_LEAK, 2500.0 + i * 100, "GAS DANGER!");
            delay(100);
            
            // Check memory after operations
            int currentMemory = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
            
            if (i % 5 == 0) {
                Serial.print(F("  Memory after "));
                Serial.print(i + 1);
                Serial.print(F(" iterations: "));
                Serial.print(currentMemory);
                Serial.println(F(" bytes"));
            }
        }
        
        int finalMemory = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
        int memoryLeak = initialMemory - finalMemory;
        
        Serial.print(F("  Final Free Memory: "));
        Serial.print(finalMemory);
        Serial.println(F(" bytes"));
        
        Serial.print(F("  Memory Leak: "));
        Serial.print(memoryLeak);
        Serial.println(F(" bytes"));
        
        logTest(memoryLeak < 100, "Memory leak acceptable");
        logTest(finalMemory > 200, "Sufficient memory remaining");
    }

    // Main stress test runner
    void runAllStressTests() {
        initialize();
        
        testMountainFireScenarios();
        testEvacuationRoutes();
        testMultiDisasterScenarios();
        testSystemPerformanceStress();
        testMemoryStress();
        
        printStressTestSummary();
    }

private:
    void updateSystemState() {
        dht.update();
        distance.update();
        barrier.update();
        trafficLight.update();
        gasSensor.update();
        
        // Check emergency conditions
        bool fireDetected = dht.isCritical();
        bool gasDetected = gasSensor.isDanger();
        
        if ((fireDetected || gasDetected) && !emergencyActive) {
            activateEmergencyMode();
        }
    }
    
    void activateEmergencyMode() {
        emergencyActive = true;
        trafficLight.setPhase(PHASE_RED);
        emergencyTraffic.allStop();
        barrier.lower();
    }
    
    bool checkBarrierResponse(const StressTestScenarios::MultiDisasterScenario& scenario) {
        if (scenario.fireActive || scenario.gasLeak) {
            return !barrier.status(); // Should be lowered
        }
        return true; // Any state acceptable for non-emergency
    }
    
    void printStressTestSummary() {
        unsigned long testDuration = millis() - testStartTime;
        
        Serial.println(F("\n🏔️ BONDOWOSO STRESS TEST SUMMARY 🏔️"));
        Serial.print(F("Total Tests Run: "));
        Serial.println(testsPassed + testsFailed);
        Serial.print(F("Tests Passed: "));
        Serial.println(testsPassed);
        Serial.print(F("Tests Failed: "));
        Serial.println(testsFailed);
        Serial.print(F("Success Rate: "));
        if (testsPassed + testsFailed > 0) {
            Serial.print((testsPassed * 100) / (testsPassed + testsFailed));
            Serial.println(F("%"));
        }
        Serial.print(F("Test Duration: "));
        Serial.print(testDuration / 1000.0, 1);
        Serial.println(F(" seconds"));
        Serial.print(F("Max Response Time: "));
        Serial.print(maxResponseTime);
        Serial.println(F("ms"));
        
        if (testsFailed == 0) {
            Serial.println(F("🎉 ALL STRESS TESTS PASSED!"));
            Serial.println(F("System ready for Bondowoso deployment!"));
        } else {
            Serial.println(F("⚠️ SOME STRESS TESTS FAILED"));
            Serial.println(F("System needs optimization before deployment"));
        }
        
        Serial.println(F("==========================================="));
    }
};

#endif
