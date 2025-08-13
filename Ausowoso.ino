#include <Servo.h>

#include "TrafficLightHandler.h"
#include "LcdDisplayHandler.h"
#include "BarrierServoHandler.h"

#define SERVO_PIN 9

BarrierControl barrier(SERVO_PIN);
LCDDisplay lcd; 
void setup() {
  // inisialisasi LCD:
  Serial.begin(115200);
//   lcd.initialize();
//     barrier.initialize();
//    RunAllTrafficLightTests();
//   TrafficLight_init();
//
  // Optional: Change pins if needed
  // TRAFFIC_RED_PIN = 5;
  // TRAFFIC_YELLOW_PIN = 6;
  // TRAFFIC_GREEN_PIN = 7;
  // Initialize systems
 
//  pinMode(echoPin, INPUT);
//  pinMode(triggerPin, OUTPUT);
}

void loop() {
//    TrafficLight_update();

//  if (detectEmergencyCondition()) {
//    triggerEmergencyResponse();
//  }
  delay(100);
}


bool detectEmergencyCondition() {
  // Add your sensor reading logic here
  return false; // Change based on actual sensors
}

void triggerEmergencyResponse() {
  // Example emergency response
  lcd.displayDisasterWarning(5, FLOOD, 85.2, "EVACUATE AREA");
//  barrierServo.write(90); // Activate barrier
  TrafficLight_setPhase(PHASE_RED); // Force red light
}

void triggerFloodProtocol(float waterLevel) {
  static bool protocolActive = false;
  
  if(!protocolActive) {
    protocolActive = true;
    uint8_t region = 3; // Get from GPS or DIP switches
    
    // Activate emergency systems
    barrier.lower();
    TrafficLight_setPhase(PHASE_RED);
    lcd.displayEmergency(region, FLOOD, waterLevel, "BARRIER DOWN", true);
    
    Serial.print("FLOOD ALERT! Region ");
    Serial.print(region);
    Serial.print(" - Water level: ");
    Serial.print(waterLevel);
    Serial.println("cm");
  }
}

void normalOperation() {
  if(!barrier.status()) {
    barrier.raise();
    lcd.displayStatic("Monitoring", "All Systems Normal");
  }
  
  // Normal traffic light operation handled by TrafficLight_update()
}
