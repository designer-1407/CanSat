#include <Wire.h>
const byte DS3231 = 0x68; // I2C адрес таймера DS3231

void setup() {
  Wire.begin(5, 4);
  Serial.begin(9600);
}

void loop() {
  Wire.beginTransmission(DS3231); // начинаем обмен с DS3231
  Wire.write(byte(0x00)); // записываем адрес регистра, с которого начинаются данные даты и времени
  Wire.endTransmission(); // завершаем передачу

  byte dateTime[7]; // 7 байтов для хранения даты и времени
  int i = 0; // индекс текущего элемента массива
  Wire.beginTransmission(DS3231); // начинаем обмен с DS3231
  Wire.requestFrom(DS3231, 7); // запрашиваем 7 байтов у DS3231
  while(Wire.available()) // пока есть данные от DS3231
  {
    dateTime[i] = Wire.read(); // читаем 1 байт и сохраняем в массив dateTime
    i+=1; // инкрементируем индекс элемента массива
  }
  Wire.endTransmission(); // завершаем передачу

  printDateTime(dateTime); // выводим дату и время
  delay(1000); // пауза на 1 сек
}

// выводит дату и время
void printDateTime(byte *dateTime) {
  if (dateTime[4]<10) Serial.print("0"); 
  Serial.print(dateTime[4], HEX); // выводим дату
  Serial.print(".");
  if (dateTime[5]<10) Serial.print("0"); 
  Serial.print(dateTime[5], HEX); // выводим месяц
  Serial.print(".20");
  Serial.print(dateTime[6], HEX); // выводим год
  Serial.print(" ");
  Serial.print(dateTime[3], HEX); // выводим год
  Serial.print(" ");
  if (dateTime[2]<10) Serial.print("0"); 
  Serial.print(dateTime[2], HEX); // выводим час
  Serial.print(":");
  if (dateTime[1]<10) Serial.print("0"); 
  Serial.print(dateTime[1], HEX); // выводим минуты
  Serial.print(":");
  if (dateTime[0]<10) Serial.print("0"); 
  Serial.print(dateTime[0], HEX); // выводим секунды
  Serial.println();
}
