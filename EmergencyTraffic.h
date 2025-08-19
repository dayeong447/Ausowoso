#ifndef EMERGENCY_TRAFFIC_H
#define EMERGENCY_TRAFFIC_H

class EmergencyTraffic {
private:
    // Pin configuration for 3 directions (Right, Left, Center)
    uint8_t rightRedPin;
    uint8_t rightGreenPin;
    uint8_t leftRedPin;
    uint8_t leftGreenPin;
    uint8_t centerRedPin;
    uint8_t centerGreenPin;

    // Current states
    bool rightActive;
    bool leftActive;
    bool centerActive;

    void setLight(uint8_t redPin, uint8_t greenPin, bool isGreen) {
        digitalWrite(redPin, isGreen ? LOW : HIGH);
        digitalWrite(greenPin, isGreen ? HIGH : LOW);
    }

public:
    EmergencyTraffic(uint8_t rightR, uint8_t rightG, 
                    uint8_t leftR, uint8_t leftG,
                    uint8_t centerR, uint8_t centerG) :
        rightRedPin(rightR), rightGreenPin(rightG),
        leftRedPin(leftR), leftGreenPin(leftG),
        centerRedPin(centerR), centerGreenPin(centerG),
        rightActive(false), leftActive(false), centerActive(false) {}

    void initialize() {
        pinMode(rightRedPin, OUTPUT);
        pinMode(rightGreenPin, OUTPUT);
        pinMode(leftRedPin, OUTPUT);
        pinMode(leftGreenPin, OUTPUT);
        pinMode(centerRedPin, OUTPUT);
        pinMode(centerGreenPin, OUTPUT);

        // Start with all red
        setLight(rightRedPin, rightGreenPin, false);
        setLight(leftRedPin, leftGreenPin, false);
        setLight(centerRedPin, centerGreenPin, false);
    }

    // Set individual directions
    void setRight(bool go) {
        rightActive = go;
        setLight(rightRedPin, rightGreenPin, go);
    }

    void setLeft(bool go) {
        leftActive = go;
        setLight(leftRedPin, leftGreenPin, go);
    }

    void setCenter(bool go) {
        centerActive = go;
        setLight(centerRedPin, centerGreenPin, go);
    }

    // Preset patterns
    void allStop() {
        setRight(false);
        setLeft(false);
        setCenter(false);
    }

    void goRight() {
        allStop();
        setRight(true);
    }

    void goLeft() {
        allStop();
        setLeft(true);
    }

    void goCenter() {
        allStop();
        setCenter(true);
    }

    // Get current states
    bool isRightGo() const { return rightActive; }
    bool isLeftGo() const { return leftActive; }
    bool isCenterGo() const { return centerActive; }
};

#endif
