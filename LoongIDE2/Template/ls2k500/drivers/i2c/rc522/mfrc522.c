
/*
 * MFRC522.c - Library to use ARDUINO RFID MODULE KIT 13.56 MHZ WITH TAGS I2C BY AROZCAN
 * MFRC522.c - Based on ARDUINO RFID MODULE KIT 13.56 MHZ WITH TAGS SPI Library BY COOQROBOT.
 * NOTE: Please also check the comments in MFRC522.h - they provide useful hints and background information.
 */

#include "bsp.h"

#if RC522_DRV

#include <stdio.h>
#include <string.h>

#include "osal.h"

#include "ls2k500.h"
#include "ls2k500_gpio.h"

#include "ls2k_i2c_bus.h"

#include "i2c/mfrc522.h"

//-------------------------------------------------------------------------------------------------

#define MFRC522_ADDRESS         0x28	    /* Addr0~2 = 000 */

#define MFRC522_BAUDRATE        400000	    /* baudrate 400K */

#define CHECK_DONE(rt) \
	do {               \
        if (0 != rt)   \
            goto lbl_done; \
    } while (0);

//-------------------------------------------------------------------------------------------------

/*
 * MFRC522 registers. Described in chapter 9 of the datasheet.
 */

/*
 * Page 0: Command and status
 */
#define	COMMANDREG			0x01 	// starts and stops command execution
#define	COMIENREG			0x02 	// enable and disable interrupt request control bits
#define	DIVIENREG			0x03 	// enable and disable interrupt request control bits
#define	COMIRQREG			0x04 	// interrupt request bits
#define	DIVIRQREG			0x05 	// interrupt request bits
#define	ERRORREG			0x06 	// error bits showing the error status of the last command executed
#define	STATUS1REG			0x07 	// communication status bits
#define	STATUS2REG			0x08 	// receiver and transmitter status bits
#define	FIFODATAREG			0x09 	// input and output of 64 byte FIFO buffer
#define	FIFOLEVELREG		0x0A 	// number of bytes stored in the FIFO buffer
#define	WATERLEVELREG		0x0B 	// level for FIFO underflow and overflow warning
#define	CONTROLREG			0x0C 	// miscellaneous control registers
#define	BITFRAMINGREG		0x0D 	// adjustments for bit-oriented frames
#define	COLLREG				0x0E 	// bit position of the first bit-collision detected on the RF interface

/*
 * Page 1: Command
 */
#define	MODEREG				0x11 	// defines general modes for transmitting and receiving
#define	TXMODEREG			0x12 	// defines transmission data rate and framing
#define	RXMODEREG			0x13 	// defines reception data rate and framing
#define	TXCONTROLREG		0x14 	// controls the logical behavior of the antenna driver pins TX1 and TX2
#define	TXASKREG			0x15 	// controls the setting of the transmission modulation
#define	TXSELREG			0x16 	// selects the internal sources for the antenna driver
#define	RXSELREG			0x17 	// selects internal receiver settings
#define	RXTHRESHOLDREG		0x18 	// selects thresholds for the bit decoder
#define	DEMODREG			0x19 	// defines demodulator settings
#define	MFTXREG				0x1C 	// controls some MIFARE communication transmit parameters
#define	MFRXREG				0x1D 	// controls some MIFARE communication receive parameters
#define	SERIALSPEEDREG		0x1F 	// selects the speed of the serial UART interface

/*
 * Page 2: Configuration
 */
#define	CRCRESULTREGH		0x21 	// shows the MSB and LSB values of the CRC calculation
#define	CRCRESULTREGL		0x22
#define	MODWIDTHREG			0x24 	// controls the ModWidth setting?
#define	RFCFGREG			0x26 	// configures the receiver gain
#define	GSNREG				0x27 	// selects the conductance of the antenna driver pins TX1 and TX2 for modulation
#define	CWGSPREG			0x28 	// defines the conductance of the p-driver output during periods of no modulation
#define	MODGSPREG			0x29 	// defines the conductance of the p-driver output during periods of modulation
#define	TMODEREG			0x2A 	// defines settings for the internal timer
#define	TPRESCALERREG		0x2B 	// the lower 8 bits of the TPrescaler value. The 4 high bits are in TMODEREG.
#define	TRELOADREGH			0x2C 	// defines the 16-bit timer reload value
#define	TRELOADREGL			0x2D
#define	TCOUNTERVALUEREGH	0x2E 	// shows the 16-bit timer value
#define	TCOUNTERVALUEREGL   0x2F

/*
 * Page 3: Test Registers
 */
#define	TESTSEL1REG			0x31 	// general test signal configuration
#define	TESTSEL2REG			0x32 	// general test signal configuration
#define	TESTPINENREG		0x33 	// enables pin output driver on pins D1 to D7
#define	TESTPINVALUEREG		0x34 	// defines the values for D1 to D7 when it is used as an I/O bus
#define	TESTBUSREG			0x35 	// shows the status of the internal test bus
#define	AUTOTESTREG			0x36 	// controls the digital self test
#define	VERSIONREG			0x37 	// shows the software version
#define	ANALOGTESTREG		0x38 	// controls the pins AUX1 and AUX2
#define	TESTDAC1REG			0x39 	// defines the test value for TestDAC1
#define	TESTDAC2REG			0x3A 	// defines the test value for TestDAC2
#define	TESTADCREG			0x3B 	// shows the value of ADC I and Q channels

//-------------------------------------------------------------------------------------------------

/*
 * MFRC522 commands. Described in chapter 10 of the datasheet.
 */
 
enum PCD_Command
{
	PCD_Idle				= 0x00,		// no action, cancels current command execution
	PCD_Mem					= 0x01,		// stores 25 bytes into the internal buffer
	PCD_GenerateRandomID	= 0x02,		// generates a 10-byte random ID number
	PCD_CalcCRC				= 0x03,		// activates the CRC coprocessor or performs a self test
	PCD_Transmit			= 0x04,		// transmits data from the FIFO buffer
	PCD_NoCmdChange			= 0x07,		// no command change, can be used to modify the COMMANDREG register bits without affecting the command, for example, the PowerDown bit
	PCD_Receive				= 0x08,		// activates the receiver circuits
	PCD_Transceive 			= 0x0C,		// transmits data from FIFO buffer to antenna and automatically activates the receiver after transmission
	PCD_MFAuthent 			= 0x0E,		// performs the MIFARE standard authentication as a reader
	PCD_SoftReset			= 0x0F		// resets the MFRC522
};

/*
 * MFRC522 RxGain[2:0] masks, defines the receiver's signal voltage gain factor (on the PCD).
 * Described in 9.3.3.6 / table 98 of the datasheet at http://www.nxp.com/documents/data_sheet/MFRC522.pdf
 */
enum PCD_RxGain
{
    RxGain_18dB			= 0x00 << 4,	// 000b - 18 dB, minimum
	RxGain_23dB			= 0x01 << 4,	// 001b - 23 dB
	RxGain_18dB_2		= 0x02 << 4,	// 010b - 18 dB, it seems 010b is a duplicate for 000b
	RxGain_23dB_2		= 0x03 << 4,	// 011b - 23 dB, it seems 011b is a duplicate for 001b
	RxGain_33dB			= 0x04 << 4,	// 100b - 33 dB, average, and typical default
	RxGain_38dB			= 0x05 << 4,	// 101b - 38 dB
	RxGain_43dB			= 0x06 << 4,	// 110b - 43 dB
	RxGain_48dB			= 0x07 << 4,	// 111b - 48 dB, maximum
	RxGain_min			= 0x00 << 4,	// 000b - 18 dB, minimum, convenience for RxGain_18dB
	RxGain_avg			= 0x04 << 4,	// 100b - 33 dB, average, convenience for RxGain_33dB
	RxGain_max			= 0x07 << 4		// 111b - 48 dB, maximum, convenience for RxGain_48dB
};

/*
 * Commands sent to the PICC.
 */
enum PICC_Command
{
	/* The commands used by the PCD to manage communication with several PICCs (ISO 14443-3, Type A, section 6.4)
     */
	PICC_CMD_REQA			= 0x26,		// REQuest command, Type A. Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
	PICC_CMD_WUPA			= 0x52,		// Wake-UP command, Type A. Invites PICCs in state IDLE and HALT to go to READY(*) and prepare for anticollision or selection. 7 bit frame.
	PICC_CMD_CT				= 0x88,		// Cascade Tag. Not really a command, but used during anti collision.
	PICC_CMD_SEL_CL1		= 0x93,		// Anti collision/Select, Cascade Level 1
	PICC_CMD_SEL_CL2		= 0x95,		// Anti collision/Select, Cascade Level 2
	PICC_CMD_SEL_CL3		= 0x97,		// Anti collision/Select, Cascade Level 3
	PICC_CMD_HLTA			= 0x50,		// HaLT command, Type A. Instructs an ACTIVE PICC to go to state HALT.

	/* The commands used for MIFARE Classic (from http://www.nxp.com/documents/data_sheet/MF1S503x.pdf, Section 9)
	   Use PCD_MFAuthent to authenticate access to a sector, then use these commands to read/write/modify the blocks on the sector.
	   The read/write commands can also be used for MIFARE Ultralight.
     */
	PICC_CMD_MF_AUTH_KEY_A	= 0x60,		// Perform authentication with Key A
	PICC_CMD_MF_AUTH_KEY_B	= 0x61,		// Perform authentication with Key B
	PICC_CMD_MF_READ		= 0x30,		// Reads one 16 byte block from the authenticated sector of the PICC. Also used for MIFARE Ultralight.
	PICC_CMD_MF_WRITE		= 0xA0,		// Writes one 16 byte block to the authenticated sector of the PICC. Called "COMPATIBILITY WRITE" for MIFARE Ultralight.
	PICC_CMD_MF_DECREMENT	= 0xC0,		// Decrements the contents of a block and stores the result in the internal data register.
	PICC_CMD_MF_INCREMENT	= 0xC1,		// Increments the contents of a block and stores the result in the internal data register.
	PICC_CMD_MF_RESTORE		= 0xC2,		// Reads the contents of a block into the internal data register.
	PICC_CMD_MF_TRANSFER	= 0xB0,		// Writes the contents of the internal data register to a block.

	/* The commands used for MIFARE Ultralight (from http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf, Section 8.6)
	   The PICC_CMD_MF_READ and PICC_CMD_MF_WRITE can also be used for MIFARE Ultralight.
     */
	PICC_CMD_UL_WRITE		= 0xA2		// Writes one 4 byte page to the PICC.
};

//-------------------------------------------------------------------------------------------------

/*
 * MIFARE constants that does not fit anywhere else
 */
enum MIFARE_Misc
{
	MF_ACK					= 0xA,		// The MIFARE Classic uses a 4 bit ACK/NAK. Any other value than 0xA is NAK.
	MF_KEY_SIZE				= 6			// A Mifare Crypto1 key is 6 bytes.
};

/*
 * PICC types we can detect. Remember to update PICC_GetTypeName() if you add more.
 */
enum PICC_Type
{
	PICC_TYPE_UNKNOWN		= 0,
	PICC_TYPE_ISO_14443_4	= 1,	// PICC compliant with ISO/IEC 14443-4
	PICC_TYPE_ISO_18092		= 2, 	// PICC compliant with ISO/IEC 18092 (NFC)
	PICC_TYPE_MIFARE_MINI	= 3,	// MIFARE Classic protocol, 320 bytes
	PICC_TYPE_MIFARE_1K		= 4,	// MIFARE Classic protocol, 1KB
	PICC_TYPE_MIFARE_4K		= 5,	// MIFARE Classic protocol, 4KB
	PICC_TYPE_MIFARE_UL		= 6,	// MIFARE Ultralight or Ultralight C
	PICC_TYPE_MIFARE_PLUS	= 7,	// MIFARE Plus
	PICC_TYPE_TNP3XXX		= 8,	// Only mentioned in NXP AN 10833 MIFARE Type Identification Procedure
	PICC_TYPE_NOT_COMPLETE	= 255	// SAK indicates UID is not complete.
};

