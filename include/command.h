#ifndef _CMD_DISPATCH_H
#define _CMD_DISPATCH_H

#include <list.h>

#define for_each_command(cmd) list_for_each_entry(cmd, &cmd_list, list)

struct command {
	const char *name;
	struct list_head list;
	int cmd_number;
	int (*cmd_handler)(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen);
};

int register_command(struct command *cmd);
int process_command(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen);

#endif
