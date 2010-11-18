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
#include <asm/arch-atxx/bootparam.h>
#include <asm/arch-atxx/mddr.h>
#include <asm/arch-atxx/cache.h>
#include <asm/arch-atxx/delay.h>

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
	0x00121f10 ,0x00121f04
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

static uint8_t calibrate_4ch (uint32_t calibrate_addr, uint32_t rev_value, uint32_t calibrate_byte)
{
	uint8_t rev_val;
	uint8_t window_val;
	uint8_t flag = 1, right_flag = 1;
	int fail_length = 0;
	int down_fail_length = 0;
	uint32_t aver = 0, min = 0, max = 0;

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
		min = rev_val;
		max = CALIBRATE_REGION;
		while (1) {
			aver = (min + max)/2;
			change_cal_reg (calibrate_addr, 4, aver, calibrate_byte);
			if (!mem_special_test (10)) {
				window_val = aver;
				break;
			} else {
				if (flag == 1) {
					min = aver;
				} else {
					max = aver;
				}

				if(right_flag == 1) {
					fail_length++;
					if (fail_length > 9) {
						fail_length = 0;
						down_fail_length++;
						min = rev_val;
						max = CALIBRATE_REGION;
						flag = 0;
						if (down_fail_length >= 2) {
							right_flag = 0;
							flag = 1;
							min = 0;
							max = rev_val;
							down_fail_length = 0;
						}
					}
				} else {
					fail_length++;
					if (fail_length > 9) {
						fail_length = 0;
						down_fail_length++;
						min = 0;
						max = rev_val;
						flag = 0;
						if (down_fail_length >= 2) {
							printf("Calibrate FAIL! Can not find the window.");
							return 0xff;
						}
					}
				}
			}

		}
	}
	printf("window: 0x%08x. ", window_val);
	return window_val;
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

	printf ("min->0x%08x. max->0x%08x. cfg->0x%08x.", min_cfg, max_cfg, rev_val);
	change_cal_reg (calibrate_addr, 1, rev_val, calibrate_byte);

	return rev_val;
}

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

	rev_val = C0_mddr_cfg_data[0];
	printf ("\n\rCalibrate d8-e4:");
	window_val = calibrate_4ch (MDDR_CTRL_27_0, rev_val, 2);
	if (window_val == 0xff) {
		goto fail;
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

int fast_mddr_calibration (mddr_f_data_t *f_mddr, uint8_t *buf)
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
	for (i = 0; i < 8; i++)
		change_cal_reg ((MDDR_CTRL_27_0 + 4*i), 1, cal_data[i], 2);

	/*if passed, use these value*/
	if (!mem_special_test (2)) {
		mmu_cache_on (memory_map);
		return 0;
	} else {
		mmu_cache_on (memory_map);
		ret = mddr_calibration (buf);
		if (!ret) {
			return 0;
		} else {
			printf("fast mddr calibration failed.\n");
			return -1;
		}
	}
}

