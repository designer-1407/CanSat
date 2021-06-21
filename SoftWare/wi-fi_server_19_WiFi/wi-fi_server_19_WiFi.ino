/*
Реализовано:

Работа в режиме SoftAP имя сети: electronic, пароль: 2003qwerty1407

запуск сервера

инициализация 2х датчиков: mpu6050 и bmp180

Реализована веб страница с таблицой, на которую выводится показания датчиков в реальном времени 

реализован вызов функции по нажатию на кнопку Старт!, а именно: прекращение работы АР, и сохранение всех данных в текстовый файл 

реализован вывод небольшого количества информации на веб страничку после нажатия кнопки результаты

реализована возможность отчистки файла по нажатию на кнопку ресет 

Время уже работает =)

*/


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "LittleFS.h"
#include <MechaQMC5883.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

Adafruit_MPU6050 mpu;       //
Adafruit_BMP085 bmp;        //
MechaQMC5883 qmc;           //




TinyGPS gps;
SoftwareSerial gpsSerial(12, 16); //номера пинов, к которым подключен модуль (RX, TX)


const byte DS3231 = 0x68; // I2C адрес таймера DS3231

/* Установите здесь свои SSID и пароль */
/*
const char* ssid = "Astronom";       // SSID
const char* password = "*0127Astr0cluB1969#";  // пароль
*/
const char* ssid = "Electronic";       // SSID
const char* password = "Jupiter2020";  // пароль

ESP8266WebServer server(80); //Указываем параметры сервера 


/**********Переменные для хранения телеметрии***********/
String Time;
String Coordinates;
String Pressure;
String Gyroscope_X;
String Gyroscope_Y;
String Gyroscope_Z;
String Accelerometer_X;
String Accelerometer_Y;
String Accelerometer_Z;
String Magnetometer_X;
String Magnetometer_Y;
String Magnetometer_Z;
String Temperature;
String Voltage;
String CurDate;
int x,y,z;
String longitude;
String latitude; 
/**********Переменные для хранения телеметрии***********/

bool newdata = false;
//unsigned long start;
int secunds = 0;
int on_gps = 0;
long lat, lon;
String string_b;
int d;

char read_symbol = ' ';       //Переменная для хранения читаемого из ФС символа
String read_data = "";        //Переменная для хранения прочитанного из ФС значения 1 юнита телеметрии
int count_data = 1;           //Переменная для счета юнитов телеметрии (от 1 до 14) 
String partHTMLtable = "";    //Переменная для хранения HTML таблици с заполненными ячейками телеметрией 
int DataFileSize = 0;         //Переменная для хранения размера текстового файла в байтах
int DataFilePosition = 0;     //Переменная для хранения расположения "курсора" внутри текстового файла в байтах DataFilePosition <= DataFileSize всегда
String write_data = "";       //Переменная для хранения строки данных для записи в ФС
int CountOfStr = 0;           //Переменная для хранения количества строк, которые выводятся на 1 страницу

int IDofStr= 1;               //Слежебная переменная 

byte dateTime[7];             //Массив для хранения данных с часов

void setup() 
{
 Serial.begin(9600);
 // Start the software serial port at the GPS's default baud
  gpsSerial.begin(9600); // установка скорости обмена с приемником
  Wire.begin(5, 4);           //Обьявление I2C шины на 5 и 4 пинах

  // подключаемся к локальной wi-fi сети
  WiFi.begin(ssid, password);

  // проверить, подключился ли wi-fi модуль к wi-fi сети
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  pinMode(LED_BUILTIN, OUTPUT);
                                                    //Обьявляем какой код выполнять при введении определенного URL
  server.on("/", handle_OnConnect);                 //При входе на главную - выполнять функцию handle_OnConnect
  server.on("/start", handle_start);                //При нажатии на кнопку старт вызывать:
  server.on("/results", handle_results);            //При нажатии на кнопку результаты вызывать:
  server.on("/resetdata", handle_resetdata);        //При нажатии на подтверждающую кнопку сброс данных вызывать:
  server.on("/next", handle_results);             //При нажатии на кнопку дальше вызывать:
  server.on("/resetsure", reset_sure);                  //При нажатии на кнопку сброс данных вызывать:
  server.onNotFound(handle_NotFound);               //При отсуствии заранее описанного URL - выполнять фу-ю handle_NotFound

  server.begin();                                   //Запускаем сервер
  
  //Serial.printf("Web server started, open %s in a web browser\n",   WiFi.softAPIP().toString().c_str());
  Serial.printf("Web server started, open %s in a web browser\n",   WiFi.localIP().toString().c_str());
  if (!mpu.begin(0x69)) {Serial.println("Failed to find MPU6050 chip");} //Запускаем MPU6050  
  Serial.println("MPU6050 Found!");

  /*******Настраиваем параметры MPU6050*******/
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);//Максимальная перегрузка 
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);      //Точность определения угловой скорости
  mpu.setFilterBandwidth(MPU6050_BAND_184_HZ);  //Частота фильтра какого то
  /*******Настраиваем параметры MPU6050*******/
  
  if (!bmp.begin()) {                           //Запускаем bmp180
  Serial.println("Could not find a valid BMP180 sensor, check wiring!");
  }
  Serial.println("BMP085 Found!");
  delay(100);

  qmc.init();

