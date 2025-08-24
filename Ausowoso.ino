#include <Arduino.h>
#include <SoftwareSerial.h>
#include "TrafficLightHandler.h"
#include "LcdDisplayHandler.h"
#include "BarrierServoHandler.h"
#include "HeatSensorHandler.h"
#include "DistanceSensorHandler.h"
#include "EmergencyTraffic.h"
#include "BuzzerHandler.h"
#include "GasSensorHandler.h"

// System Configuration
#define SYSTEM_UPDATE_INTERVAL 100    // Main loop update interval (ms)
#define EMERGENCY_CHECK_INTERVAL 500  // Emergency check interval (ms)
#define STATUS_DISPLAY_INTERVAL 1000  // Status display update interval (ms) - Reduced from 2000
#define SERIAL_BAUD_RATE 115200

// Pin Configuration
#define DHT_SENSOR_PIN 7
#define BUZZER_PIN 1
#define GAS_SENSOR_PIN A0
#define DISTANCE_TRIGGER_PIN 5
#define DISTANCE_ECHO_PIN 6
#define BARRIER_SERVO_PIN 9
#define TRAFFIC_RED_PIN 2
#define TRAFFIC_YELLOW_PIN 3
#define TRAFFIC_GREEN_PIN 4
#define RIGHT_RED_PIN 8
#define RIGHT_GREEN_PIN 9
#define LEFT_RED_PIN 10
#define LEFT_GREEN_PIN 11
#define CENTER_RED_PIN 12
#define CENTER_GREEN_PIN 13


// Emergency thresholds
#define FIRE_TEMP_THRESHOLD 25.0      // °C
#define FIRE_HUMIDITY_THRESHOLD 30.0  // %
#define OBSTACLE_DISTANCE_THRESHOLD 7.0 // cm
#define CRITICAL_TEMP_THRESHOLD 25.0  // °C for critical emergency
#define WARNING_GAS_THRESHOLD 150.0
#define DANGER_GAS_THRESHOLD 800.0

// System States
enum SystemState {
  NORMAL_OPERATION,
  EMERGENCY_DETECTED,
  BARRIER_DEPLOYING,
  EVACUATION_MODE,
  SYSTEM_ERROR
};

// Global Objects
DHTSensor dhtSensor(DHT_SENSOR_PIN);
Buzzer buzzer(BUZZER_PIN);
DistanceSensorHandler distanceSensor(DISTANCE_TRIGGER_PIN, DISTANCE_ECHO_PIN);
BarrierControl barrierControl(BARRIER_SERVO_PIN);
LCDDisplay lcdDisplay;
GasSensor co2Sensor(GAS_SENSOR_PIN);
TrafficLight trafficLight(TRAFFIC_RED_PIN, TRAFFIC_YELLOW_PIN, TRAFFIC_GREEN_PIN);
EmergencyTraffic emergencyLight(RIGHT_RED_PIN, RIGHT_GREEN_PIN,
                        LEFT_RED_PIN, LEFT_GREEN_PIN,
                        CENTER_RED_PIN, CENTER_GREEN_PIN);

// System Variables
SystemState currentState = NORMAL_OPERATION;
SystemState previousState = NORMAL_OPERATION;
unsigned long lastEmergencyCheck = 0;
unsigned long lastStatusUpdate = 0;
unsigned long lastSystemUpdate = 0;
unsigned long emergencyStartTime = 0;
bool barrierDeployed = false;
bool systemInitialized = false;
bool forceDisplayUpdate = false;  // Flag to force immediate LCD update

// EMERGENCY LIGHT PATTERN
const char EMERGENCY_PATTERN = '1'; // RRR (semua merah)
const char NORMAL_PATTERNS[4] = {'2','3','4','1'}; // GYY, YGY, YYG, RRR
int currentNormalPattern = 0;
unsigned long lastPatternUpdate = 0;


// Function Declarations
void initializeSystem();
void updateSensors();
void checkEmergencyConditions();
void handleSystemState();
void updateDisplay();
void forceUpdateDisplay();  // New function for immediate update
void handleTrafficControl();
void handleBarrierControl();
void logSystemStatus();
void activateEmergencyMode();
void deactivateEmergencyMode();
void displaySystemInfo();


