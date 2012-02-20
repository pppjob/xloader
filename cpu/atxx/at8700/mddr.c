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
#include <asm/arch-atxx/clock.h>
#include <asm/arch-atxx/bootparam.h>
#include <asm/arch-atxx/mddr.h>
#include <asm/arch-atxx/cache.h>
#include <asm/arch-atxx/delay.h>
#include <asm/arch-atxx/topctl.h>
#include <asm/arch-atxx/pmu.h>
#include <malloc.h>

#define CALIBRATE_REGION		0x80
#define MDDRMEM_TEST_LENGTH		(120)
#define MDDR_NUMBER_TESTS		(5)
#define MDDRMEM_BASE_ADDR		(0x88000000)
#define MDDR_CTRL_27_0	 		0x3ffbe0d8
#define MDDR_CTRL_27_1			0x3ffbe0dc
#define MDDR_CTRL_28_0			0x3ffbe0e0
#define MDDR_CTRL_28_1			0x3ffbe0e4
#define MDDR_CTRL_29_0			0x3ffbe0e8
#define MDDR_CTRL_29_1			0x3ffbe0ec
#define MDDR_CTRL_30_0			0x3ffbe0f0
#define MDDR_CTRL_30_1			0x3ffbe0f4
#define	SQUARE_LENGTH			16

struct boot_parameter *fd_param = (struct boot_parameter *)MDDR_BASE_ADDR;

/***************************************************************************
* mddr calibration
****************************************************************************/

static int max_win, min_win;

uint32_t square_image[SQUARE_LENGTH][SQUARE_LENGTH];
uint32_t pattern[SQUARE_LENGTH] =
{
	0xffffffff, 0x00000000, 0x0000ffff, 0xffff0000,
	0x55555555, 0xaaaaaaaa, 0x5a5a5a5a, 0xa5a5a5a5,
	0x12345678, 0x87654321, 0x13572468, 0x24681357,
	0xff5500aa, 0x0af5af05, 0x5f0aaf50, 0x0055aaff
};
   
uint32_t C0_mddr_cfg_data[2] =
{
#if   defined(CFG_DDR2)
	0xe0051f08 ,0x00051f1f
#else
	0xe0091f08 ,0x00091f1f
#endif
};

static unsigned long int next = 1;

int rand(void)
{
	next = next*1103515245 + 12345;
	return (unsigned int)(next/65536)%32768;
}

void srand (unsigned int seed)
{
	next = seed;
}

int mem_special_test (uint8_t repeat_cnt)
{
	uint32_t size, i, j, k;
	uint32_t src_addr, dst_addr = 0;
	uint32_t value_src, value_des;

	size = SQUARE_LENGTH*SQUARE_LENGTH;
	src_addr = (uint32_t)&square_image[0][0];

	udelay(50);
	for (j = 0; j < repeat_cnt; j++) {
		/*make a random addr in mddr*/
		do{
			dst_addr = rand()%MDDRMEM_TEST_LENGTH;
			dst_addr = (dst_addr<<20);
			dst_addr += MDDRMEM_BASE_ADDR;
		}while ((dst_addr&0xfff00000) == (src_addr&0xfff00000));

		dst_addr += (20*rand())%(1000*1024);
		dst_addr = dst_addr&0xfffffffc;

		for(k = 0; k < size; k++) {
			*(uint32_t *)(dst_addr + 4*k) = *(uint32_t *)(src_addr + 4*k);
		}
		for (i = 0; i < size; i++) {
			value_src = square_image[i/SQUARE_LENGTH][i%SQUARE_LENGTH];
			value_des = *(uint32_t *)(dst_addr + 4*i);
			if (value_des != value_src) {
				return 1;
			}
		}
	}
	return 0;
}

int mem_special_test_bytemask (uint32_t byte_mask)
{
	uint32_t size, i, j, k;
	uint32_t src_addr, dst_addr = 0;
	uint32_t value_src, value_des;

	size = SQUARE_LENGTH*SQUARE_LENGTH;
	src_addr = (uint32_t)&square_image[0][0];

	udelay(100);
	for (j = 0; j < MDDR_NUMBER_TESTS; j++) {
		/* make a random addr in mddr*/
		do {
			dst_addr = rand()%MDDRMEM_TEST_LENGTH;
			dst_addr = (dst_addr<<20);
			dst_addr += MDDRMEM_BASE_ADDR;
		} while ((dst_addr&0xfff00000) == (src_addr&0xfff00000));

		dst_addr += (20*rand())%(1000*1024);
		dst_addr = dst_addr&0xfffffffc;
		for (k = 0; k < size; k++) {
			*(uint32_t *)(dst_addr + 4*k) = *(uint32_t *)(src_addr + 4*k);
		}

		for (i = 0; i < size; i++) {
			value_src = square_image[i/SQUARE_LENGTH][i%SQUARE_LENGTH];
			value_des = *(uint32_t *)(dst_addr + 4*i);
			if ((value_des & byte_mask) != (value_src & byte_mask)) {
				return 1;
			}
		}
	}
	return 0;
}

static void change_cal_reg (uint32_t addr, uint8_t reg_sum, uint8_t cal_data, uint32_t cal_byte)
{
	uint32_t temp;
	uint8_t i;

	for (i = 0; i < reg_sum; i++) {
		temp = *(volatile uint32_t*)(addr + (i*4));
		if (cal_byte == 0) {
			temp = (cal_data << 24) + (cal_data << 16) + (cal_data << 8) + (cal_data);
		}
		else {
			temp &= ~(0xff << (8*(cal_byte - 1)));
			temp |= cal_data << (8*(cal_byte - 1));
		}
		*(volatile uint32_t*)(addr + (i*4)) = temp;
	}
}

