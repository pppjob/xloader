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

#include <linux/types.h>
#include <config.h>
#include <factory.h>
#include <asm/arch-atxx/bootparam.h>


struct boot_info boot_info;

// wait header program by GP
int get_boot_param(void)
{
#if 0
	const char *s;
	
	/* uboot laddr 0x */
	if ((s = getenv ("uboot-laddr")) != NULL) {
			clkv = simple_strtoul (s, NULL, 16);
		}
	/* uboot raddr 0x */
	if ((s = getenv ("uboot-raddr")) != NULL) {
			clkv = simple_strtoul (s, NULL, 16);
		}
	/* uboot nand offset 0x */
	if ((s = getenv ("uboot-nandoff")) != NULL) {
			nandoff = simple_strtoul (s, NULL, 16);
		}
#endif
	boot_info.length = (300 *1024); /*300k uboot size*/
	boot_info.laddr = CFG_LOADADDR;
	boot_info.raddr = CFG_LOADADDR;
	boot_info.nandoff = 0x5000;
	boot_info.nand_end = 0x200000;
	return 0;
}