SoftwareSerial slave1(10, 11);
SoftwareSerial slave2(12, 13);


//#define RUN_UNIT_TESTS
#define RUN_MASTER
//#define RUN_SLAVE
#ifdef RUN_UNIT_TESTS
#include "UnitTest.h"

SmartCityUnitTest testSuite(
    DHT_SENSOR_PIN, DISTANCE_TRIGGER_PIN, DISTANCE_ECHO_PIN, BARRIER_SERVO_PIN,
    TRAFFIC_RED_PIN, TRAFFIC_YELLOW_PIN, TRAFFIC_GREEN_PIN,
    RIGHT_RED_PIN, RIGHT_GREEN_PIN, 
    LEFT_RED_PIN, LEFT_GREEN_PIN,
    CENTER_RED_PIN, CENTER_GREEN_PIN, 
    BUZZER_PIN, GAS_SENSOR_PIN
);

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  testSuite.runAllStressTests();
}
void loop() {
  
}
#elif defined(RUN_MASTER)

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  slave1.begin(SERIAL_BAUD_RATE);
  slave2.begin(SERIAL_BAUD_RATE);
  
  Serial.println(F("================================"));
  Serial.println(F("🌆 SMART CITY DISASTER SYSTEM"));
  Serial.println(F("================================"));
  Serial.println(F("Initializing hardware..."));
  
  initializeSystem();
  
  Serial.println(F("✅ System Ready!"));
  Serial.println(F("================================"));
}

void loop() {
  unsigned long currentTime = millis();
  
  // Main system update loop
  if (currentTime - lastSystemUpdate >= SYSTEM_UPDATE_INTERVAL) {
    updateSensors();
    handleBarrierControl();
    handleTrafficControl();
    lastSystemUpdate = currentTime;
  }
  
  // Emergency condition checking
  if (currentTime - lastEmergencyCheck >= EMERGENCY_CHECK_INTERVAL) {
    checkEmergencyConditions();
    handleSystemState();
    lastEmergencyCheck = currentTime;
  }
  
  // Display updates (regular interval OR forced update)
  if (currentTime - lastStatusUpdate >= STATUS_DISPLAY_INTERVAL || forceDisplayUpdate) {
    updateDisplay();
    logSystemStatus();
    lastStatusUpdate = currentTime;
    forceDisplayUpdate = false;  // Reset force flag
  }
  
  
  // Small delay to prevent overwhelming the system
  delay(10);
}
#elif defined(RUN_SLAVE)

#endif

void updateSensors() {
  // Update all sensors
  dhtSensor.update();
  distanceSensor.update();
  barrierControl.update();
  co2Sensor.update();
}

void checkEmergencyConditions() {
  if (!systemInitialized) return;
  
  float temperature = dhtSensor.getTemperature();
  float humidity = dhtSensor.getHumidity();
  bool isCritical = dhtSensor.isCritical();
  bool isGasLeak = co2Sensor.isDanger();
  

  
  // Check for fire conditions
  bool fireDetected = (temperature > FIRE_TEMP_THRESHOLD && humidity < FIRE_HUMIDITY_THRESHOLD) || 
                     (temperature > CRITICAL_TEMP_THRESHOLD);
  
  // State transitions
  switch (currentState) {
    case NORMAL_OPERATION:
      if (fireDetected || isCritical || isGasLeak) {
        activateEmergencyMode();
        currentState = EMERGENCY_DETECTED;   
        emergencyStartTime = millis();
        forceDisplayUpdate = true;  // Force immediate LCD update
        Serial.println(F("🚨 EMERGENCY TRIGGERED - Updating LCD..."));
       
      }
      break;
      
    case EMERGENCY_DETECTED:
      if (!barrierDeployed && !barrierControl.isInMotion()) {
        barrierControl.lower();
        currentState = BARRIER_DEPLOYING;
        forceDisplayUpdate = true;  // Force immediate LCD update
        Serial.println(F("🚧 BARRIER DEPLOYING - Updating LCD..."));
      }
      break;
      
    case BARRIER_DEPLOYING:
      if (!barrierControl.isInMotion()) {
        barrierDeployed = true;
        currentState = EVACUATION_MODE;
        forceDisplayUpdate = true;  // Force immediate LCD update
        Serial.println(F("🏃 EVACUATION MODE - Updating LCD..."));
      }
      break;
      
    case EVACUATION_MODE:
      // Stay in evacuation mode until manually reset or conditions clear
      if (!fireDetected && !isCritical && !isGasLeak && (millis() - emergencyStartTime > 30000)) {
        // Auto-recovery after 30 seconds if conditions are safe
        deactivateEmergencyMode();
      }
      break;
      
    case SYSTEM_ERROR:
      // Manual reset required
      break;
  }
}

