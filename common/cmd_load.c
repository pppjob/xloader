/*
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Serial up- and download support
 */
#include <common.h>
#define putc serial_putc
#define tstc serial_tstc
#ifdef CONFIG_KERMIT
#include "cmd_kermit.c"
#endif
#ifdef CONFIG_XMODEM
#include "cmd_xmodem.c"
#endif

#if 0
#ifdef CFG_CMD_FAT
extern void * memcpy(void * dest,const void *src,size_t count);
#else
void * memcpy(void * dest,const void *src,size_t count)
{
	char *tmp = (char *) dest, *s = (char *) src;

	while (count--)
		*tmp++ = *s++;

	return dest;
}
#endif
#endif

/* -------------------------------------------------------------------- */
static ulong load_serial_bin (ulong offset);



int do_load_serial_bin (ulong offset, int baudrate)
{
	ulong addr;
	int rcode = 0;
#ifdef CONFIG_KERMIT
	printf ("## Ready for binary (kermit) download "
		"to 0x%08lX at %d bps...\n",
		offset,
		baudrate);
#elif define CONFIG_XMODEM
	printf ("## Ready for binary (xmodem) download "
		"to 0x%08lX at %d bps...\n",
		offset,
		baudrate);
#endif
	addr = load_serial_bin (offset);
	if (addr == ~0) {
		printf ("## Binary download aborted\n");
		rcode = 1;
	} else {
		printf ("## Start Addr      = 0x%08lX\n", addr);
	}

	return rcode;
}


static ulong load_serial_bin (ulong offset)
{
	int size;
#ifdef CONFIG_KERMIT
	int i;
	set_kerm_bin_mode ((ulong *) offset);
	size = k_recv ();

	/*
	 * Gather any trailing characters (for instance, the ^D which
	 * is sent by 'cu' after sending a file), and give the
	 * box some time (100 * 1 ms)
	 */
	for (i=0; i<100; ++i) {
		if (tstc()) {
			(void) getc();
		}
		udelay(1000);
	}
#endif
#ifdef CONFIG_XMODEM
	// put xmodem here
	size = xmodem_download((ulong *) offset);
#endif
	printf("## Total Size      = 0x%08x = %d Bytes\n", size, size);

	return offset;
}

