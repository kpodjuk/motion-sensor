#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup()
{
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  pinMode(0, INPUT);

  Serial.begin(115200);             // Ustawienie prędkości transmisji
  Serial.println("Witaj swiecie!"); // Jednorazowe wysłanie tekstu
}

void loop()
{
  // put your main code here, to run repeatedly:
  Serial.println("Witaj swiecie!"); // Jednorazowe wysłanie tekstu
  bool state = digitalRead(0);
  Serial.println(state);
  delay(1000);
}

// put function definitions here:
int myFunction(int x, int y)
{
  return x + y;
}