void handleSystemState() {
  if (currentState != previousState) {
    Serial.print(F("🔄 State Change: "));
    
    // Print state names dengan PROGMEM strings
    printStateName(previousState);
    Serial.print(F(" -> "));
    printStateName(currentState);
    Serial.println();
    
    previousState = currentState;
    forceDisplayUpdate = true;
  }
}
void sendTrafficPatterns(bool emergencyMode) {
  if (emergencyMode) {
    // Kirim pola emergency ke semua slave
    slave1.print(EMERGENCY_PATTERN);
    slave1.print(EMERGENCY_PATTERN);
    slave2.print(EMERGENCY_PATTERN);
    slave2.print(EMERGENCY_PATTERN);
  } else {
    // Kirim pola normal (berputar)
    char pattern1 = NORMAL_PATTERNS[currentNormalPattern];
    char pattern2 = NORMAL_PATTERNS[(currentNormalPattern + 1) % 4];
    char pattern3 = NORMAL_PATTERNS[(currentNormalPattern + 2) % 4];
    char pattern4 = NORMAL_PATTERNS[(currentNormalPattern + 3) % 4];
    
    slave1.print(pattern1);
    slave1.print(pattern2);
    slave2.print(pattern3);
    slave2.print(pattern4);
    
    currentNormalPattern = (currentNormalPattern + 1) % 4;
  }
}

void printStateName(SystemState state) {
  switch(state) {
    case NORMAL_OPERATION: Serial.print(F("NORMAL")); break;
    case EMERGENCY_DETECTED: Serial.print(F("EMERGENCY")); break;
    case BARRIER_DEPLOYING: Serial.print(F("DEPLOYING")); break;
    case EVACUATION_MODE: Serial.print(F("EVACUATION")); break;
    case SYSTEM_ERROR: Serial.print(F("ERROR")); break;
  }
}

void initializeSystem() {
  // Initialize LCD Display
  lcdDisplay.initialize();
  lcdDisplay.displayStatic(F("System Init"), F("Starting..."));
  delay(1000);
  
  // Initialize DHT Sensor
  dhtSensor.initialize();
  lcdDisplay.displayStatic(F("DHT Sensor"), F("Initialized"));
  delay(500);
  
  // Initialize Gas Sensor
  co2Sensor.initialize();
  co2Sensor.setThresholds(WARNING_GAS_THRESHOLD, DANGER_GAS_THRESHOLD);
  lcdDisplay.displayStatic(F("Gas Sensor"), F("Initialized"));
  delay(500);
  
  // Initialize Distance Sensor
  distanceSensor.initialize();
  lcdDisplay.displayStatic(F("Distance Sensor"), F("Initialized"));
  delay(500);
  
  // Initialize Barrier Control
  barrierControl.initialize();
  barrierControl.raise();
  lcdDisplay.displayStatic(F("Barrier Control"), F("Initialized"));
  delay(500);
  
  // Initialize Traffic Light
  trafficLight.initialize();
  lcdDisplay.displayStatic(F("Traffic Light"), F("Initialized"));
  delay(500);
  
  // Set initial safe values
  currentState = NORMAL_OPERATION;
  barrierDeployed = false;
  systemInitialized = true;
  
  lcdDisplay.displayStatic(F("System Ready"), F("Normal Operation"));
  delay(1000);
  
  // Force initial display update
  forceDisplayUpdate = true;
}


