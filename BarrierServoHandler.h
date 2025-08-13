#ifndef BARRIER_CONTROL_H
#define BARRIER_CONTROL_H

#include <Servo.h>

class BarrierControl {
  private:
    Servo barrierServo;
    uint8_t pin;
    bool isRaised;
    const uint8_t RAISED_POS = 0;    // 0 degrees (barrier up)
    const uint8_t LOWERED_POS = 90;  // 90 degrees (barrier down)

  public:
    BarrierControl(uint8_t servoPin) : pin(servoPin), isRaised(true) {}

    void initialize() {
      barrierServo.attach(pin);
      raise(); // Start with barrier raised
    }

    void raise() {
      barrierServo.write(RAISED_POS);
      isRaised = true;
    }

    void lower() {
      barrierServo.write(LOWERED_POS);
      isRaised = false;
    }

    void toggle() {
      if(isRaised) lower();
      else raise();
    }

    bool status() const {
      return isRaised;
    }
};

#endif
