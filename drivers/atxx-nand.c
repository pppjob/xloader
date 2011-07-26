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
#include <nand.h>
#include <asm/io.h>
#include "atxx-nand.h"
#include <common.h>
#include <asm/arch-atxx/regs_base.h>
#include <asm/arch-atxx/clock.h>
#include <asm/arch-atxx/delay.h>
#include <linux/string.h>
#include <linux/bitops.h>
#include <asm/errno.h>

struct nand_info nd;

static int ecc_number = 4;
static int oob_size = 128;
static int size_per_sector = 512;
#define OOB_SIZE_128			128
static int bus_width = 0;
#define CONFIG_ATXX_NAND_DMA

static  uint32_t atxx_nd_read_reg(uint32_t reg)
{
        return readl(ATXX_NAND_BASE + reg);
}

static  void atxx_nd_write_reg(uint32_t reg, uint32_t val)
{
        writel(val, ATXX_NAND_BASE + reg);
}

 void ATXX_ND_LOG(void)
{
	/* dump registers */
	printf(	"NFC_PARA0:%08x, NFC_TIMING:%08x\n, NFC_CMD:%08x, "
			"NFC_STATUS:%08x\n, NFC_CCFG:%08x, NFC_SOFT_PIN:%08x\n",
		atxx_nd_read_reg(REG_NFC_PARA0), atxx_nd_read_reg(REG_NFC_TIMING), 
		atxx_nd_read_reg(REG_NFC_CMD), atxx_nd_read_reg(REG_NFC_STATUS), 
		atxx_nd_read_reg(REG_NFC_CCFG), atxx_nd_read_reg(REG_NFC_SOFT_PIN)
			);
	printf(	"NFC_CFG0:%08x, NFC_CFG1:%08x\n, NFC_CFG2:%08x,"
			"NFC_FADDR0:%08x\n, NFC_FADDR1:%08x, NFC_FIFO_CFG:%08x\n",
		atxx_nd_read_reg(REG_NFC_CFG0), atxx_nd_read_reg(REG_NFC_CFG1), 
		atxx_nd_read_reg(REG_NFC_CFG2), atxx_nd_read_reg(REG_NFC_FADDR0), 
		atxx_nd_read_reg(REG_NFC_FADDR1), atxx_nd_read_reg(REG_NFC_FIFO_CFG)
		);
	printf("NFC_INT_CODE:%08x, NFC_INT_MASK:%08x\n, NFC_DMA_CFG:%08x,"
			"NFC_DMA_CTRL:%08x\n, NFC_DMA_STAT:%08x\n",
		atxx_nd_read_reg(REG_NFC_INT_CODE), atxx_nd_read_reg(REG_NFC_INT_MASK), 
		atxx_nd_read_reg(REG_NFC_DMA_CFG), atxx_nd_read_reg(REG_NFC_DMA_CTRL),
		atxx_nd_read_reg(REG_NFC_DMA_STAT)
			);
	printf("NFC_DEBUG0:%08x\n, NFC_DEBUG1:%08x, NFC_DEBUG2:%08x\n",
			 atxx_nd_read_reg(REG_NFC_DEBUG0), 
			 atxx_nd_read_reg(REG_NFC_DEBUG1),
			 atxx_nd_read_reg(REG_NFC_DEBUG2));
	
}

static  void NFC_RESET(void)
{
	atxx_nd_write_reg(REG_NFC_CFG2, atxx_nd_read_reg(REG_NFC_CFG2) & ~NFC_CFG2_RESET);
	udelay(10);
	atxx_nd_write_reg(REG_NFC_CFG2, atxx_nd_read_reg(REG_NFC_CFG2) | NFC_CFG2_RESET);
	udelay(10);
}

static  void WAIT_CMD_END(void)
{
	int i = 0;
	do {
		if (!(atxx_nd_read_reg(REG_NFC_INT_CODE) & 0x1)) {
			udelay(1);
		} else {
			atxx_nd_write_reg(REG_NFC_INT_CODE, 0x1);
			return;
		}
	} while (i++ < ATXX_ND_TIMEOUT);

	printf("wait command end timeout\n");
}

static  void WAIT_DECODE_CODE_READY(int code_word)
{
	int i = 0;
	do {
		if (!(atxx_nd_read_reg(REG_NFC_INT_CODE) & (1 << (8 + code_word)))) {
			udelay(1);
		} else {
			atxx_nd_write_reg(REG_NFC_INT_CODE, 1 << (8 + code_word));
			return;
		}
	} while (i++ < ATXX_ND_TIMEOUT);

	printf( "wait ECC ready timeout\n");
	ATXX_ND_LOG();
}

static  void WAIT_DMA_FINI(void)
{
	uint32_t i = 0;
	do {
		if (!(atxx_nd_read_reg(REG_NFC_INT_CODE) & NFC_INT_DMA_FINI)) {
			udelay(1);
		} else {
			atxx_nd_write_reg(REG_NFC_INT_CODE, NFC_INT_DMA_FINI);
			return;
		}
	} while (i++ < 0xffffffful);
}

static void atxx_nd_set_timing(void)
{
	uint32_t reg_data;
	uint8_t read_hold_time, read_setup_time;
	uint8_t write_hold_time, write_setup_time;
        struct clk *clk_app;
	unsigned long rate;

        clk_app = clk_get("app");	
	rate = clk_get_rate(clk_app);

	/* timing params according to clk_app */
	if (rate >= 156 * 1000 * 1000) {
		read_hold_time = read_setup_time = 3;
		write_hold_time = write_setup_time = 4;
	} else if ((rate < 156 * 1024 * 1024)
		   && (rate >= 104 * 1000 * 1000)) {
#if defined(CONFIG_BOARD_ATB1005)
		read_hold_time = 0;
		read_setup_time = 2;
		write_hold_time = 4;
		write_setup_time = 3;
#else
		read_hold_time = 0;
		read_setup_time = 2;
		write_hold_time = 3;
		write_setup_time = 3;
#endif
	} else {
		read_hold_time = read_setup_time = 3;
		write_hold_time = write_setup_time = 4;
	}

	reg_data = (0xff << 16) | (read_hold_time << NFC_READ_HOLD_TIME_SHIFT)
	    | (read_setup_time << NFC_READ_SETUP_TIME_SHIFT)
	    | (write_hold_time << NFC_WRITE_HOLD_TIME_SHIFT)
	    | (write_setup_time << NFC_WRITE_SETUP_TIME_SHIFT);
	atxx_nd_write_reg(REG_NFC_TIMING, reg_data);

}


static  void nfc_send_reset_cmd(void)
{
	atxx_nd_write_reg(REG_NFC_CMD, (NFC_CMD_RESET << NFC_CMD_CMD_SHIFT) 
			| NFC_CMD_ENABLE);
	WAIT_CMD_END();
}



/******************** cmd for read operation **********************/
static  void nfc_config_small_page_read(int width, int page)
{
	atxx_nd_write_reg(REG_NFC_PARA0, 
			atxx_nd_read_reg(REG_NFC_PARA0) | NFC_PARAR0_OLD);
	atxx_nd_write_reg(REG_NFC_CFG0, 
			atxx_nd_read_reg(REG_NFC_CFG0) & 0xffffff00);
	atxx_nd_write_reg(REG_NFC_CCFG, 
			NAND_CMD_READ1 | (NAND_CMD_READOOB << 16));

	if (width == NAND_BUSWIDTH_16) {
		atxx_nd_write_reg(REG_NFC_FADDR0, page << 8);
		atxx_nd_write_reg(REG_NFC_BADDR, page << 8);
	} else {
		atxx_nd_write_reg(REG_NFC_FADDR0, page << 8);
	}
}

static  void nfc_send_readoob_udf_cmd(u32 data_len, u32 addr_len)
{
	uint32_t cmd;

	cmd = (NFC_CMD_UDFB_READ << NFC_CMD_CMD_SHIFT)
		| (data_len << NFC_CMD_DATA_LEN_SHIFT)
		| (addr_len << NFC_CMD_ADDR_LEN_SHIFT)
		| NFC_CMD_WAIT_BUSY | NFC_CMD_INT | NFC_CMD_ENABLE;

	atxx_nd_write_reg(REG_NFC_CMD, cmd);
	WAIT_CMD_END();
}