void updateDisplay() {
  float temperature = dhtSensor.getTemperature();
  float humidity = dhtSensor.getHumidity();
  float distance = distanceSensor.getDistance();
  
  Serial.print(F("📺 Updating LCD - State: "));
  
  switch (currentState) {
    case NORMAL_OPERATION:
      {
        Serial.println(F("NORMAL"));
        
        // OPTIMIZED: Direct LCD printing instead of String concatenation
        // This saves significant RAM by avoiding String object creation
        lcdDisplay.lcd.clear();
        lcdDisplay.lcd.setCursor(0, 0);
        lcdDisplay.lcd.print(F("T:"));
        lcdDisplay.lcd.print(temperature, 1);
        lcdDisplay.lcd.print(F("C H:"));
        lcdDisplay.lcd.print(humidity, 0);
        lcdDisplay.lcd.print(F("%"));
        
        lcdDisplay.lcd.setCursor(0, 1);
        lcdDisplay.lcd.print(F("Dist:"));
        lcdDisplay.lcd.print(distance, 0);
        lcdDisplay.lcd.print(F("cm NORMAL"));
      }
      break;
      
    case EMERGENCY_DETECTED:
      Serial.println(F("EMERGENCY"));
      lcdDisplay.displayDisasterWarning(1, FIRE, temperature, "FIRE DETECTED!");
      break;
      
    case BARRIER_DEPLOYING:
      Serial.println(F("DEPLOYING"));
      lcdDisplay.displayDisasterWarning(1, FIRE, temperature, "BARRIER DEPLOY");
      break;
      
    case EVACUATION_MODE:
      Serial.println(F("EVACUATION"));
      lcdDisplay.displayDisasterWarning(1, FIRE, temperature, "EVACUATE NOW!");
      break;
      
    case SYSTEM_ERROR:
      Serial.println(F("ERROR"));
      lcdDisplay.displayStatic("SYSTEM ERROR", "Check Hardware");
      break;
  }
}

// New function for immediate display update
void forceUpdateDisplay() {
  Serial.println(F("🔥 FORCE LCD UPDATE"));
  forceDisplayUpdate = true;
  updateDisplay();
}

void handleTrafficControl() {
  switch (currentState) {
    case NORMAL_OPERATION:
      // Normal traffic light operation
      trafficLight.update();
      break;
      
    case EMERGENCY_DETECTED:
    case BARRIER_DEPLOYING:
    case EVACUATION_MODE:
      // Emergency: All red lights
      trafficLight.setPhase(PHASE_RED);
      break;
      
    case SYSTEM_ERROR:
      // Error: Flashing yellow
      static bool flashState = false;
      static unsigned long lastFlash = 0;
      if (millis() - lastFlash > 500) {
        flashState = !flashState;
        trafficLight.setPhase(flashState ? PHASE_YELLOW : PHASE_RED);
        lastFlash = millis();
      }
      break;
  }
}

void handleBarrierControl() {
  if(barrierControl.status()){
    return;
  }
  // Safety check: Stop barrier if obstacle detected
  if (barrierControl.isInMotion() && distanceSensor.isObjectDetected(OBSTACLE_DISTANCE_THRESHOLD)) {
    if (!barrierControl.isStopped()) {
      barrierControl.stop();
      Serial.println(F("⚠️ BARRIER STOPPED - Obstacle detected!"));
      forceDisplayUpdate = true;  // Update LCD when barrier stops
    }
  }
  // Resume if path is clear
  else if (barrierControl.isStopped() && !distanceSensor.isObjectDetected(OBSTACLE_DISTANCE_THRESHOLD)) {
    barrierControl.resume();
    Serial.println(F("✅ BARRIER RESUMED - Path clear"));
    forceDisplayUpdate = true;  // Update LCD when barrier resumes
  }
}

