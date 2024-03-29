/*
 * (C) Copyright 2004
 * Texas Instruments
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __COMMON_H_
#define __COMMON_H_	1

#undef	_LINUX_CONFIG_H
#define _LINUX_CONFIG_H 1	/* avoid reading Linux autoconf.h file	*/

typedef unsigned char		uchar;
typedef volatile unsigned long	vu_long;
typedef volatile unsigned short vu_short;
typedef volatile unsigned char	vu_char;
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
  * container_of - cast a member of a structure out to the containing structure
  * @ptr:        the pointer to the member.
  * @type:       the type of the container struct this is embedded in.
  * @member:     the name of the member within the struct.
  *
  */
 #define container_of(ptr, type, member) ({                      \
         const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
         (type *)( (char *)__mptr - offsetof(type,member) );})

#define MAX_ERRNO       4095
#define unlikely(x)	__builtin_expect(!!(x), 0)

#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

static inline void *ERR_PTR(long error)
{	      
	     return (void *) error;
}
 
static inline long PTR_ERR(const void *ptr)
{
	     return (long) ptr;
}
   
static inline long IS_ERR(const void *ptr)
{
	     return IS_ERR_VALUE((unsigned long)ptr); 
} 

#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))

#include <config.h>
#include <linux/types.h>
#include <stdarg.h>

#ifdef CONFIG_ARM
#define asmlinkage	/* nothing */
#endif

 
#ifdef CONFIG_ARM
# include <asm/setup.h>
# include <asm/x-load-arm.h>	/* ARM version to be fixed! */
#endif /* CONFIG_ARM */

#ifdef	CFG_PRINTF
#define printf(fmt,args...)	serial_printf (fmt ,##args)
#define getc() serial_getc()
#else
#define printf(fmt,args...)
#define getc() ' '
#endif	/* CFG_PRINTF */
    
/* board/$(BOARD)/$(BOARD).c */
int 	board_init (void);
int 	nand_init (void);
int     mmc_boot (void);
void	board_hang (void);
uint32_t main_course(char *boot_dev_name);

/* cpu/$(CPU)/cpu.c */
int 	cpu_init (void);
int timer_init (void);
int do_load_serial_bin (ulong offset, int baudrate);

void cleanup_before_boot(void);
void  flush_cache (unsigned long dummy1, unsigned long dummy2);

#ifdef  CFG_UDELAY
void 	udelay (unsigned long usec);
#endif

/* nand driver */
#define NAND_CMD_READ0		0
#define NAND_CMD_READ1		1
#define NAND_CMD_READOOB	0x50
#define NAND_CMD_STATUS		0x70
#define NAND_CMD_READID		0x90
#define NAND_CMD_RESET		0xff

/* Extended Commands for Large page devices */
#define NAND_CMD_READSTART	0x30

int 	nand_chip(void);
int 	nand_read_block(uchar *buf, ulong block_addr);

int 	onenand_chip(void);
int	onenand_read_block(unsigned char *buf, ulong block);

  
#ifdef CFG_PRINTF

/* serial driver */
int	serial_init   (void);
void	serial_setbrg (void);
void	serial_putc   (const char);
void	serial_puts   (const char *);
int	serial_getc   (void);
int	serial_tstc   (void);

/* lib/printf.c */
void	serial_printf (const char *fmt, ...);
#endif

/* lib/crc.c */
void 	nand_calculate_ecc (const u_char *dat, u_char *ecc_code);
int 	nand_correct_data (u_char *dat, u_char *read_ecc, u_char *calc_ecc);

/* lib/board.c */
void	hang		(void) __attribute__ ((noreturn));
#endif	/* __COMMON_H_ */
