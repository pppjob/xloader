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
#ifndef ___AT2600_PM__REGS_H_______
#define ___AT2600_PM_REGS_H_______

#define LDO_POWER_SHIFT                 0x7

enum at2600_pm_regs {
	AT2600_PM_REG_ID,	/* Variant & Version */
	AT2600_PM_REG_INT1,	/* Interrupt Status */
	AT2600_PM_REG_INT2,
	AT2600_PM_REG_INT3,
	AT2600_PM_REG_INT4,
	AT2600_PM_REG_INT5,
	AT2600_PM_REG_INT6,
	AT2600_PM_REG_INT7,
	AT2600_PM_REG_INT8,
	AT2600_PM_REG_M_INT1,	/* Interrupt Mask */
	AT2600_PM_REG_M_INT2,
	AT2600_PM_REG_M_INT3,
	AT2600_PM_REG_M_INT4,
	AT2600_PM_REG_M_INT5,
	AT2600_PM_REG_M_INT6,
	AT2600_PM_REG_M_INT7,
	AT2600_PM_REG_M_INT8,
	AT2600_PM_REG_ERROR,	/* Error Status */
	AT2600_PM_REG_OOCC1,	/* OOC (On-Off Controller) Control */
	AT2600_PM_REG_OOCC2,
	AT2600_PM_REG_OOCPH,	/* OOC Active Phase Control */
	AT2600_PM_REG_OOCS,	/* OOC Control */
	AT2600_PM_REG_EXCEPC1, /*temperature sensor conreol*/
	AT2600_PM_REG_RTC1 = 0x1a,	/* RTC Time */
	AT2600_PM_REG_RTC2,
	AT2600_PM_REG_RTC3,
	AT2600_PM_REG_RTC4,
	AT2600_PM_REG_RTC1A,	/* RTC Alarm */
	AT2600_PM_REG_RTC2A,
	AT2600_PM_REG_RTC3A,
	AT2600_PM_REG_RTC4A,
	AT2600_PM_REG_CBCC1,	/* CBC (Complete Battery Charger) Control */
	AT2600_PM_REG_CBCC2,
	AT2600_PM_REG_CBCC3,
	AT2600_PM_REG_CBCC4,
	AT2600_PM_REG_CBCC5,
	AT2600_PM_REG_CBCC6,
	AT2600_PM_REG_CBCS1,	/* CBC Status */
	AT2600_PM_REG_CBCS2,
	AT2600_PM_REG_PWM1S,	/* PWM (Pulse Width Modulator) Control */
	AT2600_PM_REG_PWM1D,
	AT2600_PM_REG_PWM2S,
	AT2600_PM_REG_PWM2D,
	AT2600_PM_REG_LED1C,	/* LED Control */
	AT2600_PM_REG_LED2C,
	AT2600_PM_REG_LEDCC,
	AT2600_PM_REG_GPIO1C1,	/* GPIO Control & ExREG Control */
	AT2600_PM_REG_E1REGC2,
	AT2600_PM_REG_E1REGC3,
	AT2600_PM_REG_GPIO2C1,
	AT2600_PM_REG_E2REGC2,
	AT2600_PM_REG_E2REGC3,
	AT2600_PM_REG_GPIO3C1,
	AT2600_PM_REG_E3REGC2,
	AT2600_PM_REG_E3REGC3,
	AT2600_PM_REG_GPIO4C1,
	AT2600_PM_REG_E4REGC2,
	AT2600_PM_REG_E4REGC3,
	AT2600_PM_REG_GPIO5C1,
	AT2600_PM_REG_E5REGC2,
	AT2600_PM_REG_E5REGC3,
	AT2600_PM_REG_GPIO6C1,
	AT2600_PM_REG_E6REGC2,
	AT2600_PM_REG_E6REGC3,
	AT2600_PM_REG_GPIOS, /*GPIO status*/
	AT2600_PM_REG_DCD_ST1,	/* Power Supply Control */
	AT2600_PM_REG_DCD_ST2,
	AT2600_PM_REG_DCD_ST3,
	AT2600_PM_REG_DCD_ST4,
	AT2600_PM_REG_INTC1, /*LDO INT STATUS*/
	AT2600_PM_REG_S1V2C1,
	AT2600_PM_REG_S1V2C2,
	AT2600_PM_REG_S1V2C3,
	AT2600_PM_REG_S1V8C1,
	AT2600_PM_REG_S1V8C2,
	AT2600_PM_REG_S1V8C3,
	AT2600_PM_REG_VBUSC1,
	AT2600_PM_REG_VBUSC2,
	AT2600_PM_REG_VBUSC3,
	AT2600_PM_REG_CMRC1,
	AT2600_PM_REG_CMRC2,
	AT2600_PM_REG_CMRC3,
	AT2600_PM_REG_D3V0C1 = 0X58,
	AT2600_PM_REG_D3V0C2,
	AT2600_PM_REG_D3V0C3,
	AT2600_PM_REG_SIM1C1,
	AT2600_PM_REG_SIM1C2,
	AT2600_PM_REG_SIM1C3,
	AT2600_PM_REG_SIM2C1,
	AT2600_PM_REG_SIM2C2,
	AT2600_PM_REG_SIM2C3,
	AT2600_PM_REG_SDC1 = 0X64,
	AT2600_PM_REG_SDC2,
	AT2600_PM_REG_SDC3,
	AT2600_PM_REG_FOCUSC1 = 0x6a,
	AT2600_PM_REG_FOCUSC2,
	AT2600_PM_REG_FOCUSC3,
	AT2600_PM_REG_USBC1,
	AT2600_PM_REG_USBC2,
	AT2600_PM_REG_USBC3,
	AT2600_PM_REG_ABBC1 = 0X76,
	AT2600_PM_REG_ABBC2,
	AT2600_PM_REG_ABBC3,
	AT2600_PM_REG_IMGC1,
	AT2600_PM_REG_IMGC2,
	AT2600_PM_REG_IMGC3,
	AT2600_PM_REG_R2V8C1 = 0X85,
	AT2600_PM_REG_R2V8C2,
	AT2600_PM_REG_R2V8C3,
	AT2600_PM_REG_R3V0C1,
	AT2600_PM_REG_R3V0C2,
	AT2600_PM_REG_R3V0C3,
	AT2600_PM_REG_LED1C1 = 0X8E,
	AT2600_PM_REG_LED1C2,
	AT2600_PM_REG_LED1C3,
	AT2600_PM_REG_LED2C1,
	AT2600_PM_REG_LED2C2,
	AT2600_PM_REG_LED2C3,
	AT2600_PM_REG_A2V5C1,
	AT2600_PM_REG_A2V5C2,
	AT2600_PM_REG_A2V5C3,
	AT2600_PM_REG_DCDLEDC1,
	AT2600_PM_REG_DCDLEDC2,
	AT2600_PM_REG_DCDLEDC3,
	AT2600_PM_REG_DCDLEDC4,
	AT2600_PM_REG_DCDDBGC1,
	AT2600_PM_REG_DCDDBGC2,
	AT2600_PM_REG_VIBRC1 = 0xab,
	AT2600_PM_REG_VIBRC2,
	AT2600_PM_REG_VIBRC3,
	AT2600_PM_REG_PWREN_UVLO =0xb6,
	AT2600_PM_REG_PWREN_MBG,
	AT2600_PM_REG_PWREN_INT9 = 0xbb,
	AT2600_PM_REG_PWREN_INT9_MASK,
	AT2600_PM_REG_NUM,
	AT2600_PM_REG_MAX
};

enum at2600_pm_reg_s1v2c1{
	S1V2C1_DOUT_1V2,
	S1V2C1_DOUT_1V25,	
	S1V2C1_DOUT_1V3,
	S1V2C1_DOUT_1V35,
	S1V2C1_DOUT_1V0,
	S1V2C1_DOUT_1V05,
	S1V2C1_DOUT_1V1,
	S1V2C1_DOUT_1V15,
};

enum at2600_pm_reg_s1v8c1{
	S1V8C1_DOUT_1V8,
	S1V8C1_DOUT_1V85,	
	S1V8C1_DOUT_1V9,
	S1V8C1_DOUT_1V95,
	S1V8C1_DOUT_1V6,
	S1V8C1_DOUT_1V65,
	S1V8C1_DOUT_1V7,
	S1V8C1_DOUT_1V75,
};

enum ldo_power_switch{
	LDO_POWER_ON,
	LDO_POWER_DOWN,	
};

/* pmu power supply mode */
enum power_supply_mode{
	PS_OFF = 0,
	PS_ON,
	PS_LOW,
} ;


u8 i2c_at2600_init(void);
u8 at2600_set_default_power_supply(void);

#endif /* ___AT2600_PM_REGS_H_______ */

