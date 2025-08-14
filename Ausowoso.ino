#include <Arduino.h>
#include "TrafficLightHandler.h"
#include "LcdDisplayHandler.h"
#include "BarrierServoHandler.h"
#include "HeatSensorHandler.h"
#include "DistanceSensorHandler.h"

// System Configuration
#define SYSTEM_UPDATE_INTERVAL 100    // Main loop update interval (ms)
#define EMERGENCY_CHECK_INTERVAL 500  // Emergency check interval (ms)
#define STATUS_DISPLAY_INTERVAL 1000  // Status display update interval (ms) - Reduced from 2000
#define SERIAL_BAUD_RATE 115200

// Pin Configuration
#define DHT_SENSOR_PIN 7
#define DISTANCE_TRIGGER_PIN 5
#define DISTANCE_ECHO_PIN 6
#define BARRIER_SERVO_PIN 9
#define TRAFFIC_RED_PIN 2
#define TRAFFIC_YELLOW_PIN 3
#define TRAFFIC_GREEN_PIN 4

// Emergency thresholds
#define FIRE_TEMP_THRESHOLD 30.0      // °C
#define FIRE_HUMIDITY_THRESHOLD 30.0  // %
#define OBSTACLE_DISTANCE_THRESHOLD 50.0 // cm
#define CRITICAL_TEMP_THRESHOLD 60.0  // °C for critical emergency

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
DistanceSensorHandler distanceSensor(DISTANCE_TRIGGER_PIN, DISTANCE_ECHO_PIN);
BarrierControl barrierControl(BARRIER_SERVO_PIN);
LCDDisplay lcdDisplay;
TrafficLight trafficLight(TRAFFIC_RED_PIN, TRAFFIC_YELLOW_PIN, TRAFFIC_GREEN_PIN);

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
void handleSerialCommands();
void activateEmergencyMode();
void deactivateEmergencyMode();
void displaySystemInfo();

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  
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
  
  // Handle serial commands
  handleSerialCommands();
  
  // Small delay to prevent overwhelming the system
  delay(10);
}

void initializeSystem() {
  // Initialize LCD Display
  lcdDisplay.initialize();
  lcdDisplay.displayStatic("System Init", "Starting...");
  delay(1000);
  
  // Initialize DHT Sensor
  dhtSensor.initialize();
  lcdDisplay.displayStatic("DHT Sensor", "Initialized");
  delay(500);
  
  // Initialize Distance Sensor
  distanceSensor.initialize();
  lcdDisplay.displayStatic("Distance Sensor", "Initialized");
  delay(500);
  
  // Initialize Barrier Control
  barrierControl.initialize();
  barrierControl.raise(); // Start with barrier up
  lcdDisplay.displayStatic("Barrier Control", "Initialized");
  delay(500);
  
  // Initialize Traffic Light
  trafficLight.initialize();
  lcdDisplay.displayStatic("Traffic Light", "Initialized");
  delay(500);
  
  // Set initial safe values
  currentState = NORMAL_OPERATION;
  barrierDeployed = false;
  systemInitialized = true;
  
  lcdDisplay.displayStatic("System Ready", "Normal Operation");
  delay(1000);
  
  // Force initial display update
  forceDisplayUpdate = true;
}

void updateSensors() {
  // Update all sensors
  dhtSensor.update();
  distanceSensor.update();
  barrierControl.update();
}