void mddr_core_init(uint32_t size)
{
	write64(0x3ffbe000, 0x0101010000000101ULL);
	write64(0x3ffbe008, 0x0001010100000100ULL);
	write64(0x3ffbe010, 0x0001000100000000ULL);
	write64(0x3ffbe018, 0x0101000000010001ULL);

	write64(0x3ffbe020, 0x0002020000010101ULL);

	if (size == MDDR_256) {
		write64(0x3ffbe028, 0x0003020002000003ULL);
	} else {
		write64(0x3ffbe028, 0x0003020102000003ULL);
	}
	write64(0x3ffbe030, 0x0202020200000000ULL);

	write64(0x3ffbe038, 0x0000020f07060a0fULL);

	write64(0x3ffbe040, 0x0a02000000030500ULL);
	write64(0x3ffbe048, 0x0000000000000000ULL);
	write64(0x3ffbe050, 0x0000000000000000ULL);
	write64(0x3ffbe058, 0x0c030700ff000000ULL);
	write64(0x3ffbe060, 0x0000000005050e00ULL);
	write64(0x3ffbe068, 0x0000ffff00000000ULL);
	write64(0x3ffbe070, 0x0000000000000000ULL);
	write64(0x3ffbe078, 0x00142d8900050000ULL);
	write64(0x3ffbe080, 0x00007a1200000014ULL);
	write64(0x3ffbe088, 0x0000000000000000ULL);
	write64(0x3ffbe090, 0x0000000000000000ULL);
	write64(0x3ffbe098, 0x0000000000000000ULL);
	write64(0x3ffbe0a0, 0x0000000000000000ULL);
	write64(0x3ffbe0a8, 0x0000000000000000ULL);
	write64(0x3ffbe0b0, 0x00000000aaaa8001ULL);
	write64(0x3ffbe0b8, 0x0000000000000000ULL);
	write64(0x3ffbe0c0, 0x0500010002000100ULL);
	write64(0x3ffbe0c8, 0x0000000000000200ULL);
	write64(0x3ffbe0d0, 0x0000000000000000ULL);
	write64(0x3ffbe0d8, 0x00121f1000121f10ULL);
	write64(0x3ffbe0e0, 0x00121f1000121f10ULL);
	write64(0x3ffbe0e8, 0x0012270400122704ULL);
	write64(0x3ffbe0f0, 0x0012270400122704ULL);

	write64(0x3ffbe0f8, 0x0000000000000000ULL);
	write64(0x3ffbe100, 0x0000000000000000ULL);
	write64(0x3ffbe108, 0x13000617000f1133ULL);
	write64(0x3ffbe110, 0x1300061713000617ULL);
	write64(0x3ffbe118, 0x0780000213000617ULL);
	write64(0x3ffbe120, 0x0780000207800002ULL);
	write64(0x3ffbe128, 0x0080000407800002ULL);
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

	if (size == MDDR_256) {
		write64(0x3ffbe028, 0x0003020002000003ULL);
	} else {
		write64(0x3ffbe028, 0x0003020102000003ULL);
	}

	write64(0x3ffbe018, 0x0101010000010001ULL);
	mdelay(10);
		

}

#define MDDR_128M		(128 * 1024 * 1024)
#define MDDR_TEST_DATA1		0x12345678
#define MDDR_TEST_DATA2		0x55aa7068

/* check for 128M, 256M only */
static uint32_t cal_get_mddr_size(void)
{
	uint32_t data;

	writel(MDDR_TEST_DATA1, MDDR_BASE_ADDR);
	writel(MDDR_TEST_DATA2, MDDR_BASE_ADDR + MDDR_128M);
	mdelay(5);
	data = readl(MDDR_BASE_ADDR);
	if (data == MDDR_TEST_DATA1) {
		return MDDR_256;
	} else {
		return MDDR_128;
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
	mddr_f_data_t * f_mddr;
	size_t size;

	f_data = factory_data_get(FD_MDDR);
	if (f_data) {
		b_param->mddr_data_send = 0;
		f_mddr = (mddr_f_data_t *)f_data->fd_buf;
		size = f_mddr->mddr_size;
	}else {
		printf("MDDR factory data not found\n");
		b_param->mddr_data_send = 1;
		size = get_mddr_size();		
		b_param->f_mddr.mddr_size = size;
	}

	printf("MDDR SIZE = %d MB\n", (1 << size) * 64);
	mddr_core_init(size);

	if (b_param->mddr_data_send) {
		mddr_calibration(b_param->f_mddr.mddr_cal_data);
	} else {
		fast_mddr_calibration (f_mddr, b_param->f_mddr.mddr_cal_data);
	}

	mddr_self_refresh();

	/* disable & clear MME irqs */
	write64(0x3fe00048, 0x120000000000ff00LL);

	return;
}

