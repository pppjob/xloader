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
#include <asm/arch-atxx/mddr.h>
#include "map_table.c"
#include "clock_table.c"

int board_init(void)
{

	mmu_cache_on(memory_map);
	at6600_clock_init();
	set_board_default_clock(pll_setting, div_setting,
		PLL_DEFSET_COUNT, DIV_DEFSET_COUNT);
	/* 
	* FIXME: add mddr power down code, not allow 
	* reinit without power down, when self-refresh
	*/
	mddr_init();
	return 0;
}

uint32_t main_course(char *boot_dev_name)
{

	// key press
	//load uboot from uart
	strcpy(boot_dev_name, "UART");
	//add head analysis , if need
	do_load_serial_bin(CFG_LOADADDR, CONFIG_BAUDRATE);
	//key not press
	//read uboot from nand
	return (u32)CFG_LOADADDR;
}

/* optionally do something like blinking LED */
void board_hang (void)
{
	return;
}

