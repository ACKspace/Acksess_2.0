void modeNormalGrantAccess(long address, byte UID[], byte key[]) {
	byte newSecret[4];
	byte verifySecret[4];
	byte errorSecret[4] = { 0,0,0,0 };
	generateRandomArray(sizeof(newSecret), newSecret);
	tagReaderSetSecret(UID, key, newSecret);
	tagReaderGetSecret(UID, key, verifySecret);
	
	if (memcmp(newSecret, verifySecret, sizeof(verifySecret)) == 0) { 
		tagDBSetSecret(address, newSecret); 
	}
	else { //Something went wrong while updating secret on tag. Abort and mark record in flash.
		tone(pinBuzzer, 1200, 500);
		Serial.println(F("Updating secret on tag failed. Aborting."));
		tagDBSetSecret(address, errorSecret); 
		delay(5000);
		tone (pinBuzzer, 2048, 100);
		return; 
	} 

	digitalWrite(pinDoorRelay, HIGH);
	tone (pinBuzzer, 2048, 100);
	Serial.println(F("Access granted!"));
	delay(5000);
	tone (pinBuzzer, 2048, 100);
	digitalWrite(pinDoorRelay, LOW);
}

void modeNormalCheckTag() {
	byte UID[4];

	if (tagReaderGetUID(UID) == 0) {
		long address;
		tone (pinBuzzer, 2048, 100);
		address = tagDBGetAddress(UID);
		if (address == 0xFFFFFFFF) { // UID not found in DB, emit low beep.
			tone(pinBuzzer, 200, 3000);
			nfc.PrintHex(UID, 4);
			Serial.println(F("UID not found in database"));
			delay(5000);
			tone (pinBuzzer, 2048, 100);
		}
		else { //UID found, continue verification
			byte tagSecret[4];
			byte flashSecret[4];
			byte errorSecret[4] = { 0,0,0,0 };
			byte key[6];
			int result;
			calculateKey(UID, 1, key, sizeof(key));
			tagDBGetSecret(address, flashSecret);
			result = tagReaderGetSecret(UID, key, tagSecret);
			if (result == 1) { // Read failed
				for (int i = 0; i < 1; i++) { tone(pinBuzzer, 200, 100); delay(100); }
				Serial.println(F("Reading secret failed"));
				delay(5000);
				tone (pinBuzzer, 2048, 100);
				return;
			}
			else if (result == 2) { // Authentication failed. Bad key?
				for (int i = 0; i < 4; i++) { tone(pinBuzzer, 200, 100); delay(100); }
				Serial.println(F("Failed to authenticate tag"));
				delay(5000);
				tone (pinBuzzer, 2048, 100);
				return;
			}
			
			if (memcmp(tagSecret, flashSecret, sizeof(tagSecret)) == 0) { // Everything OK. Write new secret to tag and flash, then open door.
				tone (pinBuzzer, 2048, 100);
				modeNormalGrantAccess(address, UID, key);
			}
			else if (memcmp(errorSecret, flashSecret, sizeof(tagSecret)) == 0) { // Something went wrong last time while updating secret. Try again.
				tone (pinBuzzer, 4096, 100);
				Serial.println(F("Secret update attempt failed last time. Attempting repair."));
				modeNormalGrantAccess(address, UID, key);
			}
			else { //Secret on tag and in flash mismatch. Possible copy.
				for (int i = 0; i < 4; i++) { tone(pinBuzzer, 200, 500); delay(100); }
				Serial.println(F("Secret on tag and in flash do not match."));
				delay(5000);
				tone (pinBuzzer, 2048, 100);
			}
		}
	}
}

void modeNormalCheckUART() {
	if (!(Serial.peek() == -1)) {
		char command[2];
		Serial.readString().toCharArray(command, 2);
		if (command[0] == '1') { //Option to enter admin mode
			byte UID[4];
			long address;
			unsigned long timer = millis();
			Serial.println(F("Please scan admin tag"));
			while (millis() - timer < 30000) {
				if (tagReaderGetUID(UID) == 0) {
					tone (pinBuzzer, 2048, 100);
					address = tagDBGetAddress(UID);
					if (address == 0xFFFFFFFF) { // UID not found in DB, emit low beep.
						tone(pinBuzzer, 200, 3000);
						nfc.PrintHex(UID, 4);
						Serial.println(F("UID not found in database"));
						delay(5000);
						tone (pinBuzzer, 2048, 100);
						return;
					}
					else { // UID found, continue verification
						byte tagSecret[4];
						byte flashSecret[4];
						byte errorSecret[4] = { 0,0,0,0 };
						byte key[6];
						int result;
						calculateKey(UID, 1, key, sizeof(key));
						tagDBGetSecret(address, flashSecret);
						result = tagReaderGetSecret(UID, key, tagSecret);
						if (result == 1) { // Read failed
							for (int i = 0; i < 1; i++) { tone(pinBuzzer, 200, 100); delay(100); }
							Serial.println(F("Reading secret failed"));
							delay(5000);
							tone (pinBuzzer, 2048, 100);
							return;
						}
						else if (result == 2) { // Authentication failed. Bad key?
							for (int i = 0; i < 4; i++) { tone(pinBuzzer, 200, 100); delay(100); }
							Serial.println(F("Failed to authenticate tag"));
							delay(5000);
							tone (pinBuzzer, 2048, 100);
							return;
						}
						if (tagDBGetAdminFlag(address) == 0x01) {
							tone(pinBuzzer, 200, 100);
							delay(100);
							tone(pinBuzzer, 400, 100);
							delay(100);
							tone(pinBuzzer, 600, 100);
							delay(100);
							tone(pinBuzzer, 800, 100);
							delay(100);
							tone(pinBuzzer, 2048, 100);
							delay(100);
							Serial.println(F("Accepted! Entering admin mode"));
							modeAdmin = 1;
							adminTimer = millis();
							commandListAdmin();
							return;
						}
						else {
							tone(pinBuzzer, 2048, 100);
							delay(100);
							tone(pinBuzzer, 200, 100);
							delay(100);
							Serial.println(F("Tag found, but is not an admin tag. Resuming normal mode"));
							return;
						}
					}
				}
			}
			Serial.println(F("Timeout. Canceled scanning admin tag"));
			return;
		}
		else {
			commandListNormal();
		}
	}
}