void logSystemStatus() {
  // Use F() macro for all string literals to save memory
  Serial.print(F("📊 System Status: "));
  
  // System state with emojis
  switch (currentState) {
    case NORMAL_OPERATION: 
      Serial.print(F("🟢 NORMAL")); 
      break;
    case EMERGENCY_DETECTED: 
      Serial.print(F("🚨 EMERGENCY DETECTED")); 
      break;
    case BARRIER_DEPLOYING: 
      Serial.print(F("🚧 BARRIER DEPLOYING")); 
      break;
    case EVACUATION_MODE: 
      Serial.print(F("🏃 EVACUATION MODE")); 
      break;
    case SYSTEM_ERROR: 
      Serial.print(F("❌ SYSTEM ERROR")); 
      break;
  }
  
  // Environmental sensors
  Serial.print(F(" | 🌡️ T:"));
  Serial.print(dhtSensor.getTemperature(), 1);
  Serial.print(F("°C 💧 H:"));
  Serial.print(dhtSensor.getHumidity(), 0);
  Serial.print(F("% | 📏 Dist:"));
  Serial.print(distanceSensor.getDistance(), 1);
  Serial.print(F("cm | ☁️ CO2:"));
  Serial.print(co2Sensor.getCO2());
  Serial.print(F("ppm"));
  
  // Barrier status
  Serial.print(F(" | 🚧 Barrier:"));
  if (barrierControl.isInMotion()) {
    Serial.print(barrierControl.status() ? F("🔼 RAISING") : F("🔽 LOWERING"));
  } else {
    Serial.print(barrierControl.status() ? F("🟢 UP") : F("🔴 DOWN"));
  }
  if (barrierControl.isStopped()) Serial.print(F(" (🛑 STOPPED)"));
  
  // Master traffic light status
  Serial.print(F(" | 🚦 Master Lights: "));
  switch(trafficLight.getPhase()) {
    case PHASE_RED: Serial.print(F("🔴 RED")); break;
    case PHASE_RED_YELLOW: Serial.print(F("🔴🟡 RED+YELLOW")); break;
    case PHASE_GREEN: Serial.print(F("🟢 GREEN")); break;
    case PHASE_YELLOW: Serial.print(F("🟡 YELLOW")); break;
  }
  
  // Emergency traffic status (master)
//  Serial.print(F(" | 🚨 Emergency Routes: "));
//  Serial.print(emergencyLight.isLeftGo() ? F("⬅️ ") : F("⛔ "));
//  Serial.print(emergencyLight.isCenterGo() ? F("⬆️ ") : F("⛔ "));
//  Serial.print(emergencyLight.isRightGo() ? F("➡️") : F("⛔"));
//  
  // Slave communication status
//  static unsigned long lastCommTime = 0;
//  bool commActive = (millis() - lastPatternUpdate < 5000);
//  Serial.print(F(" | 📡 Slaves: "));
//  Serial.print(commActive ? F("🟢 CONNECTED") : F("🔴 DISCONNECTED"));
//  
  // Current pattern sent to slaves
//  Serial.print(F(" | 🔄 Pattern: "));
//  if (currentState != NORMAL_OPERATION) {
//    Serial.print(F("EMERGENCY (ALL 🔴)"));
//  } else {
//    Serial.print(F("Normal "));
//    Serial.print(NORMAL_PATTERNS[currentNormalPattern]);
//    Serial.print(F(" ("));
//    Serial.print(getPatternName(NORMAL_PATTERNS[currentNormalPattern]));
//    Serial.print(F(")"));
//  }
//  
  Serial.println();
}

void activateEmergencyMode() {
  Serial.println(F("🚨 EMERGENCY MODE ACTIVATED"));
  
  // Set emergency traffic lights
  trafficLight.setPhase(PHASE_RED);

  Serial.println("Sending Emergency");
   slave1.write('1');
   slave2.write('1');
  
  // Prepare barrier for deployment
  if (barrierControl.status()) {
    // Barrier is up, prepare to lower it
    Serial.println(F("📍 Preparing barrier deployment"));
  }
  
  emergencyStartTime = millis();
  buzzer.alertDanger();
  
  // Force immediate LCD update
  forceUpdateDisplay();
}

