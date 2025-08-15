int buzzerPin = 2; // 부저 핀
int melody[] = {158, 262};       // 주파수 (Hz)
int noteDuration = 200;

void setup() {
  Serial.begin(9600);     // 시리얼 통신 속도 Python과 동일하게 설정
  pinMode(buzzerPin, OUTPUT); // 부저 핀 출력 모드로 설정
}

void loop() {
  if (Serial.available()) {       // 시리얼 데이터가 도착했는지 확인
    char data = Serial.read();    // 데이터 읽기

    if (data == '1') {            // 화재 감지
      for (int i = 0; i < 20; i++) { // melody[]의 음 2개 반복
        tone(buzzerPin, melody[i%2], noteDuration); // 주파수 출력
        delay(noteDuration * 1.1); // 음 간 간격
      }
    } 
    else if (data == '0') {       // 화재 없음
      digitalWrite(buzzerPin, LOW);  // 부저 OFF
    }
  }
}
