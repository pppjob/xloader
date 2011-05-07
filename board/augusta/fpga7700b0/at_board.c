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
#include <environment.h>
#include <factory.h>
#include <nand.h>

#include <linux/types.h>
#include <linux/string.h>
#include <asm/io.h>

#include <asm/arch-atxx/bootparam.h>
#include <asm/arch-atxx/mddr.h>
#include <asm/arch-atxx/cache.h>
#include <asm/arch-atxx/clock.h>
#include <asm/arch-atxx/topctl.h>
#include <asm/arch-atxx/regs_base.h>
#include <asm/arch-atxx/map_table.h>
#include <asm/arch-atxx/pm.h>

struct boot_parameter b_param;

int board_init(void)
{
	mmu_cache_on(memory_map);

	return 0;
}

uint32_t main_course(char *boot_dev_name)
{
	int ret;
	boot_info_t *info = &boot_info;
        unsigned int hwcfg, swcfg;
/* */
	printf("mddr memset!\n");
	memset((void *)0x88800000, 0x5a, 0x100000);
	printf("mddr memset done!\n");

	hwcfg = pm_read_reg(HWCFGR);
	if (hwcfg == 1) {
		printf("Enter HWCFG CCC mode\n");
		/* uart ccc mode */
		goto done;
	}

	/* nand read to mddr */
	ret = nand_read_skip_bad(&nd, info->nand_offset, 
		info->firm_size, 
		(u_char *)info->load_address,
		info->nand_end);
	if (ret) {
		printf("Read uboot fail\n");
		goto done;
	}
	ret = parse_header(info->load_address, 0);
	if (ret) {
		printf("Parse header fail\n");
		goto done;		
	}
	strcpy(boot_dev_name, "Nand");
	return info->run_address;

done:
	/* load uboot from uart */
	strcpy(boot_dev_name, "UART");
	/* add head analysis , if need */
	do_load_serial_bin(CFG_LOADADDR, CONFIG_BAUDRATE);
	ret = parse_header(CFG_LOADADDR, 1);
	if (ret) {
		printf("Parse header fail\n");
		goto done;		
	}
	if (info->load_address != CFG_LOADADDR) {
		memcpy((void *)info->run_address, 
			(char *)CFG_LOADADDR + sizeof(atxx_image_header_t),
			info->firm_size);
	}

	swcfg = pm_read_reg(SWCFGR);
	swcfg |= SWCFGR_POWERON_XLOAD;
	pm_write_reg(SWCFGR, swcfg);

	return (u32)info->run_address;
}

/* optionally do something like blinking LED */
void board_hang (void)
{
	return;
}