//-------------------------------------------------------------------------------------------------
// local variables
//-------------------------------------------------------------------------------------------------

/* static */ UID_t uid;

// Firmware data for self-test
// Reference values based on firmware version
// Hint: if needed, you can remove unused self-test data to save flash memory
//

// Version 0.0 (0x90)
// Philips Semiconductors; Preliminary Specification Revision 2.0 - 01 August 2005; 16.1 Sefttest
static const unsigned char MFRC522_firmware_referenceV0_0[] =
{
	0x00, 0x87, 0x98, 0x0f, 0x49, 0xFF, 0x07, 0x19,
	0xBF, 0x22, 0x30, 0x49, 0x59, 0x63, 0xAD, 0xCA,
	0x7F, 0xE3, 0x4E, 0x03, 0x5C, 0x4E, 0x49, 0x50,
	0x47, 0x9A, 0x37, 0x61, 0xE7, 0xE2, 0xC6, 0x2E,
	0x75, 0x5A, 0xED, 0x04, 0x3D, 0x02, 0x4B, 0x78,
	0x32, 0xFF, 0x58, 0x3B, 0x7C, 0xE9, 0x00, 0x94,
	0xB4, 0x4A, 0x59, 0x5B, 0xFD, 0xC9, 0x29, 0xDF,
	0x35, 0x96, 0x98, 0x9E, 0x4F, 0x30, 0x32, 0x8D
};

// Version 1.0 (0x91)
// NXP Semiconductors; Rev. 3.8 - 17 September 2014; 16.1.1 Self test
static const unsigned char MFRC522_firmware_referenceV1_0[] =
{
	0x00, 0xC6, 0x37, 0xD5, 0x32, 0xB7, 0x57, 0x5C,
	0xC2, 0xD8, 0x7C, 0x4D, 0xD9, 0x70, 0xC7, 0x73,
	0x10, 0xE6, 0xD2, 0xAA, 0x5E, 0xA1, 0x3E, 0x5A,
	0x14, 0xAF, 0x30, 0x61, 0xC9, 0x70, 0xDB, 0x2E,
	0x64, 0x22, 0x72, 0xB5, 0xBD, 0x65, 0xF4, 0xEC,
	0x22, 0xBC, 0xD3, 0x72, 0x35, 0xCD, 0xAA, 0x41,
	0x1F, 0xA7, 0xF3, 0x53, 0x14, 0xDE, 0x7E, 0x02,
	0xD9, 0x0F, 0xB5, 0x5E, 0x25, 0x1D, 0x29, 0x79
};

// Version 2.0 (0x92)
// NXP Semiconductors; Rev. 3.8 - 17 September 2014; 16.1.1 Self test
static const unsigned char MFRC522_firmware_referenceV2_0[] =
{
	0x00, 0xEB, 0x66, 0xBA, 0x57, 0xBF, 0x23, 0x95,
	0xD0, 0xE3, 0x0D, 0x3D, 0x27, 0x89, 0x5C, 0xDE,
	0x9D, 0x3B, 0xA7, 0x00, 0x21, 0x5B, 0x89, 0x82,
	0x51, 0x3A, 0xEB, 0x02, 0x0C, 0xA5, 0x00, 0x49,
	0x7C, 0x84, 0x4D, 0xB3, 0xCC, 0xD2, 0x1B, 0x81,
	0x5D, 0x48, 0x76, 0xD5, 0x71, 0x61, 0x21, 0xA9,
	0x86, 0x96, 0x83, 0x38, 0xCF, 0x9D, 0x5B, 0x6D,
	0xDC, 0x15, 0xBA, 0x3E, 0x7D, 0x95, 0x3B, 0x2F
};

// Clone
// Fudan Semiconductor FM17522 (0x88)
static const unsigned char FM17522_firmware_reference[] =
{
	0x00, 0xD6, 0x78, 0x8C, 0xE2, 0xAA, 0x0C, 0x18,
	0x2A, 0xB8, 0x7A, 0x7F, 0xD3, 0x6A, 0xCF, 0x0B,
	0xB1, 0x37, 0x63, 0x4B, 0x69, 0xAE, 0x91, 0xC7,
	0xC3, 0x97, 0xAE, 0x77, 0xF4, 0x37, 0xD7, 0x9B,
	0x7C, 0xF5, 0x3C, 0x11, 0x8F, 0x15, 0xC3, 0xD7,
	0xC1, 0x5B, 0x00, 0x2A, 0xD0, 0x75, 0xDE, 0x9E,
	0x51, 0x64, 0xAB, 0x3E, 0xE9, 0x15, 0xB5, 0xAB,
	0x56, 0x9A, 0x98, 0x82, 0x26, 0xEA, 0x2A, 0x62
};

//-------------------------------------------------------------------------------------------------
// local functions prototype
//-------------------------------------------------------------------------------------------------

static void PCD_AntennaOn(const void *bus);

static int PCD_MIFARE_Transceive(const void *bus,
                                 unsigned char *sendData,
                                 unsigned char sendLen,
                                 bool acceptTimeout);

static void PICC_DumpMifareClassic(const void *bus,
                                   UID_t *uid,
                                   unsigned char piccType,
                                   MIFARE_Key_t *key);
                                   
static void PICC_DumpMifareClassicSector(const void *bus,
                                         UID_t *uid,
                                         MIFARE_Key_t *key,
                                         unsigned char sector);
                                   
static void PICC_DumpMifareUltralight(const void *bus);


//-------------------------------------------------------------------------------------------------
// Basic interface functions for communicating with the MFRC522
//-------------------------------------------------------------------------------------------------

/*
 * Writes a byte to the specified register in the MFRC522 chip.
 * The interface is described in the datasheet section 8.1.2.
 */
static int PCD_WriteRegister(const void *bus, unsigned char reg, unsigned char value)
{
    int rt;

	rt = ls2k_i2c_send_start(bus, MFRC522_ADDRESS);	    /* start transfer */
	CHECK_DONE(rt);

	rt = ls2k_i2c_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, (void *)MFRC522_BAUDRATE); /* set baudrate */
	CHECK_DONE(rt);

	rt = ls2k_i2c_send_addr(bus, MFRC522_ADDRESS, false);	/* address device, FALSE = WRITE */
	CHECK_DONE(rt);

	rt = ls2k_i2c_write_bytes(bus, &reg, 1);	            /* 1st, 1 byte is REGISTER to be accessed. */
	rt = rt == 1 ? 0 : -1;
	CHECK_DONE(rt);
	
	rt = ls2k_i2c_write_bytes(bus, &value, 1);	            /* 2nd, 1 byte of value to write */
	rt = rt == 1 ? 0 : -1;
	
lbl_done:
	ls2k_i2c_send_stop(bus, MFRC522_ADDRESS);	            /* terminate transfer */
	
    if (rt != 0)
    {
        printk("error: return %i, %s.\r\n", rt, __func__);
    }

	return rt;
} 

/*
 * Writes a number of bytes to the specified register in the MFRC522 chip.
 * The interface is described in the datasheet section 8.1.2.
 */
static int PCD_WriteRegisters(const void *bus, unsigned char reg, unsigned char count, unsigned char *values)
{
    int rt;

    if (count == 0)
    {
        return 0;
    }

	rt = ls2k_i2c_send_start(bus, MFRC522_ADDRESS);	    /* start transfer */
	CHECK_DONE(rt);

	rt = ls2k_i2c_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, (void *)MFRC522_BAUDRATE);	/* set baudrate */
	CHECK_DONE(rt);

	rt = ls2k_i2c_send_addr(bus, MFRC522_ADDRESS, false);	/* address device, FALSE = WRITE */
	CHECK_DONE(rt);

	rt = ls2k_i2c_write_bytes(bus, &reg, 1);	            /* 1st, 1 byte is REGISTER to be accessed. */
	rt = rt == 1 ? 0 : -1;
	CHECK_DONE(rt);

	rt = ls2k_i2c_write_bytes(bus, values, (int)count);	/* 2nd, n bytes of values to write */
	rt = rt == (int)count ? 0 : -1;

lbl_done:
	ls2k_i2c_send_stop(bus, MFRC522_ADDRESS);	            /* terminate transfer */
	
    if (rt != 0)
    {
        printk("error: return %i, %s.\r\n", rt, __func__);
    }
        
	return rt;
} 

/*
 * Reads a byte from the specified register in the MFRC522 chip.
 * The interface is described in the datasheet section 8.1.2.
 */
static int PCD_ReadRegister(const void *bus, unsigned char reg, unsigned char *value)
{
    int rt;

	rt = ls2k_i2c_send_start(bus, MFRC522_ADDRESS);	    /* start transfer */
	CHECK_DONE(rt);

	rt = ls2k_i2c_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, (void *)MFRC522_BAUDRATE);	/* set baudrate */
	CHECK_DONE(rt);

	rt = ls2k_i2c_send_addr(bus, MFRC522_ADDRESS, false);	/* address device, FALSE = WRITE */
	CHECK_DONE(rt);

	rt = ls2k_i2c_write_bytes(bus, &reg, 1);	            /* 1st, 1 byte is REGISTER to be accessed. */
	rt = rt == 1 ? 0 : -1;
	CHECK_DONE(rt);

	rt = ls2k_i2c_send_addr(bus, MFRC522_ADDRESS, true);   /* 2nd, restart - address device, TRUE = READ */
	CHECK_DONE(rt);

    rt = ls2k_i2c_read_bytes(bus, value, 1);               /* 3rd, read out 1 byte */
	rt = rt == 1 ? 0 : -1;

lbl_done:
	/* terminate transfer */
	ls2k_i2c_send_stop(bus, MFRC522_ADDRESS);

    if (rt != 0)
    {
        printk("error: return %i, %s.\r\n", rt, __func__);
    }
        
	return rt;
}

/*
 * Reads a number of bytes from the specified register in the MFRC522 chip.
 * The interface is described in the datasheet section 8.1.2.
 */
static int PCD_ReadRegisters(const void *bus,
                             unsigned char reg,
                             unsigned char count,
                             unsigned char *values,
                             unsigned char rxAlign)
{
    int rt;
    unsigned char value0 = values[0];

    if (count == 0)
    {
        return 0;
    }

	rt = ls2k_i2c_send_start(bus, MFRC522_ADDRESS);	    /* start transfer */
	CHECK_DONE(rt);

	rt = ls2k_i2c_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, (void *)MFRC522_BAUDRATE);	/* set baudrate */
	CHECK_DONE(rt);

	rt = ls2k_i2c_send_addr(bus, MFRC522_ADDRESS, false);	/* address device, FALSE = WRITE */
	CHECK_DONE(rt);

	rt = ls2k_i2c_write_bytes(bus, &reg, 1);	            /* 1st, 1 byte is REGISTER to be accessed. */
	rt = rt == 1 ? 0 : -1;
	CHECK_DONE(rt);

	rt = ls2k_i2c_send_addr(bus, MFRC522_ADDRESS, true);   /* 2nd, restart - address device, TRUE = READ */
	CHECK_DONE(rt);

    rt = ls2k_i2c_read_bytes(bus, values, (int)count);     /* 3rd, read out n bytes */
	rt = rt == (int)count ? 0 : -1;

	/* Only bit positions rxAlign..7 in values[0] are updated.
     */
    if (rxAlign)
    {
        int i;
        unsigned char mask = 0;
        for (i = rxAlign; i <= 7; i++)
            mask |= (1 << i);

        values[0] = (value0 & ~mask) | (values[0] & mask);
    }
	