#define CALIBRATION_STEP 3
static uint8_t calibrate_4ch (uint32_t calibrate_addr, uint32_t rev_value, uint32_t calibrate_byte)
{
	uint8_t rev_val;
	uint8_t window_val;
	uint8_t i;
	uint8_t temp_val;
	uint8_t left_flag = 0;
	uint8_t right_flag = 0;
	uint32_t max_cfg = 0, min_cfg = 0;
	uint8_t flag = 0, down_flag;
	int pass_length = 0, fail_length = 0;
	uint32_t aver, min, max;


	/*calibrate_byte=0: calibrate all bytes, and all the bytes are same.*/
	if (calibrate_byte == 0) {
		rev_val = (uint8_t)(rev_value&0xff);
	} else {
		rev_val = (uint8_t)((rev_value >> (8*(calibrate_byte-1)))&0xff);
	}

	/*find the window*/
	change_cal_reg (calibrate_addr, 4, rev_val, calibrate_byte);
	if (!mem_special_test (10)) {
		window_val = rev_val;
	}else {
		for(i = 1; ; i++) {
			temp_val = rev_val + i * CALIBRATION_STEP;
			if(temp_val < CALIBRATE_REGION) {
				change_cal_reg (calibrate_addr, 4, temp_val, calibrate_byte);
				if (!mem_special_test (10)) {
					window_val = temp_val;
					break;
				}
			} else {
				/* reach the max value */
				right_flag = 1;  
			}

			temp_val = rev_val - i * CALIBRATION_STEP;
			/* means if(temp_val > 0) */
			if((i * CALIBRATION_STEP) < rev_val) {
				change_cal_reg (calibrate_addr, 4, temp_val, calibrate_byte);
				if (!mem_special_test (10)) {
					window_val = temp_val;
					break;
				}
			} else {
				/* reach the min value */
				left_flag = 1;
			}

			if(left_flag && right_flag) {
				printf("Calibrate FAIL! Can not find the window.");
				return 0xff;
			}
		}
	}
	printf("window: 0x%02x. ", window_val);

	min = window_val;
	max = CALIBRATE_REGION;
	fail_length = 0;
	flag = 1;
	down_flag = 1;
	while (1) {
		/*search max value*/
		if (flag) {
			aver = (min + max)/2;
			change_cal_reg (calibrate_addr, 4, aver, calibrate_byte);
			if (!mem_special_test(10)) {
				min = aver;
				pass_length++;
				if (pass_length > 9) {
					max_cfg = aver;
					flag = 0;
				}
			} else {
				fail_length++;
				pass_length = 0;
				max = aver;
				if (fail_length > 9) {
					max_cfg = aver;
					flag = 0;
				}
			}
		/*search min value*/
		} else {
			if (down_flag) {
				down_flag = 0;
				pass_length = 0;
				fail_length = 0;
				min = 0;
				max = window_val;
			}
			aver = (min + max)/2;
			change_cal_reg (calibrate_addr, 4, aver, calibrate_byte);
			if (!mem_special_test(10)) {
				max = aver;
				pass_length++;
				if(pass_length > 9) {
					min_cfg = aver;
					break;
				}
			} else {
				min = aver;
				fail_length++;
				pass_length = 0;
				if (fail_length > 9) {
					min_cfg = aver;
					break;
				}
			}
		}
	}

	rev_val = (max_cfg+min_cfg)/2;

	max_win = max_cfg;
	min_win = min_cfg;
	printf ("min->0x%02x. max->0x%02x. cfg->0x%02x.", min_cfg, max_cfg, rev_val);
	change_cal_reg (calibrate_addr, 4, rev_val, calibrate_byte);

	return rev_val;
}

