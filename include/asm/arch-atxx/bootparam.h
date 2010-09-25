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

typedef struct {
	uint32_t load_address;
	uint32_t run_address;
	size_t firm_size;
	size_t nand_offset;
	uint32_t nand_end;
}boot_info_t;

#define	IV_SIZE			16
#define	CERT_SIZE		288
#define	SIGE_SIZE		128

#define HEAD_SIGNATURE		0xB0087006

typedef struct {
	unsigned char		iv[IV_SIZE];
	unsigned int		boot_signature;
	unsigned int		load_address;
	unsigned int		run_address;
	unsigned int		firm_size;
	unsigned int		nand_offset;
	unsigned int		image_type;	/* see below */
	unsigned char		reserved[56];
	unsigned char 		certificate[CERT_SIZE];
	unsigned char 		signature[SIGE_SIZE];
} atxx_image_header_t;

#define IH_TYPE_INVALID         0       /* Invalid Image                */
#define IH_TYPE_STANDALONE      1       /* Standalone Program           */
#define IH_TYPE_KERNEL          2       /* OS Kernel Image              */
#define IH_TYPE_RAMDISK         3       /* RAMDisk Image                */
#define IH_TYPE_MULTI           4       /* Multi-File Image             */
#define IH_TYPE_FIRMWARE        5       /* Firmware Image               */
#define IH_TYPE_SCRIPT          6       /* Script file                  */
#define IH_TYPE_FILESYSTEM      7       /* Filesystem Image (any type)  */
#define IH_TYPE_FLATDT          8       /* Binary Flat Device Tree Blob */
#define IH_TYPE_KWBIMAGE        9       /* Kirkwood Boot Image          */

extern boot_info_t boot_info;
int get_boot_param(void);
int parse_header(uint32_t addr, int ignore);

#endif /* _BOOTPARAM */
