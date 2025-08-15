int echo = 2;
int trig = 3;

void setup() {
  Serial.begin(9600);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
}

void loop() {
  long duration;
  float distance;

  // 트리거 신호 초기화
  digitalWrite(trig, LOW);
  delayMicroseconds(2);

  // 트리거 HIGH 펄스 10μs
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  // 에코 핀에서 HIGH 유지 시간 측정
  duration = pulseIn(echo, HIGH);

  // 거리 계산 (cm)
  distance = (duration * 0.034) / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  delay(500);
}
