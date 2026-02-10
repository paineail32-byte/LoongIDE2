/**
 * MFRC522_I2C.h - Library to use ARDUINO RFID MODULE KIT 13.56 MHZ WITH TAGS I2C BY AROZCAN
 * MFRC522_I2C.h - Based on ARDUINO RFID MODULE KIT 13.56 MHZ WITH TAGS SPI Library BY COOQROBOT.
 * Based on code Dr.Leong   ( WWW.B2CQSHOP.COM )
 * Created by Miguel Balboa (circuitito.com), Jan, 2012.
 * Rewritten by S?ren Thing Andersen (access.thing.dk), fall of 2013 (Translation to English, refactored, comments, anti collision, cascade levels.)
 * Extended by Tom Clement with functionality to write to sector 0 of UID changeable Mifare cards.
 * Extended by Ahmet Remzi Ozcan with I2C functionality.
 * Author: arozcan @ https://github.com/arozcan/MFRC522-I2C-Library
 * Released into the public domain.
 *
 * Please read this file for an overview and then MFRC522.cpp for comments on the specific functions.
 * Search for "mf-rc522" on ebay.com to purchase the MF-RC522 board.
 *
 * There are three hardware components involved:
 * 1) The micro controller: An Arduino
 * 2) The PCD (short for Proximity Coupling Device): NXP MFRC522 Contactless Reader IC
 * 3) The PICC (short for Proximity Integrated Circuit Card): A card or tag using the ISO 14443A interface, eg Mifare or NTAG203.
 *
 * The microcontroller and card reader uses I2C for communication.
 * The protocol is described in the MFRC522 datasheet: http://www.nxp.com/documents/data_sheet/MFRC522.pdf
 *
 * The card reader and the tags communicate using a 13.56MHz electromagnetic field.
 * The protocol is defined in ISO/IEC 14443-3 Identification cards -- Contactless integrated circuit cards -- Proximity cards -- Part 3: Initialization and anticollision".
 * A free version of the final draft can be found at http://wg8.de/wg8n1496_17n3613_Ballot_FCD14443-3.pdf
 * Details are found in chapter 6, Type A ¨C Initialization and anticollision.
 *
 * If only the PICC UID is wanted, the above documents has all the needed information.
 * To read and write from MIFARE PICCs, the MIFARE protocol is used after the PICC has been selected.
 * The MIFARE Classic chips and protocol is described in the datasheets:
 *		1K:   http://www.nxp.com/documents/data_sheet/MF1S503x.pdf
 * 		4K:   http://www.nxp.com/documents/data_sheet/MF1S703x.pdf
 * 		Mini: http://www.idcardmarket.com/download/mifare_S20_datasheet.pdf
 * The MIFARE Ultralight chip and protocol is described in the datasheets:
 *		Ultralight:   http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf
 * 		Ultralight C: http://www.nxp.com/documents/short_data_sheet/MF0ICU2_SDS.pdf
 *
 * MIFARE Classic 1K (MF1S503x):
 * 		Has 16 sectors * 4 blocks/sector * 16 unsigned chars/block = 1024 bytes.
 * 		The blocks are numbered 0-63.
 * 		Block 3 in each sector is the Sector Trailer. See http://www.nxp.com/documents/data_sheet/MF1S503x.pdf sections 8.6 and 8.7:
 * 				Bytes 0-5:   Key A
 * 				Bytes 6-8:   Access Bits
 * 				Bytes 9:     User data
 * 				Bytes 10-15: Key B (or user data)
 * 		Block 0 is read-only manufacturer data.
 * 		To access a block, an authentication using a key from the block's sector must be performed first.
 * 		Example: To read from block 10, first authenticate using a key from sector 3 (blocks 8-11).
 * 		All keys are set to FFFFFFFFFFFFh at chip delivery.
 * 		Warning: Please read section 8.7 "Memory Access". It includes this text: if the PICC detects a format violation the whole sector is irreversibly blocked.
 *		To use a block in "value block" mode (for Increment/Decrement operations) you need to change the sector trailer. Use PICC_SetAccessBits() to calculate the bit patterns.
 * MIFARE Classic 4K (MF1S703x):
 * 		Has (32 sectors * 4 blocks/sector + 8 sectors * 16 blocks/sector) * 16 bytes/block = 4096 bytes.
 * 		The blocks are numbered 0-255.
 * 		The last block in each sector is the Sector Trailer like above.
 * MIFARE Classic Mini (MF1 IC S20):
 * 		Has 5 sectors * 4 blocks/sector * 16 bytes/block = 320 bytes.
 * 		The blocks are numbered 0-19.
 * 		The last block in each sector is the Sector Trailer like above.
 *
 * MIFARE Ultralight (MF0ICU1):
 * 		Has 16 pages of 4 bytes = 64 bytes.
 * 		Pages 0 + 1 is used for the 7-byte UID.
 * 		Page 2 contains the last check digit for the UID, one byte manufacturer internal data, and the lock bytes (see http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf section 8.5.2)
 * 		Page 3 is OTP, One Time Programmable bits. Once set to 1 they cannot revert to 0.
 * 		Pages 4-15 are read/write unless blocked by the lock bytes in page 2.
 * MIFARE Ultralight C (MF0ICU2):
 * 		Has 48 pages of 4 bytes = 192 bytes.
 * 		Pages 0 + 1 is used for the 7-byte UID.
 * 		Page 2 contains the last check digit for the UID, one byte manufacturer internal data, and the lock bytes (see http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf section 8.5.2)
 * 		Page 3 is OTP, One Time Programmable bits. Once set to 1 they cannot revert to 0.
 * 		Pages 4-39 are read/write unless blocked by the lock bytes in page 2.
 * 		Page 40 Lock bytes
 * 		Page 41 16 bit one way counter
 * 		Pages 42-43 Authentication configuration
 * 		Pages 44-47 Authentication key
 */