static  void nfc_send_readpage_ecc_cmd(u32 data_len, u32 addr_len)
{
	uint32_t cmd;

	cmd = (NFC_CMD_READ << NFC_CMD_CMD_SHIFT)
		| (data_len << NFC_CMD_DATA_LEN_SHIFT)
		| (addr_len << NFC_CMD_ADDR_LEN_SHIFT)
		| NFC_CMD_WAIT_BUSY | NFC_CMD_ECC | NFC_CMD_ENABLE;

	atxx_nd_write_reg(REG_NFC_CMD, cmd);
	WAIT_CMD_END();
}

static void atxx_nd_dma_read(uint8_t * addr,
		int read_buf_off, int len)
{
	uint32_t m_dma;

	m_dma = (uint32_t)addr;
	flush_cache(0, 0);
	atxx_nd_write_reg(REG_NFC_DMA_SAR, NFC_READ_BUF_ADDR + read_buf_off);
	atxx_nd_write_reg(REG_NFC_DMA_DAR, m_dma);
	atxx_nd_write_reg(REG_NFC_DMA_CTRL, 
			((len << NFC_DMA_LEN) | NFC_DMA_ENABLE) & (~NFC_DMA_RW));
	WAIT_DMA_FINI();

}

static  void nfc_config_big_page(int width, int addr)
{
	int nd_addr;

	nd_addr = addr & 0xffff0000;
	nd_addr |= (addr & 0xffff) / 2;

	if (width == NAND_BUSWIDTH_16) {
		atxx_nd_write_reg(REG_NFC_FADDR0, nd_addr);
		atxx_nd_write_reg(REG_NFC_BADDR, nd_addr);
	} else {
		atxx_nd_write_reg(REG_NFC_FADDR0, addr);
	}
}

static  void nfc_send_readoob_cmd(u32 data_len, u32 addr_len)
{
	uint32_t cmd;

	cmd = (NFC_CMD_READ << NFC_CMD_CMD_SHIFT)
		| (addr_len << NFC_CMD_ADDR_LEN_SHIFT)
		| (data_len << NFC_CMD_DATA_LEN_SHIFT)
		| NFC_CMD_WAIT_BUSY | NFC_CMD_ENABLE;

	atxx_nd_write_reg(REG_NFC_CMD, cmd);
	WAIT_CMD_END();
}

static  void nfc_send_readpage_udf_cmd(u32 data_len, u32 addr_len, u32 ecc_enable)
{
	uint32_t cmd;

	atxx_nd_write_reg(REG_NFC_CCFG, (NAND_CMD_RNDOUTSTART << 8) | NAND_CMD_RNDOUT);
	atxx_nd_write_reg(REG_NFC_FADDR0, 0);
	atxx_nd_write_reg(REG_NFC_BADDR, 0);

	if (ecc_enable) {
		cmd = (NFC_CMD_UDFA_READ << NFC_CMD_CMD_SHIFT)
			| (addr_len << NFC_CMD_ADDR_LEN_SHIFT)
			| (data_len << NFC_CMD_DATA_LEN_SHIFT)
			| NFC_CMD_ECC | NFC_CMD_ENABLE;
	} else {
		cmd = (NFC_CMD_UDFA_READ << NFC_CMD_CMD_SHIFT)
			| (addr_len << NFC_CMD_ADDR_LEN_SHIFT)
			| (data_len << NFC_CMD_DATA_LEN_SHIFT)
			| NFC_CMD_ENABLE;
	}
	atxx_nd_write_reg(REG_NFC_CMD, cmd);
}

/*******************************************
*  Bit0~3           PAGE_SIZE           BLOCK_SIZE                               
*  0000 : If = 0, we identify the nand with ID table as before.                  
*  0001              4096                   256KB                       
*  0010              4096                   512KB                       
*  0100              4096                  1024KB                        
*  1000              2048                   256KB                       
*  0011              2048                   128KB                       
*  0110              2048                   512KB                       
*  1100              2048                    64KB                       
*  1010              2048                  1024KB                       
*  1001              4096                  2048KB
*  0101              4096                  4096KB
*  0111              8192                  2048KB
*  1011              8192                  4096KB
*  1101              8192                   256KB                        
*  1110              8192                   512KB                        
*  1111              8192                  1024KB                        
*
*  bit 4~6           ecc bit             oob_size_per_512     
*  000                4                      16
*  001                8                      20    
*  010                12                     28
*  100                15                     36 
*  111                1                      16
*  110                0                      16
*  101                6                      16
*  011                10                     24
*
*  bit 7             Bus Width
*  0                    8
*  1                    16 
*  
********************************************/
static uint32_t atxx_nd_identify(struct nand_info *nd)
{  
	uint32_t io_data, value, rt = 0;
	 
	io_data = atxx_nd_read_reg(REG_NFC_SOFT_PIN2);

	printf("nand io data: 0x%02x\n", io_data);
#if defined(CONFIG_AT6600)
	if((io_data & 0xff) == 0)
		return 1;
#endif
	/*identify pagesize/blocksize.*/
	value = io_data & SIZE_BITS;
	switch (value)
	{
		case 0:
			nd->writesize = PAGE_SIZE_8K;
			nd->erasesize = 8192 * 1024;
			break;
		case 1:
			nd->writesize = PAGE_SIZE_4K;
			nd->erasesize = 256 * 1024;
			break;
		case 2:
			nd->writesize = PAGE_SIZE_4K;
			nd->erasesize = 512 * 1024;
			break;
		case 3:
			nd->writesize = PAGE_SIZE_2K;
			nd->erasesize = 128 * 1024;
			break;
		case 4:
			nd->writesize = PAGE_SIZE_4K;
			nd->erasesize = 1024 * 1024;
			break;
		case 5:
			nd->writesize = PAGE_SIZE_4K;
			nd->erasesize = 4096 * 1024;
			break;
		case 6:
			nd->writesize = PAGE_SIZE_2K;
			nd->erasesize = 512 * 1024;
			break;
		case 7:
			nd->writesize = PAGE_SIZE_8K;
			nd->erasesize = 2048 * 1024;
			break;
		case 8:
			nd->writesize = PAGE_SIZE_2K;
			nd->erasesize = 256 * 1024;
			break;
		case 9:
			nd->writesize = PAGE_SIZE_4K;
			nd->erasesize = 2048 * 1024;
			break;
		case 10:
			nd->writesize = PAGE_SIZE_2K;
			nd->erasesize = 1024 * 1024;
			break;
		case 11:
			nd->writesize = PAGE_SIZE_8K;
			nd->erasesize = 4096 * 1024;
			break;
		case 12:
			nd->writesize = PAGE_SIZE_2K;
			nd->erasesize = 64 * 1024;
			break;
		case 13:
			nd->writesize = PAGE_SIZE_8K;
			nd->erasesize = 256 * 1024;
			break;
		case 14:
			nd->writesize = PAGE_SIZE_8K;
			nd->erasesize = 512 * 1024;
			break;
		case 15:
			nd->writesize = PAGE_SIZE_8K;
			nd->erasesize = 1024 * 1024;
			break;
		default:
			break;
	}
		
	/*identify ecc and oob by nand data pin.*/
	value = (io_data & ECC_BITS) >> 4;
	switch (value)
	{
		case 0:
			ecc_number = 4;
			size_per_sector = 512;
			nd->oobsize = 16 * (nd->writesize / size_per_sector);
			break;
		case 1:
			ecc_number = 10;
			size_per_sector = 512;
			nd->oobsize = 26 * (nd->writesize / size_per_sector);
			break;
		case 2:
			ecc_number = 8;
			size_per_sector = 512;
			nd->oobsize = 22 * (nd->writesize / size_per_sector);
			break;
		case 3:
			ecc_number = 24;
			size_per_sector = 1024;
			nd->oobsize = 24 * (nd->writesize / size_per_sector);
			break;
		case 4:
			ecc_number = 15;
			size_per_sector = 1024;
			nd->oobsize = 36 * (nd->writesize / size_per_sector);
			break;
		case 5:
			ecc_number = 6;
			size_per_sector = 1024;
			nd->oobsize = 16 * (nd->writesize / size_per_sector);
			break;
		case 6:
			ecc_number = 0;
			size_per_sector = 512;
			nd->oobsize = 16 * (nd->writesize / size_per_sector);
			break;
		case 7:
			ecc_number = 1;
			size_per_sector = 1024;
			nd->oobsize = 16 * (nd->writesize / size_per_sector);
			break;
		default:
			break;
	}

	/*identify buswidth by nand data pin.*/
	value = (io_data & BUS_BITS) >> 7;
	switch (value)
	{
		case 0:
			bus_width = 0;
			break;
		case 1:
			bus_width = NAND_BUSWIDTH_16;
			break;
		default:
			break;
	}
	
	return rt;
}  


