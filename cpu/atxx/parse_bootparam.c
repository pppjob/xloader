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
#include <common.h>
#include <linux/types.h>
#include <config.h>
#include <factory.h>
#include <asm/arch-atxx/bootparam.h>
#include <environment.h>
#include <linux/vsprintf.h>
#include <asm/errno.h>


boot_info_t boot_info;

int get_boot_param(void)
{
	const char *s;
	boot_info_t *info = &boot_info;

	/* uboot laddr 0x */
	if ((s = getenv ("uboot-laddr")) != NULL) {
		info->load_address = simple_strtoul (s, NULL, 16);
	} else
		info->load_address = CFG_UBOOT_LADDR;

	/* uboot nand offset 0x */
	if ((s = getenv ("uboot-nandoff")) != NULL) {
		info->nand_offset = simple_strtoul (s, NULL, 16);
	} else
		info->nand_offset = CFG_UBOOT_OFFSET;

	info->firm_size = CFG_FIRMWARE_SIZE; /* 512 uboot size*/
	info->nand_end = CFG_FIRMWARE_END;

	return 0;
}

int parse_header(uint32_t addr, int ignore)
{
	atxx_image_header_t * header;
	boot_info_t *info = &boot_info;

	header = (atxx_image_header_t *)addr;
	if (header->boot_signature != HEAD_SIGNATURE) {
		printf("Magic is not correct\n");
		return -EINVAL;
	}
	/* if securiy verify fail, return negtive */
	if (!ignore) {
		if (header->load_address != info->load_address) {
			printf("Env load_addr does not match header\n");
			return -EINVAL;
		}
		if (header->nand_offset != info->nand_offset) {
			printf("Env nand_offset does not match header\n");
			return -EINVAL;
		}
		if (header->firm_size > info->firm_size) {
			printf("Firm_size does not match header\n");
			return -EINVAL;
		}
		if ((header->nand_offset + header->firm_size) 
				> info->nand_end) {
			printf("Nand_end does not match header\n");
			return -EINVAL;
		}
	}
	info->run_address = header->run_address;
	info->load_address = header->load_address;
	info->firm_size = header->firm_size;

	return 0;	
}