static uint8_t calibrate_bytemask (uint32_t calibrate_addr, uint8_t rev_value, uint32_t calibrate_byte, uint32_t byte_mask)
{
	uint32_t max_cfg = 0, min_cfg = 0;
	uint32_t rev_val;
	uint8_t flag = 0, down_flag;
	int pass_length = 0, fail_length = 0;
	uint32_t aver, min, max;


	min = rev_value;
	max = CALIBRATE_REGION;
	fail_length = 0;
	flag = 1;
	down_flag = 1;
	while (1) {
		/*search max value*/
		if (flag) {
			aver = (min + max)/2;
			change_cal_reg (calibrate_addr, 1, aver, calibrate_byte);
			if (!mem_special_test_bytemask (byte_mask)) {
				min = aver;
				pass_length++;
				if (pass_length > 9) {
					max_cfg = aver;
					flag = 0;
				}
			} else {
				fail_length++;
				pass_length = 0;
				max = aver;
				if (fail_length > 9) {
					max_cfg = aver;
					flag = 0;
				}
			}
		/*search min value*/
		} else {
			if (down_flag) {
				down_flag = 0;
				pass_length = 0;
				fail_length = 0;
				min = 0;
				max = rev_value;
			}
			aver = (min + max)/2;
			change_cal_reg (calibrate_addr, 1, aver, calibrate_byte);
			if (!mem_special_test_bytemask (byte_mask)) {
				max = aver;
				pass_length++;
				if(pass_length > 9) {
					min_cfg = aver;
					break;
				}
			} else {
				min = aver;
				fail_length++;
				pass_length = 0;
				if (fail_length > 9) {
					min_cfg = aver;
					break;
				}
			}
		}
	}

	rev_val = (max_cfg+min_cfg)/2;

	printf ("min->0x%02x. max->0x%02x. cfg->0x%02x.", min_cfg, max_cfg, rev_val);
	change_cal_reg (calibrate_addr, 1, rev_val, calibrate_byte);

	return rev_val;
}
#if 0
int mddr_calibration (uint8_t *buf)
{
	uint32_t rev_val;
	uint32_t i, j;
	uint8_t cal_result;
	uint8_t window_val;

	printf ("MDDR Calibration ...\n");
	mmu_cache_off();
	for (i = 0; i < SQUARE_LENGTH; i++) {
		for (j = 0; j < SQUARE_LENGTH; j++) {
			square_image[i][j] = pattern[i];
		}
	}
	srand (5);

	rev_val = C0_mddr_cfg_data[1];
	change_cal_reg (MDDR_CTRL_29_0, 4, rev_val, 2);

	rev_val = C0_mddr_cfg_data[0];
	printf ("\n\rCalibrate d8-e4:");
	window_val = calibrate_4ch (MDDR_CTRL_27_0, rev_val, 2);
	if (window_val == 0xff) {
		for (i = 0; i < 0x7f; i++) {
			change_cal_reg (MDDR_CTRL_29_0, 4, i, 2);
			rev_val = C0_mddr_cfg_data[0];
			printf ("\n\rCalibrate d8-e4:");
			window_val = calibrate_4ch (MDDR_CTRL_27_0, rev_val, 2);
			if (window_val != 0xff) {
				break;
			}
		}
	}

	printf("\n\rCalibrate d8:");
	cal_result = calibrate_bytemask (MDDR_CTRL_27_0, window_val, 2, (uint32_t)0xff);
	if (cal_result == 0xff) {
		goto fail;
	}
	buf[0] = cal_result;

	printf ("\n\rCalibrate dc:");
	cal_result = calibrate_bytemask (MDDR_CTRL_27_1, window_val, 2, (uint32_t)0xff << 8);
	if (cal_result == 0xff) {
		goto fail;
	}
	buf[1] = cal_result;

	printf ("\n\rCalibrate e0:");
	cal_result = calibrate_bytemask (MDDR_CTRL_28_0, window_val, 2, (uint32_t)0xff << 16);
	if (cal_result == 0xff) {
		goto fail;
	}
	buf[2] = cal_result;

	printf ("\n\rCalibrate e4:");
	cal_result = calibrate_bytemask (MDDR_CTRL_28_1, window_val, 2, (uint32_t)0xff << 24);
	if (cal_result == 0xff) {
		goto fail;
	}
	buf[3] = cal_result;

	rev_val = C0_mddr_cfg_data[1];
	printf ("\n\rCalibrate e8-f4:");
	window_val = calibrate_4ch (MDDR_CTRL_29_0, rev_val, 2);
	if (window_val == 0xff) {
		goto fail;
	}

	printf ("\n\rCalibrate e8:");
	cal_result = calibrate_bytemask (MDDR_CTRL_29_0, window_val, 2, (uint32_t)0xff);
	if (cal_result == 0xff) {
		goto fail;
	}
	buf[4] = cal_result;

	printf ("\n\rCalibrate ec:");
	cal_result = calibrate_bytemask (MDDR_CTRL_29_1, window_val, 2, (uint32_t)0xff << 8);
	if (cal_result == 0xff) {
		goto fail;
	}
	buf[5] = cal_result;

	printf ("\n\rCalibrate f0:");
	cal_result = calibrate_bytemask (MDDR_CTRL_30_0, window_val, 2, (uint32_t)0xff << 16);
	if (cal_result == 0xff) {
		goto fail;
	}
	buf[6] = cal_result;

	printf ("\n\rCalibrate f4:");
	cal_result = calibrate_bytemask (MDDR_CTRL_30_1, window_val, 2, (uint32_t)0xff << 24);
	if (cal_result == 0xff) {
		goto fail;
	}
	buf[7] = cal_result;

	mmu_cache_on (memory_map);
	printf ("\nMDDR Calibration end ...\n");
	return 0;
fail:
	mmu_cache_on (memory_map);
	printf ("\nMDDR Calibration failed ...\n");
	return -1;
}
#else

static int __mddr_calibration(uint8_t *max_r, uint8_t *min_r, uint8_t *max_w, uint8_t *min_w)
{
	uint32_t ddr_val;
	uint32_t rev_val;
	uint32_t i, j;
	uint8_t window_val;

	ddr_val = *(volatile uint32_t *)(0x3ffbe058);
	while (!(ddr_val & 0x1)) {
		ddr_val = *(volatile uint32_t *)(0x3ffbe058);
		printf ("\n\r0x3ffbe058:0x%08x", ddr_val);
	};

	rev_val = C0_mddr_cfg_data[1];
	change_cal_reg (MDDR_CTRL_29_0, 4, rev_val, 2);

	rev_val = C0_mddr_cfg_data[0];

	printf ("\n\rCalibrate d8-e4:");
	window_val = calibrate_4ch (MDDR_CTRL_27_0, rev_val, 2);
	if (window_val == 0xff) {
		for (i = 1; i < 0x7f; i++) {
			change_cal_reg (MDDR_CTRL_29_0, 4, i, 2);
			rev_val = C0_mddr_cfg_data[0];
			window_val = calibrate_4ch (MDDR_CTRL_27_0, rev_val, 2);
			printf ("\n\rCalibrate d8-e4: reg_val->0x%02x, window->0x%02x",
					i, window_val);
			if (window_val != 0xff) {
				break;
			}
		}
	}
	*max_r = *max_r > max_win ? max_win : *max_r;
	*min_r = *min_r > min_win ? *min_r : min_win;

	rev_val = C0_mddr_cfg_data[1];
	printf ("\n\rCalibrate e8-f4:");
	window_val = calibrate_4ch (MDDR_CTRL_29_0, rev_val, 2);
	if (window_val == 0xff) {
		return 1;
	}
	*max_w = *max_w > max_win ? max_win : *max_w;
	*min_w = *min_w > min_win ? *min_w : min_win;

	return 0;
}

