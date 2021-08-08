void calculateKey(byte UID[], byte sector, byte key[], int sizeKey) {
	byte stringToHash[sizeof(masterKey)+5]; //4 bytes for UID + 1 byte for sector number

	memcpy(stringToHash, masterKey, sizeof(masterKey));
	memcpy(stringToHash+sizeof(masterKey), UID, 4);
	stringToHash[sizeof(stringToHash)-1] = sector;

	generateHash(stringToHash, sizeof(stringToHash), key, sizeKey);	
}

void generateHash(byte data[], int dataSize, byte hash[], int sizeHash) {
  sha512.reset();
  sha512.update(data, dataSize);
  sha512.finalize(hash, sizeHash);
}

void generateRandomArray(int arraySize, byte Array[]) {
	randomSeed(analogRead(pinRandomNoise));
	for (int i = 0; i < arraySize; i++) {
		Array[i] = random();
	}
}

void enableHWSPI() {
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPI2X); // Enable SPI, Set as Master, /2 Prescaler
}

void disableHWSPI() {
	SPCR = 0;
}