static void atxx_nd_set_eccmask(void)
{
#if defined(CONFIG_AT6600)
		switch (ecc_number) {
		case 1:
			atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0xf982);
			atxx_nd_write_reg(REG_NFC_RSD_MSK0, 0xf982);
			break;
		case 2:
			atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x77276029);
			atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0x0d);
			atxx_nd_write_reg(REG_NFC_RSD_MSK0, 0x77276029);
			atxx_nd_write_reg(REG_NFC_RSD_MSK1, 0x0d);
			break;
		case 3:
			atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x9f3da90f);
			atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0x000692d7);
			atxx_nd_write_reg(REG_NFC_RSD_MSK0, 0x9f3da90f);
			atxx_nd_write_reg(REG_NFC_RSD_MSK1, 0x000692d7);
			break;
		case 4:
			atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x10a8a7b2);
			atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0x17ec9f0c);
			atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0xbe);

			atxx_nd_write_reg(REG_NFC_RSD_MSK0, 0x10a8a7b2);
			atxx_nd_write_reg(REG_NFC_RSD_MSK1, 0x17ec9f0c);
			atxx_nd_write_reg(REG_NFC_RSD_MSK2, 0xbe);
			break;
		case 5:
			atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x2e18c4b8);
			atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0xaef5928a);
			atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0x00656b40);

			atxx_nd_write_reg(REG_NFC_RSD_MSK0, 0x2e18c4b8);
			atxx_nd_write_reg(REG_NFC_RSD_MSK1, 0xaef5928a);
			atxx_nd_write_reg(REG_NFC_RSD_MSK2, 0x00656b40);
			break;
		case 6:
			atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0xb0997909);
			atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0xec17d82e);
			atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0x792279f0);
			atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0xffffff07);

			atxx_nd_write_reg(REG_NFC_RSD_MSK0, 0xb0997909);
			atxx_nd_write_reg(REG_NFC_RSD_MSK1, 0xec17d82e);
			atxx_nd_write_reg(REG_NFC_RSD_MSK2, 0x792279f0);
			atxx_nd_write_reg(REG_NFC_RSD_MSK3, 0xffffff07);
			break;
		case 7:
			atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0xfc9fcddc);
			atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0x2897fd58);
			atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0xea9ceebc);
			atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0xf435c158);

			atxx_nd_write_reg(REG_NFC_RSD_MSK0, 0xfc9fcddc);
			atxx_nd_write_reg(REG_NFC_RSD_MSK1, 0x2897fd58);
			atxx_nd_write_reg(REG_NFC_RSD_MSK2, 0xea9ceebc);
			atxx_nd_write_reg(REG_NFC_RSD_MSK3, 0xf435c158);
			break;
		case 8:
			atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x1c9aea52);
			atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0x1f9b0e16);
			atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0x20ffc948);
			atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0xa6d7530a);
			atxx_nd_write_reg(REG_NFC_RSE_MSK4, 0xffffd0d5);

			atxx_nd_write_reg(REG_NFC_RSD_MSK0, 0x1c9aea52);
			atxx_nd_write_reg(REG_NFC_RSD_MSK1, 0x1f9b0e16);
			atxx_nd_write_reg(REG_NFC_RSD_MSK2, 0x20ffc948);
			atxx_nd_write_reg(REG_NFC_RSD_MSK3, 0xa6d7530a);
			atxx_nd_write_reg(REG_NFC_RSD_MSK4, 0xffffd0d5);
			break;
		case 9:
			atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x6cb038b8);
			atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0x293b1218);
			atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0x2996988b);
			atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0x66eef7d1);
			atxx_nd_write_reg(REG_NFC_RSE_MSK4, 0x2f5b8752);
			atxx_nd_write_reg(REG_NFC_RSE_MSK5, 0xfffffffc);
			atxx_nd_write_reg(REG_NFC_RSE_MSK6, 0xffffffff);
			atxx_nd_write_reg(REG_NFC_RSE_MSK7, 0xffffffff);

			atxx_nd_write_reg(REG_NFC_RSD_MSK0, 0x6cb038b8);
			atxx_nd_write_reg(REG_NFC_RSD_MSK1, 0x293b1218);
			atxx_nd_write_reg(REG_NFC_RSD_MSK2, 0x2996988b);
			atxx_nd_write_reg(REG_NFC_RSD_MSK3, 0x66eef7d1);
			atxx_nd_write_reg(REG_NFC_RSD_MSK4, 0x2f5b8752);
			atxx_nd_write_reg(REG_NFC_RSD_MSK5, 0xfffffffc);
			atxx_nd_write_reg(REG_NFC_RSD_MSK6, 0xffffffff);
			atxx_nd_write_reg(REG_NFC_RSD_MSK7, 0xffffffff);
			break;
		case 10:
			atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x701d2365);
			atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0xe21a68ee);
			atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0xfd727776);
			atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0x4f2a8861);
			atxx_nd_write_reg(REG_NFC_RSE_MSK4, 0x8494fd61);
			atxx_nd_write_reg(REG_NFC_RSE_MSK5, 0xfffb9532);
			atxx_nd_write_reg(REG_NFC_RSE_MSK6, 0xffffffff);
			atxx_nd_write_reg(REG_NFC_RSE_MSK7, 0xffffffff);

			atxx_nd_write_reg(REG_NFC_RSD_MSK0, 0x701d2365);
			atxx_nd_write_reg(REG_NFC_RSD_MSK1, 0xe21a68ee);
			atxx_nd_write_reg(REG_NFC_RSD_MSK2, 0xfd727776);
			atxx_nd_write_reg(REG_NFC_RSD_MSK3, 0x4f2a8861);
			atxx_nd_write_reg(REG_NFC_RSD_MSK4, 0x8494fd61);
			atxx_nd_write_reg(REG_NFC_RSD_MSK5, 0xfffb9532);
			atxx_nd_write_reg(REG_NFC_RSD_MSK6, 0xffffffff);
			atxx_nd_write_reg(REG_NFC_RSD_MSK7, 0xffffffff);
			break;
		case 11:
			atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x9c8f5ec6);
			atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0x60408888);
			atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0x7d8d244b);
			atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0x0481fcd2);
			atxx_nd_write_reg(REG_NFC_RSE_MSK4, 0x1f80061c);
			atxx_nd_write_reg(REG_NFC_RSE_MSK5, 0x03c4c03c);
			atxx_nd_write_reg(REG_NFC_RSE_MSK6, 0xffffffd7);
			atxx_nd_write_reg(REG_NFC_RSE_MSK7, 0xffffffff);

			atxx_nd_write_reg(REG_NFC_RSD_MSK0, 0x9c8f5ec6);
			atxx_nd_write_reg(REG_NFC_RSD_MSK1, 0x60408888);
			atxx_nd_write_reg(REG_NFC_RSD_MSK2, 0x7d8d244b);
			atxx_nd_write_reg(REG_NFC_RSD_MSK3, 0x0481fcd2);
			atxx_nd_write_reg(REG_NFC_RSD_MSK4, 0x1f80061c);
			atxx_nd_write_reg(REG_NFC_RSD_MSK5, 0x03c4c03c);
			atxx_nd_write_reg(REG_NFC_RSD_MSK6, 0xffffffd7);
			atxx_nd_write_reg(REG_NFC_RSD_MSK7, 0xffffffff);
			break;
		case 12:
			atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x18565d76);
			atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0x118deeed);
			atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0xaace25cc);
			atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0xefb49fe8);
			atxx_nd_write_reg(REG_NFC_RSE_MSK4, 0x03380e2e);
			atxx_nd_write_reg(REG_NFC_RSE_MSK5, 0x610e910f);
			atxx_nd_write_reg(REG_NFC_RSE_MSK6, 0xffe818b8);
			atxx_nd_write_reg(REG_NFC_RSE_MSK7, 0xffffffff);

			atxx_nd_write_reg(REG_NFC_RSD_MSK0, 0x18565d76);
			atxx_nd_write_reg(REG_NFC_RSD_MSK1, 0x118deeed);
			atxx_nd_write_reg(REG_NFC_RSD_MSK2, 0xaace25cc);
			atxx_nd_write_reg(REG_NFC_RSD_MSK3, 0xefb49fe8);
			atxx_nd_write_reg(REG_NFC_RSD_MSK4, 0x03380e2e);
			atxx_nd_write_reg(REG_NFC_RSD_MSK5, 0x610e910f);
			atxx_nd_write_reg(REG_NFC_RSD_MSK6, 0xffe818b8);
			atxx_nd_write_reg(REG_NFC_RSD_MSK7, 0xffffffff);
			break;
		case 13:
			atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x5afeedfe);
			atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0x2552f26f);
			atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0x23a3ffa6);
			atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0x3d7088c9);
			atxx_nd_write_reg(REG_NFC_RSE_MSK4, 0xc936d959);
			atxx_nd_write_reg(REG_NFC_RSE_MSK5, 0x0ef6ea69);
			atxx_nd_write_reg(REG_NFC_RSE_MSK6, 0xdb7b74f6);
			atxx_nd_write_reg(REG_NFC_RSE_MSK7, 0xfffffc68);

			atxx_nd_write_reg(REG_NFC_RSD_MSK0, 0x5afeedfe);
			atxx_nd_write_reg(REG_NFC_RSD_MSK1, 0x2552f26f);
			atxx_nd_write_reg(REG_NFC_RSD_MSK2, 0x23a3ffa6);
			atxx_nd_write_reg(REG_NFC_RSD_MSK3, 0x3d7088c9);
			atxx_nd_write_reg(REG_NFC_RSD_MSK4, 0xc936d959);
			atxx_nd_write_reg(REG_NFC_RSD_MSK5, 0x0ef6ea69);
			atxx_nd_write_reg(REG_NFC_RSD_MSK6, 0xdb7b74f6);
			atxx_nd_write_reg(REG_NFC_RSD_MSK7, 0xfffffc68);
			break;
		case 14:
			atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x6800166c);
			atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0x9a965149);
			atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0x963daf5f);
			atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0xaf0b5e2b);
			atxx_nd_write_reg(REG_NFC_RSE_MSK4, 0x646e6ec1);
			atxx_nd_write_reg(REG_NFC_RSE_MSK5, 0xfd72b90e);
			atxx_nd_write_reg(REG_NFC_RSE_MSK6, 0xa4914ecb);
			atxx_nd_write_reg(REG_NFC_RSE_MSK7, 0xfa3a75cf);

			atxx_nd_write_reg(REG_NFC_RSD_MSK0, 0x6800166c);
			atxx_nd_write_reg(REG_NFC_RSD_MSK1, 0x9a965149);
			atxx_nd_write_reg(REG_NFC_RSD_MSK2, 0x963daf5f);
			atxx_nd_write_reg(REG_NFC_RSD_MSK3, 0xaf0b5e2b);
			atxx_nd_write_reg(REG_NFC_RSD_MSK4, 0x646e6ec1);
			atxx_nd_write_reg(REG_NFC_RSD_MSK5, 0xfd72b90e);
			atxx_nd_write_reg(REG_NFC_RSD_MSK6, 0xa4914ecb);
			atxx_nd_write_reg(REG_NFC_RSD_MSK7, 0xfa3a75cf);
			break;
		case 15:
			atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x5f53e72f);
			atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0xa386af50);
			atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0x95a106d8);
			atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0x25ea175e);
			atxx_nd_write_reg(REG_NFC_RSE_MSK4, 0x2fe78812);
			atxx_nd_write_reg(REG_NFC_RSE_MSK5, 0x1b7bf2b1);
			atxx_nd_write_reg(REG_NFC_RSE_MSK6, 0x0d5cbb45);
			atxx_nd_write_reg(REG_NFC_RSE_MSK7, 0x88a37868);
			atxx_nd_write_reg(REG_NFC_RSE_MSK8, 0xffffdbd0);

			atxx_nd_write_reg(REG_NFC_RSD_MSK0, 0x5f53e72f);
			atxx_nd_write_reg(REG_NFC_RSD_MSK1, 0xa386af50);
			atxx_nd_write_reg(REG_NFC_RSD_MSK2, 0x95a106d8);
			atxx_nd_write_reg(REG_NFC_RSD_MSK3, 0x25ea175e);
			atxx_nd_write_reg(REG_NFC_RSD_MSK4, 0x2fe78812);
			atxx_nd_write_reg(REG_NFC_RSD_MSK5, 0x1b7bf2b1);
			atxx_nd_write_reg(REG_NFC_RSD_MSK6, 0x0d5cbb45);
			atxx_nd_write_reg(REG_NFC_RSD_MSK7, 0x88a37868);
			atxx_nd_write_reg(REG_NFC_RSD_MSK8, 0xffffdbd0);
			break;
		default:
			printf(
			 "ATXX NFC does not support this ecc:%d\n",
					 ecc_number);
		break;
		}
