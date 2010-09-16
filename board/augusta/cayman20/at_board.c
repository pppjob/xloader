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
#include <asm/arch-atxx/cache.h>
#include "map_table.c"
#include "clock_table.c"
#include <asm/arch-atxx/clock.h>
#include <environment.h>
#include <factory.h>
#include <nand.h>
#include <asm/arch-atxx/bootparam.h>
#include <asm/arch-atxx/mddr.h>
#include <linux/string.h>

struct boot_parameter b_param;

int board_init(void)
{
	mmu_cache_on(memory_map);
	at6600_clock_init();
	set_board_default_clock(pll_setting, div_setting,
		PLL_DEFSET_COUNT, DIV_DEFSET_COUNT);
	return 0;
}

uint32_t main_course(char *boot_dev_name)
{
	int ret;
	struct boot_parameter *parameter = &b_param;

	/* read config data area for clock information */
	ret = env_init();
	/* enviroment exist, follow its setting */
	if(!ret){
		regulate_clock();
	}
	
	mddr_init(parameter);

	if (parameter->mddr_data_send) {
		memcpy(parameter->magic, 
			BOOT_MAGIC_STRING, BOOT_MAGIC_LENGTH);
	} else {
		memset(parameter, 0, sizeof(struct boot_parameter));
	}
#if 0
	/* read uboot boot address in config data area */
	if(envs.flags == 0)
		goto done;
	ret = get_boot_param();
	if (ret) {
		printf("Get boot parameter fail\n");
		goto done;
	}
	/* nand read to mddr */
	ret = nand_read_skip_bad(&nd, boot_info.nandoff, 
		&boot_info.length, (u_char *)boot_info.laddr,
		boot_info.nand_end);
	if (ret) {
		printf("Read uboot fail\n");
		goto done;
	}
	
	/* check security header, if fail goto done */
	strcpy(boot_dev_name, "Nand");
	return boot_info.raddr;
done:
#endif
	/* load uboot from uart */
	strcpy(boot_dev_name, "UART");
	/* add head analysis , if need */
	do_load_serial_bin(CFG_LOADADDR, CONFIG_BAUDRATE);
	/* check security header, if fail goto done */
	return (u32)CFG_RUNADDR;
}

/* optionally do something like blinking LED */
void board_hang (void)
{
	return;
}

