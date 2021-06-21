#include <SoftwareSerial.h>

// если вы используете не плату NodeMCU, а другой модуль ESP8266,
// вам могут понадобиться другие GPIO-контакты:
SoftwareSerial mySerial(12, 16);  //  RX, TX

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;  //  ждем подключения последовательного порта 
       // (нужно только для штатного USB-порта)
  }

  // задаем скорость передачи данных через порт SoftwareSerial:
  mySerial.begin(9600);

}

void loop() {
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }
  if (Serial.available()) {
    mySerial.write(Serial.read());
  }
}
