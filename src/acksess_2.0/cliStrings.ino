void commandListNormal() {
	Serial.println(F("-----------------------------------------------------------------------------------------------------------------------------------"));
	Serial.println(F("                      # Please pick from the following commands and type the command number into the Serial console #              "));
	Serial.println(F("                                                     --------------------------------                                              "));
	Serial.println();
	Serial.println(F("  1. Enter Admin Mode"));
	Serial.println(F(" ----------------------------------------------------------------------------------------------------------------------------------"));
}

void commandListAdmin() {
	Serial.println(F("-----------------------------------------------------------------------------------------------------------------------------------"));
	Serial.println(F("                      # Please pick from the following commands and type the command number into the Serial console #              "));
	Serial.println(F("                                                     --------------------------------                                              "));
	Serial.println();
	Serial.println(F("  1. Get Tag List"));
	Serial.println(F("  2. Add tag"));
	Serial.println(F("  3. Delete tag"));
	Serial.println(F("  4. Delete tag by UID"));
	Serial.println(F("  5. Add admin to tag"));
	Serial.println(F("  6. Clear keys and custom data from tag"));
	Serial.println(F("  0. Exit admin mode"));
	Serial.println(F(" ----------------------------------------------------------------------------------------------------------------------------------"));
}
