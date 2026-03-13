#include <Arduino.h>
#include "SoftwareSerial.h"

SoftwareSerial swSerial(D5, D6); // RX, TX

void setup()
{
  Serial.begin(115200);
  swSerial.begin(9600);
  delay(100);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, OUTPUT);

  Serial.println("Serial ready");
  Serial.println("Type L1 to turn LED on, L0 to turn LED off");
}

void loop()
{
  if (!Serial.available())
  {
    return;
  }

  String command = Serial.readStringUntil('\n');
  command.trim();

  if (command == "L1")
  {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("LED ON");
  }
  else if (command == "L0")
  {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("LED OFF");
  }
  else
  {
    Serial.print("Unknown command: ");
    Serial.println(command);
  }
}
