/*
Data about RFID tags is stored according to the following specification
Byte # => Data contained
0 => Used flag (0x01 if flash page in use, otherwise 0xFF)
1-4 => Tag UID
5-10 => Reserved (used to be key, but no longer needed as it gets calculated from the master key, UID en sector number)
11-14 => Secret stored in read-protected area of tag
15-16 => Reserved (Might get used to store date of latest key change later. Intended format: days since 2000/01/01)
17 => Flags (currently only 0x00 for normal tag and 0x01 for admin tag)
*/

int UIDOffset = 1;
int keyOffset = 5;
int secretOffset = 11;
int flagOffset = 17;

void tagDBUpdateEntry(long address, byte dbEntry[]) {
	long sectorStart = 0xFFFFF000 & address;
	long sectorEnd = (0xFFFFF000 & address) + 0xFFF;
	byte sectorArray [16][tagEntrySize];
	
	//Read all entries in sector to memory
	for (long i = sectorStart; i < sectorEnd; i+=0x100) {
		int pageIndex = (0x00000F00 & i) >> 8;
		flashReadData(i, tagEntrySize, sectorArray[pageIndex]);
	}

	//Update the specified entry in memory.
	int pageIndexToUpdate = (0x00000F00 & address) >> 8;
	for (int i = 0; i < tagEntrySize; i++) {
		sectorArray[pageIndexToUpdate][i] = dbEntry[i];
	}

	//Erase sector so it can be rewritten
	flashEraseSector(address);

	//Write the new data to the sector
	for (long i = sectorStart; i < sectorEnd; i+=0x100) {
		int pageIndex = (0x00000F00 & i) >> 8;
		if (sectorArray[pageIndex][0] == 0x01) {
			flashWriteData(i, tagEntrySize, sectorArray[pageIndex]);
		}
	}
}

void tagDBDeleteByAddress(long address) {
	byte eraseBytes[tagEntrySize];
	for (int i = 0; i < tagEntrySize; i++) {
		eraseBytes[i] = 0xFF;
	}
	tagDBUpdateEntry(address, eraseBytes);
}

void tagDBDeleteByUID(byte UID[]) {
	long address = tagDBGetAddress(UID);
	if (!(address == 0xFFFFFFFF)) {
		tagDBDeleteByAddress(address);
	}
}

void tagDBPrintAll() {
	byte usedFlag[1];
	byte tagData[tagEntrySize];
	for (long i = startAddress; i < endAddress; i+=0x100) {
		flashReadData(i, 1, usedFlag);
		if (usedFlag[0] == 0x01) {
			flashReadData(i, tagEntrySize, tagData);
			nfc.PrintHex(tagData, tagEntrySize);
		}
	}
}

// Searched for the specified UID in flash and returns its address if found. Returns 0xFFFFFFFF otherwise.
long tagDBGetAddress(byte UID[]) {
	byte usedFlag[1];
	long resultAddress = 0xFFFFFFFF;
	int found = 0;

	// Find a page with data
	for (long i = startAddress; i < endAddress; i+=0x100) {
		flashReadData(i, 1, usedFlag);
		if (usedFlag[0] == 0x01) {
			// When data found, compare if UID matches. If so, set found flag.
			for (int j = 0; j < 4; j++) {
				byte readByte[1];
				flashReadData(i+j+1, 1, readByte);
				if (readByte[0] == UID[j]) {
					found = 1;
				}
				else {
					found = 0;
					break;
				}
			}
		}
		// If UID found, return address end stop searching
		if (found == 1) {
			resultAddress = i;
			break;
		}
	}
	return resultAddress;
}

//Adds a tag to the DB with the specified UID. Returns the address where the new entry was saved. Returns 0xFFFFFFFF if UID already exists and won't create a duplicate.
long tagDBAdd(byte UID[], byte secret[], int secretSize) {
	byte tagEntry[tagEntrySize];
	long entryAddress;

	if (!(tagDBGetAddress(UID) == 0xFFFFFFFF)) {
		return 0xFFFFFFFF;
	}

	entryAddress = flashGetNextAvailablePage();

	//Assemble the entry to write to flash into a single array
	tagEntry[0] = 0x01;
	memcpy(tagEntry+UIDOffset, UID, 4);
	for (int i = keyOffset; i < secretOffset; i++) {
		tagEntry[i] = 0xFF;
	}
	memcpy(tagEntry+secretOffset, secret, secretSize);
	tagEntry[15] = 0x00;
	tagEntry[16] = 0x00;
	tagEntry[17] = 0x00;

	//Write the entry to flash
	flashWriteData(entryAddress, sizeof(tagEntry), tagEntry);
	
	return entryAddress;
}

void tagDBSetSecret(long address, byte secret[]) {
	byte key[6];
	byte tagEntry[tagEntrySize];
	int secretSize = 4;

	
	flashReadData(address, tagEntrySize, tagEntry);
	memcpy(tagEntry+secretOffset, secret, secretSize);

	tagDBUpdateEntry(address, tagEntry);
}

void tagDBSetAdminFlag(long address, byte flag) {
	byte tagEntry[tagEntrySize];
	
	flashReadData(address, tagEntrySize, tagEntry);
	tagEntry[flagOffset] = flag;

	tagDBUpdateEntry(address, tagEntry);
}

/*
void tagDBGetKey(long address, byte key[]) {
	flashReadData(address+keyOffset, 6, key);
}
*/

void tagDBGetSecret(long address, byte secret[]) {
	flashReadData(address+secretOffset, 4, secret);
}

byte tagDBGetAdminFlag(long address) {
	byte flag[1];
	flashReadData(address+flagOffset, 1, flag);
	return flag[0];
}
