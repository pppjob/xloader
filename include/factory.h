#ifndef _FACTORY_H_
#define _FACTORY_H_

typedef struct mddr_f_data {
	uint8_t mddr_size;
	uint8_t mddr_cal_data[8];
} mddr_f_data_t;

enum factory_data_index {
	FD_MDDR = 0,
	FD_BATTERY,
	FD_RTP,
	FD_LSENSOR,
	FD_BLUETOOTH,
	FD_WIFI,
	FD_GSM,
	FD_MENU_CONSOLE,
	FD_SN,
	FD_IMEI,
	FD_MASK,
	FD_ETHERNET,
	FD_CAMERA,
	FD_GSM_0,
	FD_GSM_1,
	FD_GSM_2,
	FD_GSM_3,
	FD_GSM_4,
	FD_GSM_5,
	FD_GSM_6,
	FD_GSM_7,
	FD_GSM_8,
	FD_GSM_9,
	FD_GSM_10,
	FD_GSM_11,
	FD_GSM_12,
	FD_GSM_13,
	FD_GSM_14,
	FD_GSM_15,
	FD_CONFIG,
	FD_MAX
};

typedef	struct factory_data {
	int	fd_index;	/* factory data index */
	int	fd_length;	/* data length */
	char fd_buf[4064];	/* customized data area, if we use 8k pagesize nand, we need consider it! */
} factory_data_t;

struct factory_data_page {
	char		magic[MAGIC_LENGTH];	/* magic string */
	int		flag;			/* preserved */
	uint32_t	padding;
	factory_data_t	fd_user;		/* factory data */
};

int factory_init(void* buf);
factory_data_t *factory_data_get(int fd_index);

#endif /* _FACTORY_H_ */