//Запускаем FS
  Serial.println("Inizializing FS...");
  if (LittleFS.begin()){
        Serial.println("FS done.");
  }else{
        Serial.println("FS fail.");
  }
  
}

void loop() 
{
  server.handleClient();                            //"Ловим" клиента, и определяем URL
}

void handle_OnConnect()                             //В случае корневого URL
{
  sensors_event_t a, g, temp;                       //Опрашиваем MPU6050
  mpu.getEvent(&a, &g, &temp);                      //Опрашиваем MPU6050
  DataFilePosition = 0;
  get_time();

      newdata = readgps();
      if (newdata)
      {
        gps.get_position(&lat, &lon);
        latitude = lat;
        longitude = lon;
      }
      //readgps_serial();
  Temperature = bmp.readTemperature();
  Gyroscope_X = g.gyro.x;
  Gyroscope_Y = g.gyro.y; 
  Gyroscope_Z = g.gyro.z;
  Accelerometer_X = a.acceleration.x;
  Accelerometer_Y = a.acceleration.y;
  Accelerometer_Z = a.acceleration.z;
  Pressure = bmp.readAltitude();
  Voltage = analogRead(A0);

  qmc.read(&x,&y,&z);

  Magnetometer_X = x;
  Magnetometer_Y = y;
  Magnetometer_Z = z;
  /******Заносим всю измеряемую телеметрию в переменные*******/
  server.send(200, "text/html", SendHTML(CurDate, Time, latitude, longitude, Pressure, Gyroscope_X,Gyroscope_Y,Gyroscope_Z,Accelerometer_X,Accelerometer_Y,Accelerometer_Z,Magnetometer_X,Magnetometer_Y,Magnetometer_Z,Temperature, Voltage)); 
  //Отправляем данные клиенту, веб страничку из String SendHTML, и измерянные значения 
  delay(100);
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

void handle_start()                                 //При нажатии кнопки старт:
{
  server.send(200, "text/plain", "CanSat started!");//Выводим сообщение клиенту в браузере
  delay(300);
  WiFi.mode(WIFI_OFF);                              //Выключаем WiFi для экономии заряда 
  File DataFile = LittleFS.open(F("/telemetry.txt"), "a");//Открываем файл telemetry.txt в режиме для записи "а"
                                                    //О режимах открытия файла тут https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
  DataFileSize = DataFile.size();                   //Определяем размер текстового файла на данный момент
  //DataFile.close();                                 //Закрываем файл
  
  FSInfo fs_info;                                   //Заполняет структуру FSInfo информацией о файловой системе. 
  LittleFS.info(fs_info);                         
  
  Serial.print("totalBytes: ");
  Serial.println(fs_info.totalBytes);               //Количество доступных в FS байт для хранения информации
  // and IDofStr < 500
  while(fs_info.totalBytes > DataFileSize){         //До тех пор, пока размер текстового файла меньше доступного места в FS
    File DataFile = LittleFS.open(F("/telemetry.txt"), "a");//Открываем telemetry.txt в режиме записи в конец "а"
    
    readgps_serial();
    
    for(int i = 0; i <= 50; i++){
      
      sensors_event_t a, g, temp;                     //Опрашиваем мпу6050
      mpu.getEvent(&a, &g, &temp);                    //Опрашиваем мпу6050
      get_time();
      qmc.read(&x,&y,&z);

      Magnetometer_X = x;
      Magnetometer_Y = y;
      Magnetometer_Z = z;

      newdata = readgps();
      if (newdata)
      {
        gps.get_position(&lat, &lon);
        latitude = lat;
        longitude = lon;
      }
      readgps_serial();
      if (DataFile){                                  //Если с файлом все ок
        //Serial.println("Write file content!");    //Сохраняем в переменную write_data через "|" по 1 значению всех датчиков в порядке:
        
        write_data = Time;                           //Время
        write_data += "|";
        write_data += latitude;                     //Координаты
        write_data += ", ";
        write_data += longitude;
        write_data += "|";
        write_data += bmp.readAltitude();           //Высоту
        write_data += "|";
        write_data += g.gyro.x;                     //Угловую скорость по х
        write_data += "|";
        write_data += g.gyro.y;                     //Угловую скорость по y 
        write_data += "|";
        write_data += g.gyro.z;                     //Угловую скорость по z
        write_data += "|";
        write_data += a.acceleration.x;             //Ускорение по х
        write_data += "|";
        write_data += a.acceleration.y;             //Ускорение по y
        write_data += "|";
        write_data += a.acceleration.z;             //Ускорение по z
        write_data += "|";  
        write_data += Magnetometer_X;                         //Магнитометр по х
        write_data += "|";
        write_data += Magnetometer_Y;                         //Магнитометр по y
        write_data += "|";
        write_data += Magnetometer_Z;                         //Магнитометр по z
        write_data += "|";      
        write_data += bmp.readTemperature();        //Температуру
        write_data += "|";
        write_data += analogRead(A0);               //Напряжение на аккумуляторе
        write_data += "|";
        readgps_serial();
        DataFile.print(write_data);                 //Заносим получившуюся строку в текстовый файл
        DataFileSize = DataFile.size();             //Определяем размер файла после записи новых данных
        //Serial.println(DataFileSize);                            
        write_data = "";                            //Обнуляем переменную с строкой телеметрии
        IDofStr++;
      }
    
    
    else{Serial.println("Problem on create file!");}//Жалуемся если что то не так с файлом
    }
      DataFile.close();                           //Закрываем файл   
      readgps_serial();   
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(50);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      
      readgps_serial();
    //delay(50);
  }
  
  Serial.println("Data safed!");
  while(1){
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);
  }
}

