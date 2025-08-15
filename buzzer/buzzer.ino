int buzzerPin = 2;
int melody[]= {158, 262}; // 도레미파솔라시도 주파수 (Hz)
int noteDurations[]={4, 4, 4, 4, 4, 4, 4, 4}; // 각 음표의 길이 (1/4분음표기준)

void setup()
{
  pinMode(buzzerPin, OUTPUT); // 부저 핀을 출력으로 설정
}

void playNote(int note, int duration) {
  tone(buzzerPin, note); // 부저에 음표 주파수 출력
  delay(duration* 250); // 음표 길이만큼 대기 (250ms = 1/4분음표)
  noTone(buzzerPin); // 부저 소리 멈춤 
  delay(50); // 음표 사이 간격 
}

void loop() {
  for (int i = 0; i < 8; i++) {
    playNote(melody[i%2], noteDurations[i]); // 멜로디 배열의 음표를 순서대로 재생
  }
  delay(100); // 멜로디 재생 후 1초 대기
}
