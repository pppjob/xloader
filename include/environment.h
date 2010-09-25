/*
 * (C) Copyright 2002
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

#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_	1

/**************************************************************************
 *
 * The "environment" is stored as a list of '\0' terminated
 * "name=value" strings. The end of the list is marked by a double
 * '\0'. New entries are always added at the end. Deleting an entry
 * shifts the remaining entries to the front. Replacing an entry is a
 * combination of deleting the old value and adding the new one.
 *
 * The environment is preceeded by a 32 bit CRC over the data part.
 *
 **************************************************************************
 */



#define ENV_HEADER_SIZE	(sizeof(uint32_t))


#define ENV_SIZE (CONFIG_ENV_SIZE - ENV_HEADER_SIZE)

typedef	struct environment_s {
	uint32_t	crc;		/* CRC32 over data bytes	*/
	unsigned char	data[ENV_SIZE]; /* Environment data */
	unsigned char		flags;
} env_t;

/* Function that returns a character from the environment */
unsigned char env_get_char (int);

/* Function that returns a pointer to a value from the environment */
unsigned char *env_get_addr(int);
unsigned char env_get_char_memory (int index);

/* Function that updates CRC of the enironment */
void env_crc_update (void);

/* [re]set to the default environment */
void set_default_env(void);
char *getenv (char *name);
extern env_t envs;
int env_init(void);

#endif	/* _ENVIRONMENT_H_ */