void handle_results()                               //При нажатии кнопки проверки результат:
{
    partHTMLtable = "";                           //Сбрасываем в 0 переменную, в которой лежит заполненная данными таблина
    CountOfStr = 0;
    File DataFile = LittleFS.open(F("/telemetry.txt"), "r");//Открываем файл для чтения
    DataFileSize = DataFile.size();                 //Определяем его размер в байтах
    Serial.println(DataFileSize);                   
    Serial.println(DataFilePosition);
    DataFile.seek(DataFilePosition, SeekSet);       //Выставляем курсор для чтения из нужного места после нажатия кнопки next
    while(DataFileSize > DataFilePosition and CountOfStr < 50){//Делать, пока DataFileSize (размер в байтах текстового файла с данными) больше,  
                                                    //чем DataFilePosition (адресс в байтах курсора внутри текстового файла), и на странице меньше, чем 50 строк 
                                                    //Курсор перемещается по мере чтения данных с файла        
        read_symbol = DataFile.read();              //Читаем 1 символ из текстового файла
        DataFilePosition = DataFile.position();     //Определяем положение курсора в файле
        if(read_symbol == '|'){                     //Если прочитанный символ равен |
            switch (count_data) {                   //Надо определить какому параметру соответствуют прочитанные значения 
              case 1:                               //count_data хранит порядковый номер читаемого на данный момент параметра
                  Time = read_data;                 //1 - время
                  count_data++;                     //После записи значений в переменную - увеличиваем значение счетчика
              break;
              case 2:
                  Coordinates = read_data;          //2 - координаты, и так далее
                  count_data++;
              break;
              case 3:
                  Pressure = read_data;
                  count_data++;
              break;
              case 4:
                  Gyroscope_X = read_data;
                  count_data++;
              break;
              case 5:
                  Gyroscope_Y = read_data;
                  count_data++;
              break;
              case 6:
                  Gyroscope_Z = read_data;
                  count_data++;
              break;
              case 7:
                  Accelerometer_X = read_data;
                  count_data++;
              break;
              case 8:
                  Accelerometer_Y = read_data;
                  count_data++;
              break;
              case 9:
                  Accelerometer_Z = read_data;
                  count_data++;
              break;
              case 10:
                  Magnetometer_X = read_data;
                  count_data++;
              break;
              case 11:
                  Magnetometer_Y = read_data;
                  count_data++;
              break;
              case 12:
                  Magnetometer_Z = read_data;
                  count_data++;
              break;
              case 13:
                  Temperature = read_data;
                  count_data++;
              break;
              case 14:
                  Voltage = read_data;              //После прочтения последнего параметра в строке
                  count_data = 1;                   //Возвращаем счетчик на начальное положение
                  CountOfStr++;                     //Увеличиваем на 1 счетчик строк, выведенных на текущую страницу
                        partHTMLtable +="<tr>\n";   //Формируем часть html кода, в который интегрируем прочитанные из памяти значения
                        partHTMLtable +="<td>";     //На каждый цикл count_data от 1 до 14 приходиться 1 строка в таблице
                        partHTMLtable += Time;      //Все последующие циклы заносят значения в эту же переменную
                        partHTMLtable +="</td>";    //После предыдущих
                        partHTMLtable +="<td>";     //Так после чтения всех значений в файле, имеем String partHTMLtable c многостройчной таблицей и свеми значениями в ней 
                        partHTMLtable += Coordinates;
                        partHTMLtable +="</td>";
                        partHTMLtable +="<td>";
                        partHTMLtable += Pressure;
                        partHTMLtable +="</td>";
                        partHTMLtable +="<td>";
                        partHTMLtable += Gyroscope_X;
                        partHTMLtable +="</td>";
                        partHTMLtable +="<td>";
                        partHTMLtable += Gyroscope_Y;
                        partHTMLtable +="</td>";
                        partHTMLtable +="<td>";
                        partHTMLtable += Gyroscope_Z;
                        partHTMLtable +="</td>";
                        partHTMLtable +="<td>";
                        partHTMLtable += Accelerometer_X;
                        partHTMLtable +="</td>";
                        partHTMLtable +="<td>";
                        partHTMLtable += Accelerometer_Y;
                        partHTMLtable +="</td>";
                        partHTMLtable +="<td>";
                        partHTMLtable +=Accelerometer_Z;
                        partHTMLtable +="</td>";
                        partHTMLtable +="<td>";
                        partHTMLtable +=Magnetometer_X;
                        partHTMLtable +="</td>";
                        partHTMLtable +="<td>";
                        partHTMLtable +=Magnetometer_Y;
                        partHTMLtable +="</td>";
                        partHTMLtable +="<td>";
                        partHTMLtable +=Magnetometer_Z;
                        partHTMLtable +="</td>";
                        partHTMLtable +="<td>";
                        partHTMLtable += Temperature;
                        partHTMLtable +="</td>";
                        partHTMLtable +="<td>";
                        partHTMLtable += Voltage;
                        partHTMLtable +="</td>";
                        partHTMLtable +="</tr>\n";
                        break;
            }
            read_data = "";                          //Сбрасываем значения переменной, хранящей значение параметра
            
        }
        else{                                        //Если прочитанный символ не "|"
            read_data += read_symbol;                //Заносим его в read_data, сохраняя предыдущие значения этой переменной, тем самым собирая в 1 переменной значение параметра
        }
        
    }
    DataFile.close();                                //Закрываем текстовый файл 
    server.send(200, "text/html", SendResult(partHTMLtable));
    //Отправляем данные клиенту, веб страничку из String SendResult, и вставляем наполненную таблицу переменной partHTMLtable
    
}