#ifndef _MFRC522_H
#define _MFRC522_H

#include <stdbool.h>

//-------------------------------------------------------------------------------------------------

#define RC522_RESET_PIN         87      // GPIO ±àºÅ 2K500-J99-24½Å

#define RC522_IRQ_PIN           85      // GPIO ±àºÅ 2K500-J99-23½Å

//-------------------------------------------------------------------------------------------------
// Return codes from the functions in this class.
// Remember to update GetStatusCodeName() if you add more.
//-------------------------------------------------------------------------------------------------

#define STATUS_OK				0	    // Success
#define STATUS_ERROR			2	    // Error in communication
#define STATUS_COLLISION		3	    // Collission detected
#define STATUS_TIMEOUT			4	    // Timeout in communication.
#define STATUS_NO_ROOM			5	    // A buffer is not big enough.
#define STATUS_INTERNAL_ERROR   6	    // Internal error in the code. Should not happen ;-)
#define STATUS_INVALID			7	    // Invalid argument.
#define STATUS_CRC_WRONG		8	    // The CRC_A does not match
#define STATUS_MIFARE_NACK		9		// A MIFARE PICC responded with NAK.

//-------------------------------------------------------------------------------------------------
// A struct used for passing the UID of a PICC.
//-------------------------------------------------------------------------------------------------

typedef struct UID
{
	char	size;			// Number of bytes in the UID. 4, 7 or 10.
	char	uidByte[10];
	char	sak;			// The SAK (Select acknowledge) byte returned from the PICC after successful selection.
} UID_t;
	
//-------------------------------------------------------------------------------------------------
// A struct used for passing a MIFARE Crypto1 key
//-------------------------------------------------------------------------------------------------

typedef struct MIFARE_Key
{
	char keyByte[6];        // MF_KEY_SIZE
} MIFARE_Key_t;

//-------------------------------------------------------------------------------------------------
// Member variables
//-------------------------------------------------------------------------------------------------

extern UID_t uid;		    // Used by PICC_ReadCardSerial().

//-------------------------------------------------------------------------------------------------
// RC522 functions
//-------------------------------------------------------------------------------------------------

int PCD_Init(const void *bus);

bool PICC_IsNewCardPresent(const void *bus);
bool PICC_ReadCardSerial(const void *bus);

int PICC_GetType(unsigned char sak);
const char *PICC_GetTypeName(unsigned char piccType);
int PICC_HaltA(const void *bus);

void PICC_Dump(const void *bus, UID_t *uid);

int PCD_Authenticate(const void *bus,
                     unsigned char command,
                     unsigned char blockAddr,
                     MIFARE_Key_t *key,
                     UID_t *uid);

int MIFARE_Read(const void *bus,
                unsigned char blockAddr,
                unsigned char *buffer,
                unsigned char *bufferSize);
                
int MIFARE_Write(const void *bus,
                 unsigned char blockAddr,
                 unsigned char *buffer,
                 unsigned char bufferSize);
                        
int MIFARE_Increment(const void *bus, unsigned char blockAddr, int delta);
int MIFARE_Decrement(const void *bus, unsigned char blockAddr, int delta);

bool MIFARE_OpenUidBackdoor(const void *bus, bool logErrors);

bool MIFARE_SetUid(const void *bus, unsigned char *newUid, unsigned char uidSize, bool logErrors);

bool MIFARE_UnbrickUidSector(const void *bus, bool logErrors);

#endif // _MFRC522_H

