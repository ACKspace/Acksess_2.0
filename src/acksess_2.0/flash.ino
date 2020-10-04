//Returns the status register. Bit 0 is busy flag, which is set to 1 when a write or erase operation is in progress.
byte flashReadStatusRegister() {
	char statusRegister;
	digitalWrite(pinFlashNotCs,LOW);

	SPDR = 0x05;
	while(!(SPSR & (1<<SPIF)));

	SPDR = 0x00;
	while(!(SPSR & (1<<SPIF)));
	statusRegister = SPDR;
	
	digitalWrite(pinFlashNotCs,HIGH);	

	return statusRegister;
}

void flashWriteEnable() {
	digitalWrite(pinFlashNotCs,LOW);

	SPDR = 0x06;
	while(!(SPSR & (1<<SPIF)));

	digitalWrite(pinFlashNotCs,HIGH);
}

long flashReadJedec() {
	uint8_t b1, b2, b3;
	long result;
	digitalWrite(pinFlashNotCs,LOW);

	SPDR = 0x9F;
	while(!(SPSR & (1<<SPIF)));
	
	SPDR = 0x00;
	while(!(SPSR & (1<<SPIF)));
	b1 = SPDR;
	
	SPDR = 0x00;
	while(!(SPSR & (1<<SPIF)));
	b2 = SPDR;
	
	SPDR = 0x00;
	while(!(SPSR & (1<<SPIF)));
	b3 = SPDR;

	digitalWrite(pinFlashNotCs,HIGH);

	result = ((long)b1 << 16) | ((long)b2 << 8) | b3;
	return result;
}

// Reads data from provided address for a provided lenght and writes the result into a provided array.
// Make sure the size of the array is equal to or larger then the length, or you will be writing where in memory where there be dragons.
void flashReadData (long longAddress, byte readLength, byte dataArray[]) {
	byte address[3];
	address[0] = longAddress >> 16;
	address[1] = longAddress >> 8;
	address[2] = longAddress;

	digitalWrite(pinFlashNotCs,LOW);

	SPDR = 0x03;
	while(!(SPSR & (1<<SPIF)));

	for (int i = 0; i <= 2 ; i++) {
		SPDR = address[i];
		while(!(SPSR & (1<<SPIF)));
	}

	for (int i = 0; i < readLength; i++) {
		SPDR = 0x00;
		while(!(SPSR & (1<<SPIF)));
		dataArray[i] = SPDR;
	}
	
	digitalWrite(pinFlashNotCs,HIGH);
}

//Takes a provided byte array and writes it to flash at a specified address.
//Note that each written byte increases the address by one. If you were to reach the end of a page (address 0x------FF) it will loop back around to the start of the page rather then continuing.
//e.g. if you were to write 0x1122 to address 0x005555FF address 0x005555FF will contain 0x11 and address 0x00555500 will contain 0x22
//So take care that your data still fits in the page when writing.
//Also not that you can only write to a page which hasn't been written to before. To reuse a page you need to erase the page (together with the entire sector) first.
//A write operation takes between 0.7 and 3 ms
void flashWriteData (long longAddress, byte writeLength, byte dataArray[]) {
	byte busy = 1;
	byte address[3];
	address[0] = longAddress >> 16;
	address[1] = longAddress >> 8;
	address[2] = longAddress;

	flashWriteEnable();

	digitalWrite(pinFlashNotCs,LOW);
	digitalWrite(pinFlashNotWriteProtect,HIGH);

	SPDR = 0x02;
	while(!(SPSR & (1<<SPIF)));

	for (int i = 0; i <= 2 ; i++) {
		SPDR = address[i];
		while(!(SPSR & (1<<SPIF)));
	}

	for (int i = 0; i < writeLength; i++) {
		SPDR = dataArray[i];
		while(!(SPSR & (1<<SPIF)));
	}
	
	digitalWrite(pinFlashNotCs,HIGH);

	//Wait for write cycle to complete
	int statusRegister = 0x01;
	while (busy == 1) {
		statusRegister = flashReadStatusRegister();
		if (0x01 & statusRegister) {delay(1);}
		else {busy = 0;}
	}
	digitalWrite(pinFlashNotWriteProtect,LOW);	
}

//Erases the entire sector in which the specified address is located (last 3 nibbles don't matter as it erases the whole 4KB sector where the address is in). Takes between 45 and 400 ms.
void flashEraseSector(long longAddress) {
	byte busy = 1;
	byte address[3];
	address[0] = longAddress >> 16;
	address[1] = longAddress >> 8;
	address[2] = longAddress;
	
	flashWriteEnable();
	
	digitalWrite(pinFlashNotCs,LOW);
	digitalWrite(pinFlashNotWriteProtect,HIGH);

	SPDR = 0x20;
	while(!(SPSR & (1<<SPIF)));

	for (int i = 0; i <= 2 ; i++) {
		SPDR = address[i];
		while(!(SPSR & (1<<SPIF)));
	}
	
	digitalWrite(pinFlashNotCs,HIGH);

	//Wait for erase cycle to complete
	int statusRegister = 0x01;
	while (busy == 1) {
		statusRegister = flashReadStatusRegister();
		if (0x01 & statusRegister) {delay(1);}
		else {busy = 0;}
	}
	digitalWrite(pinFlashNotWriteProtect,LOW);
}

//Erase the entire flash chip. Takes between 20 and 100 seconds
void flashEraseChip() {
	byte busy = 1;
	
	flashWriteEnable();
	
	digitalWrite(pinFlashNotCs,LOW);
	digitalWrite(pinFlashNotWriteProtect,HIGH);

	SPDR = 0x60;
	while(!(SPSR & (1<<SPIF)));

	digitalWrite(pinFlashNotCs,HIGH);

	//Wait for erase cycle to complete
	int statusRegister = 0x01;
	while (busy == 1) {
		statusRegister = flashReadStatusRegister();
		Serial.println(statusRegister);
		if (0x01 & statusRegister) {delay(1000);}
		else {busy = 0;}
	}

	digitalWrite(pinFlashNotWriteProtect,LOW);
}

//Finds a page that is not used yet by checking the first byte of each page until 0xFF is found.
long flashGetNextAvailablePage() {
	long resultAddress;
	byte data[1];

	for (long i = startAddress; i < endAddress; i+=0x100) {
		flashReadData(i, 1, data);
		if (data[0] == 0xff) {
			resultAddress = i;
			break;
		}
	}
	return resultAddress;
}
