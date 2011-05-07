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
 -----------------------------------------------------------------------------*/

#include <linux/types.h>
#include <asm/io.h>
#include <config.h>
#include <common.h>
#include <factory.h>
#include <asm/arch-atxx/io.h>
#include <asm/arch-atxx/regs_base.h>
#include <asm/arch-atxx/topctl.h>
#include <asm/arch-atxx/bootparam.h>
#include <asm/arch-atxx/mddr.h>
#include <asm/arch-atxx/delay.h>

static void sdram_core_init(uint32_t size)
{
	write64(0x3ffbe000, 0x0101010000000100ULL);	//DENALI_CTL_00 
	write64(0x3ffbe008, 0x0000010100000100ULL);      //{0x3ffbe008                 
	write64(0x3ffbe010, 0x0001000100000000ULL);	//DENALI_CTL_02           
	write64(0x3ffbe018, 0x0101000000010000ULL);	//DENALI_CTL_03           
	write64(0x3ffbe020, 0x0002020300010100ULL);	//DENALI_CTL_04       
	write64(0x3ffbe028, 0x0003020100000003ULL);	//DENALI_CTL_05          
	write64(0x3ffbe030, 0x0102020000000000ULL);	//DENALI_CTL_06
	write64(0x3ffbe038, 0x0000020f06080a0fULL);	//DENALI_CTL_07   
	write64(0x3ffbe040, 0x0a02000000030500ULL);
	write64(0x3ffbe048, 0x0000007f16161616ULL);	//DENALI_CTL_09              
	write64(0x3ffbe050, 0x0f0d0d0d0d010000ULL);	//DENALI_CTL_10      
	write64(0x3ffbe058, 0x1103070000493600ULL);	//DENALI_CTL_11           
	write64(0x3ffbe060, 0x0000000000050e01ULL);	//DENALI_CTL_12               
	write64(0x3ffbe068, 0x0000ffff00000000ULL);  
	write64(0x3ffbe070, 0x0000000000000000ULL);
	write64(0x3ffbe078, 0x00144e1600010000ULL);	//DENALI_CTL_15                 
	write64(0x3ffbe080, 0x0000001100000014ULL);	//DENALI_CTL_16
	write64(0x3ffbe088, 0x0000000000000000ULL);	//DENALI_CTL_17
	write64(0x3ffbe090, 0x0000000000000000ULL);	//DENALI_CTL_18
	write64(0x3ffbe098, 0x0000000000000000ULL);	//DENALI_CTL_19
	write64(0x3ffbe0a0, 0x0000000000000000ULL);	//DENALI_CTL_20
	write64(0x3ffbe0a8, 0x0000000000000000ULL);	//DENALI_CTL_21
	write64(0x3ffbe0b0, 0x0000000000000000ULL);	//DENALI_CTL_22
	write64(0x3ffbe0b8, 0x0000000000000000ULL);	//DENALI_CTL_23
	write64(0x3ffbe0c0, 0x0201000100000000ULL);	//DENALI_CTL_24
	write64(0x3ffbe0c8, 0x0301000001000001ULL);	//DENALI_CTL_25
	write64(0x3ffbe0d0, 0x0000000002000103ULL);	//DENALI_CTL_26
	write64(0x3ffbe0d8, 0x0032000000000000ULL);	//DENALI_CTL_27
	write64(0x3ffbe0e0, 0x0000000000000032ULL);	//DENALI_CTL_28
	write64(0x3ffbe0e8, 0x0000000000000000ULL);	//DENALI_CTL_29
	write64(0x3ffbe018, 0x0101010000010000ULL);     //DENALI_CTL_03

	mdelay(10);	
}

void memory_init(struct boot_parameter *b_param)
{
	size_t size;

#ifdef CFG_SDRAM
	topctl_write_reg(TOPCTL10, topctl_read_reg(TOPCTL10) | (1 << 31));

	size = MDDR_128;
	printf("SDRAM SIZE = %d MB\n", (1 << size) * 64);
	sdram_core_init(size);
#else
#endif

	/* disable & clear MME irqs */
	write64(0x3fe00048, 0x120000000000ff00LL);
	return;
}

