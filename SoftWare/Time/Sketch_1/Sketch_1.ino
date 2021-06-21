#include <Wire.h>

void setup() {
  Wire.begin(5, 4); // старт i2c
  Wire.beginTransmission(0x68); // начинаем обмен с DS3231 с i2c адресом 0x68
  byte arr[] = {0x00, 0x00, 0x09, 0x12, 0x07, 0x06, 0x06, 0x21};
  //byte arr[] = {0x02, 0x04, 0x01, 0x08, 0x05, 0x08};
  //byte arr[] = {0x02, 0x14};
  Wire.write(arr, 8); // записываем 8 байтов массива arr
  Wire.endTransmission(); // завершение передачи
}

void loop() {
// здесь не делаем ничего
}
