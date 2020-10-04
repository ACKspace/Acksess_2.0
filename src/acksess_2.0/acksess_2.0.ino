#include <avr/io.h>
#include <util/delay.h>
//RFID libraries
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
//SHA512 hashing
#include <SHA512.h>

byte masterKey[] = "DeKatKrabtDeKrollenVanDeTrap"; //Placeholder masterkey. To be changed right before uploading final version with lock bits enabled on Arduino.
int tagEntrySize = 18;

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
	pinMode(pinTagReaderCs, OUTPUT);
	pinMode(pinFlashNotWriteProtect, OUTPUT);
	pinMode(pinSck, OUTPUT);
	pinMode(pinMiso, INPUT);
	pinMode(pinMosi, OUTPUT);
	pinMode(pinFlashNotCs, OUTPUT);
	pinMode(pinRandomNoise, INPUT);
	pinMode(pinDoorRelay, OUTPUT);
	pinMode(pinBuzzer, OUTPUT);
	
	digitalWrite(pinFlashNotCs, HIGH);
	Serial.begin(115200);

	//Initialize RFID Reader
	nfc.begin();
	nfc.SAMConfig(); // configure PN532 board to read RFID tags

	uint32_t versiondata = nfc.getFirmwareVersion();
	if (! versiondata) {
		Serial.print(F("Didn't find PN53x board"));
		while (1); // halt
	}
	Serial.print(F("Found chip PN5")); Serial.println((versiondata>>24) & 0xFF, HEX); 
	Serial.print(F("PN532 Firmware ver. ")); Serial.print((versiondata>>16) & 0xFF, DEC); 
	Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
	//Done initializing RFID Reader
	Serial.println(F("Acksess v2.0"));
	
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPI2X); // Enable SPI, Set as Master, /2 Prescaler
}

void loop() {

	if (modeAdmin == 0) {
		modeNormalCheckTag();
		modeNormalCheckUART();
	}
	else {
		modeAdminCheckUART();
	}
}
