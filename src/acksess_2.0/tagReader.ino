int tagReaderGetUID(byte UID[]) {
	byte RFIDsuccess;
	byte uidLength;
	byte tempUID[7] = {0,0,0,0,0,0,0};
	int errorCode = 0;

	disableHWSPI();
	RFIDsuccess = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, tempUID, &uidLength, 50);
	enableHWSPI();

	if (RFIDsuccess) {
		if (!(uidLength == 4)) {
			errorCode = 2;
		}
		else {
			for (int i = 0; i < 4; i++) {
				UID[i] = tempUID[i];
			}
		}
	}
	else {
		errorCode = 1;
	}
	return errorCode;
}

int tagReaderGetSecret(byte UID[], byte key[], byte secret[]) {
	byte RFIDsuccess;
	byte uidLength = 4;
	int errorCode = 0;

	disableHWSPI();
	RFIDsuccess = nfc.mifareclassic_AuthenticateBlock(UID, uidLength, 4, 0, key);
	if (RFIDsuccess) {
		byte data[16];
		RFIDsuccess = nfc.mifareclassic_ReadDataBlock(4, data);
		if (RFIDsuccess) {
			for(int i = 0; i < 4; i++) {
				secret[i] = data[i];
			}
		}
		else {
			errorCode = 1;
		}
	}
	else {
		errorCode = 2;
	}
	enableHWSPI();
	return errorCode;
}

int tagReaderSetSecret(byte UID[], byte key[], byte secret[]) {
	byte RFIDsuccess;
	byte uidLength = 4;
	int errorCode = 0;

	disableHWSPI();
	RFIDsuccess = nfc.mifareclassic_AuthenticateBlock(UID, uidLength, 4, 0, key);
	if (RFIDsuccess) {
		byte data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		for(int i = 0; i < 4; i++) {
			data[i] = secret[i];
		}
    	RFIDsuccess = nfc.mifareclassic_WriteDataBlock (4, data);
		if (!RFIDsuccess) {
			errorCode = 1;
		}
	}
	else {
		errorCode = 2;
	}
	enableHWSPI();
	return errorCode;
}

//Calculates the keys to be used by concatenating the master key, UID and sector, calculating a 6 byte hash and use that as the keys for the sectors
int tagReaderSetKeys(byte UID[]) {
	byte RFIDsuccess;
	byte uidLength = 4;
	byte data[16];
	byte newKey[6];
	byte oldKey[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	int errorCode = 0;
	
	//Access bytes set to KeyB may be read and only allow reading from data blocks with KeyA.
	data[6] = 0xFF;
	data[7] = 0x07;
	data[8] = 0x80;
  data[9] = 0xFF;

	disableHWSPI();
	for (int sector = 0; sector < 16; sector++) {
		int trailerBlock = (sector * 4) + 3;
		calculateKey(UID, sector, newKey, sizeof(newKey));
		//nfc.PrintHex(newKey, sizeof(newKey));
		for (int i = 0; i < 6; i++) {
			data[i] = newKey[i];
		}
    for (int i = 10; i < 16; i++) {
      data[i] = newKey[i-10];
    }
    
		RFIDsuccess = nfc.mifareclassic_AuthenticateBlock(UID, uidLength, trailerBlock, 0, oldKey);
		if (RFIDsuccess) {
			RFIDsuccess = nfc.mifareclassic_WriteDataBlock (trailerBlock, data);
			if (!RFIDsuccess) {
				errorCode = 1;
			}
		}
		else {
			errorCode = 2;
		}
	}
	enableHWSPI();
	return errorCode;
}

//Old version of software didn't set KeyB. This fixes tag which were created using that old version.
int tagReaderFixKeys(byte UID[]) {
  byte RFIDsuccess;
  byte uidLength = 4;
  byte data[16];
  byte key[6];
  int errorCode = 0;
  
  //Access bytes set to KeyB may be read and only allow reading from data blocks with KeyA.
  data[6] = 0xFF;
  data[7] = 0x07;
  data[8] = 0x80;
  data[9] = 0xFF;

  disableHWSPI();
  for (int sector = 0; sector < 16; sector++) {
    int trailerBlock = (sector * 4) + 3;
    calculateKey(UID, sector, key, sizeof(key));
    for (int i = 0; i < 6; i++) {
      data[i] = key[i];
    }
    for (int i = 10; i < 16; i++) {
      data[i] = key[i-10];
    }

    RFIDsuccess = nfc.mifareclassic_AuthenticateBlock(UID, uidLength, trailerBlock, 0, key);
    if (RFIDsuccess) {
      RFIDsuccess = nfc.mifareclassic_WriteDataBlock (trailerBlock, data);
      if (!RFIDsuccess) {
        errorCode = 1;
      }
    }
    else {
      errorCode = 2;
    }
  }
  enableHWSPI();
  return errorCode;
}

int tagReaderClearKeys(byte UID[]) {
	byte RFIDsuccess;
	byte uidLength = 4;
	byte data[16];
	byte key[6];
	int errorCode = 0;

	for (int i = 0; i < 6; i++) {
		data[i] = 0xFF;
	}

	//Access bytes set to KeyB may be read and only allow reading from data blocks with KeyA.
	data[6] = 0xFF;
	data[7] = 0x07;
	data[8] = 0x80;
  data[9] = 0xFF;

	for (int i = 10; i < 16; i++) {
		data[i] = 0xFF;
	}

	disableHWSPI();
	for (int sector = 0; sector < 16; sector++) {
		int trailerBlock = (sector * 4) + 3;
		calculateKey(UID, sector, key, sizeof(key));
		RFIDsuccess = nfc.mifareclassic_AuthenticateBlock(UID, uidLength, trailerBlock, 0, key);
		if (RFIDsuccess) {
			RFIDsuccess = nfc.mifareclassic_WriteDataBlock (trailerBlock, data);
			if (!RFIDsuccess) {
				errorCode = 1;
			}
		}
		else {
			errorCode = 2;
		}
	}
	enableHWSPI();
	return errorCode;
}
