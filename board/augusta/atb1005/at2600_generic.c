/*------------------------------------------------------------------------------
* (c) Copyright, Augusta Technology, Inc., 2006-present.
* (c) Copyright, Augusta Technology USA, Inc., 2006-present.
*
* This software, document, web pages, or material (the "Work") is copyrighted
* by its respective copyright owners.  The Work may be confidential and
* proprietary.	The Work may be further protected by one or more patents and
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
#include <asm/io.h>
#include <common.h>
#include <linux/types.h>
#include <asm/arch-atxx/regs_base.h>
#include <asm/arch-atxx/pmu.h>
#include "../../../drivers/atxx_i2c.h"

#define at2600_ID		0x1
#define at2600_ADDR		0x70

static void at2600_write_reg(u8 reg_addr, u8 reg_value)
{
	i2c_reg_write(at2600_ADDR, reg_addr, reg_value);
}

static u8 at2600_read_reg(u8 reg_addr, u8 *reg_value)
{
	*reg_value = i2c_reg_read(at2600_ADDR, reg_addr);
	return *reg_value;
}
/**********************************************************************/
/**
 * at2600_pm_ap_core_power_on_off - Power Supply for AP core
 *
 * @mode:		PPS_ON/PPS_OFF
 *
 ********************************************************************/
void at2600_pm_ap_core_power_control(enum power_supply_mode mode, int voltage)
{
	uint8_t		ldoregc1;

	ldoregc1 = 0;

	if (PS_OFF == mode) 
	{
		ldoregc1 |= LDO_POWER_DOWN << LDO_POWER_SHIFT;	

	}
	else
	{
		ldoregc1 |= LDO_POWER_ON << LDO_POWER_SHIFT; 
	}

	ldoregc1 |= voltage;

	at2600_write_reg(AT2600_PM_REG_S1V2C1,ldoregc1);
}

/**********************************************************************/
/**
 * at2600_pm_mddr_power_on_off - Power Supply for AP core
 *
 * @mode:		PPS_ON/PPS_OFF
 *
 ********************************************************************/
void at2600_pm_mddr_power_control(enum power_supply_mode mode, int voltage)
{
	uint8_t		ldoregc1;

	ldoregc1 = 0;

	if (PS_OFF == mode) 
	{
		ldoregc1 |= LDO_POWER_DOWN << LDO_POWER_SHIFT;	

	}
	else
	{
		ldoregc1 |= LDO_POWER_ON << LDO_POWER_SHIFT; 
	}

	ldoregc1 |= voltage;

	at2600_write_reg(AT2600_PM_REG_S1V8C1,ldoregc1);
}

/*--------------------power operation-----------------------------------*/

u8 at2600_set_default_power_supply(int arm_core_vol, int mddr_vol)
{
	u8 regvalue;

	switch (arm_core_vol) {
		case S1V2C1_DOUT_1V45:
			printf("\nset arm to 1.45V.");
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V25);
			mdelay(1);
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V3);
			mdelay(1);
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V35);
			mdelay(1);
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V4);
			mdelay(1);
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V45);
			mdelay(1);
			break;
		case S1V2C1_DOUT_1V4:
			printf("\nset arm to 1.4V.");
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V25);
			mdelay(1);
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V3);
			mdelay(1);
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V35);
			mdelay(1);
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V4);
			mdelay(1);
			break;
		case S1V2C1_DOUT_1V35:
			printf("\nset arm to 1.35V.");
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V25);
			mdelay(1);
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V3);
			mdelay(1);
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V35);
			mdelay(1);
			break;
		case S1V2C1_DOUT_1V3:
			printf("\nset arm to 1.35V.");
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V25);
			mdelay(1);
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V3);
			mdelay(1);
			break;
		case S1V2C1_DOUT_1V25:
			printf("\nset arm to 1.25V.");
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V25);
			mdelay(1);
			break;
		case S1V2C1_DOUT_1V2:
			printf("\nset arm to 1.2V.");
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V2);
			mdelay(1);
			break;
		case S1V2C1_DOUT_1V1:
			printf("\nset arm to 1.1V.");
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V1);
			mdelay(1);
			break;
		case S1V2C1_DOUT_1V0:
			printf("\nset arm to 1.0V.");
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V1);
			mdelay(1);
			at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V0);
			mdelay(1);
			break;
		default:
			printf("arm voltage Not support, \n");
			break;
	}

	printf("\nset mddr to 1.8V.");
	at2600_pm_mddr_power_control(PS_ON, mddr_vol);
	mdelay(1);
}

u8 i2c_at2600_init(void)
{
	u8 buf;
	uint32_t gpio_reg;

	/* disable at2600 26M and I2C,solve the conflict of ctp and at2600 */
	gpio_reg = readl(ATXX_GPIOB_BASE+0x04);
	gpio_reg |= 0x1 << 6;
	writel(gpio_reg, ATXX_GPIOB_BASE+0x04);
	gpio_reg = readl(ATXX_GPIOB_BASE+0x0);
	gpio_reg |= 0x1 << 6;
	writel(gpio_reg,  ATXX_GPIOB_BASE+0x0);
	mdelay(10);
	/* enable at2600 26M and I2C */
	gpio_reg = readl(ATXX_GPIOB_BASE+0x04);
	gpio_reg |= 0x1 << 6;
	writel(gpio_reg, ATXX_GPIOB_BASE+0x04);
	gpio_reg = readl(ATXX_GPIOB_BASE+0x0);
	gpio_reg &= ~(0x1 << 6);
	writel(gpio_reg,  ATXX_GPIOB_BASE+0x0);
	mdelay(10);

	/*Init i2c*/
	i2c_init(1,at2600_ADDR);

	/* read a readonly register */
	at2600_read_reg(AT2600_PM_REG_ID, &buf);

	printf("\nPMU id value = 0x%x", buf);
}


