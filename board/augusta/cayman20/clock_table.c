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

/* array order is same as struct div_name */
struct clock_default_setting div_setting[] = {
	{UNCHANGED, 1, 624 * MHZ},	/*"arm" */
	{UNCHANGED, 1, 52 * MHZ},	/*"dsphclk" */
	{UNCHANGED, 0, 52 * MHZ},	/*"comm TV " */
	{UNCHANGED, 1, 57 * MHZ},	/*"uart" */
	{UNCHANGED, 0, UNCHANGED},	/*"ms" memory stick*/
	{UNCHANGED, 0, 52 * MHZ},	/*"camclk"*/
	{UNCHANGED, 0, UNCHANGED},	/*"camera"*/
	{UNCHANGED, 0, 52 * MHZ},	/*"vpclk"*/
	{UNCHANGED, 1, 104 * MHZ},	/*"dspcore"*/
	{UNCHANGED, 1, 156 * MHZ},	/*"axi"*/
	{UNCHANGED, 0, UNCHANGED},	/*"spi0"*/
	{UNCHANGED, 0, UNCHANGED},	/*"spi1"*/
	{UNCHANGED, 0, UNCHANGED},	/*"spi2"*/
	{UNCHANGED, 1, UNCHANGED},	/*"sd"*/
	{PLL1,      0, 48 * MHZ},	/*"usb1"*/
	{PLL1,      0, 48 * MHZ},	/*"bt"*/
	{UNCHANGED, 1, 52 * MHZ},	/*"proc arc"*/
	{UNCHANGED, 1, 52 * MHZ},	/*"app"*/
	{UNCHANGED, 1, 52 * MHZ},	/*"gclk" graphic mm_top */
	{UNCHANGED, 1, UNCHANGED},	/*"pclk" apb clock */
	{UNCHANGED, 1, 156 * MHZ},	/*"mddr"*/
	{UNCHANGED, 0, UNCHANGED},	/*"gps2"*/
	{UNCHANGED, 0, UNCHANGED},	/*"hdmi_s0"*/
	{UNCHANGED, 0, UNCHANGED},	/*"hdmi"*/
	{UNCHANGED, 0, UNCHANGED},	/*"i2s0_s0"*/
	{UNCHANGED, 0, UNCHANGED},	/*"i2s0"*/
	{UNCHANGED, 0, UNCHANGED},	/*"i2s1_s0"*/
	{UNCHANGED, 0, UNCHANGED},	/*"i2s1"*/
	{UNCHANGED, 0, UNCHANGED},	/*"i2s2_s0"*/
	{UNCHANGED, 0, UNCHANGED},	/*"i2s2"*/
	{UNCHANGED, 0, UNCHANGED},	/*"vsclk"*/
	{UNCHANGED, 0, UNCHANGED},	/*"d_not_use0"*/
	{PLL1,      0, 48 * MHZ},	/*"usb0"*/
	{PLL1,      0, 26 * MHZ},	/*"iqadc_s0"*/
	{PLL1,      0, 24 * MHZ},	/*"vgaclk"*/
	{UNCHANGED, 0, UNCHANGED},	/*"tvclk"*/
	{UNCHANGED, 0, UNCHANGED},	/*"iqadc"*/
};

struct clock_default_setting pll_setting[] = {
	{UNCHANGED, 1, 624 * MHZ},	/*"pll1"*/
	{UNCHANGED, 0, 708 * MHZ},	/*"pll2"*/
	{UNCHANGED, 0, 806 * MHZ},	/*"pll3"*/
	
};
#define	PLL_DEFSET_COUNT		ARRAY_SIZE(pll_setting)
#define	DIV_DEFSET_COUNT		ARRAY_SIZE(div_setting)

