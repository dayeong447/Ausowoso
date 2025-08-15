#include <DHT.h>

#define DHTPIN 2     // DHT11 센서 연결 핀
#define DHTTYPE DHT11   // DHT11 타입

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(hum, 1);
  Serial.print(" %, Temp: ");
  Serial.print(temp, 1);
  Serial.println(" Celsius");

  delay(2000);
}
