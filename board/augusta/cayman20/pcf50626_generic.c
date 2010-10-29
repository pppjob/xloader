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
#include "pcf50626_generic.h"

#define PCF50626_ID		0x31
#define PCF50626_ADDR		0x70

static void pcf50626_write_reg(u8 reg_addr, u8 reg_value)
{
	i2c_reg_write(PCF50626_ADDR, reg_addr, reg_value);
}

static u8 pcf50626_read_reg(u8 reg_addr, u8 *reg_value)
{
	*reg_value = i2c_reg_read(PCF50626_ADDR, reg_addr);
	return *reg_value;
}
/*----------------------------------------------------------------------------*/

/*--------------------power operation-----------------------------------*/

void pcf50626_DCD1_power_supply(u32 voltage, int mode)
{
	u32 reg1, reg3;

	reg1 = (voltage - 600)/25;
	reg3 = mode? 0xe0 : 0x00;
	pcf50626_write_reg( DCD1C1, reg1);
	pcf50626_write_reg( DCD1C3, 0x00);
	pcf50626_write_reg( DCD1C4, 0xf);
	pcf50626_write_reg( DCD1C2, reg3);
}

void pcf50626_DCD2_power_supply(u32 voltage, int mode)
{
	u32 reg1, reg3;

	reg1 = (voltage - 600)/25;
	reg3 = mode? 0xe0 : 0x00;
	pcf50626_write_reg( DCD2C1, reg1);
	pcf50626_write_reg( DCD2C3, 0x00);
	pcf50626_write_reg( DCD2C4, 0xf);
	pcf50626_write_reg( DCD2C2, reg3);
}
/**
  * pcf50626_set_default_power_supply - Change power supply to default
  *
  */
u8 pcf50626_set_default_power_supply(void)
{
	pcf50626_DCD1_power_supply(1300, 1);
	pcf50626_DCD2_power_supply(1800, 1);
}

u8 i2c_pcf50626_init(void)
{
    u8 buf;

    /*Init i2c*/
    i2c_init(1,PCF50626_ADDR);

    /* read a readonly register */
    pcf50626_read_reg(ID, &buf);

    if(buf!=PCF50626_ID){
		printf("\nPMU read ID failed, wrong id value = 0x%x", buf);
		return -1;
    } else{
		return 0;
    }
}


