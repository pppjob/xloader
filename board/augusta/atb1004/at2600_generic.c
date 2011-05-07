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
#include "../../../drivers/atxx_i2c.h"
#include "at2600_generic.h"

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

u8 at2600_set_default_power_supply(void)
{
	u8 regvalue;
	printf("\nset arm to 1.2V.");
	at2600_pm_ap_core_power_control(PS_ON,S1V2C1_DOUT_1V2);
	printf("\nset mddr to 1.8V.");
	at2600_pm_mddr_power_control(PS_ON,S1V8C1_DOUT_1V8);

}

u8 i2c_at2600_init(void)
{
	u8 buf;

	/*Init i2c*/
	i2c_init(1,at2600_ADDR);

	/* read a readonly register */
	at2600_read_reg(AT2600_PM_REG_ID, &buf);

	printf("\nPMU id value = 0x%x", buf);
}