void handle_resetdata()                              //Если подтверждена кнопка сброс информации
{
  File DataFile = LittleFS.open(F("/telemetry.txt"), "w");//Открываем файл в режиме, который стирает все содержимое "w"
  DataFile.close();                                   //Закрываем файл
  partHTMLtable = "";                                 //Сбрасываем переменную, в которой хранится наполненная данными таблица
  CountOfStr = 0;
  server.send(200, "text/plain", "Data reseted!");    //Выводим соответствубщее сообщение на сайт
}  

void reset_sure()                              //Если нажата кнопка сброс информации
{
  server.send(200, "text/html", ResetSure());
}  

void get_time() {
  Wire.beginTransmission(DS3231); // начинаем обмен с DS3231
  Wire.write(byte(0x00)); // записываем адрес регистра, с которого начинаются данные даты и времени
  Wire.endTransmission(); // завершаем передачу

  int i = 0; // индекс текущего элемента массива
  Wire.beginTransmission(DS3231); // начинаем обмен с DS3231
  Wire.requestFrom(DS3231, 7); // запрашиваем 7 байтов у DS3231
  while(Wire.available()) // пока есть данные от DS3231
  {
    dateTime[i] = Wire.read(); // читаем 1 байт и сохраняем в массив dateTime
    i+=1; // инкрементируем индекс элемента массива
  }
  Wire.endTransmission(); // завершаем передачу
  if (dateTime[4]<10){ CurDate = "0"; 
  CurDate += String(dateTime[4], HEX);}
  else{CurDate = String(dateTime[4], HEX);}
  CurDate += (".");
  if (dateTime[5]<10) CurDate += "0"; 
  CurDate += String(dateTime[5], HEX);
  CurDate += (".");
  if (dateTime[6]<10) CurDate += "0"; 
  CurDate += String(dateTime[6], HEX);
  CurDate += ("  ");
  if (dateTime[2]<10) CurDate += "0"; 
  CurDate += String(dateTime[2], HEX);
  CurDate += (":");
  if (dateTime[1]<10) CurDate += "0"; 
  CurDate += String(dateTime[1], HEX);
  CurDate += (":");
  if (dateTime[0]<10) CurDate += "0"; 
  CurDate += String(dateTime[0], HEX);

  Time = "";
  if (dateTime[2]<10) Time += "0"; 
  Time += String(dateTime[2], HEX);
  Time += (":");
  if (dateTime[1]<10) Time += "0"; 
  Time += String(dateTime[1], HEX);
  Time += (":");
  if (dateTime[0]<10) Time += "0"; 
  Time += String(dateTime[0], HEX);
}