lbl_done:
	/* terminate transfer */
	ls2k_i2c_send_stop(bus, MFRC522_ADDRESS);

    if (rt != 0)
    {
        printk("error: return %i, %s.\r\n", rt, __func__);
    }
        
	return rt;
} 

/*
 * Sets the bits given in mask in register reg.
 */
static int PCD_SetRegisterBitMask(const void *bus, unsigned char reg, unsigned char mask)
{
    unsigned char tmp = 0;
    PCD_ReadRegister(bus, reg, &tmp);
    return PCD_WriteRegister(bus, reg, tmp | mask);
}

/*
 * Clears the bits given in mask from register reg.
 */
static int PCD_ClearRegisterBitMask(const void *bus, unsigned char reg, unsigned char mask)
{
    unsigned char tmp = 0;
    PCD_ReadRegister(bus, reg, &tmp);
    return PCD_WriteRegister(bus, reg, tmp & (~mask));
}

/*
 * Use the CRC coprocessor in the MFRC522 to calculate a CRC_A.
 *
 * @return 0 on success, -1 otherwise.
 */
static int PCD_CalculateCRC(const void *bus, unsigned char *data, unsigned char length, unsigned char *result)
{
    int i = 5000;

    PCD_WriteRegister(bus, COMMANDREG, PCD_Idle);		    // Stop any active command.
    PCD_WriteRegister(bus, DIVIRQREG, 0x04);				// Clear the CRCIRq interrupt request bit
    PCD_SetRegisterBitMask(bus, FIFOLEVELREG, 0x80);		// FlushBuffer = 1, FIFO initialization
    PCD_WriteRegisters(bus, FIFODATAREG, length, data);	    // Write data to the FIFO
    PCD_WriteRegister(bus, COMMANDREG, PCD_CalcCRC);		// Start the calculation

    /* Wait for the CRC calculation to complete. Each iteration of the while-loop takes 17.73?s.
     */
    while (1)
    {
        unsigned char tmp = 0;
        PCD_ReadRegister(bus, DIVIRQREG, &tmp);     /* DIVIRQREG[7..0] bits are: Set2 reserved reserved MfinActIRq
                                                       reserved CRCIRq reserved reserved */
        if (tmp & 0x04)  						    // CRCIRq bit set - calculation done
        {
            break;
        }

        if (--i == 0)  						        /* The emergency break. We will eventually terminate on this one
                                                       after 89ms. Communication with the MFRC522 might be down. */
        {
            return -1;
        }
    }
    
    PCD_WriteRegister(bus, COMMANDREG, PCD_Idle);   // Stop calculating CRC for new content in the FIFO.

    // Transfer the result from the registers to the result buffer
    PCD_ReadRegister(bus, CRCRESULTREGL, &result[0]);
    PCD_ReadRegister(bus, CRCRESULTREGH, &result[1]);

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Functions for manipulating the MFRC522
//-------------------------------------------------------------------------------------------------

/*
 * Initializes the MFRC522 chip.
 */
/*static */int PCD_Init(const void *bus)
{
#if 0
    // Set the resetPowerDownPin as digital output, do not reset or power down.
    pinMode(_resetPowerDownPin, OUTPUT);

    if (digitalRead(_resetPowerDownPin) == LOW)  	//The MFRC522 chip is in power down mode.
    {
        digitalWrite(_resetPowerDownPin, HIGH);		// Exit power down mode. This triggers a hard reset.
        // Section 8.8.2 in the datasheet says the oscillator start-up time is the start up time of the crystal + 37,74?s. Let us be generous: 50ms.
        delay(100);
    }
    else   // Perform a soft reset
    {
        PCD_Reset(bus);
    }
#else

    gpio_enable(RC522_RESET_PIN, GPIO_OUT);     // reset pin
    gpio_enable(RC522_IRQ_PIN, GPIO_IN);        // irq pin

    /* hardware reset */
    gpio_write(RC522_RESET_PIN, 0);
    osal_msleep(50);
    gpio_write(RC522_RESET_PIN, 1);

#endif

    /* When communicating with a PICC we need a timeout if something goes wrong.
       f_timer = 13.56 MHz / (2*TPreScaler+1) where TPreScaler = [TPrescaler_Hi:TPrescaler_Lo].
       TPrescaler_Hi are the four low bits in TMODEREG. TPrescaler_Lo is TPRESCALERREG. */
    
    PCD_WriteRegister(bus, TMODEREG, 0x80);	        // TAuto=1; timer starts automatically at the end of the transmission in all communication modes at all speeds
    PCD_WriteRegister(bus, TPRESCALERREG, 0xA9);    // TPreScaler = TMODEREG[3..0]:TPRESCALERREG, ie 0x0A9 = 169 => f_timer=40kHz, ie a timer period of 25?s.
    PCD_WriteRegister(bus, TRELOADREGH, 0x03);		// Reload timer with 0x3E8 = 1000, ie 25ms before timeout.
    PCD_WriteRegister(bus, TRELOADREGL, 0xE8);

    PCD_WriteRegister(bus, TXASKREG, 0x40);		    // Default 0x00. Force a 100 % ASK modulation independent of the MODGSPREG register setting
    PCD_WriteRegister(bus, MODEREG, 0x3D);		    // Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)
    PCD_AntennaOn(bus);						        // Enable the antenna driver pins TX1 and TX2 (they were disabled by the reset)
    
    return 0;
}

/*
 * Performs a soft reset on the MFRC522 chip and waits for it to be ready again.
 */
static int PCD_Reset(const void *bus)
{
    int tmo = 100;
    unsigned char tmp = 0;
    
    PCD_WriteRegister(bus, COMMANDREG, PCD_SoftReset);	// Issue the SoftReset command.
    
    /* The datasheet does not mention how long the SoftRest command takes to complete.
       But the MFRC522 might have been in soft power-down mode (triggered by bit 4 of COMMANDREG)
       Section 8.8.2 in the datasheet says the oscillator start-up time is the start up time of
       the crystal + 37,74?s. Let us be generous: 50ms. */
       
    while (tmo-- > 0)
    {
        osal_msleep(1);
        PCD_ReadRegister(bus, COMMANDREG, &tmp);
        if (!(tmp & (1 << 4)))
        {
            return 0;
        }
    }
    
    return -1;

#if 0
    osal_msleep(50);
    
    // Wait for the PowerDown bit in COMMANDREG to be cleared
    while (PCD_ReadRegister(COMMANDREG) & (1<<4))
    {
        printkln("PCD Still restarting after SoftReset");
        // PCD still restarting - unlikely after waiting 50ms, but better safe than sorry.
    }
#endif

}

/*
 * Turns the antenna on by enabling pins TX1 and TX2.
 * After a reset these pins are disabled.
 */
static void PCD_AntennaOn(const void *bus)
{
    unsigned char tmp = 0;
    
    PCD_ReadRegister(bus, TXCONTROLREG, &tmp);
    if ((tmp & 0x03) != 0x03)
    {
        PCD_WriteRegister(bus, TXCONTROLREG, tmp | 0x03);
    }
}

/*
 * Turns the antenna off by disabling pins TX1 and TX2.
 */
static void PCD_AntennaOff(const void *bus)
{
    PCD_ClearRegisterBitMask(bus, TXCONTROLREG, 0x03);
}

/*
 * Get the current MFRC522 Receiver Gain (RxGain[2:0]) value.
 * See 9.3.3.6 / table 98 in http://www.nxp.com/documents/data_sheet/MFRC522.pdf
 * NOTE: Return value scrubbed with (0x07<<4)=01110000b as RCFfgReg may use reserved bits.
 *
 * @return Value of the RxGain, scrubbed to the 3 bits used.
 */
static unsigned char PCD_GetAntennaGain(const void *bus)
{
    unsigned char tmp = 0;
    PCD_ReadRegister(bus, RFCFGREG, &tmp);
    return tmp & (0x07<<4);
}

/*
 * Set the MFRC522 Receiver Gain (RxGain) to value specified by given mask.
 * See 9.3.3.6 / table 98 in http://www.nxp.com/documents/data_sheet/MFRC522.pdf
 * NOTE: Given mask is scrubbed with (0x07<<4)=01110000b as RCFfgReg may use reserved bits.
 */
static void PCD_SetAntennaGain(const void *bus, unsigned char mask)
{
    if (PCD_GetAntennaGain(bus) != mask)  						    // only bother if there is a change
    {
        PCD_ClearRegisterBitMask(bus, RFCFGREG, (0x07<<4));		// clear needed to allow 000 pattern
        PCD_SetRegisterBitMask(bus, RFCFGREG, mask & (0x07<<4));	// only set RxGain[2:0] bits
    }
}

/*
 * Performs a self-test of the MFRC522
 * See 16.1.1 in http://www.nxp.com/documents/data_sheet/MFRC522.pdf
 *
 * @return Whether or not the test passed.
 */
static bool PCD_PerformSelfTest(const void *bus)
{
    int i;
    unsigned char version, result[64], ZEROES[25] = {0x00};
    const unsigned char *reference;
    
    /* This follows directly the steps outlined in 16.1.1
     */
    
    // 1. Perform a soft reset.
    PCD_Reset(bus);

    // 2. Clear the internal buffer by writing 25 bytes of 00h
    PCD_SetRegisterBitMask(bus, FIFOLEVELREG, 0x80);	// flush the FIFO buffer
    PCD_WriteRegisters(bus, FIFODATAREG, 25, ZEROES);	// write 25 bytes of 00h to FIFO
    PCD_WriteRegister(bus, COMMANDREG, PCD_Mem);		// transfer to internal buffer

    // 3. Enable self-test
    PCD_WriteRegister(bus, AUTOTESTREG, 0x09);

    // 4. Write 00h to FIFO buffer
    PCD_WriteRegister(bus, FIFODATAREG, 0x00);

    // 5. Start self-test by issuing the CalcCRC command
    PCD_WriteRegister(bus, COMMANDREG, PCD_CalcCRC);

    // 6. Wait for self-test to complete
    for (i = 0; i < 0xFF; i++)
    {
        unsigned char tmp;
        PCD_ReadRegister(bus, DIVIRQREG, &tmp);	    // DIVIRQREG[7..0] bits are: Set2 reserved reserved MfinActIRq reserved CRCIRq reserved reserved
        if (tmp & 0x04)  					        // CRCIRq bit set - calculation done
        {
            break;
        }
    }
    
    PCD_WriteRegister(bus, COMMANDREG, PCD_Idle);   // Stop calculating CRC for new content in the FIFO.

    // 7. Read out resulting 64 bytes from the FIFO buffer.
    PCD_ReadRegisters(bus, FIFODATAREG, 64, result, 0);

    // Auto self-test done
    // Reset AUTOTESTREG register to be 0 again. Required for normal operation.
    PCD_WriteRegister(bus, AUTOTESTREG, 0x00);

    // Determine firmware version (see section 9.3.4.8 in spec)
    PCD_ReadRegister(bus, VERSIONREG, &version);

    // Pick the appropriate reference values
    switch (version)
    {
        case 0x88:	    // Fudan Semiconductor FM17522 clone
            reference = FM17522_firmware_reference;
            break;
        case 0x90:	    // Version 0.0
            reference = MFRC522_firmware_referenceV0_0;
            break;
        case 0x91:	    // Version 1.0
            reference = MFRC522_firmware_referenceV1_0;
            break;
        case 0x92:	    // Version 2.0
            reference = MFRC522_firmware_referenceV2_0;
            break;
        default:	    // Unknown version
            return false;
    }

#if 0
    // Verify that the results match up to our expectations
    for (i = 0; i < 64; i++)
    {
        if (result[i] != pgm_read_byte(&(reference[i])))
            return false;
    }
#endif

    return true;        // Test passed; all is good.
}

//-------------------------------------------------------------------------------------------------
// Functions for communicating with PICCs
//-------------------------------------------------------------------------------------------------

/*
 * Transfers data to the MFRC522 FIFO, executes a command, waits for completion and transfers data
 * back from the FIFO. CRC validation can only be done if backData and backLen are specified.
 *
 * @return 0 on success, -1 otherwise.
 */
static int PCD_CommunicateWithPICC(const void *bus,
                                   unsigned char command,
                                   unsigned char waitIRq,
                                   unsigned char *sendData,
                                   unsigned char sendLen,
                                   unsigned char *backData,
                                   unsigned char *backLen,
                                   unsigned char *validBits,
                                   unsigned char rxAlign,
                                   bool checkCRC )
{
    int i;
    unsigned char tmp, _validBits, txLastBits, bitFraming, errorRegValue;

    // Prepare values for BITFRAMINGREG
    txLastBits = validBits ? *validBits : 0;
    bitFraming = (rxAlign << 4) + txLastBits;		            // RxAlign = BITFRAMINGREG[6..4]. TxLastBits = BITFRAMINGREG[2..0]

    PCD_WriteRegister(bus, COMMANDREG, PCD_Idle);			    // Stop any active command.
    PCD_WriteRegister(bus, COMIRQREG, 0x7F);					// Clear all seven interrupt request bits
    PCD_SetRegisterBitMask(bus, FIFOLEVELREG, 0x80);			// FlushBuffer = 1, FIFO initialization
    PCD_WriteRegisters(bus, FIFODATAREG, sendLen, sendData);	// Write sendData to the FIFO
    PCD_WriteRegister(bus, BITFRAMINGREG, bitFraming);		    // Bit adjustments
    PCD_WriteRegister(bus, COMMANDREG, command);				// Execute the command
    if (command == PCD_Transceive)
    {
        PCD_SetRegisterBitMask(bus, BITFRAMINGREG, 0x80);	    // StartSend=1, transmission of data starts
    }

    /* Wait for the command to complete.
       In PCD_Init() we set the TAuto flag in TMODEREG. This means the timer automatically starts when the PCD stops transmitting.
       Each iteration of the do-while-loop takes 17.86?s. */
       
    i = 2000;
    while (1)
    {
        PCD_ReadRegister(bus, COMIRQREG, &tmp);	    // COMIRQREG[7..0] bits are: Set1 TxIRq RxIRq IdleIRq HiAlertIRq LoAlertIRq ErrIRq TimerIRq
        if (tmp & waitIRq)  					    // One of the interrupts that signal success has been set.
        {
            break;
        }

        if (tmp & 0x01)  						    // Timer interrupt - nothing received in 25ms
        {
            return STATUS_TIMEOUT;
        }

        if (--i == 0)  					 	        // The emergency break. If all other condions fail we will eventually terminate on this one after 35.7ms.
                                                    // Communication with the MFRC522 might be down.
        {
            return STATUS_TIMEOUT;
        }
    }

    // Stop now if any errors except collisions were detected.
    PCD_ReadRegister(bus, ERRORREG, &errorRegValue);    // ERRORREG[7..0] bits are: WrErr TempErr reserved BufferOvfl CollErr CRCErr ParityErr ProtocolErr
    if (errorRegValue & 0x13)  	                        // BufferOvfl ParityErr ProtocolErr
    {
        return STATUS_ERROR;
    }

    // If the caller wants data back, get it from the MFRC522.
    if (backData && backLen)
    {
        PCD_ReadRegister(bus, FIFOLEVELREG, &tmp);		// Number of bytes in the FIFO
        if (tmp > *backLen)
        {
            return STATUS_NO_ROOM;
        }

        *backLen = tmp;											        // Number of bytes returned
        PCD_ReadRegisters(bus, FIFODATAREG, tmp, backData, rxAlign);	// Get received data from FIFO
        PCD_ReadRegister(bus, CONTROLREG, &_validBits);
        _validBits &= 0x07;		                        // RxLastBits[2:0] indicates the number of valid bits in the last received byte.
                                                        // If this value is 000b, the whole byte is valid.
        if (validBits)
        {
            *validBits = _validBits;
        }
    }

    // Tell about collisions
    if (errorRegValue & 0x08)  		// CollErr
    {
        return STATUS_COLLISION;
    }

    // Perform CRC_A validation if requested.
    if (backData && backLen && checkCRC)
    {
        unsigned char controlBuffer[2];
        
        // In this case a MIFARE Classic NAK is not OK.
        if (*backLen == 1 && _validBits == 4)
        {
            return STATUS_MIFARE_NACK;
        }

        // We need at least the CRC_A value and all 8 bits of the last byte must be received.
        if (*backLen < 2 || _validBits != 0)
        {
            return STATUS_CRC_WRONG;
        }

        // Verify CRC_A - do our own calculation and store the control in controlBuffer.
        
        tmp = PCD_CalculateCRC(bus, &backData[0], *backLen - 2, &controlBuffer[0]);
        if (tmp != 0)
        {
            return tmp;
        }

        if ((backData[*backLen - 2] != controlBuffer[0]) || (backData[*backLen - 1] != controlBuffer[1]))
        {
            return STATUS_CRC_WRONG;
        }
    }

    return 0;
}

/*
 * Executes the Transceive command.
 * CRC validation can only be done if backData and backLen are specified.
 *
 * @return 0 on success, -1 otherwise.
 */
static int PCD_TransceiveData(const void *bus,
                              unsigned char *sendData,
                              unsigned char sendLen,
                              unsigned char *backData,
                              unsigned char *backLen,
                              unsigned char *validBits,
                              unsigned char rxAlign,
                              bool checkCRC )
{
    unsigned char waitIRq = 0x30;		// RxIRq and IdleIRq

    return PCD_CommunicateWithPICC(bus,
                                   PCD_Transceive,
                                   waitIRq,
                                   sendData,
                                   sendLen,
                                   backData,
                                   backLen,
                                   validBits,
                                   rxAlign,
                                   checkCRC);
}

/*
 * Transmits REQA or WUPA commands.
 * Beware: When two PICCs are in the field at the same time I often get STATUS_TIMEOUT - probably due do bad antenna design.
 *
 * @return 0 on success, -1 otherwise.
 */
static int PICC_REQA_or_WUPA(const void *bus, unsigned char command, unsigned char *bufferATQA, unsigned char *bufferSize)
{
    unsigned char validBits;
    int status;

    if (bufferATQA == NULL || *bufferSize < 2)  	// The ATQA response is 2 bytes long.
    {
        return -1; // STATUS_NO_ROOM;
    }
    
    PCD_ClearRegisterBitMask(bus, COLLREG, 0x80);	// ValuesAfterColl=1 => Bits received after collision are cleared.
    validBits = 7;									// For REQA and WUPA we need the short frame format - transmit only 7 bits of the last (and only) byte. TxLastBits = BITFRAMINGREG[2..0]
    status = PCD_TransceiveData(bus,
                                &command,
                                1,
                                bufferATQA,
                                bufferSize,
                                &validBits,
                                0,
                                false);
    if (status != STATUS_OK)
    {
        return status;
    }

    if (*bufferSize != 2 || validBits != 0)  		// ATQA must be exactly 16 bits.
    {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

/*
 * Transmits a REQuest command, Type A. Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
 * Beware: When two PICCs are in the field at the same time I often get STATUS_TIMEOUT - probably due do bad antenna design.
 *
 * @return 0 on success, -1 otherwise.
 */
static int PICC_RequestA(const void *bus, unsigned char *bufferATQA, unsigned char *bufferSize)
{
    return PICC_REQA_or_WUPA(bus, PICC_CMD_REQA, bufferATQA, bufferSize);
}

/*
 * Transmits a Wake-UP command, Type A. Invites PICCs in state IDLE and HALT to go to READY(*) and prepare for anticollision or selection. 7 bit frame.
 * Beware: When two PICCs are in the field at the same time I often get STATUS_TIMEOUT - probably due do bad antenna design.
 *
 * @return 0 on success, -1 otherwise.
 */
static int PICC_WakeupA(const void *bus, unsigned char *bufferATQA, unsigned char *bufferSize)
{
    return PICC_REQA_or_WUPA(bus, PICC_CMD_WUPA, bufferATQA, bufferSize);
}

/*
 * Transmits SELECT/ANTICOLLISION commands to select a single PICC.
 * Before calling this function the PICCs must be placed in the READY(*) state by calling PICC_RequestA() or PICC_WakeupA().
 * On success:
 * 		- The chosen PICC is in state ACTIVE(*) and all other PICCs have returned to state IDLE/HALT. (Figure 7 of the ISO/IEC 14443-3 draft.)
 * 		- The UID size and value of the chosen PICC is returned in *uid along with the SAK.
 *
 * A PICC UID consists of 4, 7 or 10 bytes.
 * Only 4 bytes can be specified in a SELECT command, so for the longer UIDs two or three iterations are used:
 * 		UID size	Number of UID bytes		Cascade levels		Example of PICC
 * 		========	===================		==============		===============
 * 		single				 4						1				MIFARE Classic
 * 		double				 7						2				MIFARE Ultralight
 * 		triple				10						3				Not currently in use?
 *
 * @return 0 on success, -1 otherwise.
 */
static int PICC_Select(const void *bus, UID_t *uid, unsigned char validBits)
{
    bool uidComplete;
    bool selectDone;
    bool useCascadeTag;
    unsigned char cascadeLevel = 1;
    unsigned char result;
    unsigned char count;
    unsigned char index;
    unsigned char uidIndex;			// The first index in uid->uidByte[] that is used in the current Cascade Level.
    char currentLevelKnownBits;		// The number of known UID bits in the current Cascade Level.
    unsigned char buffer[9];		// The SELECT/ANTICOLLISION commands uses a 7 byte standard frame + 2 bytes CRC_A
    unsigned char bufferUsed;		// The number of bytes used in the buffer, ie the number of bytes to transfer to the FIFO.
    unsigned char rxAlign;			// Used in BITFRAMINGREG. Defines the bit position for the first bit received.
    unsigned char txLastBits;		// Used in BITFRAMINGREG. The number of valid bits in the last transmitted byte.
    unsigned char *responseBuffer;
    unsigned char responseLength;

    // Description of buffer structure:
    //		Byte 0: SEL 				Indicates the Cascade Level: PICC_CMD_SEL_CL1, PICC_CMD_SEL_CL2 or PICC_CMD_SEL_CL3
    //		Byte 1: NVB					Number of Valid Bits (in complete command, not just the UID): High nibble: complete bytes, Low nibble: Extra bits.
    //		Byte 2: UID-data or CT		See explanation below. CT means Cascade Tag.
    //		Byte 3: UID-data
    //		Byte 4: UID-data
    //		Byte 5: UID-data
    //		Byte 6: BCC					Block Check Character - XOR of bytes 2-5
    //		Byte 7: CRC_A
    //		Byte 8: CRC_A
    // The BCC and CRC_A is only transmitted if we know all the UID bits of the current Cascade Level.
    //
    // Description of bytes 2-5: (Section 6.5.4 of the ISO/IEC 14443-3 draft: UID contents and cascade levels)
    //		UID size	Cascade level	Byte2	Byte3	Byte4	Byte5
    //		========	=============	=====	=====	=====	=====
    //		 4 bytes		1			uid0	uid1	uid2	uid3
    //		 7 bytes		1			CT		uid0	uid1	uid2
    //						2			uid3	uid4	uid5	uid6
    //		10 bytes		1			CT		uid0	uid1	uid2
    //						2			CT		uid3	uid4	uid5
    //						3			uid6	uid7	uid8	uid9

    // Sanity checks
    if (validBits > 80)
    {
        return STATUS_INVALID;
    }

    // Prepare MFRC522
    PCD_ClearRegisterBitMask(bus, COLLREG, 0x80);		// ValuesAfterColl=1 => Bits received after collision are cleared.

    // Repeat Cascade Level loop until we have a complete UID.
    uidComplete = false;
    while (!uidComplete)
    {
        unsigned char bytesToCopy;
        
        // Set the Cascade Level in the SEL byte, find out if we need to use the Cascade Tag in byte 2.
        switch (cascadeLevel)
        {
            case 1:
                buffer[0] = PICC_CMD_SEL_CL1;
                uidIndex = 0;
                useCascadeTag = validBits && uid->size > 4;	// When we know that the UID has more than 4 bytes
                break;

            case 2:
                buffer[0] = PICC_CMD_SEL_CL2;
                uidIndex = 3;
                useCascadeTag = validBits && uid->size > 7;	// When we know that the UID has more than 7 bytes
                break;

            case 3:
                buffer[0] = PICC_CMD_SEL_CL3;
                uidIndex = 6;
                useCascadeTag = false;						// Never used in CL3.
                break;

            default:
                return STATUS_INTERNAL_ERROR;
                break;
        }

        // How many UID bits are known in this Cascade Level?
        currentLevelKnownBits = validBits - (8 * uidIndex);
        if (currentLevelKnownBits < 0)
        {
            currentLevelKnownBits = 0;
        }
        // Copy the known bits from uid->uidByte[] to buffer[]
        index = 2; // destination index in buffer[]
        if (useCascadeTag)
        {
            buffer[index++] = PICC_CMD_CT;
        }
        
        bytesToCopy = currentLevelKnownBits / 8 + (currentLevelKnownBits % 8 ? 1 : 0); // The number of bytes needed to represent the known bits for this level.
        if (bytesToCopy)
        {
            unsigned char maxBytes;

            maxBytes = useCascadeTag ? 3 : 4; // Max 4 bytes in each Cascade Level. Only 3 left if we use the Cascade Tag
            if (bytesToCopy > maxBytes)
            {
                bytesToCopy = maxBytes;
            }
            for (count = 0; count < bytesToCopy; count++)
            {
                buffer[index++] = uid->uidByte[uidIndex + count];
            }
        }
        
        // Now that the data has been copied we need to include the 8 bits in CT in currentLevelKnownBits
        if (useCascadeTag)
        {
            currentLevelKnownBits += 8;
        }

        // Repeat anti collision loop until we can transmit all UID bits + BCC and receive a SAK - max 32 iterations.
        selectDone = false;
        while (!selectDone)
        {
            // Find out how many bits and bytes to send and receive.
            if (currentLevelKnownBits >= 32)   // All UID bits in this Cascade Level are known. This is a SELECT.
            {
                //printk("SELECT: currentLevelKnownBits=")); printkln(currentLevelKnownBits, DEC);
                buffer[1] = 0x70; // NVB - Number of Valid Bits: Seven whole bytes
                // Calculate BCC - Block Check Character
                buffer[6] = buffer[2] ^ buffer[3] ^ buffer[4] ^ buffer[5];
                // Calculate CRC_A
                result = PCD_CalculateCRC(bus, buffer, 7, &buffer[7]);
                if (result != STATUS_OK)
                {
                    return result;
                }
                
                txLastBits		= 0; // 0 => All 8 bits are valid.
                bufferUsed		= 9;
                // Store response in the last 3 bytes of buffer (BCC and CRC_A - not needed after tx)
                responseBuffer	= &buffer[6];
                responseLength	= 3;
            }
            else   // This is an ANTICOLLISION.
            {
                //printk("ANTICOLLISION: currentLevelKnownBits=")); printkln(currentLevelKnownBits, DEC);
                txLastBits		= currentLevelKnownBits % 8;
                count			= currentLevelKnownBits / 8;	// Number of whole bytes in the UID part.
                index			= 2 + count;					// Number of whole bytes: SEL + NVB + UIDs
                buffer[1]		= (index << 4) + txLastBits;	// NVB - Number of Valid Bits
                bufferUsed		= index + (txLastBits ? 1 : 0);
                // Store response in the unused part of buffer
                responseBuffer	= &buffer[index];
                responseLength	= sizeof(buffer) - index;
            }

            // Set bit adjustments
            rxAlign = txLastBits;											// Having a seperate variable is overkill. But it makes the next line easier to read.
            PCD_WriteRegister(bus, BITFRAMINGREG, (rxAlign << 4) + txLastBits);	// RxAlign = BITFRAMINGREG[6..4]. TxLastBits = BITFRAMINGREG[2..0]

            // Transmit the buffer and receive the response.
            result = PCD_TransceiveData(bus,
                                        buffer,
                                        bufferUsed,
                                        responseBuffer,
                                        &responseLength,
                                        &txLastBits,
                                        rxAlign,
                                        false);
            if (result == STATUS_COLLISION)   // More than one PICC in the field => collision.
            {
                unsigned char collisionPos;
                
                PCD_ReadRegister(bus, COLLREG, &result); // COLLREG[7..0] bits are: ValuesAfterColl reserved CollPosNotValid CollPos[4:0]
                if (result & 0x20)           // CollPosNotValid
                {
                    return STATUS_COLLISION; // Without a valid collision position we cannot continue
                }
                
                collisionPos = result & 0x1F; // Values 0-31, 0 means bit 32.
                if (collisionPos == 0)
                {
                    collisionPos = 32;
                }
                if (collisionPos <= currentLevelKnownBits)   // No progress - should not happen
                {
                    return STATUS_INTERNAL_ERROR;
                }
                
                // Choose the PICC with the bit set.
                currentLevelKnownBits = collisionPos;
                count			= (currentLevelKnownBits - 1) % 8; // The bit to modify
                index			= 1 + (currentLevelKnownBits / 8) + (count ? 1 : 0); // First byte is index 0.
                buffer[index]	|= (1 << count);
            }
            else if (result != STATUS_OK)
            {
                return result;
            }
            else   // STATUS_OK
            {
                if (currentLevelKnownBits >= 32)   // This was a SELECT.
                {
                    selectDone = true; // No more anticollision
                    // We continue below outside the while.
                }
                else   // This was an ANTICOLLISION.
                {
                    // We now have all 32 bits of the UID in this Cascade Level
                    currentLevelKnownBits = 32;
                    // Run loop again to do the SELECT.
                }
            }
        } // End of while (!selectDone)

        // We do not check the CBB - it was constructed by us above.

        // Copy the found UID bytes from buffer[] to uid->uidByte[]
        index			= (buffer[2] == PICC_CMD_CT) ? 3 : 2; // source index in buffer[]
        bytesToCopy		= (buffer[2] == PICC_CMD_CT) ? 3 : 4;
        for (count = 0; count < bytesToCopy; count++)
        {
            uid->uidByte[uidIndex + count] = buffer[index++];
        }

        // Check response SAK (Select Acknowledge)
        if (responseLength != 3 || txLastBits != 0)   // SAK must be exactly 24 bits (1 byte + CRC_A).
        {
            return STATUS_ERROR;
        }
        // Verify CRC_A - do our own calculation and store the control in buffer[2..3] - those bytes are not needed anymore.
        result = PCD_CalculateCRC(bus, responseBuffer, 1, &buffer[2]);
        if (result != STATUS_OK)
        {
            return result;
        }
        if ((buffer[2] != responseBuffer[1]) || (buffer[3] != responseBuffer[2]))
        {
            return STATUS_CRC_WRONG;
        }
        if (responseBuffer[0] & 0x04)   // Cascade bit set - UID not complete yes
        {
            cascadeLevel++;
        }
        else
        {
            uidComplete = true;
            uid->sak = responseBuffer[0];
        }
    } 

    // Set correct uid->size
    uid->size = 3 * cascadeLevel + 1;

    return STATUS_OK;
}

/*
 * Instructs a PICC in state ACTIVE(*) to go to state HALT.
 *
 * @return 0 on success, -1 otherwise.
 */
/*static*/ int PICC_HaltA(const void *bus)
{
    unsigned char result, buffer[4];

    // Build command buffer
    buffer[0] = PICC_CMD_HLTA;
    buffer[1] = 0;
    // Calculate CRC_A
    result = PCD_CalculateCRC(bus, buffer, 2, &buffer[2]);
    if (result != STATUS_OK)
    {
        return result;
    }

    // Send the command.
    // The standard says:
    //		If the PICC responds with any modulation during a period of 1 ms after the end of the frame containing the
    //		HLTA command, this response shall be interpreted as 'not acknowledge'.
    // We interpret that this way: Only STATUS_TIMEOUT is an success.
    
    result = PCD_TransceiveData(bus,
                                buffer,
                                sizeof(buffer),
                                NULL,
                                0,
                                NULL,
                                0,
                                false);
    if (result == STATUS_TIMEOUT)
    {
        return STATUS_OK;
    }
    if (result == STATUS_OK)   // That is ironically NOT ok in this case ;-)
    {
        return STATUS_ERROR;
    }
    
    return result;
} // End PICC_HaltA()

//-------------------------------------------------------------------------------------------------
// Functions for communicating with MIFARE PICCs
//-------------------------------------------------------------------------------------------------

/*
 * Executes the MFRC522 MFAuthent command.
 * This command manages MIFARE authentication to enable a secure communication to any MIFARE Mini, MIFARE 1K and MIFARE 4K card.
 * The authentication is described in the MFRC522 datasheet section 10.3.1.9 and http://www.nxp.com/documents/data_sheet/MF1S503x.pdf section 10.1.
 * For use with MIFARE Classic PICCs.
 * The PICC must be selected - ie in state ACTIVE(*) - before calling this function.
 * Remember to call PCD_StopCrypto1() after communicating with the authenticated PICC - otherwise no new communications can start.
 *
 * All keys are set to FFFFFFFFFFFFh at chip delivery.
 *
 * @return 0 on success, -1 otherwise. Probably STATUS_TIMEOUT if you supply the wrong key.
 */
/*static */int PCD_Authenticate(const void *bus,
                                unsigned char command,
                                unsigned char blockAddr,
                                MIFARE_Key_t *key,
                                UID_t *uid)
{
    int i;
    unsigned char waitIRq = 0x10;		    // IdleIRq
    unsigned char sendData[12];             // Build command buffer
    
    sendData[0] = command;
    sendData[1] = blockAddr;
    for (i = 0; i < MF_KEY_SIZE; i++)  	    // 6 key bytes
    {
        sendData[2+i] = key->keyByte[i];
    }
    
    for (i = 0; i < 4; i++)  				// The last 4 bytes of the UID
    {
        sendData[8+i] = uid->uidByte[i+uid->size-4];
    }

    // Start the authentication.
    return PCD_CommunicateWithPICC(bus,
                                   PCD_MFAuthent,
                                   waitIRq,
                                   &sendData[0],
                                   sizeof(sendData),
                                   NULL,
                                   NULL,
                                   NULL,
                                   0,
                                   false);
}

/*
 * Used to exit the PCD from its authenticated state.
 * Remember to call this function after communicating with an authenticated PICC - otherwise no new communications can start.
 */
static int PCD_StopCrypto1(const void *bus)
{
    // Clear MFCrypto1On bit
    return PCD_ClearRegisterBitMask(bus, STATUS2REG, 0x08); // STATUS2REG[7..0] bits are: TempSensClear I2CForceHS reserved reserved MFCrypto1On ModemState[2:0]
}

/*
 * Reads 16 bytes (+ 2 bytes CRC_A) from the active PICC.
 *
 * For MIFARE Classic the sector containing the block must be authenticated before calling this function.
 *
 * For MIFARE Ultralight only addresses 00h to 0Fh are decoded.
 * The MF0ICU1 returns a NAK for higher addresses.
 * The MF0ICU1 responds to the READ command by sending 16 bytes starting from the page address defined by the command argument.
 * For example; if blockAddr is 03h then pages 03h, 04h, 05h, 06h are returned.
 * A roll-back is implemented: If blockAddr is 0Eh, then the contents of pages 0Eh, 0Fh, 00h and 01h are returned.
 *
 * The buffer must be at least 18 bytes because a CRC_A is also returned.
 * Checks the CRC_A before returning STATUS_OK.
 *
 * @return 0 on success, -1 otherwise.
 */
/*static*/ int MIFARE_Read(const void *bus,
                           unsigned char blockAddr,
                           unsigned char *buffer,
                           unsigned char *bufferSize)
{
    unsigned char result;

    // Sanity check
    if (buffer == NULL || *bufferSize < 18)
    {
        return STATUS_NO_ROOM;
    }

    // Build command buffer
    buffer[0] = PICC_CMD_MF_READ;
    buffer[1] = blockAddr;
    // Calculate CRC_A
    result = PCD_CalculateCRC(bus, buffer, 2, &buffer[2]);
    if (result != STATUS_OK)
    {
        return result;
    }

    // Transmit the buffer and receive the response, validate CRC_A.
    return PCD_TransceiveData(bus,
                              buffer,
                              4,
                              buffer,
                              bufferSize,
                              NULL,
                              0,
                              true);
}

/*
 * Writes 16 bytes to the active PICC.
 *
 * For MIFARE Classic the sector containing the block must be authenticated before calling this function.
 *
 * For MIFARE Ultralight the operation is called "COMPATIBILITY WRITE".
 * Even though 16 bytes are transferred to the Ultralight PICC, only the least significant 4 bytes (bytes 0 to 3)
 * are written to the specified address. It is recommended to set the remaining bytes 04h to 0Fh to all logic 0.
 * *
 * @return 0 on success, -1 otherwise.
 */
/*static*/ int MIFARE_Write(const void *bus,
                            unsigned char blockAddr,
                            unsigned char *buffer,
                            unsigned char bufferSize)
{
    unsigned char result, cmdBuffer[2];

    // Sanity check
    if (buffer == NULL || bufferSize < 16)
    {
        return STATUS_INVALID;
    }

    // Mifare Classic protocol requires two communications to perform a write.
    // Step 1: Tell the PICC we want to write to block blockAddr.
    cmdBuffer[0] = PICC_CMD_MF_WRITE;
    cmdBuffer[1] = blockAddr;
    result = PCD_MIFARE_Transceive(bus, cmdBuffer, 2, false); // Adds CRC_A and checks that the response is MF_ACK.
    if (result != STATUS_OK)
    {
        return result;
    }

    // Step 2: Transfer the data
    result = PCD_MIFARE_Transceive(bus, buffer, bufferSize, false); // Adds CRC_A and checks that the response is MF_ACK.
    if (result != STATUS_OK)
    {
        return result;
    }

    return STATUS_OK;
}

/*
 * Writes a 4 byte page to the active MIFARE Ultralight PICC.
 *
 * @return 0 on success, -1 otherwise.
 */
static int MIFARE_Ultralight_Write(const void *bus,
                                   unsigned char page,
                                   unsigned char *buffer,
                                   unsigned char bufferSize)
{
    unsigned char result, cmdBuffer[6];

    // Sanity check
    if (buffer == NULL || bufferSize < 4)
    {
        return STATUS_INVALID;
    }

    // Build commmand buffer
    cmdBuffer[0] = PICC_CMD_UL_WRITE;
    cmdBuffer[1] = page;
    memcpy(&cmdBuffer[2], buffer, 4);

    // Perform the write
    result = PCD_MIFARE_Transceive(bus, cmdBuffer, 6, false); // Adds CRC_A and checks that the response is MF_ACK.
    if (result != STATUS_OK)
    {
        return result;
    }
    
    return STATUS_OK;
}

static int MIFARE_TwoStepHelper(const void *bus, unsigned char command, unsigned char blockAddr, int data);

/*
 * MIFARE Decrement subtracts the delta from the value of the addressed block, and stores the result in a volatile memory.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 * Use MIFARE_Transfer() to store the result in a block.
 *
 * @return 0 on success, -1 otherwise.
 */
/*static*/ int MIFARE_Decrement(const void *bus, unsigned char blockAddr, int delta)
{
    return MIFARE_TwoStepHelper(bus, PICC_CMD_MF_DECREMENT, blockAddr, delta);
}

/*
 * MIFARE Increment adds the delta to the value of the addressed block, and stores the result in a volatile memory.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 * Use MIFARE_Transfer() to store the result in a block.
 *
 * @return 0 on success, -1 otherwise.
 */
/*static*/ int MIFARE_Increment(const void *bus, unsigned char blockAddr, int delta)
{
    return MIFARE_TwoStepHelper(bus, PICC_CMD_MF_INCREMENT, blockAddr, delta);
}

/*
 * MIFARE Restore copies the value of the addressed block into a volatile memory.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 * Use MIFARE_Transfer() to store the result in a block.
 *
 * @return 0 on success, -1 otherwise.
 */
static int MIFARE_Restore(const void *bus, unsigned char blockAddr)
{
    // The datasheet describes Restore as a two step operation, but does not explain what data to transfer in step 2.
    // Doing only a single step does not work, so I chose to transfer 0L in step two.
    return MIFARE_TwoStepHelper(bus, PICC_CMD_MF_RESTORE, blockAddr, 0L);
}

/*
 * Helper function for the two-step MIFARE Classic protocol operations Decrement, Increment and Restore.
 *
 * @return 0 on success, -1 otherwise.
 */
static int MIFARE_TwoStepHelper(const void *bus, unsigned char command, unsigned char blockAddr, int data)
{
    unsigned char result, cmdBuffer[2]; // We only need room for 2 bytes.

    // Step 1: Tell the PICC the command and block address
    cmdBuffer[0] = command;
    cmdBuffer[1] = blockAddr;
    result = PCD_MIFARE_Transceive(bus, cmdBuffer, 2, false); // Adds CRC_A and checks that the response is MF_ACK.
    if (result != STATUS_OK)
    {
        return result;
    }

    // Step 2: Transfer the data
    result = PCD_MIFARE_Transceive(bus,	(unsigned char *)&data, 4, true); // Adds CRC_A and accept timeout as success.
    if (result != STATUS_OK)
    {
        return result;
    }

    return STATUS_OK;
}

/*
 * MIFARE Transfer writes the value stored in the volatile memory into one MIFARE Classic block.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 *
 * @return 0 on success, -1 otherwise.
 */
static int MIFARE_Transfer(const void *bus, unsigned char blockAddr)
{
    unsigned char result, cmdBuffer[2]; // We only need room for 2 bytes.

    // Tell the PICC we want to transfer the result into block blockAddr.
    cmdBuffer[0] = PICC_CMD_MF_TRANSFER;
    cmdBuffer[1] = blockAddr;
    result = PCD_MIFARE_Transceive(bus,	cmdBuffer, 2, false); // Adds CRC_A and checks that the response is MF_ACK.
    if (result != STATUS_OK)
    {
        return result;
    }
    
    return STATUS_OK;
}

/*
 * Helper routine to read the current value from a Value Block.
 *
 * Only for MIFARE Classic and only for blocks in "value block" mode, that
 * is: with access bits [C1 C2 C3] = [110] or [001]. The sector containing
 * the block must be authenticated before calling this function.
 *
 * @param[in]   blockAddr   The block (0x00-0xff) number.
 * @param[out]  value       Current value of the Value Block.
 * @return 0 on success, -1 otherwise.
  */
static int MIFARE_GetValue(const void *bus, unsigned char blockAddr, int *value)
{
    unsigned char status;
    unsigned char buffer[18];
    unsigned char size = sizeof(buffer);

    // Read the block
    status = MIFARE_Read(bus, blockAddr, buffer, &size);
    if (status == STATUS_OK)
    {
        // Extract the value
        *value = ((int)buffer[3]<<24) | ((int)buffer[2]<<16) | ((int)buffer[1]<<8) | (int)buffer[0];
    }
    return status;
}

/*
 * Helper routine to write a specific value into a Value Block.
 *
 * Only for MIFARE Classic and only for blocks in "value block" mode, that
 * is: with access bits [C1 C2 C3] = [110] or [001]. The sector containing
 * the block must be authenticated before calling this function.
 *
 * @param[in]   blockAddr   The block (0x00-0xff) number.
 * @param[in]   value       New value of the Value Block.
 * @return 0 on success, -1 otherwise.
 */
static int MIFARE_SetValue(const void *bus, unsigned char blockAddr, int value)
{
    unsigned char buffer[18];

    // Translate the long into 4 bytes; repeated 2x in value block
    buffer[0] = buffer[ 8] = (value & 0xFF);
    buffer[1] = buffer[ 9] = (value & 0xFF00) >> 8;
    buffer[2] = buffer[10] = (value & 0xFF0000) >> 16;
    buffer[3] = buffer[11] = (value & 0xFF000000) >> 24;
    // Inverse 4 bytes also found in value block
    buffer[4] = ~buffer[0];
    buffer[5] = ~buffer[1];
    buffer[6] = ~buffer[2];
    buffer[7] = ~buffer[3];
    // Address 2x with inverse address 2x
    buffer[12] = buffer[14] = blockAddr;
    buffer[13] = buffer[15] = ~blockAddr;

    // Write the whole data block
    return MIFARE_Write(bus, blockAddr, buffer, 16);
}

//-------------------------------------------------------------------------------------------------
// Support functions
//-------------------------------------------------------------------------------------------------

/*
 * Wrapper for MIFARE protocol communication.
 * Adds CRC_A, executes the Transceive command and checks that the response is MF_ACK or a timeout.
 *
 * @return 0 on success, -1 otherwise.
 */
static int PCD_MIFARE_Transceive(const void *bus, unsigned char *sendData, unsigned char sendLen, bool acceptTimeout)
{
    unsigned char result;
    unsigned char cmdBuffer[18]; // We need room for 16 bytes data and 2 bytes CRC_A.

    // Sanity check
    if (sendData == NULL || sendLen > 16)
    {
        return STATUS_INVALID;
    }

    // Copy sendData[] to cmdBuffer[] and add CRC_A
    memcpy(cmdBuffer, sendData, sendLen);
    result = PCD_CalculateCRC(bus, cmdBuffer, sendLen, &cmdBuffer[sendLen]);
    if (result != STATUS_OK)
    {
        return result;
    }
    sendLen += 2;

    // Transceive the data, store the reply in cmdBuffer[]
    unsigned char waitIRq = 0x30;		// RxIRq and IdleIRq
    unsigned char cmdBufferSize = sizeof(cmdBuffer);
    unsigned char validBits = 0;
    result = PCD_CommunicateWithPICC(bus,
                                     PCD_Transceive,
                                     waitIRq,
                                     cmdBuffer,
                                     sendLen,
                                     cmdBuffer,
                                     &cmdBufferSize,
                                     &validBits,
                                     0,
                                     false);
    if (acceptTimeout && result == STATUS_TIMEOUT)
    {
        return STATUS_OK;
    }
    if (result != STATUS_OK)
    {
        return result;
    }
    // The PICC must reply with a 4 bit ACK
    if (cmdBufferSize != 1 || validBits != 4)
    {
        return STATUS_ERROR;
    }
    if (cmdBuffer[0] != MF_ACK)
    {
        return STATUS_MIFARE_NACK;
    }
    
    return STATUS_OK;
}

/*
 * Returns a string pointer to a status code name.
 *
 * @return const char *
 */
const char *GetStatusCodeName(unsigned char code)
{
    switch (code)
    {
        case STATUS_OK:
            return "Success.";

        case STATUS_ERROR:
            return "Error in communication.";

        case STATUS_COLLISION:
            return "Collission detected.";

        case STATUS_TIMEOUT:
            return "Timeout in communication.";

        case STATUS_NO_ROOM:
            return "A buffer is not big enough.";

        case STATUS_INTERNAL_ERROR:
            return "Internal error in the code. Should not happen.";

        case STATUS_INVALID:
            return "Invalid argument.";

        case STATUS_CRC_WRONG:
            return "The CRC_A does not match.";

        case STATUS_MIFARE_NACK:
            return "A MIFARE PICC responded with NAK.";

        default:
            break;
    }
    
    return "Unknown error";
} 

/*
 * Translates the SAK (Select Acknowledge) to a PICC type.
 *
 * @return PICC_Type
 */
/*static */int PICC_GetType(unsigned char sak)
{
    if (sak & 0x04)   // UID not complete
    {
        return PICC_TYPE_NOT_COMPLETE;
    }

    switch (sak)
    {
        case 0x09:
            return PICC_TYPE_MIFARE_MINI;

        case 0x08:
            return PICC_TYPE_MIFARE_1K;

        case 0x18:
            return PICC_TYPE_MIFARE_4K;

        case 0x00:
            return PICC_TYPE_MIFARE_UL;

        case 0x10:
        case 0x11:
            return PICC_TYPE_MIFARE_PLUS;

        case 0x01:
            return PICC_TYPE_TNP3XXX;

        default:
            break;
    }

    if (sak & 0x20)
    {
        return PICC_TYPE_ISO_14443_4;
    }

    if (sak & 0x40)
    {
        return PICC_TYPE_ISO_18092;
    }

    return PICC_TYPE_UNKNOWN;
}

/*
 * Returns a string pointer to the PICC type name.
 *
 * @return const char *
 */
const char *PICC_GetTypeName(unsigned char piccType)
{
    switch (piccType)
    {
        case PICC_TYPE_ISO_14443_4:
            return "PICC compliant with ISO/IEC 14443-4";

        case PICC_TYPE_ISO_18092:
            return "PICC compliant with ISO/IEC 18092 (NFC)";

        case PICC_TYPE_MIFARE_MINI:
            return "MIFARE Mini, 320 bytes";

        case PICC_TYPE_MIFARE_1K:
            return "MIFARE 1KB";

        case PICC_TYPE_MIFARE_4K:
            return "MIFARE 4KB";

        case PICC_TYPE_MIFARE_UL:
            return "MIFARE Ultralight or Ultralight C";

        case PICC_TYPE_MIFARE_PLUS:
            return "MIFARE Plus";

        case PICC_TYPE_TNP3XXX:
            return "MIFARE TNP3XXX";
    
        case PICC_TYPE_NOT_COMPLETE:
            return "SAK indicates UID is not complete.";
   
        case PICC_TYPE_UNKNOWN:
        default:
            break;
    }
    
    return "Unknown type";
}

/*
 * Dumps debug info about the connected PCD to Serial.
 * Shows all known firmware versions
 */
/*static*/ void PCD_DumpVersion(const void *bus)
{
    // Get the MFRC522 firmware version
    unsigned char tmp;

    PCD_ReadRegister(bus, VERSIONREG, &tmp);
    printk("Firmware Version: 0x%02X", tmp);

    // Lookup which version
    switch (tmp)
    {
        case 0x88:
            printk(" = (clone)\r\n");
            break;
        case 0x90:
            printk(" = v0.0\r\n");
            break;
        case 0x91:
            printk(" = v1.0\r\n");
            break;
        case 0x92:
            printk(" = v2.0\r\n");
            break;
        case 0x12:
            printk(" = counterfeit chip\r\n");
            break;
        default:
            printk(" = (unknown)\r\n");
    }
    
    // When 0x00 or 0xFF is returned, communication probably failed
    if ((tmp == 0x00) || (tmp == 0xFF))
        printk("WARNING: Communication failure, is the MFRC522 properly connected?\r\n");
} 

/*
 * Dumps debug info about the selected PICC to Serial.
 * On success the PICC is halted after dumping the data.
 * For MIFARE Classic the factory default key of 0xFFFFFFFFFFFF is tried.
 */
/*static*/ void PICC_Dump(const void *bus, UID_t *uid)
{
    int i;
    MIFARE_Key_t key;

    // UID
    printk("Card UID: ");
    for (i = 0; i < uid->size; i++)
    {
        printk(" %02X", uid->uidByte[i]);
    }
    printk("\r\n");

    // PICC type
    unsigned char piccType = PICC_GetType(uid->sak);

    printk("PICC type: %s\r\n", PICC_GetTypeName(piccType));

    // Dump contents
    switch (piccType)
    {
        case PICC_TYPE_MIFARE_MINI:
        case PICC_TYPE_MIFARE_1K:
        case PICC_TYPE_MIFARE_4K:
            // All keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
            for (i = 0; i < 6; i++)
            {
                key.keyByte[i] = 0xFF;
            }
            PICC_DumpMifareClassic(bus, uid, piccType, &key);
            break;

        case PICC_TYPE_MIFARE_UL:
            PICC_DumpMifareUltralight(bus);
            break;

        case PICC_TYPE_ISO_14443_4:
        case PICC_TYPE_ISO_18092:
        case PICC_TYPE_MIFARE_PLUS:
        case PICC_TYPE_TNP3XXX:
            printk("Dumping memory contents not implemented for that PICC type.\r\n");
            break;

        case PICC_TYPE_UNKNOWN:
        case PICC_TYPE_NOT_COMPLETE:
        default:
            break; // No memory dump here
    }

    printk("\r\n");
    PICC_HaltA(bus); // Already done if it was a MIFARE Classic PICC.
}

/*
 * Dumps memory contents of a MIFARE Classic PICC.
 * On success the PICC is halted after dumping the data.
 */
static void PICC_DumpMifareClassic(const void *bus,
                                   UID_t *uid,
                                   unsigned char piccType,
                                   MIFARE_Key_t *key)
{
    int i;
    unsigned char no_of_sectors = 0;
    
    switch (piccType)
    {
        case PICC_TYPE_MIFARE_MINI:
            // Has 5 sectors * 4 blocks/sector * 16 bytes/block = 320 bytes.
            no_of_sectors = 5;
            break;

        case PICC_TYPE_MIFARE_1K:
            // Has 16 sectors * 4 blocks/sector * 16 bytes/block = 1024 bytes.
            no_of_sectors = 16;
            break;

        case PICC_TYPE_MIFARE_4K:
            // Has (32 sectors * 4 blocks/sector + 8 sectors * 16 blocks/sector) * 16 bytes/block = 4096 bytes.
            no_of_sectors = 40;
            break;

        default: // Should not happen. Ignore.
            break;
    }

    // Dump sectors, highest address first.
    if (no_of_sectors)
    {
        printk("Sector Block   0  1  2  3   4  5  6  7   8  9 10 11  12 13 14 15  AccessBits\r\n");
        for (i = no_of_sectors - 1; i >= 0; i--)
        {
            PICC_DumpMifareClassicSector(bus, uid, key, i);
        }
    }
    
    PICC_HaltA(bus); // Halt the PICC before stopping the encrypted session.
    PCD_StopCrypto1(bus);
}

/*
 * Dumps memory contents of a sector of a MIFARE Classic PICC.
 * Uses PCD_Authenticate(), MIFARE_Read() and PCD_StopCrypto1.
 * Always uses PICC_CMD_MF_AUTH_KEY_A because only Key A can always read the sector trailer access bits.
 */
static void PICC_DumpMifareClassicSector(const void *bus,
                                         UID_t *uid,
                                         MIFARE_Key_t *key,
                                         unsigned char sector)
{
    unsigned char status;
    unsigned char firstBlock;		// Address of lowest address to dump actually last block dumped)
    unsigned char no_of_blocks;		// Number of blocks in sector
    bool isSectorTrailer;	        // Set to true while handling the "last" (ie highest address) in the sector.

    // The access bits are stored in a peculiar fashion.
    // There are four groups:
    //		g[3]	Access bits for the sector trailer, block 3 (for sectors 0-31) or block 15 (for sectors 32-39)
    //		g[2]	Access bits for block 2 (for sectors 0-31) or blocks 10-14 (for sectors 32-39)
    //		g[1]	Access bits for block 1 (for sectors 0-31) or blocks 5-9 (for sectors 32-39)
    //		g[0]	Access bits for block 0 (for sectors 0-31) or blocks 0-4 (for sectors 32-39)
    // Each group has access bits [C1 C2 C3]. In this code C1 is MSB and C3 is LSB.
    // The four CX bits are stored together in a nible cx and an inverted nible cx_.
    
    unsigned char c1, c2, c3;		// Nibbles
    unsigned char c1_, c2_, c3_;	// Inverted nibbles
    bool invertedError;		        // True if one of the inverted nibbles did not match
    unsigned char g[4];				// Access bits for each of the four groups.
    unsigned char group;			// 0-3 - active group for access bits
    bool firstInGroup;		        // True for the first block dumped in the group

    // Determine position and size of sector.
    if (sector < 32)   // Sectors 0..31 has 4 blocks each
    {
        no_of_blocks = 4;
        firstBlock = sector * no_of_blocks;
    }
    else if (sector < 40)   // Sectors 32-39 has 16 blocks each
    {
        no_of_blocks = 16;
        firstBlock = 128 + (sector - 32) * no_of_blocks;
    }
    else   // Illegal input, no MIFARE Classic PICC has more than 40 sectors.
    {
        return;
    }

    // Dump blocks, highest address first.
    int blockOffset, index;
    unsigned char byteCount;
    unsigned char buffer[18];
    unsigned char blockAddr;
    isSectorTrailer = true;
    
    for (blockOffset = no_of_blocks - 1; blockOffset >= 0; blockOffset--)
    {
        blockAddr = firstBlock + blockOffset;
        // Sector number - only on first line
        if (isSectorTrailer)
        {
            if (sector < 10)
                printk("   %i   ", sector);     // Pad with spaces
            else
                printk("  %i   ", sector);      // Pad with spaces
        }
        else
        {
            printk("       ");
        }
        
        // Block number
        if (blockAddr < 10)
            printk("   %i  ", blockAddr);       // Pad with spaces
        else if (blockAddr < 100)
            printk("  %i  ", blockAddr);        // Pad with spaces
        else
            printk(" %i  ", blockAddr);         // Pad with spaces

        // Establish encrypted communications before reading the first block
        if (isSectorTrailer)
        {
            status = PCD_Authenticate(bus, PICC_CMD_MF_AUTH_KEY_A, firstBlock, key, uid);
            if (status != STATUS_OK)
            {
                printk("PCD_Authenticate() failed: %s\r\n", GetStatusCodeName(status));
                return;
            }
        }
        
        // Read block
        byteCount = sizeof(buffer);
        status = MIFARE_Read(bus, blockAddr, buffer, &byteCount);
        if (status != STATUS_OK)
        {
            printk("MIFARE_Read() failed: %s\r\n", GetStatusCodeName(status));
            continue;
        }
        
        // Dump data
        for (index = 0; index < 16; index++)
        {
            printk(" %02X", buffer[index]);
            
            if ((index % 4) == 3)
            {
                printk(" ");
            }
        }
        
        // Parse sector trailer data
        if (isSectorTrailer)
        {
            c1  = buffer[7] >> 4;
            c2  = buffer[8] & 0xF;
            c3  = buffer[8] >> 4;
            c1_ = buffer[6] & 0xF;
            c2_ = buffer[6] >> 4;
            c3_ = buffer[7] & 0xF;
            invertedError = (c1 != (~c1_ & 0xF)) || (c2 != (~c2_ & 0xF)) || (c3 != (~c3_ & 0xF));
            g[0] = ((c1 & 1) << 2) | ((c2 & 1) << 1) | ((c3 & 1) << 0);
            g[1] = ((c1 & 2) << 1) | ((c2 & 2) << 0) | ((c3 & 2) >> 1);
            g[2] = ((c1 & 4) << 0) | ((c2 & 4) >> 1) | ((c3 & 4) >> 2);
            g[3] = ((c1 & 8) >> 1) | ((c2 & 8) >> 2) | ((c3 & 8) >> 3);
            isSectorTrailer = false;
        }

        // Which access group is this block in?
        if (no_of_blocks == 4)
        {
            group = blockOffset;
            firstInGroup = true;
        }
        else
        {
            group = blockOffset / 5;
            firstInGroup = (group == 3) || (group != (blockOffset + 1) / 5);
        }

        if (firstInGroup)
        {
            // Print access bits
            printk(" [ %i %i %i ]", (g[group] >> 2) & 1, (g[group] >> 1) & 1, (g[group] >> 0) & 1);

            if (invertedError)
            {
                printk(" Inverted access bits did not match! ");
            }
        }

        if (group != 3 && (g[group] == 1 || g[group] == 6))   // Not a sector trailer, a value block
        {
            int value = ((int)buffer[3]<<24) | ((int)buffer[2]<<16) | ((int)buffer[1]<<8) | (int)buffer[0];
            printk(" Value=0x%08X  Adr=0x%02X", value, buffer[12]);
        }
        
        printk("\r\n");
    }

    return;
}

/*
 * Dumps memory contents of a MIFARE Ultralight PICC.
 */
static void PICC_DumpMifareUltralight(const void *bus)
{
    unsigned char status;
    unsigned char byteCount;
    unsigned char buffer[18];
    unsigned char i, page, offset;

    printk("Page  0  1  2  3\r\n");
    
    // Try the mpages of the original Ultralight. Ultralight C has more pages.
    for (page = 0; page < 16; page +=4)   // Read returns data for 4 pages at a time.
    {
        // Read pages
        byteCount = sizeof(buffer);
        status = MIFARE_Read(bus, page, buffer, &byteCount);
        if (status != STATUS_OK)
        {
            printk("MIFARE_Read() failed: %s\r\n", GetStatusCodeName(status));
            break;
        }
        
        // Dump data
        for (offset = 0; offset < 4; offset++)
        {
            int index;
            
            i = page + offset;
            if (i < 10)
                printk("  %i  ", i);    // Pad with spaces
            else
                printk(" %i  ", i);     // Pad with spaces
 
            for (index = 0; index < 4; index++)
            {
                i = 4 * offset + index;
                printk(" %02X", buffer[i]);
            }
            
            printk("\r\n");
        }
    }
}

/*
 * Calculates the bit pattern needed for the specified access bits. In the [C1 C2 C3] tupples C1 is MSB (=4) and C3 is LSB (=1).
 */
static void MIFARE_SetAccessBits(unsigned char *accessBitBuffer,
                                 unsigned char g0,
                                 unsigned char g1,
                                 unsigned char g2,
                                 unsigned char g3)
{
    unsigned char c1 = ((g3 & 4) << 1) | ((g2 & 4) << 0) | ((g1 & 4) >> 1) | ((g0 & 4) >> 2);
    unsigned char c2 = ((g3 & 2) << 2) | ((g2 & 2) << 1) | ((g1 & 2) << 0) | ((g0 & 2) >> 1);
    unsigned char c3 = ((g3 & 1) << 3) | ((g2 & 1) << 2) | ((g1 & 1) << 1) | ((g0 & 1) << 0);

    accessBitBuffer[0] = (~c2 & 0xF) << 4 | (~c1 & 0xF);
    accessBitBuffer[1] =          c1 << 4 | (~c3 & 0xF);
    accessBitBuffer[2] =          c3 << 4 | c2;
}

/*
 * Performs the "magic sequence" needed to get Chinese UID changeable
 * Mifare cards to allow writing to sector 0, where the card UID is stored.
 *
 * Note that you do not need to have selected the card through REQA or WUPA,
 * this sequence works immediately when the card is in the reader vicinity.
 * This means you can use this method even on "bricked" cards that your reader does
 * not recognise anymore (see MFRC522::MIFARE_UnbrickUidSector).
 *
 * Of course with non-bricked devices, you're free to select them before calling this function.
 */
/*static*/ bool MIFARE_OpenUidBackdoor(const void *bus, bool logErrors)
{
    // Magic sequence:
    // > 50 00 57 CD (HALT + CRC)
    // > 40 (7 bits only)
    // < A (4 bits only)
    // > 43
    // < A (4 bits only)
    // Then you can write to sector 0 without authenticating

    PICC_HaltA(bus); // 50 00 57 CD

    unsigned char cmd = 0x40;
    unsigned char validBits = 7;    /* Our command is only 7 bits. After receiving card response,
						               this will contain amount of valid response bits. */
    unsigned char response[32];     // Card's response is written here
    unsigned char received;
    unsigned char status = PCD_TransceiveData(bus,
                                              &cmd,
                                              1,
                                              response,
                                              &received,
                                              &validBits,
                                              0,
                                              false); // 40
    if (status != STATUS_OK)
    {
        if (logErrors)
        {
            printk("Card did not respond to 0x40 after HALT command. Are you sure it is a UID changeable one?\r\n");
            printk("Error name: %s\r\n", GetStatusCodeName(status));
        }
        return false;
    }
    
    if (received != 1 || response[0] != 0x0A)
    {
        if (logErrors)
        {
            printk("Got bad response on backdoor 0x40 command: %02X (%i valid bits)\r\n", response[0], validBits);
        }
        return false;
    }

    cmd = 0x43;
    validBits = 8;
    status = PCD_TransceiveData(bus,
                                &cmd,
                                1,
                                response,
                                &received,
                                &validBits,
                                0,
                                false); // 43
    if (status != STATUS_OK)
    {
        if (logErrors)
        {
            printk("Error in communication at command 0x43, after successfully executing 0x40\r\n");
            printk("Error name: %s\r\n", GetStatusCodeName(status));
        }
        return false;
    }
    
    if (received != 1 || response[0] != 0x0A)
    {
        if (logErrors)
        {
            printk("Got bad response on backdoor 0x43 command: %02X (%i valid bits)\r\n", response[0], validBits);
        }
        return false;
    }

    // You can now write to sector 0 without authenticating!
    return true;
}

/*
 * Reads entire block 0, including all manufacturer data, and overwrites
 * that block with the new UID, a freshly calculated BCC, and the original
 * manufacturer data.
 *
 * It assumes a default KEY A of 0xFFFFFFFFFFFF.
 * Make sure to have selected the card before this function is called.
 */
/*static*/ bool MIFARE_SetUid(const void *bus, unsigned char *newUid, unsigned char uidSize, bool logErrors)
{
    // UID + BCC byte can not be larger than 16 together
    if (!newUid || !uidSize || uidSize > 15)
    {
        if (logErrors)
        {
            printk("New UID buffer empty, size 0, or size > 15 given\r\n");
        }
        return false;
    }

    // Authenticate for reading
    MIFARE_Key_t key = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
    unsigned char status = PCD_Authenticate(bus, PICC_CMD_MF_AUTH_KEY_A, 1, &key, &uid);
    if (status != STATUS_OK)
    {
        if (status == STATUS_TIMEOUT)
        {
            // We get a read timeout if no card is selected yet, so let's select one

            // Wake the card up again if sleeping
        #if 0
			unsigned char atqa_answer[2];
			unsigned char atqa_size = 2;
			PICC_WakeupA(bus, atqa_answer, &atqa_size);
        #endif

            if (!PICC_IsNewCardPresent(bus) || !PICC_ReadCardSerial(bus))
            {
                printk("No card was previously selected, and none are available. Failed to set UID.\r\n");
                return false;
            }

            status = PCD_Authenticate(bus, PICC_CMD_MF_AUTH_KEY_A, 1, &key, &uid);
            if (status != STATUS_OK)
            {
                // We tried, time to give up
                if (logErrors)
                {
                    printk("Failed to authenticate to card for reading, could not set UID: \r\n"
                           "%s \r\n", GetStatusCodeName(status));
                }
                return false;
            }
        }
        else
        {
            if (logErrors)
            {
                printk("PCD_Authenticate() failed: %s\r\n", GetStatusCodeName(status));
            }
            return false;
        }
    }

    // Read block 0
    unsigned char block0_buffer[18];
    unsigned char byteCount = sizeof(block0_buffer);
    status = MIFARE_Read(bus, 0, block0_buffer, &byteCount);
    if (status != STATUS_OK)
    {
        if (logErrors)
        {
            printk("MIFARE_Read() failed: %s\r\n", GetStatusCodeName(status));
            printk("Are you sure your KEY A for sector 0 is 0xFFFFFFFFFFFF?\r\n");
        }
        return false;
    }

    // Write new UID to the data we just read, and calculate BCC byte
    int i;
    unsigned char bcc = 0;
    for (i = 0; i < uidSize; i++)
    {
        block0_buffer[i] = newUid[i];
        bcc ^= newUid[i];
    }

    // Write BCC byte to buffer
    block0_buffer[uidSize] = bcc;

    // Stop encrypted traffic so we can send raw bytes
    PCD_StopCrypto1(bus);

    // Activate UID backdoor
    if (!MIFARE_OpenUidBackdoor(bus, logErrors))
    {
        if (logErrors)
        {
            printk("Activating the UID backdoor failed.\r\n");
        }
        return false;
    }

    // Write modified block 0 back to card
    status = MIFARE_Write(bus, 0, block0_buffer, 16);
    if (status != STATUS_OK)
    {
        if (logErrors)
        {
            printk("MIFARE_Write() failed: %s\r\n", GetStatusCodeName(status));
        }
        return false;
    }

    // Wake the card up again
    unsigned char atqa_answer[2];
    unsigned char atqa_size = 2;
    PICC_WakeupA(bus, atqa_answer, &atqa_size);

    return true;
}

/*
 * Resets entire sector 0 to zeroes, so the card can be read again by readers.
 */
/*static*/ bool MIFARE_UnbrickUidSector(const void *bus, bool logErrors)
{
    MIFARE_OpenUidBackdoor(bus, logErrors);

    unsigned char block0_buffer[] = {0x01, 0x02, 0x03, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // Write modified block 0 back to card
    unsigned char status = MIFARE_Write(bus, 0, block0_buffer, 16);
    if (status != STATUS_OK)
    {
        if (logErrors)
        {
            printk("MIFARE_Write() failed: %s\r\n", GetStatusCodeName(status));
        }
        return false;
    }
    return true;
}

//-------------------------------------------------------------------------------------------------
// Convenience functions - does not add extra functionality
//-------------------------------------------------------------------------------------------------

/*
 * Returns true if a PICC responds to PICC_CMD_REQA.
 * Only "new" cards in state IDLE are invited. Sleeping cards in state HALT are ignored.
 *
 * @return bool
 */
bool PICC_IsNewCardPresent(const void *bus)
{
    unsigned char bufferATQA[2];
    unsigned char bufferSize = sizeof(bufferATQA);
    unsigned char result = PICC_RequestA(bus, bufferATQA, &bufferSize);
    return (result == STATUS_OK || result == STATUS_COLLISION);
}

/*
 * Simple wrapper around PICC_Select.
 * Returns true if a UID could be read.
 * Remember to call PICC_IsNewCardPresent(), PICC_RequestA() or PICC_WakeupA() first.
 * The read UID is available in the class variable uid.
 *
 * @return bool
 */
bool PICC_ReadCardSerial(const void *bus)
{
    unsigned char result = PICC_Select(bus, &uid, 0);
    return (result == STATUS_OK);
}

#endif // #if RC522_DRV


