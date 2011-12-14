/*------------------------------------------------------------------------------
* (c) Copyright, Augusta Technology, Inc., 2006-present.
* (c) Copyright, Augusta Technology USA, Inc., 2006-present.
*
* This software, document, web pages, or material (the "Work") is copyrighted
* by its respective copyright owners.  The Work may be confidential and
* proprietary.  The Work may be further protected by one or more patents and
* be protected as a part of a trade secret package.
*
* No part of the Work may be copied, photocopied, reproduced, translated, or
* reduced to any electronic medium or machine-readable form, in whole or in
* part, without prior written consent of the copyright owner. Any other
* reproduction in any form without the permission of the copyright owner is
* prohibited.
*
* All Work are protected by the copyright laws of all relevant jurisdictions,
* including protection under the United States copyright laws, and may not be
* reproduced, distributed, transmitted, displayed, published, or broadcast
* without the prior written permission of the copyright owner.
*
------------------------------------------------------------------------------*/

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_ATXX
#define CONFIG_AT7700B0
#define CONFIG_BOARD_ATB1005

/* memory layout */
#define CONFIG_UNCONTINUOUS_MEM
#define CONFIG_SYS_MALLOC_START		0x40220000

#define	CONFIG_SYS_UBOOT_BASE		0x40400400
#define	CONFIG_SYS_PHY_UBOOT_BASE	0x40400400

/* Envioment */
/* Fixme :reduce size , not use crc check */
#define CONFIG_ENV_SIZE			8192	/*8192 byte only*/
#define CONFIG_ENV_OFFSET		0x400000
#define CONFIG_ENV_RANGE		0x200000

/* Factory data */
#define FACTORY_DATA_BASE_OFFSET	0x1000000
#define FACTORY_DATA_MAX		0x1400000

#define	MAGIC_STRING			"ATXXFACTORYDATA"
#define	MAGIC_LENGTH			16

/* printf buffer size */
#define CFG_PBSIZE			256

#define MDDR_BASE_ADDR			0x88000000
//#define CFG_DDR2
/* data send from x-loader to u-boot address*/
#define CONFIG_CAL_SIZE			(8)

/* magic string for boot parameter */
#define	BOOT_MAGIC_STRING		"ATXXBOOTPARA"
#define	BOOT_MAGIC_LENGTH		16

/*uboot xmodem load address */
#define CFG_LOADADDR			0x88800000

/* uboot load-addr and nand-offset */
#define	CFG_UBOOT_LADDR			0x88007e00
#define	CFG_UBOOT_OFFSET		0x00200000	/* 2M */

#define CFG_FIRMWARE_SIZE		(512 *1024)
#define CFG_FIRMWARE_END		(0x400000)

/* Uart setting */
#define CONFIG_XMODEM
#define CFG_PRINTF
#define CFG_UART_CLOCK_FREQ 		(3686400*16)
#define CONFIG_BAUDRATE			921600
#define CFG_UART_LOOPENABLE 		0

/* Skip malloc */
#define __HAVE_ARCH_STRDUP

/* Nand config */
#define CFG_NAND
#define CFG_UDELAY

/* I2C config */
#define CONFIG_PM_AT2600

#endif /* __CONFIG_H */
