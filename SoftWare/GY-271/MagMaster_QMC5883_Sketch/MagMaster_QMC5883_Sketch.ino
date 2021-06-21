#include <Wire.h>
#include <MechaQMC5883.h>

MechaQMC5883 qmc;

void setup() {
  Wire.begin(5, 4);
  Serial.begin(9600);
  qmc.init();
  //qmc.setMode(Mode_Continuous,ODR_200Hz,RNG_2G,OSR_256);
}

void loop() {
  int x,y,z;
  qmc.read(&x,&y,&z);

  Serial.flush(); 
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.print(",");
  Serial.print(z);
  Serial.println();
  delay(100);
}
