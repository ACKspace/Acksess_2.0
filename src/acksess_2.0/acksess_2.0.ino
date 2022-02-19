#include <avr/io.h>
#include <util/delay.h>
//RFID libraries
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
//SHA512 hashing
#include <SHA512.h>

//Masterkey moved to file masterKey.ino, which is not synced to github. To create one, simply add a tab with varialbe definition: byte masterKey[] = "YOURMASTERKEYHERE";
int tagEntrySize = 18;

int pinTagReaderVCCEnable = 7;
int pinTagReaderCs = 8;
int pinFlashNotWriteProtect = 9;
int pinFlashNotCs = 10;
int pinMosi = 11;
int pinMiso = 12;
int pinSck = 13;
int pinRandomNoise = A0; //Analog pin intended to be left unconnected to initialize random number generator
int pinDoorRelay = 2;
int pinBuzzer = 3;

long startAddress = 0x010000; //First address in flash where RFID tags can be stored
long endAddress = 0x7EFFFF; //Last address in flash where RFID tags can be stored.

int modeAdmin = 0;
unsigned long adminTimer;

Adafruit_PN532 nfc(pinSck, pinMiso, pinMosi, pinTagReaderCs); //Uses software SPI. TODO: Figure out why HW SPI won't work and fix. (Or write own library to replace this one)
SHA512 sha512;

void setup() {
  pinMode(pinTagReaderVCCEnable, OUTPUT);
	pinMode(pinTagReaderCs, OUTPUT);
	pinMode(pinFlashNotWriteProtect, OUTPUT);
	pinMode(pinSck, OUTPUT);
	pinMode(pinMiso, INPUT);
	pinMode(pinMosi, OUTPUT);
	pinMode(pinFlashNotCs, OUTPUT);
	pinMode(pinRandomNoise, INPUT);
	pinMode(pinDoorRelay, OUTPUT);
	pinMode(pinBuzzer, OUTPUT);

  digitalWrite(pinTagReaderVCCEnable, HIGH);
  
	digitalWrite(pinFlashNotCs, HIGH);
	Serial.begin(115200);

	//Initialize RFID Reader
	nfc.begin();
	nfc.SAMConfig(); // configure PN532 board to read RFID tags

	uint32_t versiondata = nfc.getFirmwareVersion();
	if (! versiondata) {
		detectNFCBoard();
	}

	//Done initializing RFID Reader
	Serial.println(F("Acksess v2.0"));
	
	enableHWSPI(); // Enable SPI, Set as Master, /2 Prescaler
}

void detectNFCBoard() {
  disableHWSPI();
  uint32_t versiondata = 0;
  versiondata = nfc.getFirmwareVersion();
  if (versiondata == 0) {
    Serial.println(F("PN53x not responding. Power cycling board"));
    digitalWrite(pinTagReaderVCCEnable, LOW);
    delay(4000);
    digitalWrite(pinTagReaderVCCEnable, HIGH);
    delay(1000);
    disableHWSPI();
    nfc.begin();
    nfc.SAMConfig();
    enableHWSPI();
    versiondata = nfc.getFirmwareVersion();
    if (! versiondata) {
      Serial.println(F("Power cycle failed. Waiting 1 minute before trying again."));
      delay(60000);
    }
  }
  enableHWSPI();
}

void loop() {

  //Detect if PN53x board gets stuck. If so, power cycle the board
  detectNFCBoard();
  
  //BOOTSTRAPPING: Comment the following if/else block
	if (modeAdmin == 0) {
		modeNormalCheckTag();
		modeNormalCheckUART();
	}
	else {
		modeAdminCheckUART();
	}
 //BOOTSTRAPPING: Ucomment the following, flash, connect UART, add tag and add admin, comment the below and uncomment the above again, flash again.
 //modeAdminCheckUART();

  //Reconfigure tag reader to cover edge case where tag reader gets reset.
  disableHWSPI();
  nfc.SAMConfig();
  enableHWSPI();
}