void checkEmergencyConditions() {
  if (!systemInitialized) return;
  
  float temperature = dhtSensor.getTemperature();
  float humidity = dhtSensor.getHumidity();
  bool isCritical = dhtSensor.isCritical();
  
  // Check for fire conditions
  bool fireDetected = (temperature > FIRE_TEMP_THRESHOLD && humidity < FIRE_HUMIDITY_THRESHOLD) || 
                     (temperature > CRITICAL_TEMP_THRESHOLD);
  
  // State transitions
  switch (currentState) {
    case NORMAL_OPERATION:
      if (fireDetected || isCritical) {
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
      if (!fireDetected && !isCritical && (millis() - emergencyStartTime > 30000)) {
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
    
    // Print state names instead of numbers
    switch(previousState) {
      case NORMAL_OPERATION: Serial.print(F("NORMAL")); break;
      case EMERGENCY_DETECTED: Serial.print(F("EMERGENCY")); break;
      case BARRIER_DEPLOYING: Serial.print(F("DEPLOYING")); break;
      case EVACUATION_MODE: Serial.print(F("EVACUATION")); break;
      case SYSTEM_ERROR: Serial.print(F("ERROR")); break;
    }
    
    Serial.print(F(" -> "));
    
    switch(currentState) {
      case NORMAL_OPERATION: Serial.print(F("NORMAL")); break;
      case EMERGENCY_DETECTED: Serial.print(F("EMERGENCY")); break;
      case BARRIER_DEPLOYING: Serial.print(F("DEPLOYING")); break;
      case EVACUATION_MODE: Serial.print(F("EVACUATION")); break;
      case SYSTEM_ERROR: Serial.print(F("ERROR")); break;
    }
    
    Serial.println();
    
    previousState = currentState;
    
    // Force immediate display update on state change
    forceDisplayUpdate = true;
    Serial.println(F("📺 Forcing LCD update due to state change"));
  }
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
        String line1 = "T:" + String(temperature, 1) + "C H:" + String(humidity, 0) + "%";
        String line2 = "Dist:" + String(distance, 0) + "cm NORMAL";
        lcdDisplay.displayStatic(line1, line2);
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
  Serial.print(F("📊 Status: "));
  
  switch (currentState) {
    case NORMAL_OPERATION: Serial.print(F("NORMAL")); break;
    case EMERGENCY_DETECTED: Serial.print(F("🚨EMERGENCY")); break;
    case BARRIER_DEPLOYING: Serial.print(F("🚧DEPLOYING")); break;
    case EVACUATION_MODE: Serial.print(F("🏃EVACUATION")); break;
    case SYSTEM_ERROR: Serial.print(F("❌ERROR")); break;
  }
  
  Serial.print(F(" | T:"));
  Serial.print(dhtSensor.getTemperature());
  Serial.print(F("°C H:"));
  Serial.print(dhtSensor.getHumidity());
  Serial.print(F("% | Dist:"));
  Serial.print(distanceSensor.getDistance());
  Serial.print(F("cm | Barrier:"));
  Serial.print(barrierControl.status() ? "UP" : "DOWN");
  
  if (barrierControl.isInMotion()) Serial.print(F("(MOVING)"));
  if (barrierControl.isStopped()) Serial.print(F("(STOPPED)"));
  
  Serial.println();
}

void handleSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    if (command == "status") {
      displaySystemInfo();
    }
    else if (command == "emergency") {
      Serial.println(F("🚨 Manual Emergency Activation"));
      activateEmergencyMode();
      currentState = EMERGENCY_DETECTED;
      forceUpdateDisplay();  // Force immediate LCD update
    }
    else if (command == "normal") {
      Serial.println(F("✅ Manual Normal Mode"));
      deactivateEmergencyMode();
      forceUpdateDisplay();  // Force immediate LCD update
    }
    else if (command == "barrier") {
      if (barrierControl.status()) {
        barrierControl.lower();
        Serial.println(F("🚧 Lowering barrier"));
      } else {
        barrierControl.raise();
        Serial.println(F("🚧 Raising barrier"));
      }
      forceUpdateDisplay();  // Force immediate LCD update
    }
    else if (command == "lcd") {
      Serial.println(F("🔄 Manual LCD Update"));
      forceUpdateDisplay();
    }
    else if (command == "info") {
      Serial.println(F("\n=== SYSTEM INFORMATION ==="));
      Serial.println(F("Commands available:"));
      Serial.println(F("- 'status': Show detailed status"));
      Serial.println(F("- 'emergency': Force emergency mode"));
      Serial.println(F("- 'normal': Return to normal mode"));
      Serial.println(F("- 'barrier': Toggle barrier"));
      Serial.println(F("- 'lcd': Force LCD update"));
      Serial.println(F("- 'info': Show this help"));
    }
    else if (command.length() > 0) {
      Serial.println(F("❌ Unknown command. Type 'info' for help."));
    }
  }
}

void activateEmergencyMode() {
  Serial.println(F("🚨 EMERGENCY MODE ACTIVATED"));
  
  // Set emergency traffic lights
  trafficLight.setPhase(PHASE_RED);
  
  // Prepare barrier for deployment
  if (barrierControl.status()) {
    // Barrier is up, prepare to lower it
    Serial.println(F("📍 Preparing barrier deployment"));
  }
  
  emergencyStartTime = millis();
  
  // Force immediate LCD update
  forceUpdateDisplay();
}

void deactivateEmergencyMode() {
  Serial.println(F("✅ RETURNING TO NORMAL OPERATION"));
  
  currentState = NORMAL_OPERATION;
  barrierDeployed = false;
  
  // Raise barrier if it's down
  if (!barrierControl.status()) {
    barrierControl.raise();
    Serial.println(F("🚧 Raising barrier"));
  }
  
  // Resume normal traffic operation
  trafficLight.initialize(); // Reset to normal cycle
  
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
