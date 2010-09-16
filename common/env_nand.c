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

#include <linux/types.h>
#include <common.h>
#include <nand.h>
#include <environment.h>
#include <crc32.h>

env_t envs;

int readenv (size_t offset, u_char * buf)
{
	size_t end = offset + CONFIG_ENV_RANGE;
	size_t amount_loaded = 0;
	size_t blocksize, len;
	u_char *char_ptr;

	blocksize = nd.erasesize;
	len = min(blocksize, CONFIG_ENV_OFFSET);

	char_ptr = &buf[amount_loaded];
	if (nand_read_skip_bad(&nd, offset, &len, char_ptr, end))
		return 1;
	amount_loaded += len;

	if (amount_loaded != CONFIG_ENV_OFFSET)
		return 1;

	return 0;
}

uchar *env_get_addr (int index)
{
	return ( ((uchar *)(envs.data + index)) );
}

uchar env_get_char_memory (int index)
{
	return ( *((uchar *)(envs.data + index)) );
}

uchar env_get_char (int index)
{
	uchar c;

	c = env_get_char_memory(index);
	return (c);
}

/************************************************************************
 * Match a name / name=value pair
 *
 * s1 is either a simple 'name', or a 'name=value' pair.
 * i2 is the environment index for a 'name2=value2' pair.
 * If the names match, return the index for the value2, else NULL.
 */

int envmatch (uchar *s1, int i2)
{

	while (*s1 == env_get_char(i2++))
		if (*s1++ == '=')
			return(i2);
	if (*s1 == '\0' && env_get_char(i2-1) == '=')
		return(i2);
	return(-1);
}


/************************************************************************
 * Look up variable from environment,
 * return address of storage for that variable,
 * or NULL if not found
 */

char *getenv (char *name)
{
	int i, nxt;

	if (envs.flags == 0)
		return NULL;
	for (i = 0; env_get_char(i) != '\0'; i = nxt + 1) {
		int val;

		for (nxt = i; env_get_char(nxt) != '\0'; ++nxt) {
			if (nxt >= ENV_SIZE) {
				return (NULL);
			}
		}
		if ((val = envmatch((uchar *)name, i)) < 0)
			continue;
		return ((char *)env_get_addr(val));
	}

	return (NULL);
}

int env_init(void)
{
	int crc1_ok = 0;

	if (readenv(CONFIG_ENV_OFFSET, (u_char *) &envs)) {
		printf("No Valid Environment Area Found \n");
		goto done;
	}
	crc1_ok = (crc32(0, envs.data, ENV_SIZE) == envs.crc);
	if(crc1_ok)		
		envs.flags = 1;
	else
		goto done;
	return 0;
done:
	envs.flags = 0;
	return -1;
}

