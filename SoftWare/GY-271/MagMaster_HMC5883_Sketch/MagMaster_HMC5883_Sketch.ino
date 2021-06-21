#include <Wire.h>
#include <HMC5883L.h>
HMC5883L compass;
void setup()
{
 Serial.begin(9600);
 Wire.begin(5, 4);
 compass = HMC5883L();
 compass.SetScale(1.3);
 compass.SetMeasurementMode(Measurement_Continuous);
}
void loop()
{
 MagnetometerRaw raw = compass.ReadRawAxis();
 MagnetometerScaled scaled = compass.ReadScaledAxis();
 float xHeading = atan2(scaled.YAxis, scaled.XAxis);
 float yHeading = atan2(scaled.ZAxis, scaled.XAxis);
 float zHeading = atan2(scaled.ZAxis, scaled.YAxis);
 if(xHeading < 0) xHeading += 2*PI;
 if(xHeading > 2*PI) xHeading -= 2*PI;
 if(yHeading < 0) yHeading += 2*PI; 
 if(yHeading > 2*PI) yHeading -= 2*PI;
 if(zHeading < 0) zHeading += 2*PI;
 if(zHeading > 2*PI) zHeading -= 2*PI;
 float xDegrees = xHeading * 180/M_PI;
 float yDegrees = yHeading * 180/M_PI;
 float zDegrees = zHeading * 180/M_PI;
 Serial.print(xDegrees);
 Serial.print(",");
 Serial.print(yDegrees);
 Serial.print(",");
 Serial.print(zDegrees);
 Serial.println(";");
 delay(200);
}
