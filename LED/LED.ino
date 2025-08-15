// LED1
int r1 = 11;
int g1 = 10;
int b1 = 9;

// LED2
int r2 = 8;
int g2 = 7;
int b2 = 6;

// LED3
int r3 = 5;
int g3 = 4;
int b3 = 3;

void setup() {
  int pins[] = {r1, g1, b1, r2, g2, b2, r3, g3, b3};
  for (int i = 0; i < 9; i++) {
    pinMode(pins[i], OUTPUT);
  }
}

void RGB_color(int rPin, int gPin, int bPin, int rv, int gv, int bv) {
  analogWrite(rPin, rv);
  analogWrite(gPin, gv);
  analogWrite(bPin, bv);
}

void loop() {
  // LED1 = 빨강
  RGB_color(r1, g1, b1, 255, 0, 0);
  // LED2 = 노랑 (빨강+초록)
  RGB_color(r2, g2, b2, 255, 255, 0);
  // LED3 = 초록
  RGB_color(r3, g3, b3, 0, 255, 0);
  delay(1000);

  // 세 개 끄기
  RGB_color(r1, g1, b1, 0, 0, 0);
  RGB_color(r2, g2, b2, 0, 0, 0);
  RGB_color(r3, g3, b3, 0, 0, 0);
  delay(1000);
}