int mddr_calibration (uint8_t *buf)
{
	uint32_t rev_val, ddr_val;
	uint32_t i, j;
	uint8_t window_val_r, window_val_w;
	uint8_t max_r, max_w;
	uint8_t min_r, min_w;

	min_r = min_w = 0;
	max_r = max_w = 0x80;
	printf ("MDDR Calibration ...\n");
	mmu_cache_off();
	for (i = 0; i < SQUARE_LENGTH; i++) {
		for (j = 0; j < SQUARE_LENGTH; j++) {
			square_image[i][j] = pattern[i];
		}
	}
	srand (5);


	/* Do calibration when set arm-core to 1.4v */
	at2600_set_default_power_supply(S1V2C1_DOUT_1V4, S1V8C1_DOUT_1V8);
	__mddr_calibration(&max_r, &min_r, &max_w, &min_w);
#if 0
	/* Do calibration when set arm-core to 1.3v */
	at2600_set_default_power_supply(S1V2C1_DOUT_1V3, S1V8C1_DOUT_1V8);
	__mddr_calibration(&max_r, &min_r, &max_w, &min_w);
#endif
	clk_set_arm (312000000);
	clk_set_axi (156000000);

	/* Do calibration when set arm-core to 1.2v */
	at2600_set_default_power_supply(S1V2C1_DOUT_1V2, S1V8C1_DOUT_1V8);
	__mddr_calibration(&max_r, &min_r, &max_w, &min_w);

#if 0
	/* Do calibration when set arm-core to 1.1v */
	at2600_set_default_power_supply(S1V2C1_DOUT_1V1, S1V8C1_DOUT_1V8);
	__mddr_calibration(&max_r, &min_r, &max_w, &min_w);
#endif


	/* Do calibration when set arm-core to 1.0v */
	at2600_set_default_power_supply(S1V2C1_DOUT_1V0, S1V8C1_DOUT_1V8);
	__mddr_calibration(&max_r, &min_r, &max_w, &min_w);

	printf ("\nmin_r->0x%02x. max_r->0x%02x. min_w->0x%02x. max_w->0x%02x", 
		min_r, max_r, min_w, max_w);

	window_val_r = (max_r + min_r)/2;
	buf[0] = buf[1] = buf[2] = buf[3] = window_val_r;
	change_cal_reg (MDDR_CTRL_27_0, 4, window_val_r, 2);

	window_val_w = (max_w + min_w)/2;
	buf[4] = buf[5] = buf[6] = buf[7] = window_val_w;
	change_cal_reg (MDDR_CTRL_29_0, 4, window_val_w, 2);

	printf ("\nwindow_r->0x%02x. window_w->0x%02x", window_val_r, window_val_w);

	mmu_cache_on (memory_map);
	printf ("\nMDDR Calibration end ...\n");
	return 0;
fail:
	mmu_cache_on (memory_map);
	printf ("\nMDDR Calibration failed ...\n");
	return -1;
}
#endif
int fast_mddr_calibration (mddr_f_data_t *f_mddr, struct boot_parameter *b_param)
{
	uint8_t *cal_data;
	uint8_t i, j;
	int ret;

	mmu_cache_off ();
	for (i = 0; i < SQUARE_LENGTH; i++) {
		for (j = 0; j < SQUARE_LENGTH; j++) {
			square_image[i][j] = pattern[i];
		}
	}
	srand (5);

	cal_data = f_mddr->mddr_cal_data;
	for (i = 0; i < 4; i++)
		change_cal_reg ((MDDR_CTRL_27_0 + 4*i), 1, cal_data[i], 2);
	if (mem_special_test (2))
		goto calibration;

	for (i = 4; i < 8; i++)
		change_cal_reg ((MDDR_CTRL_27_0 + 4*i), 1, cal_data[i], 2);
	if (mem_special_test (2))
		goto calibration;

	printf ("Fast MDDR Calibration end ...\n");
	b_param->mddr_data_send = 0;
	mmu_cache_on (memory_map);
	return 0;

calibration:
	mmu_cache_on (memory_map);
	b_param->mddr_data_send = 1;
	ret = mddr_calibration (b_param->f_mddr.mddr_cal_data);
	if (!ret) {
		return 1;
	} else {
		printf("Fast MDDR Calibration failed.\n");
		return -1;
	}
}


