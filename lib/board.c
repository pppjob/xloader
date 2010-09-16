/*
 * Copyright (C) 2005 Texas Instruments.
 *
 * (C) Copyright 2004
 * Jian Zhang, Texas Instruments, jzhang@ti.com.
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <part.h>
#include <asm/arch/bootparam.h>
#include <asm/string.h>
#ifdef CFG_CMD_FAT
#include <fat.h>
#endif
#ifdef CONFIG_MMC
#include <mmc.h>
#endif

#ifdef CFG_PRINTF
int print_info(void)
{
	printf ("\n\nX-Loader 1.5 ("
		__DATE__ " - " __TIME__ ")\n");
	return 0;
}
#endif
typedef int (init_fnc_t) (void);
typedef int (boot_start) (uint32_t);

init_fnc_t *init_sequence[] = {
	cpu_init,		/* basic cpu dependent setup */
	board_init,		/* basic board dependent setup */
#ifdef CFG_PRINTF
 	serial_init,		/* serial communications setup */
	print_info,
#endif
#ifdef CFG_NAND
   	nand_init,		/* board specific nand init */
#endif
	NULL,
};
#if 0
#ifdef CFG_CMD_FAT
extern char * strcpy(char * dest,const char *src);
#else
char * strcpy(char * dest,const char *src)
{
	 char *tmp = dest;

	 while ((*dest++ = *src++) != '\0')
	         /* nothing */;
	 return tmp;
}
#endif
#endif

#ifdef CONFIG_MMC
int mmc_read_bootloader(int dev, int part)
{
	long size;
	unsigned long offset = CFG_LOADADDR;
	block_dev_desc_t *dev_desc = NULL;
	unsigned char ret = 0;

	ret = mmc_init(dev);
	if (ret != 0){
		printf("\n MMC init failed \n");
		return -1;
	}

	if (part) {	/* FAT Read for extenal SD card */
		dev_desc = mmc_get_dev(dev);
		size = file_fat_read("u-boot.bin", (unsigned char *)offset, 0);
		if (size == -1)
			return -1;
	} else {	/* RAW read for EMMC */
		ret = mmc_read(dev, 0x400, (unsigned char *)offset, 0x60000);
		if (ret != 1)
			return -1;
	}

	return 0;
}
#endif

extern int do_load_serial_bin(ulong offset, int baudrate);

#define __raw_readl(a)	(*(volatile unsigned int *)(a))

void start_armboot (void)
{
	u32	addr;
  	init_fnc_t **init_fnc_ptr;
	char boot_dev_name[8];

   	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}

	addr = main_course(boot_dev_name);
	
	/* go run U-Boot and never return */
	printf("Starting u-boot from %s ...\n", boot_dev_name);
	cleanup_before_boot();
	((boot_start *)addr)((uint32_t)&b_param);

	/* should never come here */
	printf("should never come here\n");
	hang();
}

void hang (void)
{
	/* call board specific hang function */
	board_hang();
	
	/* if board_hang() returns, hange here */
	printf("X-Loader hangs\n");
	for (;;);
}