#else
	if(size_per_sector == 512) {
	switch (ecc_number) {
	case 4:
		atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x731A62BB);
		atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0x28C952BB);
		atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0x0000F768);
	break;
	case 8:
		atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x90D35B12);
		atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0x3A2164E8);
		atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0x4BEC2ACF);
		atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0x6641028A);
		atxx_nd_write_reg(REG_NFC_RSE_MSK4, 0xBBDE209A);
	break;
	case 10:
		atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x40B8ADB7);
		atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0xC7071A00);
		atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0x8CA0D551);
		atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0x9ADD989A);
		atxx_nd_write_reg(REG_NFC_RSE_MSK4, 0x0B3A4270);
		atxx_nd_write_reg(REG_NFC_RSE_MSK5, 0xF624A164);
		atxx_nd_write_reg(REG_NFC_RSE_MSK6, 0x000000CD);
		break;
	case 12:
		atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0xC3AC74BE);
		atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0xA4FA44BA);
		atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0x8E0D5317);
		atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0xFC9AF48F);
		atxx_nd_write_reg(REG_NFC_RSE_MSK4, 0x932BEE9B);
		atxx_nd_write_reg(REG_NFC_RSE_MSK5, 0x7829A12B);
		atxx_nd_write_reg(REG_NFC_RSE_MSK6, 0x51101FF7);
		atxx_nd_write_reg(REG_NFC_RSE_MSK7, 0x00005279);
		break;
	default:
		printf(
		 "ATXX NFC does not support this ecc:%d\n",
				 ecc_number);
		break;
		}
	} else {
	switch (ecc_number) {
	case 12:
		atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0xBB55817D);
		atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0x96652FD9);
		atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0x9C0AB380);
		atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0x1C183452);
		atxx_nd_write_reg(REG_NFC_RSE_MSK4, 0x5C812C35);
		atxx_nd_write_reg(REG_NFC_RSE_MSK5, 0xF05A99D1);
		atxx_nd_write_reg(REG_NFC_RSE_MSK6, 0x74BC85C9);
		atxx_nd_write_reg(REG_NFC_RSE_MSK7, 0x00000CB7);
		break;
	case 20:
		atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x14D09A6A);
		atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0xC4CD9056);
		atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0xD4E75487);
		atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0x1FC38B7E);
		atxx_nd_write_reg(REG_NFC_RSE_MSK4, 0xA5D61845);
		atxx_nd_write_reg(REG_NFC_RSE_MSK5, 0xA258FAB3);
		atxx_nd_write_reg(REG_NFC_RSE_MSK6, 0x758DC42D);
		atxx_nd_write_reg(REG_NFC_RSE_MSK7, 0x559D44E5);
		atxx_nd_write_reg(REG_NFC_RSE_MSK8, 0xCFF0957A);
		atxx_nd_write_reg(REG_NFC_RSE_MSK9, 0xCE5A9E67);
		atxx_nd_write_reg(REG_NFC_RSE_MSK10,0xA13ED6B3);
		atxx_nd_write_reg(REG_NFC_RSE_MSK11,0xB94E8815);
		atxx_nd_write_reg(REG_NFC_RSE_MSK12,0xFFFF47B0);
		break;
	case 22:
		atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0xC9D1A6E1);
		atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0xDE8CBDF1);
		atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0xCC4A362A);
		atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0xFCCBA97C);
		atxx_nd_write_reg(REG_NFC_RSE_MSK4, 0xE5107C7C);
		atxx_nd_write_reg(REG_NFC_RSE_MSK5, 0x27453FC5);
		atxx_nd_write_reg(REG_NFC_RSE_MSK6, 0x7C254C32);
		atxx_nd_write_reg(REG_NFC_RSE_MSK7, 0x41D956AF);
		atxx_nd_write_reg(REG_NFC_RSE_MSK8, 0xF6E6DA51);
		atxx_nd_write_reg(REG_NFC_RSE_MSK9, 0xE4CEB35E);
		atxx_nd_write_reg(REG_NFC_RSE_MSK10,0xC731FF3A);
		atxx_nd_write_reg(REG_NFC_RSE_MSK11,0x953E282F);
		atxx_nd_write_reg(REG_NFC_RSE_MSK12,0xB79CBA54);
		atxx_nd_write_reg(REG_NFC_RSE_MSK13,0xFF462220);
		atxx_nd_write_reg(REG_NFC_RSE_MSK14,0xFFFFFFFF);
		break;
	case 24:
		atxx_nd_write_reg(REG_NFC_RSE_MSK0, 0x3FCE9F6B);
		atxx_nd_write_reg(REG_NFC_RSE_MSK1, 0x6BE36897);
		atxx_nd_write_reg(REG_NFC_RSE_MSK2, 0x3030B956);
		atxx_nd_write_reg(REG_NFC_RSE_MSK3, 0x3CFE3C1C);
		atxx_nd_write_reg(REG_NFC_RSE_MSK4, 0x2982FA4F);
		atxx_nd_write_reg(REG_NFC_RSE_MSK5, 0xAB02BD6D);
		atxx_nd_write_reg(REG_NFC_RSE_MSK6, 0xCA9A8C9E);
		atxx_nd_write_reg(REG_NFC_RSE_MSK7, 0xDD80D630);
		atxx_nd_write_reg(REG_NFC_RSE_MSK8, 0x969DB66A);
		atxx_nd_write_reg(REG_NFC_RSE_MSK9, 0x30F88E60);
		atxx_nd_write_reg(REG_NFC_RSE_MSK10,0xFBFC8B28);
		atxx_nd_write_reg(REG_NFC_RSE_MSK11,0x94B86945);
		atxx_nd_write_reg(REG_NFC_RSE_MSK12,0x795F25E0);
		atxx_nd_write_reg(REG_NFC_RSE_MSK13,0xAE1CBC27);
		atxx_nd_write_reg(REG_NFC_RSE_MSK14,0x5648E21E);
		break;
	default:
		printf(
		 "ATXX NFC does not support this ecc:%d\n",
				 ecc_number);
		break;
		}
	}

	atxx_nd_write_reg(REG_NFC_RSD_MSK0, atxx_nd_read_reg(REG_NFC_RSE_MSK0));
	atxx_nd_write_reg(REG_NFC_RSD_MSK1, atxx_nd_read_reg(REG_NFC_RSE_MSK1));
	atxx_nd_write_reg(REG_NFC_RSD_MSK2, atxx_nd_read_reg(REG_NFC_RSE_MSK2));
	atxx_nd_write_reg(REG_NFC_RSD_MSK3, atxx_nd_read_reg(REG_NFC_RSE_MSK3));
	atxx_nd_write_reg(REG_NFC_RSD_MSK4, atxx_nd_read_reg(REG_NFC_RSE_MSK4));
	atxx_nd_write_reg(REG_NFC_RSD_MSK5, atxx_nd_read_reg(REG_NFC_RSE_MSK5));
	atxx_nd_write_reg(REG_NFC_RSD_MSK6, atxx_nd_read_reg(REG_NFC_RSE_MSK6));
	atxx_nd_write_reg(REG_NFC_RSD_MSK7, atxx_nd_read_reg(REG_NFC_RSE_MSK7));
	atxx_nd_write_reg(REG_NFC_RSD_MSK8, atxx_nd_read_reg(REG_NFC_RSE_MSK8));
	atxx_nd_write_reg(REG_NFC_RSD_MSK9, atxx_nd_read_reg(REG_NFC_RSE_MSK9));
	atxx_nd_write_reg(REG_NFC_RSD_MSK10,atxx_nd_read_reg(REG_NFC_RSE_MSK10));
	atxx_nd_write_reg(REG_NFC_RSD_MSK11,atxx_nd_read_reg(REG_NFC_RSE_MSK11));
	atxx_nd_write_reg(REG_NFC_RSD_MSK12,atxx_nd_read_reg(REG_NFC_RSE_MSK12));
	atxx_nd_write_reg(REG_NFC_RSD_MSK13,atxx_nd_read_reg(REG_NFC_RSE_MSK13));
	atxx_nd_write_reg(REG_NFC_RSD_MSK14,atxx_nd_read_reg(REG_NFC_RSE_MSK14));