void deactivateEmergencyMode() {
  Serial.println(F("✅ RETURNING TO NORMAL OPERATION"));
  
  currentState = NORMAL_OPERATION;
  barrierDeployed = false;

  Serial.println("Sending Normal");
  slave1.write("0");
  slave2.write("0");
  
  // Raise barrier if it's down
  if (!barrierControl.status()) {
    barrierControl.raise();
    Serial.println(F("🚧 Raising barrier"));
  }
  
  // Resume normal traffic operation
  trafficLight.initialize(); // Reset to normal cycle

  buzzer.stop();
  // Force immediate LCD update
  forceUpdateDisplay();
}

void displaySystemInfo() {
  Serial.println(F("\n=== DETAILED SYSTEM STATUS ==="));
  
  // Environmental Sensors
  Serial.println(F("🌡️ Environmental:"));
  Serial.print(F("  Temperature: ")); Serial.print(dhtSensor.getTemperature()); Serial.println(F("°C"));
  Serial.print(F("  Humidity: ")); Serial.print(dhtSensor.getHumidity()); Serial.println(F("%"));
  Serial.print(F("  Critical: ")); Serial.println(dhtSensor.isCritical() ? "YES" : "NO");
  Serial.print(F("  Fire Threshold: T>")); Serial.print(FIRE_TEMP_THRESHOLD); Serial.print(F("°C, H<")); Serial.print(FIRE_HUMIDITY_THRESHOLD); Serial.println(F("%"));
  
  // Distance Sensor
  Serial.println(F("📏 Distance Sensor:"));
  Serial.print(F("  Distance: ")); Serial.print(distanceSensor.getDistance()); Serial.println(F("cm"));
  Serial.print(F("  Obstacle: ")); Serial.println(distanceSensor.isObjectDetected() ? "DETECTED" : "CLEAR");
  
  // Barrier Status
  Serial.println(F("🚧 Barrier Control:"));
  Serial.print(F("  Position: ")); Serial.println(barrierControl.status() ? "UP" : "DOWN");
  Serial.print(F("  Moving: ")); Serial.println(barrierControl.isInMotion() ? "YES" : "NO");
  Serial.print(F("  Stopped: ")); Serial.println(barrierControl.isStopped() ? "YES" : "NO");
  Serial.print(F("  Current Pos: ")); Serial.println(barrierControl.getCurrentPosition());
  Serial.print(F("  Target Pos: ")); Serial.println(barrierControl.getTargetPosition());
  
  // Traffic Light
  Serial.println(F("🚦 Traffic Light:"));
  Serial.print(F("  Phase: "));
  switch(trafficLight.getPhase()) {
    case PHASE_RED: Serial.println(F("RED")); break;
    case PHASE_RED_YELLOW: Serial.println(F("RED+YELLOW")); break;
    case PHASE_GREEN: Serial.println(F("GREEN")); break;
    case PHASE_YELLOW: Serial.println(F("YELLOW")); break;
    default: Serial.println(F("UNKNOWN")); break;
  }
  Serial.print(F("  Manual Mode: ")); Serial.println(trafficLight.isManual() ? "YES" : "NO");
  
  // System State
  Serial.println(F("🏛️ System State:"));
  Serial.print(F("  Current State: "));
  switch (currentState) {
    case NORMAL_OPERATION: Serial.println(F("NORMAL OPERATION")); break;
    case EMERGENCY_DETECTED: Serial.println(F("EMERGENCY DETECTED")); break;
    case BARRIER_DEPLOYING: Serial.println(F("BARRIER DEPLOYING")); break;
    case EVACUATION_MODE: Serial.println(F("EVACUATION MODE")); break;
    case SYSTEM_ERROR: Serial.println(F("SYSTEM ERROR")); break;
  }
  Serial.print(F("  Uptime: ")); Serial.print(millis() / 1000); Serial.println(F(" seconds"));
  
  if (currentState != NORMAL_OPERATION) {
    Serial.print(F("  Emergency Duration: ")); 
    Serial.print((millis() - emergencyStartTime) / 1000); 
    Serial.println(F(" seconds"));
  }
  
  Serial.println(F("========================\n"));
}