/***************************************************************************
* mddr initialize 
****************************************************************************/
static uint64_t ddr2_init_table[18][6] = {
	{0,    156000000, 210000000, 269000000, 340000000, 403000000 }, 

	{ 0x3ffbe030, 0x0203020300000001ULL,0x0203020300000001ULL,
	0x0204030300000001ULL,0x0305040300000001ULL,0x0306040300000001ULL}, 

	{ 0x3ffbe038, 0x0000020f05060a0fULL,0x0000020f05060a0fULL,
	0x0000020f07080a0fULL,0x0000020f090a0a0fULL,0x0000020f090a0a0fULL}, 

	{ 0x3ffbe040, 0x0c02000000030600ULL,0x0c02000000030600ULL,
	0x1002000000040800ULL,0x1402000000050a00ULL,0x1702000000050b00ULL}, 

	{ 0x3ffbe058, 0x0003090000000000ULL,0x0003090000000000ULL,
	0x00040c0000000000ULL,0x00050f0000000000ULL,0x0005120000000000ULL}, 

	{ 0x3ffbe060,  0x0000000005061800ULL,0x0000000005061800ULL,
	0x0000000005082000ULL,0x00000000050a2800ULL,0x00000000050c3000ULL},

	{ 0x3ffbe078,  0x002436b0000200c8ULL,0x002436b0000200c8ULL,
	0x002c48eb000200c8ULL,0x00355b26000200c8ULL,0x003d6d60000200c8ULL},

	{ 0x3ffbe080,  0x00009c40000000c5ULL,0x00009c40000000c5ULL,
	0x0000d056000000c4ULL,0x0001046b000000c3ULL,0x00013880000000c3ULL},

	{ 0x3ffbe0c0,  0x0700010002000200ULL,0x0700010002000200ULL,
	0x0700010002000300ULL,0x0700010002000400ULL,0x0700010002000400ULL},

	{ 0x3ffbe0c8,  0x0612061202000200ULL,0x0612061202000200ULL,
	0x081b081b02000200ULL,0x0a240a2402000200ULL,0x0c2d0c2d02000200ULL},

	{ 0x3ffbe0d0,  0x0000000000000612ULL,0x0000000000000612ULL,
	0x000000000000081bULL,0x0000000000000a24ULL,0x0000000000000c2dULL},

	{ 0x3ffbe260,  0x0202000102020100ULL,0x0202000102020100ULL,
	0x0202000102030100ULL,0x0202000103040100ULL,0x0202000103040100ULL},

	{ 0x3ffbe270,  0x0000030003020303ULL,0x0000030003020303ULL,
	0x0000040003020404ULL,0x0000040003020505ULL,0x0000050003020505ULL},

	{ 0x3ffbe280,  0x0612061206120000ULL,0x0612061206120000ULL,
	0x081b081b081b0000ULL,0x0a240a240a240000ULL,0x0c2d0c2d0c2d0000ULL},

	{ 0x3ffbe288,  0x0046004604330433ULL,0x0046004604330433ULL,
	0x0046004606430643ULL,0x0046004608530653ULL,0x000600060a530653ULL},

	{ 0x3ffbe298,  0x0000000002010201ULL,0x0000000002010201ULL,
	0x0001000002010201ULL,0x0001000002010201ULL,0x0001000002010201ULL},

	{ 0x3ffbe2a0,  0x000000001a090000ULL,0x000000001a090000ULL,
	0x00000000220c0000ULL,0x000000002b0f0000ULL,0x0000000033120000ULL},

	{ 0x3ffbe2a8,  0x0000000000500000ULL,0x0000000000500000ULL,
	0x00000000006b0000ULL,0x0000000000860000ULL,0x0000000000a00000ULL},
};

