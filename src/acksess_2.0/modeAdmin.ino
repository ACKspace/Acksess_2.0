void modeAdminCheckUART() {
	if (!(Serial.peek() == -1)) {
		byte UID[4] = {0,0,0,0};
		char command[2];
		Serial.readString().toCharArray(command, 2);
		
		if (command[0] == '1') {
			tagDBPrintAll();
		}
		else if (command[0] == '2') {
			Serial.println(F("Please scan tag to add."));
			if (modeAdminReadTag(UID)) { return; }
			modeAdminAddTag(UID);
		}
		else if (command[0] == '3') {
			Serial.println(F("Please scan tag to delete."));
			if (modeAdminReadTag(UID)) { return; }
			modeAdminDeleteTag(UID);
		}
		else if (command[0] == '4') {
			Serial.println(F("Please enter tag to delete (format 12ABCDEF)"));
			if (modeAdminGetUIDUART(UID)) { return; }
			modeAdminDeleteTagByUID(UID);
		}
		else if (command[0] == '5') {
			Serial.println(F("Please scan tag to grant admin rights."));
			if (modeAdminReadTag(UID)) { return; }
			modeAdminSetAdminFlag(UID);
		}
		else if (command[0] == '6') {
			Serial.println(F("Please scan tag to clear."));
			if (modeAdminReadTag(UID)) { return; }
			modeAdminClearTag(UID);
		}
		else if (command[0] == '0') {
			Serial.println(F("Logging out."));
			modeAdmin = 0;
			commandListNormal();
			return;
		}
		commandListAdmin();
		adminTimer = millis();
	}
	if ((millis() - adminTimer) > 600000) {
		Serial.println(F("Timed out. Logging out."));
		modeAdmin = 0;
		commandListNormal();
	}
}

int modeAdminGetUIDUART(byte UID[]) {
	unsigned long timer = millis();
	char buf[9];
	UID;
	while (Serial.peek() == -1) {
		if (millis() - timer > 30000) {
			Serial.println("Timeout!");
			return 1;
		}
	}
	
	Serial.readString().toCharArray(buf, 9);
	for (int i = 0; i < 8; i++) {
		if (buf[i] >= 0x30 && buf[i] <= 0x39) {
			UID[i/2] |= (buf[i] - 0x30);
		}
		else if (buf[i] >= 0x41 && buf[i] <= 0x5A) {
			UID[i/2] |= (buf[i] - 0x37);
		}
		else if (buf[i] >= 0x61 && buf[i] <= 0x7A) {
			UID[i/2] |= (buf[i] - 0x57);
		}
		else {
			Serial.println("Invalid UID.");
			return 1;
		}
		if ((i % 2) == 0) {
			UID[i/2] = UID[i/2] << 4;
		}
	}
	return 0;
}

int modeAdminReadTag(byte UID[]) {
	long address;
	long timer = millis();
	while (millis() - timer < 30000) {
		if (tagReaderGetUID(UID) == 0) {
			tone (pinBuzzer, 1000, 100);
			return 0;
		}
	}
	Serial.println(F("Timed out. Reading cancelled."));
	return 1;
}

void modeAdminAddTag(byte UID[]) {
	byte secret[4];
	byte defaultKey[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	
	generateRandomArray(sizeof(secret), secret);

	Serial.println(F("Setting secret and keys on tag. Please do not remove the tag from the reader until done."));
	if (!(tagReaderSetSecret(UID, defaultKey, secret) == 0)) {
		Serial.println(F("Setting secret to tag failed. Cancelled."));
		return;
	}
	if (!(tagReaderSetKeys(UID) == 0)) {
		Serial.println(F("Setting keys failed. You may need to remove the tag from the system and try again."));
	}
	Serial.println(F("Done. Please remove the tag from the reader"));
	if ((tagDBAdd(UID, secret, sizeof(secret))) == 0xFFFFFFFF) {
		Serial.println(F("UID already exists in DB. Cancelled."));
	}
}

void modeAdminSetAdminFlag(byte UID[]) {
	long address;
	
	address = tagDBGetAddress(UID);
	if (address == 0xFFFFFFFF) {
		Serial.println(F("Tag not found on flash. Cancelled."));
		return;
	}
	
	tagDBSetAdminFlag(address, 0x01);
}

void modeAdminDeleteTag(byte UID[]) {
	long address;
	int sector = 1;
	byte key[6];
	byte secret[4] = {0,0,0,0};
	int result;

	calculateKey(UID, sector, key, sizeof(key)); 
	address = tagDBGetAddress(UID);
	if (address == 0xFFFFFFFF) {
		Serial.println(F("Tag not found on flash. Cancelled."));
		return;
	}

	Serial.println(F("Removing secret and keys from tag. Please do not remove the tag from the reader until done."));
	result = tagReaderSetSecret(UID, key, secret);
	if (!(result == 0)) {
		Serial.println(F("Clearing secret from tag failed. Cancelled."));
		Serial.println(result);
		return;
	}
	
	if (!(tagReaderClearKeys(UID) == 0)) {
		Serial.println(F("Clearing keys failed. You may need to clear them manually."));
	}
	Serial.println(F("Done. Please remove the tag from the reader"));
	
	tagDBDeleteByAddress(address);
}

void modeAdminDeleteTagByUID(byte UID[]) {
	long address;
	
	address = tagDBGetAddress(UID);
	if (address == 0xFFFFFFFF) {
		Serial.println(F("Tag not found on flash. Cancelled."));
		return;
	}
	tagDBDeleteByAddress(address);
}

void modeAdminClearTag(byte UID[]) {
	int sector = 1;
	byte key[6];
	byte secret[4] = {0,0,0,0};
	int result;

	calculateKey(UID, sector, key, sizeof(key)); 

	Serial.println(F("Removing secret and keys from tag. Please do not remove the tag from the reader until done."));
	result = tagReaderSetSecret(UID, key, secret);
	if (!(result == 0)) {
		Serial.println(F("Clearing secret from tag failed. Cancelled."));
		Serial.println(result);
		return;
	}
	
	if (!(tagReaderClearKeys(UID) == 0)) {
		Serial.println(F("Clearing keys failed. You may need to clear them manually."));
	}
	Serial.println(F("Done. Please remove the tag from the reader"));
}
