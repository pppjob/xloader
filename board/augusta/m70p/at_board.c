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
#include <asm/arch-atxx/pmu.h>

#include "clock_table.c"

struct boot_parameter b_param;

int board_init(void)
{
    unsigned int proccfg;

    /* set gpio 38 as low to enable pmu work */
    atxx_request_gpio(38);
    atxx_set_gpio_direction(38, 0);
    atxx_gpio_set(38,0);
    mdelay(10);

	/* improve arm power before set default clock */
	i2c_at2600_init();
	at2600_set_default_power_supply(S1V2C1_DOUT_1V35, S1V8C1_DOUT_1V8);

	mmu_cache_on(memory_map);
	atxx_clock_init();
	set_board_default_clock(pll_setting, div_setting,
		PLL_DEFSET_COUNT, DIV_DEFSET_COUNT);

    /* Disable axi and mddr clock sync mode */
    proccfg = pm_read_reg(PROCCFGR);
    proccfg |= (1 << 10);
    pm_write_reg(PROCCFGR, proccfg);



	return 0;
}

uint32_t main_course(char *boot_dev_name)
{
	int ret;
	struct clk *clk;
	unsigned long  old_clkarm, old_clkaxi;
	struct boot_parameter *parameter = &b_param;
	boot_info_t *info = &boot_info;
        unsigned int hwcfg, swcfg;

	/* read config data area for clock information */
	ret = env_init();
	/* enviroment exist, follow its setting */
	if(!ret) {
		regulate_clock();
	} else {
		regulate_clock_fd();
	}
	
	/* set arm core voltage to 1.xv when mddr initilize */
	clk = clk_get("arm");
	old_clkarm = clk_get_rate(clk);
	clk = clk_get("axi");
	old_clkaxi = clk_get_rate(clk);

	memory_init(parameter);
	at2600_set_default_power_supply(S1V2C1_DOUT_1V35, S1V8C1_DOUT_1V8);
	clk_set_arm (old_clkarm);
	clk_set_axi (old_clkaxi);

	hwcfg = pm_read_reg(HWCFGR);
	if (hwcfg == 1) {
		printf("Enter HWCFG CCC mode\n");
		/* uart ccc mode */
		goto done;
	}

	if (parameter->mddr_data_send) {
		memcpy(parameter->magic, 
			BOOT_MAGIC_STRING, BOOT_MAGIC_LENGTH);
	} else {
		memset(parameter, 0, sizeof(struct boot_parameter));
	}
	/* read uboot boot address in config data area */
	ret = get_boot_param();
	if (ret) {
		printf("Get boot parameter fail\n");
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