#endif

}


/**
 * atxx_nd_select_chip - control -CE line
 * @nd:	MTD device structure
 * @chip:	chipnumber to select, -1 for deselect
 */
static void atxx_nd_select_chip(struct nand_info *nd, int chip)
{
	int para0;

	switch (chip) {
	case 0:
		/* select bank 0 chip */
		para0 = atxx_nd_read_reg(REG_NFC_PARA0);
		para0 &= ~(NFC_PARA0_BANK_CE1 |
			       NFC_PARA0_BANK_CE2 | NFC_PARA0_BANK_CE3);
		para0 |= NFC_PARA0_BANK_CE0;
		atxx_nd_write_reg(REG_NFC_PARA0, para0);
		break;
	case 1:
		/* select bank 1 chip */
		para0 = atxx_nd_read_reg(REG_NFC_PARA0);
		para0 &= ~(NFC_PARA0_BANK_CE0 |
			       NFC_PARA0_BANK_CE2 | NFC_PARA0_BANK_CE3);
		para0 |= NFC_PARA0_BANK_CE1;
		atxx_nd_write_reg(REG_NFC_PARA0, para0);
		break;
	case 2:
		/* select bank 2 chip */
		para0 = atxx_nd_read_reg(REG_NFC_PARA0);
		para0 &= ~(NFC_PARA0_BANK_CE0 |
			       NFC_PARA0_BANK_CE1 | NFC_PARA0_BANK_CE3);
		para0 |= NFC_PARA0_BANK_CE2;
		atxx_nd_write_reg(REG_NFC_PARA0, para0);
		break;
	case 3:
		/* select bank 3 chip */
		para0 = atxx_nd_read_reg(REG_NFC_PARA0);
		para0 &= ~(NFC_PARA0_BANK_CE0 | NFC_PARA0_BANK_CE1 |
			       NFC_PARA0_BANK_CE2);
		para0 |= NFC_PARA0_BANK_CE3;
		atxx_nd_write_reg(REG_NFC_PARA0, para0);
		break;
	case -1:
		/* de-select all */
		para0 = atxx_nd_read_reg(REG_NFC_PARA0);
		para0 &= ~(NFC_PARA0_BANK_CE0 | NFC_PARA0_BANK_CE1
			       | NFC_PARA0_BANK_CE2 | NFC_PARA0_BANK_CE3);
		atxx_nd_write_reg(REG_NFC_PARA0, para0);
		break;
	}
}

