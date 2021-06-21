
void setup()
{
  Serial.begin(115200);
  Serial.println();
}

void loop()
{

  Serial.print("I read");
  Serial.println(analogRead(A0));
  Serial.print("It mean");
  Serial.println(analogRead(A0)/256);
  delay(1000);
}
