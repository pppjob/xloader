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

#ifndef _BOOTPARAM
#define _BOOTPARAM
#include <factory.h>

struct boot_parameter{
	char magic[MAGIC_LENGTH];
	int mddr_data_send;
	mddr_f_data_t f_mddr;
};
extern struct boot_parameter b_param;

struct boot_info {
	uint32_t laddr;
	uint32_t raddr;
	size_t length;
	size_t nandoff;
	uint32_t nand_end;
};
extern struct boot_info boot_info;
int get_boot_param(void);

#endif /* _BOOTPARAM */