static  void nfc_send_readid_cmd(void)
{
	uint32_t cmd;
	atxx_nd_write_reg(REG_NFC_FADDR0, ID_DATA_ADDR);
	cmd = (NFC_CMD_READ_ID << NFC_CMD_CMD_SHIFT) |
		(4 << NFC_CMD_DATA_LEN_SHIFT) |
		(1 << NFC_CMD_ADDR_LEN_SHIFT) | NFC_CMD_ENABLE;
	atxx_nd_write_reg(REG_NFC_CMD, cmd);
	WAIT_CMD_END();
}

/***********************************************************
*   Nand initialize operation
***********************************************************/
void atxx_nd_read_ids(int * first_id_byte,
		int * second_id_byte, int * fourth_id_byte)
{
	uint32_t *p = (uint32_t *) ((uint8_t *) NFC_READ_BUF + ID_DATA_ADDR);

	nfc_send_readid_cmd();

	/* we are interested in 1st, 2nd and 4th id bytes */
	*first_id_byte = *p & 0xff;
	*second_id_byte = (*p >> 8) & 0xff;
	*fourth_id_byte = (*p >> 24) & 0xff;

	atxx_nd_debug("ID:%08x\n", *p);
}


/*
 * Get the flash and manufacturer id and lookup if the type is supported
 */
static struct nand_flash_dev * atxx_nd_get_flash_type(
			struct nand_info *nd, int *maf_id)
{
	struct nand_flash_dev *type = NULL;
	int i, dev_id, maf_idx, ext_id, ext_id_bak;
	uint32_t rt;


	/* Select the device */
	atxx_nd_select_chip(nd, 0);
	atxx_nd_read_ids(maf_id, &dev_id, &ext_id);
	ext_id_bak = ext_id;

	/* Lookup the flash id */
	for (i = 0; nand_flash_ids[i].name != NULL; i++) {
		if (dev_id == nand_flash_ids[i].id) {
			type = &nand_flash_ids[i];
			break;
		}
	}

	if (!type)
		return ERR_PTR(-ENODEV);


	nd->chipsize = type->chipsize << 20;
	/* Newer devices have all the information in additional id bytes */
	if (!type->pagesize) {
		nd->writesize = 1024 << (ext_id & 0x3);
		ext_id >>= 2;
		/* Calc oobsize */
		nd->oobsize = (8 << (ext_id & 0x01)) * (nd->writesize >> 9);
		ext_id >>= 2;
		/* Calc blocksize. Blocksize is multiples of 64KiB */
		nd->erasesize = (64 * 1024) << (ext_id & 0x03);
		ext_id >>= 2;
		/* Get buswidth information */
		bus_width = (ext_id & 0x01) ? NAND_BUSWIDTH_16 : 0;

	} else {
		/*
		 * Old devices have chip data hardcoded in the device id table
		 */
		nd->erasesize = type->erasesize;
		nd->writesize = type->pagesize;
		nd->oobsize = nd->writesize / 32;
		bus_width = type->options & NAND_BUSWIDTH_16;
	}

	/* Try to identify manufacturer */
	for (maf_idx = 0; nand_manuf_ids[maf_idx].id != 0x0; maf_idx++) {
		if (nand_manuf_ids[maf_idx].id == *maf_id)
			break;
	}

	/*
	* we have used nand data pin to identify nand
	* because lots of new flash do not following the ID format.
	*/
	rt = atxx_nd_identify(nd);
	if(rt == 1) {
#if defined(CONFIG_AT6600)
		/* fix the nand ID identify for Hynix H27UAG8T2A */
		if ((*maf_id == 0xad) && (dev_id == 0xd5) && (ext_id_bak == 0x25)) {
			printf("nand fix.\n");
			nd->erasesize = 512 * 1024;
			nd->writesize = PAGE_SIZE_4K;
			ecc_number = 11;
			oob_size = 224;
		}


		if (nd->writesize == PAGE_SIZE_4K)
			nd->oobsize = oob_size;
#else
		return ERR_PTR(-ENODEV);
#endif
	}
	/* Set the bad block position */
	nd->badblockpos = nd->writesize > 512 ?
	    NAND_LARGE_BADBLOCK_POS : NAND_SMALL_BADBLOCK_POS;
	nd->chip_shift = ffs(nd->chipsize) - 1;
	/* Calculate the address shift from the page size */
	nd->page_shift = ffs(nd->writesize) - 1;
	/* Convert chipsize to number of pages per chip -1. */
	nd->pagemask = (nd->chipsize >> nd->page_shift) - 1;

	printf("NAND device: Manufacturer ID:"
		   " 0x%02x, Chip ID: 0x%02x (%s %s %d %d %d)\n", *maf_id, dev_id,
		   nand_manuf_ids[maf_idx].name, type->name, nd->writesize, 
		   nd->erasesize, nd->oobsize);

	return type;
}

/**
 * atxx_nd_scan - [NAND Interface] Scan for the NAND device
 * @mtd:	MTD device structure
 * @maxchips:	Number of chips to scan for
 *
 * This fills out all the uninitialized function pointers
 * with the defaults.
 * The flash ID is read and the mtd/chip structures are
 * filled with the appropriate values.
 * The mtd->owner field must be set to the module of the caller
 *
 */
