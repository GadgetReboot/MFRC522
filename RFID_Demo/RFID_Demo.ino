/*
   RFID demo put together by Gadget Reboot

   Reads RFID tags/cards using the MFRC522 reader and based on the UID's stored in the sketch
   for specific tags, access will be granted or denied.

   The status will be displayed on a red/green LED, a 16x2 LCD, and audio tones and voice prompts.

   Compiled for Uno, using Arduino IDE 1.8.10

   Libraries:
   Speech Synthesis - Talkie (ArminJo) v1.0.2  https://github.com/ArminJo/Talkie
   RFID - MFRC522 v1.4.4 https://github.com/miguelbalboa/rfid
   LCD - HD44780 v1.0.1
   Tone Generator - Timer Free Tone v1.5 https://bitbucket.org/teckel12/arduino-timer-free-tone/wiki/Home

*/

#include <Wire.h>
#include <hd44780.h>                       // lcd  hd44780 library
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

hd44780_I2Cexp lcd;                        // lcd object

#include <SPI.h>
#include <MFRC522.h>                       // RFID reader library
#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);             // RFID object

// hard coded tag/card NUID's for access granted status
const byte nuidValid1[4] = {0x80, 0x9D, 0x71, 0xA3};
const byte nuidValid2[4] = {0xF6, 0x13, 0x20, 0xA3};
const byte nuidValid3[4] = {0x7B, 0x45, 0x5D, 0x79};
const byte nuidValid4[4] = {0x89, 0x49, 0x16, 0x89};
const byte nuidValid5[4] = {0xF7, 0x18, 0x5C, 0x79};

#include "Talkie.h"
#include "Vocab_US_Large.h"
#include "Vocab_US_Clock.h"

Talkie voice;                 // voice synth object

#include <TimerFreeTone.h>    // generate tones without conflicts with other libraries that use timers
#define TONE_PIN 3            // pin 3 is audio output

#define ledPin1  4            // bidirectional LED pins  
#define ledPin2  5


void setup() {

  Serial.begin(9600);

  // configure status LED and default to off
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);

  voice.doNotUseUseInvertedOutput();  // Talkie: only use one audio output pin to free up SPI pin

  Serial.println(F("MIFARE Classic NUID RFID card/tag reader\n"));

  // init 16x2 LCD display
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Scan ID Tag");

  // init MFRC522 reader
  SPI.begin();
  rfid.PCD_Init();
}

void loop() {

  // reset the loop if no card is detected by reader
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // verify if the NUID has been read
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("Reading tag... "));
  boolean accessGranted = false;       // default to denying access until a valid card is read

  // PICC = Proximity Integrated Circuit Card (RFID Tag/Card)
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // verify the PICC type is Classic MIFARE
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("RFID tag/card is not MIFARE Classic type. Aborting."));
    return;
  }

  // check scanned tag NUID against known valid tag NUID's and grant access if valid
  if ((rfid.uid.uidByte[0] == nuidValid1[0] &&
       rfid.uid.uidByte[1] == nuidValid1[1] &&
       rfid.uid.uidByte[2] == nuidValid1[2] &&
       rfid.uid.uidByte[3] == nuidValid1[3] )
      ||
      (rfid.uid.uidByte[0] == nuidValid2[0] &&
       rfid.uid.uidByte[1] == nuidValid2[1] &&
       rfid.uid.uidByte[2] == nuidValid2[2] &&
       rfid.uid.uidByte[3] == nuidValid2[3] )
      ||
      (rfid.uid.uidByte[0] == nuidValid3[0] &&
       rfid.uid.uidByte[1] == nuidValid3[1] &&
       rfid.uid.uidByte[2] == nuidValid3[2] &&
       rfid.uid.uidByte[3] == nuidValid3[3] )
      ||
      (rfid.uid.uidByte[0] == nuidValid4[0] &&
       rfid.uid.uidByte[1] == nuidValid4[1] &&
       rfid.uid.uidByte[2] == nuidValid4[2] &&
       rfid.uid.uidByte[3] == nuidValid4[3] )
      ||
      (rfid.uid.uidByte[0] == nuidValid5[0] &&
       rfid.uid.uidByte[1] == nuidValid5[1] &&
       rfid.uid.uidByte[2] == nuidValid5[2] &&
       rfid.uid.uidByte[3] == nuidValid5[3] ))
  {
    accessGranted = true;
  }

  if (!accessGranted) {
    Serial.println(F("Access Denied!"));

    // turn on red LED
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, LOW);

    lcd.clear();
    lcd.print("Access Denied!");
    lcd.setCursor(0, 1);
    lcd.print("Invalid ID...");

    Serial.print(F("Tag NUID: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();

    // access denied tone
    // syntax: Tone output pin, frequency, duration (mS), volume [1..10]
    TimerFreeTone(TONE_PIN, 200, 100, 4);
    TimerFreeTone(TONE_PIN, 100, 100, 4);
    TimerFreeTone(TONE_PIN, 200, 100, 4);
    TimerFreeTone(TONE_PIN, 100, 100, 4);
    delay(200);

    // Vocab_US_Large.h
    // VM61002/3/4/5 ROMs, male US voice
    voice.say(sp4_YOU);
    voice.say(sp4_HAVE);
    voice.say(sp4_NO);
    voice.say(sp5_CLEARANCE);

    delay(1000);

  }
  else
  {
    Serial.print(F("Access Granted!  Welcome "));

    // turn on green LED
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, HIGH);

    lcd.clear();
    lcd.print("Access Granted!");
    lcd.setCursor(0, 1);

    // define auth. key - all keys are set to FFFFFFFFFFFFh from factory
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

    // tag reading variables
    byte bufferTxt[18];
    byte block = 4;
    byte len = 18;
    MFRC522::StatusCode status;

    // read name data from tag
    status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(rfid.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Authentication failed: "));
      Serial.println(rfid.GetStatusCodeName(status));
      return;
    }

    status = rfid.MIFARE_Read(block, bufferTxt, &len);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Reading failed: "));
      Serial.println(rfid.GetStatusCodeName(status));
      return;
    }

    // display name data, allowing only certain ascii characters
    for (uint8_t i = 0; i < 16; i++) {
      if ((bufferTxt[i] >= 33) && (bufferTxt[i] <= 126)) {
        Serial.write(bufferTxt[i]);
        lcd.write(bufferTxt[i] );
      }
    }

    Serial.println();

    // access granted tone
    // syntax: Tone output pin, frequency, duration (mS), volume [1..10]
    TimerFreeTone(TONE_PIN, 500, 100, 4);
    TimerFreeTone(TONE_PIN, 1300, 150, 4);
    TimerFreeTone(TONE_PIN, 1800, 200, 4);
    delay(200);

    // Vocab_US_Clock.h
    // VM61002 ROM (clock vocabulary, female US voice)
    voice.say(spc_GOOD);
    voice.say(spc_MORNING);

    delay(1000);
  }

  // turn off LED
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);

  Serial.println();
  lcd.clear();
  lcd.print("Scan ID Tag");

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

}

// dump a byte array as hex values to Serial
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
