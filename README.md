# Acksess_2.0

This is the second version of the Acksess project.  
The goal is to create an RFID based door access system with MiFare classic cards.  
It also implements a mitigation of the vulnerability of MiFare classic.

***
## Installation
TBD: Bootstrapping flash storage currently not implemented.  
Needs a way to clear the flash chip and add the first admin tag.

***
## Usage
### As a user
Scan your tag to open the door. It's that easy!

### As an admin
- Connect via USB to the arduino with a serial console. (baud rate: 115200)  
(We recommend one which has local echo and local line editing capabilities, or the Arduino Serial Monitor)
- Send an empty command in order to receive the command list
- Send "1" to log in as admin
- Scan a tag with admin access in order to enter the command menu.
- Send an empty command again to receive the admin command list. It should be pretty self explanatory.

***
## Process flow when a tag is presented
### A normal scan:  
- Tag is presented to reader
- UID is read and looked up in flash storage
- When found, the key is calculated (see below) in order to unlock block 1 of the tag
- Block 1 gets unlocked
- The first 4 bytes of the block (the secret) are read and compared to the value stored in flash
- If all ok, a new 4 byte value is generated and written to both flash and tag
- The door is opened.

### Scan but UID not found
- Tag is presented to reader
- UID is read, but after searching through the entire flash storage (Takes a few seconds!), it comes up empty
- Reader waits 5 seconds before accepting a new scan

### Scan but unable to unlock block (or tag was removed during the process)
- Tag is presented to reader
- UID is read and looked up in flash storage
- When found, the key is calculated (see below) in order to unlock block 1 of the tag
- Block 1 fails to unlock
- Readers waits 5 seconds before accepting new scan

### Scan but secret failed to verify (or tag was removed during the process)
- Tag is presented to reader
- UID is read and looked up in flash storage
- When found, the key is calculated (see below) in order to unlock block 1 of the tag
- Block 1 gets unlocked
- The first 4 bytes of the block are read and compared to the value stored in flash
- Values to not match. Do not open the door.

### Everything is OK, but the new secret failed to update
- Tag is presented to reader
- UID is read and looked up in flash storage
- When found, the key is calculated (see below) in order to unlock block 1 of the tag
- Block 1 gets unlocked
- The first 4 bytes of the block (the secret) are read and compared to the value stored in flash
- All ok, attempt to update secret on tag
- Fails, write 0x00000000 as secret in flash
- Do not open door.
- Tag is presented to reader again
- When found, the key is calculated (see below) in order to unlock block 1 of the tag
- Block 1 gets unlocked
- Reader sees that secret in flash is set to 0x00000000 and skips the check
- New secret is generated and written to tag and flash
- The door is opened

### Key calculation
- The key to be used it calculated by concatenating the master password with the UID of the tag and the sector number that is being locked. It then grabs a 6-byte SHA-512 hash from this.  
- This is used to lock every block of the tag in order to make it more time consuming to maliciously clone the tag

---
## Beepcodes
\* = short high beep  
\- = short medium beep  
\_ = short low beep  
\+ = medium low beep  
0 = long low beep  

\* \* \* = Access granted  
\* 0 = UID not found. Access denied.  
\* \_ \_ \_ \_ = Unlocking block failed. Access denied.  
\* \_ = Reading secret from tag failed. Access denied.  
\* \+ \+ \+ \+ = Secret on tag does not match secreton flash.  
\* \* \- = Tag authenticated, but new information failed to write. Access denied  
\* \* \- \* = Read a tag which failed to update last time. Fixed it and access granted  
\* = Timeout passed and new tag can be scanned. Door closes if it was opened.