int atxx_nd_scan(struct nand_info *nd, int maxchips)
{
	uint32_t reg_data;
	struct nand_flash_dev *type;
	int i, maf_id;
	uint32_t val;

	/* Read the flash type */
	type = atxx_nd_get_flash_type(nd, &maf_id);
	if (IS_ERR(type)) {
		printf("No NAND device found!!!\n");
		atxx_nd_select_chip(nd, -1);
		return PTR_ERR(type);
	}

	/* Check for a chip array */
	for (i = 1; i < maxchips; i++) {
		int first_id, second_id, fourth_id;
		atxx_nd_select_chip(nd, i);
		atxx_nd_read_ids(&first_id, &second_id, &fourth_id);

		/* Read manufacturer and device IDs */
		if (maf_id != first_id || type->id != second_id)
			break;
	}

	nd->chip_num = i;

	/* Store the number of chips and calc total size for nand */
	nd->size = nd->chip_num * nd->chipsize;
	/*
	 * Setting ecc structure and layout
	 */
	atxx_nd_write_reg(REG_NFC_ECC_CFG, NFC_ECC_EN_RESET | 
				NFC_ECC_DE_RESET | ecc_number);

	nd->ecc.size = size_per_sector;
	if(ecc_number){
#if	defined(CONFIG_AT6600)
		/* (2*(ecc_number * 9bit)/8) byte + 1byte/0byte */
		nd->ecc.bytes = ((ecc_number * 9) + 3)/4;
#else
		nd->ecc.bytes = ((ecc_number * 10) + 3)/4;
#endif
	}
	else {
		nd->ecc.bytes = 0x08;
	}
	nd->ecc.steps = nd->writesize / nd->ecc.size;
	nd->ecc.total = nd->ecc.steps * nd->ecc.bytes;

	/*we need 1 bytes for badblock in the beginning of oob*/
	if ((nd->ecc.bytes + 1) > nd->oobsize/nd->ecc.steps) {
		printf("Not enough oob space for ecc!!!\n");
		atxx_nd_select_chip(nd, -1);
		return -ENOSPC;
	}

	reg_data = 0xffff0000 | ((nd->oobsize/nd->ecc.steps - 1) << 8) 
			| (nd->oobsize/nd->ecc.steps - nd->ecc.bytes);
	atxx_nd_write_reg(REG_NFC_ECC_CBC1, reg_data);
	atxx_nd_write_reg(REG_NFC_ECC_CBC2, 0xffffffff);

	atxx_nd_set_eccmask();

	/* setup page size */
	switch (nd->writesize) {
	case 8192:
		reg_data = atxx_nd_read_reg(REG_NFC_PARA0);
		reg_data &= ~(0xf << NFC_PARA0_PAGE_SIZE_SHIFT);
		reg_data |= (3 << NFC_PARA0_PAGE_SIZE_SHIFT);
		reg_data &= ~(1 << 16);
		atxx_nd_write_reg(REG_NFC_PARA0, reg_data);
		break;
	case 4096:
		reg_data = atxx_nd_read_reg(REG_NFC_PARA0);
		reg_data &= ~(0xf << NFC_PARA0_PAGE_SIZE_SHIFT);
		reg_data |= (2 << NFC_PARA0_PAGE_SIZE_SHIFT);
		reg_data &= ~(1 << 16);
		atxx_nd_write_reg(REG_NFC_PARA0, reg_data);
		break;
	case 2048:
		reg_data = atxx_nd_read_reg(REG_NFC_PARA0);
		reg_data &= ~(0xf << NFC_PARA0_PAGE_SIZE_SHIFT);
		reg_data |= (0x01 << NFC_PARA0_PAGE_SIZE_SHIFT);
		reg_data &= ~(1 << 16);
		atxx_nd_write_reg(REG_NFC_PARA0, reg_data);
		break;
	case 512:
		reg_data = atxx_nd_read_reg(REG_NFC_PARA0);
		reg_data &= ~(0xf << NFC_PARA0_PAGE_SIZE_SHIFT);
		reg_data |= (1 << 16);
		reg_data &= ~0xF0;
		atxx_nd_write_reg(REG_NFC_PARA0, reg_data);
	default:
		break;
	}

	if (size_per_sector == 1024)
		atxx_nd_write_reg(REG_NFC_PARA0, atxx_nd_read_reg(REG_NFC_PARA0) | 0x80);

	if (bus_width == NAND_BUSWIDTH_16) {
		reg_data = atxx_nd_read_reg(REG_NFC_PARA0);
		reg_data |= NFC_PARA0_DEVICE_BUS;
		atxx_nd_write_reg(REG_NFC_PARA0, reg_data);
	}

    /*oob check baseaddr should set to pagesize only to make sure there is enough space.*/
	if ((nd->oobsize / nd->ecc.steps) > 32)
		reg_data = nd->writesize | (0x40 << 16);
	else
		reg_data = (nd->writesize + nd->oobsize)
	    	| ((nd->oobsize / nd->ecc.steps) << 16);
	atxx_nd_write_reg(REG_NFC_PARA1, reg_data);

	/* De-select the device */
	atxx_nd_select_chip(nd, -1);
	/* Set the internal oob buffer location, just after the page data */
	nd->oob_poi = nd->databuf + nd->writesize;

	return 0;
}

/* nand depend on app clock , so if app clock is changed, 
 *  you should run nand initial again.
 */
int nand_init(void)
{
	uint32_t reg_data;

	NFC_RESET();
	
	reg_data = atxx_nd_read_reg(REG_NFC_PARA0);
	reg_data &= ~(NFC_PARA0_WP | NFC_PARAR0_MANUAL_CE | NFC_PARA0_DEVICE_BUS);
	atxx_nd_write_reg(REG_NFC_PARA0, reg_data);

	/* timing params according to clk_app */
	atxx_nd_set_timing();

	/* setup fifo low/high level */
	reg_data = NFC_FIFO_LOW_LEV | (NFC_FIFO_HIGH_LEV <<
					   NFC_FIFO_CFG_HIGH_SHIFT);
	atxx_nd_write_reg(REG_NFC_FIFO_CFG, reg_data);

	/* disable all interrupts */
	reg_data = atxx_nd_read_reg(REG_NFC_INT_MASK);
	reg_data &= ~(NFC_INT_FIFO_HIGH_MASK | NFC_INT_FIFO_LOW_MASK
			  | NFC_INT_CMD_MASK | NFC_INT_ECCE_RDY_MASK
			  | NFC_INT_ECCD_RDY_MASK|NFC_INT_DMA_FINI);
	atxx_nd_write_reg(REG_NFC_INT_MASK, reg_data);

	/* set DMA burst_size to 8 */
	atxx_nd_write_reg(REG_NFC_DMA_CFG, 8);

	/* reset nand flash */
	nfc_send_reset_cmd();
	mdelay(100);		/*reset time, max 100ms */

	atxx_nd_scan(&nd, 4);

	return 0;
}


/**
 * atxx_nd_read_page_hwecc - [Intern] atxx page read function(hardware ecc)
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @buf:	data buffer
 */
static int atxx_nd_read_page_hwecc(struct nand_info *nd,
			uint8_t * buf, int page)
{
	int addr_len, status1, status2;
#ifdef CONFIG_ATXX_NAND_DEBUG
	uint8_t *oob;
	int i;
#endif

	atxx_nd_debug("atxx_nd_read_page_hwecc - page:%08x,buf:%08x\n", 
				page, (int)buf);

	if (nd->writesize <= 512) {	/* small page */
		nfc_config_small_page_read(bus_width, page);
		if (nd->chipsize > (32 << 20)) {
			addr_len = 4;
		} else {
			addr_len = 3;
		}

		if (bus_width == NAND_BUSWIDTH_16) {
			nfc_send_readoob_udf_cmd(nd->oobsize / 2, addr_len);
		} else {
			nfc_send_readoob_udf_cmd(nd->oobsize, addr_len);
		}

		if (bus_width == NAND_BUSWIDTH_16) {
			nfc_send_readpage_ecc_cmd(nd->writesize / 2, addr_len);
		} else {
			nfc_send_readpage_ecc_cmd(nd->writesize, addr_len);
		}

		/* ecc status */
		status1 = atxx_nd_read_reg(REG_NFC_ECCSTATUS1);
		if (status1 & NFC_ECC_UNCORRECT1){
			atxx_nd_err("Nand ecc error, page:%08x, ecc status1:%08x\n",
					page, status1);
			goto err;
		}

		/* for verify, we can get data in readbuf, (buf=null) */
		if (buf) {
#ifndef CONFIG_ATXX_NAND_DMA
			memcpy((uint8_t *) buf, (uint8_t *) NFC_READ_BUF,
					nd->writesize);
#else
			atxx_nd_dma_read(buf, 0, nd->writesize);
#endif
		}
	} else {
		/* clear RSD ready for each code */
		atxx_nd_write_reg(REG_NFC_INT_CODE, 0xff00);
		nfc_config_big_page(bus_width, nd->writesize | (page << 16));
		if (nd->chipsize > (128 << 20)) {
			addr_len = 5;
			atxx_nd_write_reg(REG_NFC_FADDR1, page >> 16);
		} else {
			addr_len = 4;
		}
		/* read oob */
		if (bus_width == NAND_BUSWIDTH_16) {
			nfc_send_readoob_cmd(nd->oobsize / 2, addr_len);
		} else {
			nfc_send_readoob_cmd(nd->oobsize, addr_len);
		}
#ifdef CONFIG_ATXX_NAND_DEBUG	
		oob = (uint8_t *) NFC_READ_BUF + nd->writesize;
		atxx_nd_debug("page:%08x, oob data:", page);

		for (i = 0; i < nd->oobsize; i++) {
			printf("%02x ",oob[i]);
		}
		printf("\n");
#endif
		/* read data, like 2048 bytes */
		/* Write UDFA cmd1 = 0x05, UDFA cmd2 = 0xE0 */
		if (bus_width == NAND_BUSWIDTH_16) {
			nfc_send_readpage_udf_cmd(nd->writesize / 2, 2, 1);
		} else {
			nfc_send_readpage_udf_cmd(nd->writesize, 2, 1);
		}
#ifdef CONFIG_ATXX_NAND_DEBUG		
		ATXX_ND_LOG();
#endif
		if (buf) {
#ifndef CONFIG_ATXX_NAND_DMA
			WAIT_CMD_END();
			memcpy((uint8_t *) buf, (uint8_t *) NFC_READ_BUF,
					nd->writesize);
#else
			/*using code word check only for 4k pagesize 
			  to improve read speed */
			if (nd->writesize == 4096) {
				WAIT_DECODE_CODE_READY(4);
				atxx_nd_dma_read(buf, 0, 0x800);

				WAIT_CMD_END();
				atxx_nd_dma_read(buf + 0x800, 0x800,
						0x800);

			} else {
				WAIT_CMD_END();
				atxx_nd_dma_read(buf, 0, nd->writesize);
			}
#endif
		} else {
			WAIT_CMD_END();
		}
	/* change to uboot */
	status1 = atxx_nd_read_reg(REG_NFC_ECCSTATUS1);
	status2 = atxx_nd_read_reg(REG_NFC_ECCSTATUS2);
	if (nd->writesize == 4096) {	
		/* ecc status */
		if ((status1 & (NFC_ECC_UNCORRECT1 | NFC_ECC_UNCORRECT2 |
				NFC_ECC_UNCORRECT3 | NFC_ECC_UNCORRECT4)) |
			(status2 & (NFC_ECC_UNCORRECT1 | NFC_ECC_UNCORRECT2 |
				    NFC_ECC_UNCORRECT3 | NFC_ECC_UNCORRECT4))) {
			atxx_nd_err("Nand ecc error, page:%08x, ecc status1:%08x"
					"ecc status2:%08x\n",
					page, status1, status2);
			goto err;
		}
	} else if (nd->writesize == 2048) {	
		/* ecc status */
		if (status1 & (NFC_ECC_UNCORRECT1 | NFC_ECC_UNCORRECT2 |
				NFC_ECC_UNCORRECT3 | NFC_ECC_UNCORRECT4)) {
			atxx_nd_err("Nand ecc error, page:%08x, ecc status:%08x\n",
					page, status1);
			goto err;
		}
	} else {
		if (status1 & NFC_ECC_UNCORRECT1) {
			atxx_nd_err("Nand ecc error, page:%08x, ecc status:%08x\n",
					page, status1);
			goto err;
		} 
	}

	}
#ifdef CONFIG_ATXX_NAND_DEBUG
	for (i = 0; i < nd->writesize; i ++) {
		printf("%02x ",buf[i]);
	}
	ATXX_ND_LOG();
#endif
	return 0;
err:
	return -EBADMSG;
}

