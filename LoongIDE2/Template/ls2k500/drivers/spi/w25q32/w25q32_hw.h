/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * w25q32_hw.h
 *
 * created: 2022-03-11
 *  author: Bian
 */

#ifndef _W25Q32_HW_H
#define _W25Q32_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * w25q32 instruction set
 */
#define W25Q32_CMD_WREN  		0x06	/* write enable */
#define W25Q32_CMD_WRDIS		0x04	/* write disable */

#define W25Q32_CMD_RDSR 		0x05	/* read status register bit[7:0] */
#define W25Q32_CMD_RDSR2		0x35	/* read status register bit[15:8] */
#define W25Q32_CMD_WRSR			0x01	/* write status, CMD, bit[7:0], bit[15:8] */

#define W25Q32_CMD_READ			0x03	/* read data:
 	 	 	 	 	 	 	 	 	 	 * CMD, A23-A16, A15-A8, A7-A0, D7-D0, ...
 	 	 	 	 	 	 	 	 	 	 */
#define W25Q32_CMD_FASTRD		0x0B	/* fast read data:
 	 	 	 	 	 	 	 	 	 	 * CMD, A23-A16, A15-A8, A7-A0, DUMMY, D7-D0, ...
 	 	 	 	 	 	 	 	 	 	 */

#define W25Q32_CMD_PP			0x02	/* page program, upto 256 bytes
 	 	 	 	 	 	 	 	 	 	 * CMD, A23-A16, A15-A8, A7-A0, DUMMY, D7-D0, ...
 	 	 	 	 	 	 	 	 	 	 */

#define W25Q32_CMD_SE4K			0x20	/* sector erase, 4KB
 	 	 	 	 	 	 	 	 	 	 * CMD, A23-A16, A15-A8, A7-A0
 	 	 	 	 	 	 	 	 	 	 */
#define W25Q32_CMD_BE32K		0x52	/* block erase, 32KB
 	 	 	 	 	 	 	 	 	 	 * CMD, A23-A16, A15-A8, A7-A0
 	 	 	 	 	 	 	 	 	 	 */
#define W25Q32_CMD_BE64K		0xD8	/* block erase, 64KB
 	 	 	 	 	 	 	 	 	 	 * CMD, A23-A16, A15-A8, A7-A0
 	 	 	 	 	 	 	 	 	 	 */

#define W25Q32_CMD_CE			0xC7	/* chip erase */
#define W25Q32_CMD_CE_1			0x60	/* chip erase */

#define W25Q32_CMD_PD			0xB9	/* power down */
#define W25Q32_CMD_REPD_ID		0xAB	/* release from power down & get device id
 	 	 	 	 	 	 	 	 	 	 * CMD, DUMMY, DUMMY, DUMMY, ID7-ID0
 	 	 	 	 	 	 	 	 	 	 */
 	 	 	 	 	 	 	 	 	 	 
#define W25Q32_CMD_RDMDID		0x90	/* read manufacturer / device ID
 	 	 	 	 	 	 	 	 	 	 * CMD, DUMMY, DUMMY, 0x00, M7-M0, ID7-ID0
 	 	 	 	 	 	 	 	 	 	 */
#define W25Q32_CMD_RDJEDECID	0x9F	/* read ID: manufactor ID, memory type, capactity
 	 	 	 	 	 	 	 	 	 	 * CMD, M7-M0, ID15-ID8, ID7-ID0
 	 	 	 	 	 	 	 	 	 	 */
#define W25Q32_CMD_RDUNIQEID	0x4B	/* read unique ID
 	 	 	 	 	 	 	 	 	 	 * CMD, DUMMY, DUMMY, DUMMY, DUMMY, ID63-ID0
 	 	 	 	 	 	 	 	 	 	 */

/*
 * w25q32 status register bits
 */
#define	W25Q32_SR_SRP			(1<<7)		/* status register protect */
#define W25Q32_SR_TB 			(1<<5)		/* top/bottom write protect */
#define W25Q32_SR_BP_2 		    (1<<4)
#define W25Q32_SR_BP_1 		    (1<<3)
#define W25Q32_SR_BP_0 		    (1<<2)
#define W25Q32_SR_BP_MASK 	    0x1C		/* block protect bits */
#define W25Q32_SR_BP_SHIFT	    2
#define W25Q32_SR_WEL			(1<<1)		/* write enable latch */
#define W25Q32_SR_BUSY		    (1<<0)		/* erase or write in progress */

/*
 * w25q32 write protect table
 */
typedef struct
{
	unsigned char sr_tb;
	unsigned char sr_bp;
	unsigned int  addr_begin;
	unsigned int  addr_end;
} W25Q32_bp_t;

/*
 * hardware information
 */
#define W25Q32_BAUDRATE			50000000		/* 10M baudrate */
#define W25Q32_PAGE_SIZE		256				/* page size 256 bytes */
#define W25Q32_PAGE_COUNT       0x4000          /* 16K */
#define W25Q32_CHIP_SIZE		0x400000		/* chip size 4M */

#define W25Q32_SECTOR_SIZE_4	0x1000			/* sector size 4K */
#define W25Q32_BLOCK_SIZE_32	0x8000			/* block size 32K */
#define W25Q32_BLOCK_SIZE_64	0x10000			/* block size 64K */

#define W25Q32_BITSPERCHAR		8

/*
 * chip parameter, driver use
 */
typedef struct
{
    unsigned int baudrate;				/* tfr rate, bits per second     */
    bool     	 erase_before_program;
    unsigned int empty_state;			/* value of erased cells         */
    unsigned int page_size;				/* programming page size in byte */
    unsigned int sector_size;			/* erase sector size in byte     */
    unsigned int mem_size;				/* total capacity in byte        */
} W25Q32_param_t;


#ifdef __cplusplus
}
#endif

#endif // _W25Q32_HW_H

