#include <SoftwareSerial.h>
#include <TinyGPS.h> //подключение необходимых для работы библиотек

TinyGPS gps;
SoftwareSerial gpsSerial(12, 16); //номера пинов, к которым подключен модуль (RX, TX)

bool newdata = false;
unsigned long start;
long lat, lon;
//unsigned long time, date;
String string_b;
void setup(){
gpsSerial.begin(9600); // установка скорости обмена с приемником
Serial.begin(115200);
Serial.println("Waiting data of GPS...");
}

void loop(){
newdata = readgps();
if (newdata)
{
gps.get_position(&lat, &lon);
Serial.print("Lat: "); Serial.print(lat);
Serial.print(" Long: "); Serial.print(lon);
}
}

// проверка наличия данных

bool readgps()
{
while (gpsSerial.available())
{
char d = gpsSerial.read();
int b = d;
Serial.write(d);
//в библиотеке TinyGPS имеется ошибка: не обрабатываются данные с \r и \n
if('\r' != b)
{
if (gps.encode(b))
 return true;
}
}
return false;
}