/***********************************************************
*   Nand read operation
***********************************************************/
/**
 * atxx_nd_read_oob_std - [REPLACABLE] the most common OOB data read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:	page number to read
 * @sndcmd:	flag whether to issue read command or not
 */
static int atxx_nd_read_oob_std(struct nand_info *nd, int page, int sndcmd)
{
	int addr_len;

	//atxx_nd_debug("atxx_nd_read_oob_std - page:%08x\n", page);
	if (nd->writesize <= 512) {	/* small page */
		nfc_config_small_page_read(bus_width, page);
		if (nd->chipsize > (32 << 20)) {
			addr_len = 4;
		} else {
			addr_len = 3;
		}
		if (bus_width == NAND_BUSWIDTH_16) {
			nfc_send_readoob_udf_cmd(nd->oobsize / 2, addr_len);
		} else {
			nfc_send_readoob_udf_cmd(nd->oobsize, addr_len);
		}
	} else {
		/* Write NFC_FADDR0, NFC_FADDR1 with desired page address */
		nfc_config_big_page(bus_width, nd->writesize | (page << 16));

		if (nd->chipsize > (128 << 20)) {
			addr_len = 5;
			atxx_nd_write_reg(REG_NFC_FADDR1, page >> 16);
		} else {
			addr_len = 4;
		}
		/* read oob */
		if (bus_width == NAND_BUSWIDTH_16) {
			nfc_send_readoob_cmd(nd->oobsize / 2, addr_len);
		} else {
			nfc_send_readoob_cmd(nd->oobsize, addr_len);
		}
	}

	memcpy((void *)nd->oob_poi,
			(void *)(NFC_READ_BUF_ADDR + nd->writesize),
			nd->oobsize);

	return 0;
}


/**
 * nand_block_bad - [DEFAULT] Read bad block marker from the chip
 * @mtd:	MTD device structure
 * @ofs:	offset from device start
 * @getchip:	0, if the chip is already selected
 *
 * Check, if the block is bad.
 */
static int nand_block_bad(struct nand_info *nd, size_t ofs)
{
	int page, res = 0;
	int i;

	page = (int)(ofs >> nd->page_shift) & nd->pagemask;

	/* read oob and compare with 0xff */
	for (i = 0; i < 2; i++) {
		atxx_nd_read_oob_std(nd, page + i, 0);
		if ((nd->oob_poi[nd->badblockpos] & 0xff) != 0xff) {
			res = 1;
			break;
		}
	}

	return res;
}

/**
 * nand_block_isbad - [MTD Interface] Check if block at offset is bad
 * @mtd:	MTD device structure
 * @offs:	offset relative to mtd start, page alignment
 */
static int nand_block_isbad(struct nand_info *nd, size_t offs)
{
	int chipnr;

	/* Check for invalid offset */
	if (offs > nd->size)
		return -EINVAL;
	
	chipnr = (int)(offs >> nd->chip_shift);
	if (nd->chip_num <= chipnr)
		return -EINVAL;
	atxx_nd_select_chip(nd, chipnr);
	return nand_block_bad(nd, offs);
}

/**
 * get_len_incl_bad
 *
 * Check if length including bad blocks fits into device.
 *
 * @param nand NAND device
 * @param offset offset in flash
 * @return image length including bad blocks
 */
static int find_first_block (struct nand_info *nd, size_t *offset)
{

	do {
		if (nand_block_isbad (nd, *offset & ~(nd->erasesize - 1))){
			*offset += nd->erasesize;
		}else
			break;

		if (*offset >= nd->size) {
			printf("Not available nand space\n");
			return -ENOSPC ;
		}
	} while(1);
	
	return 0;
}

/**
 * nand_read_skip_bad:
 *
 * Read image from NAND flash.
 * Blocks that are marked bad are skipped and the next block is readen
 * instead as long as the image is short enough to fit even after skipping the
 * bad blocks.
 *
 * @param nand NAND device
 * @param offset offset in flash
 * @param length buffer length, on return holds remaining bytes to read
 * @param buffer buffer to write to
 * @return 0 in case of success
 */
int nand_read_skip_bad(struct nand_info *nd, size_t offset, size_t length,
		       u_char *buffer, size_t end)
{
	int rval;
	size_t left_to_read = length;
	size_t addr = offset, page_addr, align, step;
	u_char *p_buffer = buffer;

	do {
		rval = find_first_block(nd, &addr);
		if (rval < 0) {
			printf("Nand get offset fail\n");
			goto err;
		}
		if (addr >= end) {
			printf("Too much bad block, out %x\n", end);
			goto err;
		}
		do {

			align = (nd->writesize - 1);
			page_addr = addr - (addr & align);
			
			if ((page_addr != addr) || (left_to_read < nd->writesize)) {
				rval = atxx_nd_read_page_hwecc(nd, nd->databuf, 
					(page_addr >> nd->page_shift));
				step = min(left_to_read, nd->writesize - (addr & align));
				memcpy(p_buffer, nd->databuf + (addr & align), step);
				p_buffer += step;
				left_to_read -= step;
			}
			else {
				step = nd->writesize;
				rval = atxx_nd_read_page_hwecc(nd, p_buffer, (page_addr >> nd->page_shift));
				p_buffer += step;
				left_to_read -= step;
			}
#ifdef CONFIG_ATXX_NAND_DEBUG	
				int i;
				for (i = 0; i < step; i ++) {
					printf("%02x ",(buffer)[i]);
				}
				printf("\n");
#endif
			if (rval){
				printf("Nand read error\n");
				goto err;
			}
			addr = page_addr + nd->writesize;
			if ((addr & ~(nd->erasesize - 1))
				!= (page_addr & ~(nd->erasesize - 1)))
				break;
		}while (left_to_read);
	}while (left_to_read);
	
	return 0;
err:
	printf ("NAND read from offset %x failed %d\n",
		offset, rval);
	return rval;

}