bool readgps()
{
while (gpsSerial.available())
{
int b = gpsSerial.read();
if('\r' != b)
{
if (gps.encode(b))
 return true;
}
}
return false;
}

void readgps_serial()
{
while (gpsSerial.available())
{
Serial.write(gpsSerial.read());
Serial.flush();
}
}

/***********************Основная веб страничка*********************************/
String SendHTML(String CurDate, String Time, String latitude, String longitude, String Pressure, String Gyroscope_X, String Gyroscope_Y, String Gyroscope_Z, String Accelerometer_X, String Accelerometer_Y, String Accelerometer_Z, String Magnetometer_X, String Magnetometer_Y, String Magnetometer_Z, String Temperature, String Voltage)
{
  String ptr = "<!DOCTYPE html> <html>  \n";
    ptr +="<head><meta name= \"viewport \" content= \"width=device-width, initial-scale=1.0, user-scalable=no \">    \n";
    ptr +="<title>CanSat telemetry</title>  \n";
    ptr +="<style>html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #333333;}  \n";
    ptr +="body{margin: 15px auto 15px;}  \n";
    ptr +="h1 {margin: 15px auto 15px;}  \n";
    ptr +="table, td{font-family: \"Lucida Sans Unicode\", \"Lucida Grande\", Sans-Serif;font-size: 13px;border-collapse: collapse;text-align: center;margin: 2px auto 2px;}\n";
    ptr +="th {background: #4a89c0;color: white;padding: 10px 10px;}\n";
    ptr +="th, td{border-style: solid;border-width: 0 1px 1px 0;border-color: white;}\n";
    ptr +="td {background: #D8E6F3;font-size: 15px;}\n";
    ptr +=".floating-button {text-decoration: none;display: inline-block;width: 140px;height: 45px; line-height: 45px;border-radius: 45px;margin: 50px 20px;font-family: 'Montserrat', sans-serif;font-size: 13px;text-transform: uppercase;text-align: center;letter-spacing: 3px; font-weight: 600;color: #ffffff;background: #4a89c0;transition: .3s;}\n";
    ptr +=".floating-button:hover {background: #fd3c3c;color: white;}\n";
    ptr +="</style>  \n";
    ptr +="</head>\n";
    ptr +="<script>\n";
    ptr +="setInterval(loadDoc,200);\n";
    ptr +="function loadDoc() {\n";
    ptr +="var xhttp = new XMLHttpRequest();\n";
    ptr +="xhttp.onreadystatechange = function() {\n";
    ptr +="if (this.readyState == 4 && this.status == 200) {\n";
    ptr +="document.getElementById(\"webpage\").innerHTML =this.responseText}\n";
    ptr +="};\n";
    ptr +="xhttp.open(\"GET\", \"/\", true);\n";
    ptr +="xhttp.send();\n";
    ptr +="}\n";
    ptr +="</script>\n";
    ptr +="<body>  \n";
    ptr +="<div id= \"webpage\">  \n";
    ptr +="<h1>Real time telemetry \n";
    ptr += CurDate;
    ptr +="</h1>  \n";
    ptr +="<div class= \"data \">  \n";
    ptr +="<table id=\"telemetry\">\n";
    ptr +="<tr>\n";
    ptr +="<th>Time</th>\n";
    ptr +="<th>Coordinates lat, lon</th>\n";
    ptr +="<th>Altitude m</th>\n";
    ptr +="<th>Gyroscope X rad/s</th>\n";
    ptr +="<th>Gyroscope Y rad/s</th>\n";
    ptr +="<th>Gyroscope Z rad/s</th>\n";
    ptr +="<th>Accelerometer X m/s^2</th>\n";
    ptr +="<th>Accelerometer Y m/s^2</th>\n";
    ptr +="<th>Accelerometer Z m/s^2</th>\n";
    ptr +="<th>Magnetometer X</th>\n";
    ptr +="<th>Magnetometer Y</th>\n";
    ptr +="<th>Magnetometer Z</th>\n";
    ptr +="<th>Temperature °C</th>\n";
    ptr +="<th>Voltage V</th>\n";
    ptr +="</tr>\n";
    ptr +="<tr>\n";
    ptr +="<td>";
    ptr += Time;
    ptr +="</td>";
    ptr +="<td>";
    ptr += latitude;
    ptr +=", ";
    ptr += longitude;
    ptr +="</td>";
    ptr +="<td>";
    ptr += Pressure;
    ptr +="</td>";
    ptr +="<td>";
    ptr += Gyroscope_X;
    ptr +="</td>";
    ptr +="<td>";
    ptr += Gyroscope_Y;
    ptr +="</td>";
    ptr +="<td>";
    ptr += Gyroscope_Z;
    ptr +="</td>";
    ptr +="<td>";
    ptr += Accelerometer_X;
    ptr +="</td>";
    ptr +="<td>";
    ptr += Accelerometer_Y;
    ptr +="</td>";
    ptr +="<td>";
    ptr +=Accelerometer_Z;
    ptr +="</td>";
    ptr +="<td>";
    ptr +=Magnetometer_X;
    ptr +="</td>";
    ptr +="<td>";
    ptr +=Magnetometer_Y;
    ptr +="</td>";
    ptr +="<td>";
    ptr +=Magnetometer_Z;
    ptr +="</td>";
    ptr +="<td>";
    ptr += Temperature;
    ptr +="</td>";
    ptr +="<td>";
    ptr += Voltage;
    ptr +="</td>";
    ptr +="</tr>\n";
    ptr +="</table>\n";
    ptr +="</div>  \n";
    ptr +="<a href=\"/start\" class=\"floating-button\">START</a>\n";//Кнопки видут на указанный вверху URL
    ptr +="<a href=\"/results\" class=\"floating-button\">RESULTS</a>\n";
    ptr +="<a href=\"/resetsure\" class=\"floating-button\">RESET DATA</a>\n";
    ptr +="</body>  \n";
    ptr +="</html>  \n";
  return ptr;
}

