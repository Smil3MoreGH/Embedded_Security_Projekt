#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Hallo!");

  nfc.begin();

  uint32_t versionsdaten = nfc.getFirmwareVersion();
  if (!versionsdaten) {
    Serial.print("PN53x-Board nicht gefunden");
    while (1); // Halt
  }
  // Daten erhalten, Ausgabe
  Serial.print("Chip PN5 gefunden: "); Serial.println((versionsdaten>>24) & 0xFF, HEX);
  Serial.print("Firmware-Version: "); Serial.print((versionsdaten>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versionsdaten>>8) & 0xFF, DEC);

  Serial.println("Warten auf eine ISO14443A-Karte ...");
}

void loop(void) {
  uint8_t erfolg;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; // Puffer zum Speichern der UID
  uint8_t uidLaenge; // Länge der UID (4 oder 7 Bytes je nach ISO14443A-Kartentyp)

  // Warten auf ISO14443A-Karten (Mifare usw.). Wenn eine gefunden wird,
  // wird 'uid' mit der UID befüllt und uidLaenge gibt an,
  // ob die UID 4 Bytes (Mifare Classic) oder 7 Bytes (Mifare Ultralight) hat
  erfolg = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLaenge);

  if (erfolg) {
    // Grundlegende Informationen über die Karte anzeigen
    Serial.println("ISO14443A-Karte gefunden");
    Serial.print("  UID-Länge: "); Serial.print(uidLaenge, DEC); Serial.println(" Bytes");
    Serial.print("  UID-Wert: ");
    nfc.PrintHex(uid, uidLaenge);
    Serial.println("");

    if (uidLaenge == 4) {
      // Wahrscheinlich eine Mifare Classic-Karte...
      Serial.println("Scheint eine Mifare Classic-Karte zu sein (4-Byte-UID)");

      // Jetzt versuchen, sie für Lese-/Schreibzugriff zu authentifizieren
      // Versuchen mit dem werkseitigen Standard-KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      Serial.println("Authentifizierung von Block 4 mit dem Standard KEYA-Wert");
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

      // Mit Block 4 beginnen (erster Block von Sektor 1), da Sektor 0
      // die Herstellerdaten enthält und es wahrscheinlich besser ist,
      // sie unangetastet zu lassen, es sei denn, man weiß, was man tut
      erfolg = nfc.mifareclassic_AuthenticateBlock(uid, uidLaenge, 4, 0, keya);

      if (erfolg) {
        Serial.println("Sektor 1 (Blöcke 4..7) wurde authentifiziert");
        uint8_t daten[16];

        // Wenn etwas in Block 4 geschrieben werden soll, um es zu testen, die folgende Zeile einkommentieren
        //memcpy(daten, (const uint8_t[]){ 'a', 'd', 'a', 'f', 'r', 'u', 'i', 't', '.', 'c', 'o', 'm', 0, 0, 0, 0 }, sizeof daten);
        // erfolg = nfc.mifareclassic_WriteDataBlock (4, daten);

        // Versuchen, den Inhalt von Block 4 zu lesen
        erfolg = nfc.mifareclassic_ReadDataBlock(4, daten);

        if (erfolg) {
          // Daten wurden gelesen ... ausgeben
          Serial.println("Lese Block 4:");
          nfc.PrintHexChar(daten, 16);
          Serial.println("");

          // Kurze Verzögerung, bevor die Karte erneut gelesen wird
          delay(1000);
        }
        else {
          Serial.println("Oops... Der angeforderte Block konnte nicht gelesen werden. Anderen Schlüssel versuchen?");
        }
      }
      else {
        Serial.println("Oops... Authentifizierung fehlgeschlagen. Anderen Schlüssel versuchen?");
      }
    }

    if (uidLaenge == 7) {
      // Wahrscheinlich eine Mifare Ultralight-Karte...
      Serial.println("Scheint eine Mifare Ultralight-Tag (7-Byte-UID) zu sein");

      // Versuchen, die erste allgemeine Benutzerseite (#4) zu lesen
      Serial.println("Lese Seite 4");
      uint8_t daten[32];
      erfolg = nfc.mifareultralight_ReadPage (4, daten);
      if (erfolg) {
        // Daten wurden gelesen ... ausgeben
        nfc.PrintHexChar(daten, 4);
        Serial.println("");

        // Kurze Verzögerung, bevor die Karte erneut gelesen wird
        delay(1000);
      }
      else {
        Serial.println("Die angeforderte Seite konnte nicht gelesen werden.");
      }
    }
  }
}