void mddr_core_init(uint32_t size)
{
	int i, j;
	struct clk *clk;
	unsigned long  clk_value;

	clk = clk_get ("mddr");
	clk_value = clk_get_rate (clk);

#if   defined(CFG_DDR2)
        topctl_write_reg(TOPCTL9, 0xaaaa8000);
        topctl_write_reg(TOPCTL10, 0x40000400);

	write64(0x3ffbe000, 0x0101010000000100ULL);
	write64(0x3ffbe008, 0x0000010100000100ULL);
	write64(0x3ffbe010, 0x0001000100000000ULL);
	write64(0x3ffbe018, 0x0101000000010000ULL);
	write64(0x3ffbe020, 0x0002020000000101ULL);
	write64(0x3ffbe028, 0x0000030200000003ULL);
	write64(0x3ffbe048, 0x0000000000000000ULL);
	write64(0x3ffbe050, 0x0000000000000000ULL);
	write64(0x3ffbe068, 0x0000ffff00000000ULL);
	write64(0x3ffbe070, 0x0000000000000000ULL);
	write64(0x3ffbe088, 0x0000000000000000ULL);
	write64(0x3ffbe090, 0x0000000000000000ULL);
	write64(0x3ffbe098, 0x0000000000000000ULL);
	write64(0x3ffbe0a0, 0x0000000000000000ULL);
	write64(0x3ffbe0a8, 0x0000000000000000ULL);
	write64(0x3ffbe0b0, 0x0000000000000000ULL);
	write64(0x3ffbe0b8, 0x0000000000000000ULL);
	write64(0x3ffbe0d8, 0xe0059d08e0059d08ULL);
	write64(0x3ffbe0e0, 0xe0059d08e0059d08ULL);
	write64(0x3ffbe0e8, 0x00051f1f00051f1fULL);
	write64(0x3ffbe0f0, 0x00051f1f00051f1fULL);
	write64(0x3ffbe0f8, 0x0000000000000000ULL);
	write64(0x3ffbe100, 0x0000000000000000ULL);
	write64(0x3ffbe108, 0xf4013b27000f3100ULL);
	write64(0x3ffbe110, 0xf4013b27f4013b27ULL);
	write64(0x3ffbe118, 0x26c002c0f4013b27ULL);
	write64(0x3ffbe120, 0x26c002c026c002c0ULL);
	write64(0x3ffbe128, 0x0000000526c002c0ULL);
	write64(0x3ffbe130, 0x0000000000000000ULL);
	write64(0x3ffbe138, 0x0000000000000000ULL);
	write64(0x3ffbe140, 0x0000000000000000ULL);
	write64(0x3ffbe148, 0x0000000000000000ULL);
	write64(0x3ffbe150, 0x0000000000000000ULL);
	write64(0x3ffbe158, 0x0000000000000000ULL);
	write64(0x3ffbe160, 0x0000000000000000ULL);
	write64(0x3ffbe168, 0x0000000000000000ULL);
	write64(0x3ffbe170, 0x0000000000000000ULL);
	write64(0x3ffbe178, 0x0000000000000000ULL);
	write64(0x3ffbe180, 0x0000000000000000ULL);
	write64(0x3ffbe188, 0x0000000000000000ULL);
	write64(0x3ffbe190, 0x0000000000000000ULL);
	write64(0x3ffbe198, 0x0000000000000000ULL);
	write64(0x3ffbe1a0, 0x0000000000000000ULL);
	write64(0x3ffbe1a8, 0x0000000000000000ULL);
	write64(0x3ffbe1b0, 0x0000000000000000ULL);
	write64(0x3ffbe1b8, 0x0000000000000000ULL);
	write64(0x3ffbe1c0, 0x0000000000000000ULL);
	write64(0x3ffbe1c8, 0x0000000000000000ULL);
	write64(0x3ffbe1d0, 0x0000000000000000ULL);
	write64(0x3ffbe1d8, 0x0000000000000000ULL);
	write64(0x3ffbe1e0, 0x0000000000000000ULL);
	write64(0x3ffbe1e8, 0x0000000000000000ULL);
	write64(0x3ffbe1f0, 0x0000000000000000ULL);
	write64(0x3ffbe1f8, 0x0000000000000000ULL);
	write64(0x3ffbe200, 0x0000000000000000ULL);
	write64(0x3ffbe208, 0x0000000000000000ULL);
	write64(0x3ffbe210, 0x0000000000000000ULL);
	write64(0x3ffbe218, 0x0000000000000000ULL);
	write64(0x3ffbe220, 0x0000000000000000ULL);
	write64(0x3ffbe228, 0x0000000000000000ULL);
	write64(0x3ffbe230, 0x0000000000000000ULL);
	write64(0x3ffbe238, 0x0000000000000000ULL);
	write64(0x3ffbe240, 0x0000000000000000ULL);
	write64(0x3ffbe248, 0x0000000000000000ULL);
	write64(0x3ffbe250, 0x0000000000000000ULL);
	write64(0x3ffbe258, 0x0000000000000000ULL);
	write64(0x3ffbe268, 0x0403030001000102ULL);
	write64(0x3ffbe278, 0x0000000000000000ULL);
	write64(0x3ffbe290, 0x0101000000000000ULL);
	write64(0x3ffbe2b0, 0x0000000000000000ULL);
	write64(0x3ffbe2b8, 0x0000000000000000ULL);
	write64(0x3ffbe2c0, 0x0000000000000000ULL);
	write64(0x3ffbe2c8, 0x0000000000000000ULL);

	for (i = 1; i < 6; i++) {
		if (clk_value <= ddr2_init_table[0][i]) {
			for (j = 1; j < 18; j++) {
				write64(ddr2_init_table[j][0], ddr2_init_table[j][i]);
			}
			break;
		}
	}

	write64(0x3ffbe018, 0x0101010000010000ULL);
#else
	topctl_write_reg(TOPCTL9, 0xaaaa8001);
	topctl_write_reg(TOPCTL10, 0x00000400);

	write64(0x3ffbe000, 0x0101010000000100ULL);
	write64(0x3ffbe008, 0x0000010100000000ULL);
	write64(0x3ffbe010, 0x0001000100000000ULL);
	write64(0x3ffbe018, 0x0101000000010000ULL);
	write64(0x3ffbe020, 0x0002020000010101ULL);
#if 0	/* how to set the mddr size? */
	if (size == MDDR_256) {
		write64(0x3ffbe028, 0x0003020002000003ULL);
	} else {
		write64(0x3ffbe028, 0x0003020102000003ULL);
	}
#endif
	write64(0x3ffbe028, 0x0000020200000003ULL);
	write64(0x3ffbe030, 0x0202020100000001ULL);
	write64(0x3ffbe038, 0x0000020f07060a0fULL);
	write64(0x3ffbe040, 0x0a02000000030500ULL);
	write64(0x3ffbe048, 0x0000000000000000ULL);
	write64(0x3ffbe050, 0x0000000000000000ULL);
	write64(0x3ffbe058, 0x0003070000000000ULL);
	write64(0x3ffbe060, 0x0000000005028300ULL);
	write64(0x3ffbe068, 0x0000ffff00000000ULL);
	write64(0x3ffbe070, 0x0000000000000000ULL);
	write64(0x3ffbe078, 0x00142d9300010000ULL);
	write64(0x3ffbe080, 0x0000823600000017ULL);
	write64(0x3ffbe088, 0x0000000000000000ULL);
	write64(0x3ffbe090, 0x0000000000000000ULL);
	write64(0x3ffbe098, 0x0000000000000000ULL);
	write64(0x3ffbe0a0, 0x0000000000000000ULL);
	write64(0x3ffbe0a8, 0x0000000000000000ULL);
	write64(0x3ffbe0b0, 0x0000000000000000ULL);
	write64(0x3ffbe0b8, 0x0000000000000000ULL);
	write64(0x3ffbe0c0, 0x0600010002000100ULL);
	write64(0x3ffbe0c8, 0x0283028302000200ULL);
	write64(0x3ffbe0d0, 0x0000000000000283ULL);
	write64(0x3ffbe0d8, 0xe0091e08e0091e08ULL);
	write64(0x3ffbe0e0, 0xe0091e08e0091e08ULL);
	write64(0x3ffbe0e8, 0x0009151f0009151fULL);
	write64(0x3ffbe0f0, 0x0009151f0009151fULL);
	write64(0x3ffbe0f8, 0x0000000000000000ULL);
	write64(0x3ffbe100, 0x0000000000000000ULL);
	write64(0x3ffbe108, 0xf3003a27000f1100ULL);
	write64(0x3ffbe110, 0xf3003a27f3003a27ULL);
	write64(0x3ffbe118, 0x074002d0f3003a27ULL);
	write64(0x3ffbe120, 0x074002d0074002d0ULL);
	write64(0x3ffbe128, 0x00800004074002d0ULL);
	write64(0x3ffbe130, 0x0000000000000000ULL);
	write64(0x3ffbe138, 0x0000000000000000ULL);
	write64(0x3ffbe140, 0x0000000000000000ULL);
	write64(0x3ffbe148, 0x0000000000000000ULL);
	write64(0x3ffbe150, 0x0000000000000000ULL);
	write64(0x3ffbe158, 0x0000000000000000ULL);
	write64(0x3ffbe160, 0x0000000000000000ULL);
	write64(0x3ffbe168, 0x0000000000000000ULL);
	write64(0x3ffbe170, 0x0000000000000000ULL);
	write64(0x3ffbe178, 0x0000000000000000ULL);
	write64(0x3ffbe180, 0x0000000000000000ULL);
	write64(0x3ffbe188, 0x0000000000000000ULL);
	write64(0x3ffbe190, 0x0000000000000000ULL);
	write64(0x3ffbe198, 0x0000000000000000ULL);
	write64(0x3ffbe1a0, 0x0000000000000000ULL);
	write64(0x3ffbe1a8, 0x0000000000000000ULL);
	write64(0x3ffbe1b0, 0x0000000000000000ULL);
	write64(0x3ffbe1b8, 0x0000000000000000ULL);
	write64(0x3ffbe1c0, 0x0000000000000000ULL);
	write64(0x3ffbe1c8, 0x0000000000000000ULL);
	write64(0x3ffbe1d0, 0x0000000000000000ULL);
	write64(0x3ffbe1d8, 0x0000000000000000ULL);
	write64(0x3ffbe1e0, 0x0000000000000000ULL);
	write64(0x3ffbe1e8, 0x0000000000000000ULL);
	write64(0x3ffbe1f0, 0x0000000000000000ULL);
	write64(0x3ffbe1f8, 0x0000000000000000ULL);
	write64(0x3ffbe200, 0x0000000000000000ULL);
	write64(0x3ffbe208, 0x0000000000000000ULL);
	write64(0x3ffbe210, 0x0000000000000000ULL);
	write64(0x3ffbe218, 0x0000000000000000ULL);
	write64(0x3ffbe220, 0x0000000000000000ULL);
	write64(0x3ffbe228, 0x0000000000000000ULL);
	write64(0x3ffbe230, 0x0000000000000000ULL);
	write64(0x3ffbe238, 0x0000000000000000ULL);
	write64(0x3ffbe240, 0x0000000000000000ULL);
	write64(0x3ffbe248, 0x0000000000000000ULL);
	write64(0x3ffbe250, 0x0000000000000000ULL);
	write64(0x3ffbe258, 0x0000000000000000ULL);
	write64(0x3ffbe260, 0x0202000200010100ULL);
	write64(0x3ffbe268, 0x0101010001000001ULL);
	write64(0x3ffbe270, 0x0000020001010303ULL);
	write64(0x3ffbe278, 0x0000000000000000ULL);
	write64(0x3ffbe280, 0x0283028302830000ULL);
	write64(0x3ffbe288, 0x0000000000320032ULL);
	write64(0x3ffbe290, 0x0100000000000000ULL);
	write64(0x3ffbe298, 0x0000000002010201ULL);
	write64(0x3ffbe2a0, 0x000000000c0a0000ULL);
	write64(0x3ffbe2a8, 0x0000000000000000ULL);
	write64(0x3ffbe2b0, 0x0000000000000000ULL);
	write64(0x3ffbe2b8, 0x0000000000000000ULL);
	write64(0x3ffbe2c0, 0x0000000000000000ULL);
	write64(0x3ffbe2c8, 0x0000000000000000ULL);

	write64(0x3ffbe018, 0x0101010000010000ULL);
	mdelay(10);
#endif		
}

