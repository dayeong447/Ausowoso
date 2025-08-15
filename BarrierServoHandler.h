#ifndef BARRIER_SERVO_HANDLER_H
#define BARRIER_SERVO_HANDLER_H
#include <Servo.h>

class BarrierControl {
  private:
    Servo barrierServo;
    uint8_t pin;
    uint8_t currentPos;
    uint8_t targetPos;
    bool isRaised;
    bool isMoving;
    bool stopped;
    
    // Configurable positions
    static const uint8_t RAISED_POS = 90;     // Barrier up
    static const uint8_t LOWERED_POS = 0;   // Barrier down
    static const uint8_t SPEED = 1;           // Degrees per update
    
    unsigned long lastUpdate;
  public:
    BarrierControl(uint8_t servoPin) : 
      pin(servoPin), currentPos(LOWERED_POS), targetPos(LOWERED_POS), 
      isRaised(false), isMoving(false), stopped(false), lastUpdate(0) {}
    
    void initialize() {
      barrierServo.attach(pin);
//      barrierServo.write(LOWERED_POS);
      currentPos = RAISED_POS;
      delay(200);
    }
    
    void update() {
    unsigned long now = millis();
    if (now - lastUpdate < 30) return;  // ~33Hz update rate
    
    if (isMoving && !stopped) {
        // Periksa apakah sudah mencapai target
        if (currentPos == targetPos) {
            isMoving = false;  // Pastikan flag diupdate
        } 
        else {
            // Move towards target
            if (currentPos < targetPos) {
                currentPos = min(currentPos + SPEED, targetPos);
            } else {
                currentPos = max(currentPos - SPEED, targetPos);
            }
            
            barrierServo.write(currentPos);
            
            // Update status ketika tepat mencapai target
            if (currentPos == targetPos) {
                isMoving = false;
                isRaised = (targetPos == RAISED_POS);
            }
        }
    }
    lastUpdate = now;
}
    
    void raise() {
      if (!isRaised) {
        isRaised = true;
        targetPos = RAISED_POS;
        isMoving = true;
        stopped = false;
      }
    }
    
    void lower() {
      if (isRaised || stopped) {
        isRaised = false;
        targetPos = LOWERED_POS;
        isMoving = true;
        stopped = false;
      }
    }
    
    void toggle() {
      isRaised ? lower() : raise();
    }
    
    void stop() {
      stopped = true;
      isMoving = false;
    }
    
    void resume() {
      if (stopped && currentPos != targetPos) {
        stopped = false;
        isMoving = true;
      }
    }
    
    // Getters
    bool status() const { return isRaised; }
    bool isInMotion() const { return isMoving; }
    bool isStopped() const { return stopped; }
    uint8_t getCurrentPosition() const { return currentPos; }
    uint8_t getTargetPosition() const { return targetPos; }
};

#endif
