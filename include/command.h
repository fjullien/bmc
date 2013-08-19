#ifndef _CMD_DISPATCH_H
#define _CMD_DISPATCH_H

#include <list.h>

#define COMMAND_OK		(0)
#define COMMAND_NOT_SUPPORTED	(-1)
#define NO_PLATFORM_HANDLER	(-2)

#define for_each_command(cmd) list_for_each_entry(cmd, &cmd_list, list)

#define SSIF_HEADER	0
#define CMD		1
#define COMPLETION	2

/* rxbuffer */
#define SSIF_HEADER	0
#define CMD		1
#define REQUEST_BYTE_1	2

#define RESP_NETFFN_LUN(netfn_lun)	(netfn_lun | (1 << 2))

/* This one is missing in freeipmi_bmc_intf.h */
#define BMC_IPMI_COMMAND_OK	0

#define SET_COMPLETION_NO_DATA(txlen, txbuffer, rxbuffer, comp) \
		*txlen = 3; \
		txbuffer[SSIF_HEADER] = RESP_NETFFN_LUN(rxbuffer[SSIF_HEADER]); \
		txbuffer[CMD] = rxbuffer[CMD]; \
		txbuffer[COMPLETION] = comp

#define SET_COMPLETION_OK(txbuffer, rxbuffer) \
		txbuffer[SSIF_HEADER] = RESP_NETFFN_LUN(rxbuffer[SSIF_HEADER]); \
		txbuffer[CMD] = rxbuffer[CMD]; \
		txbuffer[COMPLETION] = 0x00

struct command {
	const char *name;
	struct list_head list;
	int cmd_number;
	int (*cmd_handler)(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen);
};

int register_command(struct command *cmd);
int process_command(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen);

#endif
