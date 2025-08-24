//#include <Arduino.h>
//#include <SoftwareSerial.h>
//#include "TrafficLightHandler.h"
//#include "LcdDisplayHandler.h"
//#include "BarrierServoHandler.h"
//#include "HeatSensorHandler.h"
//#include "DistanceSensorHandler.h"
//#include "EmergencyTraffic.h"
//
//// Pin Configuration for slave intersection
//#define DHT_SENSOR_PIN 7
//#define DISTANCE_TRIGGER_PIN 5
//#define DISTANCE_ECHO_PIN 6
//#define BARRIER_SERVO_PIN 9
//#define TRAFFIC_RED_PIN 2
//#define TRAFFIC_YELLOW_PIN 3
//#define TRAFFIC_GREEN_PIN 4
//#define RIGHT_RED_PIN 8
//#define RIGHT_GREEN_PIN 9
//#define LEFT_RED_PIN 10
//#define LEFT_GREEN_PIN 11
//#define CENTER_RED_PIN 12
//#define CENTER_GREEN_PIN 13
//
//// Communication
//#define MASTER_RX 10
//#define MASTER_TX 11
//SoftwareSerial masterSerial(MASTER_RX, MASTER_TX);
//
//// Zone number for this intersection
//const int ZONE_NUMBER = 2; // Set this to 2, 3, or 4 for each slave
//
//bool emergencyMode = false;
//
//void setup() {
//  Serial.begin(115200);
//  masterSerial.begin(9600);
//  
//  // Initialize components
//  lcdDisplay.initialize();
//  dhtSensor.initialize();
//  distanceSensor.initialize();
//  barrierControl.initialize();
//  trafficLight.initialize();
//  traffic.initialize();
//  
//  barrierControl.raise();
//  
//  lcdDisplay.displayStatic("Zone " + String(ZONE_NUMBER), "Initializing...");
//  delay(1000);
//  
//  Serial.println("Zone " + String(ZONE_NUMBER) + " Controller Ready");
//  lcdDisplay.displayStatic("Zone " + String(ZONE_NUMBER), "Ready");
//}
//
//void loop() {
//  // Handle incoming messages
//  if (masterSerial.available()) {
//    String message = masterSerial.readStringUntil('\n');
//    processMasterMessage(message);
//  }
//  
//  // Update sensors
//  dhtSensor.update();
//  distanceSensor.update();
//  
//  // Check local emergency conditions
//  checkLocalEmergency();
//  
//  // Update traffic control
//  if (emergencyMode) {
//    trafficLight.setPhase(PHASE_RED);
//    traffic.allStop();
//  } else {
//    trafficLight.update();
//  }
//  
//  // Update barrier
//  barrierControl.update();
//  
//  // Update display
//  updateDisplay();
//  
//  // Send status updates periodically
//  if (millis() - lastStatusUpdate > 5000) {
//    sendStatus();
//    lastStatusUpdate = millis();
//  }
//  
//  delay(10);
//}
//
//void processMasterMessage(String message) {
//  int separator = message.indexOf(':');
//  if (separator == -1) return;
//  
//  String type = message.substring(0, separator);
//  String data = message.substring(separator + 1);
//  
//  if (type == "EMER") {
//    emergencyMode = (data == "1");
//    if (emergencyMode) {
//      trafficLight.setPhase(PHASE_RED);
//      traffic.allStop();
//      barrierControl.lower();
//      Serial.println("Emergency mode activated by master");
//    } else {
//      trafficLight.initialize();
//      barrierControl.raise();
//      Serial.println("Emergency mode deactivated by master");
//    }
//  }
//}
//
//void checkLocalEmergency() {
//  float temperature = dhtSensor.getTemperature();
//  float humidity = dhtSensor.getHumidity();
//  
//  bool fireDetected = (temperature > FIRE_TEMP_THRESHOLD && humidity < FIRE_HUMIDITY_THRESHOLD) || 
//                     (temperature > CRITICAL_TEMP_THRESHOLD);
//  
//  if (fireDetected && !emergencyMode) {
//    emergencyMode = true;
//    trafficLight.setPhase(PHASE_RED);
//    traffic.allStop();
//    barrierControl.lower();
//    
//    // Notify master
//    masterSerial.print("EMER:1\n");
//    
//    Serial.println("Local emergency detected in zone " + String(ZONE_NUMBER));
//  }
//}
//
//void updateDisplay() {
//  float temperature = dhtSensor.getTemperature();
//  float humidity = dhtSensor.getHumidity();
//  
//  if (emergencyMode) {
//    lcdDisplay.displayDisasterWarning(ZONE_NUMBER, FIRE, temperature, "EVACUATE NOW!");
//  } else {
//    String line1 = "Zone " + String(ZONE_NUMBER) + " Normal";
//    String line2 = "T:" + String(temperature, 1) + "C H:" + String(humidity, 0) + "%";
//    lcdDisplay.displayStatic(line1, line2);
//  }
//}
//
//void sendStatus() {
//  String message = "STAT:" + String(trafficLight.getPhase()) + "\n";
//  masterSerial.print(message);
//}
