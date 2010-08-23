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
#define CONFIG_AT6600

/* memory layout */
#define CONFIG_UNCONTINUOUS_MEM
#define CONFIG_SYS_MALLOC_START		0x40220000

#define	CONFIG_SYS_UBOOT_BASE		0x40400400
#define	CONFIG_SYS_PHY_UBOOT_BASE	0x40400400

#define CONFIG_ENV_SIZE			8192	/* 8KB */

/* reserved for malloc */
#define CONFIG_SYS_MALLOC_LEN		\
	(CONFIG_ENV_SIZE + 24*1024)
/* reserved for initial data */
#define CONFIG_SYS_GBL_DATA_SIZE	(128)
#define CFG_PBSIZE			256


#define MDDR_BASE_ADDR			0x88000000
#define CFG_LOADADDR			0x88008000

/* Uart setting */
#define CONFIG_XMODEM
#define CFG_PRINTF
#define CFG_UART_CLOCK_FREQ 		(3686400*16)
#define CONFIG_BAUDRATE			921600
#define CFG_UART_LOOPENABLE 		0

/* Skip malloc */
#define __HAVE_ARCH_STRDUP

#endif /* __CONFIG_H */