#define MDDR_128M		(128 * 1024 * 1024)
#define MDDR_256M		(256 * 1024 * 1024)
#define MDDR_512M		(512 * 1024 * 1024)
#define MDDR_TEST_DATA1		0x12345678
#define MDDR_TEST_DATA2		0x55aa7068
#define MDDR_TEST_DATA3		0xaa556870
#define MDDR_TEST_DATA4		0x7068aa55

/* check for 128M, 256M 512M 1G */
uint32_t cal_get_mddr_size(void)
{
	uint32_t data;

	writel(MDDR_TEST_DATA1, MDDR_BASE_ADDR);
	writel(MDDR_TEST_DATA2, MDDR_BASE_ADDR + MDDR_128M);
	data = readl(MDDR_BASE_ADDR);
	if (data == MDDR_TEST_DATA2) {
		return MDDR_128;
	}

	writel(MDDR_TEST_DATA3, MDDR_BASE_ADDR + MDDR_256M);
	data = readl(MDDR_BASE_ADDR);
	if (data == MDDR_TEST_DATA3) {
		return MDDR_256;
	}

	writel(MDDR_TEST_DATA4, MDDR_BASE_ADDR + MDDR_512M);
	data = readl(MDDR_BASE_ADDR);
	if (data == MDDR_TEST_DATA4) {
		return MDDR_512;
	} else	{
		return MDDR_1024;
	}
}

static uint32_t get_mddr_size(void)
{
	uint32_t size = 0;

	mddr_core_init(MDDR_256);
	size = cal_get_mddr_size();
	return size;

}

/* will not call it, it reduces mddr r/w speed, use for deepsleep mode */
static void mddr_self_refresh(void)
{
	uint64_t val;

	/*
	* Enter low power mode. 40~44 bit: lowpower control;
	* 32~36 bit: lowpower auto enable.
	*/
	writel(0x00ff, 0x3ffbe070);
	writel(0xff00, 0x3ffbe074);

	/*
	* Memory Self-Refresh with Memory and Controller
	* Clock Gating (Mode 5)
	*/
	val = readl(0x3ffbe044);
	val |= 0x0101;
	writel(val, 0x3ffbe044);

	mdelay(1);
	printf("Enable MDDR self-refresh mode\n");
}

