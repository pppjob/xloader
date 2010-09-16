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
