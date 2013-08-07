#ifndef _CMD_DISPATCH_H
#define _CMD_DISPATCH_H

#include <list.h>

#define for_each_command(cmd) list_for_each_entry(cmd, &cmd_list, list)

#define SSIF_HEADER	0
#define CMD		1
#define COMPLETION	2

#define RESP_NETFFN_LUN(netfn_lun)	(netfn_lun | (1 << 2))

struct command {
	const char *name;
	struct list_head list;
	int cmd_number;
	int (*cmd_handler)(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen);
};

int register_command(struct command *cmd);
int process_command(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen);

#endif