void mddr_init(struct boot_parameter *b_param)
{
	factory_data_t * f_data;
	mddr_f_data_t * f_mddr = NULL;
	char *p_mddr;
	size_t size = 0 ,i = 0;
	int ret;

	f_data = factory_data_get(FD_CONFIG);
	if (f_data) {
		b_param->mddr_data_send = 1;

		if((p_mddr = strstr(f_data->fd_buf, "phys_mem")) == NULL) {
			size = MDDR_256;
		} else {
			while(*p_mddr < '0' || *p_mddr > '9')
				p_mddr ++;
			while(*p_mddr >= '0' && *p_mddr <= '9') {
				size *= 10;
				size += (*p_mddr -'0');
				p_mddr ++;
			}

			while((size / 64) >> i)
				i++;
			size = i -1;
		}
		b_param->f_mddr.mddr_size = size;
	} else {
		printf("MDDR factory data not found\n");
		b_param->mddr_data_send = 0;
		size = MDDR_512;
		b_param->f_mddr.mddr_size = size;
	}

	printf("MDDR SIZE = %d MB\n", (1 << size) * 64);

	mddr_core_init(size);

	/* mddr need do stress calibration everytime in mass production. */
	mddr_calibration(b_param->f_mddr.mddr_cal_data);

	if (b_param->mddr_data_send) {
		memcpy(fd_param->f_mddr.mddr_cal_data, b_param->f_mddr.mddr_cal_data, 8);
	} 

	fd_param->mddr_data_send = b_param->mddr_data_send;
	if (fd_param->mddr_data_send) {
		fd_param->f_mddr.mddr_size = size;
		memcpy(fd_param->magic,
                        BOOT_MAGIC_STRING, BOOT_MAGIC_LENGTH);
        } else {
                memset(fd_param, 0, sizeof(struct boot_parameter));
        }

	mddr_self_refresh();

	/* disable & clear MME irqs */
	write64(0x3fe00048, 0x120000000000ff00LL);

	return;
}

/***************************************************************************
* mddr initialize 
****************************************************************************/
static void sdram_core_init(uint32_t size)
{
        write64(0x3ffbe000, 0x0101010000000100ULL);     //DENALI_CTL_00 
        write64(0x3ffbe008, 0x0000010100000100ULL);      //{0x3ffbe008                 
        write64(0x3ffbe010, 0x0001000100000000ULL);     //DENALI_CTL_02           
        write64(0x3ffbe018, 0x0101000000010000ULL);     //DENALI_CTL_03           
        write64(0x3ffbe020, 0x0002020300010100ULL);     //DENALI_CTL_04       
        write64(0x3ffbe028, 0x0003020100000003ULL);     //DENALI_CTL_05          
        write64(0x3ffbe030, 0x0102020000000000ULL);     //DENALI_CTL_06
        write64(0x3ffbe038, 0x0000020f06080a0fULL);     //DENALI_CTL_07   
        write64(0x3ffbe040, 0x0a02000000030500ULL);
        write64(0x3ffbe048, 0x0000007f16161616ULL);     //DENALI_CTL_09              
        write64(0x3ffbe050, 0x0f0d0d0d0d010000ULL);     //DENALI_CTL_10      
        write64(0x3ffbe058, 0x1103070000493600ULL);     //DENALI_CTL_11           
        write64(0x3ffbe060, 0x0000000000050e01ULL);     //DENALI_CTL_12               
        write64(0x3ffbe068, 0x0000ffff00000000ULL);
        write64(0x3ffbe070, 0x0000000000000000ULL);
        write64(0x3ffbe078, 0x00144e1600010000ULL);     //DENALI_CTL_15                 
        write64(0x3ffbe080, 0x0000001100000014ULL);     //DENALI_CTL_16
        write64(0x3ffbe088, 0x0000000000000000ULL);     //DENALI_CTL_17
        write64(0x3ffbe090, 0x0000000000000000ULL);     //DENALI_CTL_18
        write64(0x3ffbe098, 0x0000000000000000ULL);     //DENALI_CTL_19
        write64(0x3ffbe0a0, 0x0000000000000000ULL);     //DENALI_CTL_20
        write64(0x3ffbe0a8, 0x0000000000000000ULL);     //DENALI_CTL_21
        write64(0x3ffbe0b0, 0x0000000000000000ULL);     //DENALI_CTL_22
        write64(0x3ffbe0b8, 0x0000000000000000ULL);     //DENALI_CTL_23
        write64(0x3ffbe0c0, 0x0201000100000000ULL);     //DENALI_CTL_24
        write64(0x3ffbe0c8, 0x0301000001000001ULL);     //DENALI_CTL_25
        write64(0x3ffbe0d0, 0x0000000002000103ULL);     //DENALI_CTL_26
        write64(0x3ffbe0d8, 0x0032000000000000ULL);     //DENALI_CTL_27
        write64(0x3ffbe0e0, 0x0000000000000032ULL);     //DENALI_CTL_28
        write64(0x3ffbe0e8, 0x0000000000000000ULL);     //DENALI_CTL_29
        write64(0x3ffbe018, 0x0101010000010000ULL);     //DENALI_CTL_03

        mdelay(10);
}


void memory_init(struct boot_parameter *b_param)
{
#if   defined(CFG_SDRAM)
	topctl_write_reg(TOPCTL10, topctl_read_reg(TOPCTL10) | (1 << 31));

	size = MDDR_128;
	printf("SDRAM SIZE = %d MB\n", (1 << size) * 64);
	sdram_core_init(size);
#else 
	mddr_init(b_param);
#endif

	return;
}