/***********************Основная веб страничка*********************************/
/***********************Веб страничка для вывода сохраненной телеметрии*********************************/
String SendResult(String partHTMLtable)
{
  String ptr = "<!DOCTYPE html> <html>  \n";
    ptr +="   <head><meta name= \"viewport \" content= \"width=device-width, initial-scale=1.0, user-scalable=no \">    \n";
    ptr +="<title>CanSat telemetry</title>  \n";
    ptr +="<style>html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #333333;}  \n";
    ptr +="body{margin: 15px auto 15px;}  \n";
    ptr +="h1 {margin: 15px auto 15px;}  \n";
    ptr +="table, td{font-family: \"Lucida Sans Unicode\", \"Lucida Grande\", Sans-Serif;font-size: 13px;border-collapse: collapse;text-align: center;margin: 2px auto 2px;}\n";
    ptr +="th {background: #4a89c0;color: white;padding: 10px 10px;}\n";
    ptr +="th, td{border-style: solid;border-width: 0 1px 1px 0;border-color: white;}\n";
    ptr +="td {background: #D8E6F3;font-size: 15px;}\n";
    ptr +=".floating-button {text-decoration: none;display: inline-block;width: 140px;height: 45px; line-height: 45px;border-radius: 45px;margin: 50px 20px;font-family: 'Montserrat', sans-serif;font-size: 13px;text-transform: uppercase;text-align: center;letter-spacing: 3px; font-weight: 600;color: #ffffff;background: #4a89c0;transition: .3s;}\n";
    ptr +=".floating-button:hover {background: #fd3c3c;color: white;}\n";
    ptr +="</style>  \n";
    ptr +="</head>\n";
    ptr +="<body>  \n";
    ptr +="<div id= \"webpage\">  \n";
    ptr +="<h1>Results\n";
    ptr +="</h1>  \n";
    ptr +="<div class= \"data \">  \n";
    ptr +="<table id=\"telemetry\">\n";
    ptr +="<tr>\n";
    ptr +="<th>Time</th>\n";
    ptr +="<th>Coordinates</th>\n";
    ptr +="<th>Altitude m</th>\n";
    ptr +="<th>Gyroscope X rad/s</th>\n";
    ptr +="<th>Gyroscope Y rad/s</th>\n";
    ptr +="<th>Gyroscope Z rad/s</th>\n";
    ptr +="<th>Accelerometer X m/s^2</th>\n";
    ptr +="<th>Accelerometer Y m/s^2</th>\n";
    ptr +="<th>Accelerometer Z m/s^2</th>\n";
    ptr +="<th>Magnetometer X</th>\n";
    ptr +="<th>Magnetometer Y</th>\n";
    ptr +="<th>Magnetometer Z</th>\n";
    ptr +="<th>Temperature °C</th>\n";
    ptr +="<th>Voltage V*4</th>\n";
    ptr +="</tr>\n";
    ptr +=partHTMLtable;
    ptr +="</table>\n";
    ptr +="<a href=\"/\" class=\"floating-button\">HOME</a>\n";//Кнопки видут на указанный вверху URL
    if(CountOfStr == 50){
        ptr +="<a href=\"/next\" class=\"floating-button\">NEXT</a>\n";
    }
    ptr +="</div>\n";
    ptr +="</body>\n";
    ptr +="</html>\n";
  return ptr;
}
/***********************Веб страничка для вывода сохраненной телеметрии*********************************/
/***********************Веб страничка для подтверждения стирания памяти*********************************/
String ResetSure()
{
  String ptr = "<!DOCTYPE html> <html>  \n";
    ptr +="   <head><meta name= \"viewport \" content= \"width=device-width, initial-scale=1.0, user-scalable=no \">    \n";
    ptr +="<title>CanSat telemetry</title>  \n";
    ptr +="<style>html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #333333;}  \n";
    ptr +="body{margin: 15px auto 15px;}  \n";
    ptr +="h1 {margin: 15px auto 15px;}  \n";
    ptr +=".floating-button {text-decoration: none;display: inline-block;width: 140px;height: 45px; line-height: 45px;border-radius: 45px;margin: 50px 20px;font-family: 'Montserrat', sans-serif;font-size: 13px;text-transform: uppercase;text-align: center;letter-spacing: 3px; font-weight: 600;color: #ffffff;background: #4a89c0;transition: .3s;}\n";
    ptr +=".floating-button:hover {background: #fd3c3c;color: white;}\n";
    ptr +="</style>  \n";
    ptr +="</head>\n";
    ptr +="<body>  \n";
    ptr +="<div id= \"webpage\">  \n";
    ptr +="<h1>Are you sure you want to reset saved data?\n";
    ptr +="</h1>  \n";
    ptr +="<div class= \"data \">  \n";
    ptr +="<a href=\"/\" class=\"floating-button\">HOME</a>\n";//Кнопки видут на указанный вверху URL
    ptr +="<a href=\"/resetdata\" class=\"floating-button\">RESET DATA</a>\n";
    ptr +="</div>\n";
    ptr +="</body>\n";
    ptr +="</html>\n";
  return ptr;
}
/***********************Веб страничка для подтверждения стирания памяти*********************************/
