#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C myPN532_I2C(Wire);
NfcAdapter myNfcAdapter = NfcAdapter(myPN532_I2C);
String myTagId = "None";
byte myNuidPICC[4];

void setup(void)
{
  Serial.begin(115200);
  Serial.println("System gestartet!");
  Serial.println("Folgendes wurde entdeckt:");
  myNfcAdapter.begin();
}

void loop()
{
  readNFC();
}

void readNFC()
{
  if (myNfcAdapter.tagPresent())
  {
    Serial.println("");
    Serial.println("");
    Serial.println("Chip erkannt!");
    Serial.println("");
    Serial.println("");
    NfcTag myTag = myNfcAdapter.read();
    myTag.print();
    myTagId = myTag.getUidString();
    Serial.println("");
    Serial.println("");
    Serial.println("Ende der Nachricht.");
    Serial.println("");
    Serial.println("");
  }
  Serial.println("Bitte berühren Sie einen Chip");
  delay(4000);
}
