#ifndef _CMD_DISPATCH_H
#define _CMD_DISPATCH_H

#include <list.h>

struct command {
	const char *name;
	struct list_head list;
	int cmd_number;
	int (*cmd_handler)(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen);
};

int prepare_answer(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen);

#endif
