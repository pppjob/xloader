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
#include <config.h>
#include <nand.h>
#include <common.h>
#include <factory.h>
#include <linux/string.h>


/***************************************
*   factory data base operation
***************************************/
static struct factory_data_page f_page;

factory_data_t *factory_data_get(int fd_index)
{
	unsigned int offset;
	size_t length;
	int ret;

	offset = FACTORY_DATA_BASE_OFFSET;
	offset += fd_index * max(nd.writesize, 4096);
	length = sizeof(struct factory_data_page);
	
	/* read out the data */	
	ret = nand_read_skip_bad(&nd, offset, length, (u_char *)&f_page, FACTORY_DATA_MAX);
	if (ret) {
		printf("factory_data_get nand read fail\n");
		return 0;
	}

 	if (strcmp(f_page.magic, MAGIC_STRING) != 0) {
		return 0;
	}
 
 	return &(f_page.fd_user);
}


