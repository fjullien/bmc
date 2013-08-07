/*
 * cmd_core.c
 *
 * Copyright (c) 2013 Franck Jullien <elec4fun@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdio.h>
#include <common.h>
#include <stdlib.h>
#include <command.h>
#include <list.h>
#include <linux/stddef.h>

#include <freeipmi/freeipmi_bmc_intf.h>

LIST_HEAD(cmd_list);

int register_command(struct command *cmd)
{
	list_add_tail(&cmd->list, &cmd_list);
	return 0;
}

int process_command(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen)
{
	struct command *cmd;
	int cmd_supported = 0;

	for_each_command(cmd) {
		if (rxbuffer[CMD] == cmd->cmd_number) {
			cmd->cmd_handler(rxbuffer, rxlen, txbuffer, txlen);
			cmd_supported = 1;
			break;
		}
	}

	if (!cmd_supported) {
		txbuffer[SSIF_HEADER] = RESP_NETFFN_LUN(rxbuffer[SSIF_HEADER]);
		txbuffer[CMD] = rxbuffer[CMD];
		txbuffer[COMPLETION] = BMC_IPMI_INVALID_COMMAND;
		*txlen = 3;
	}

	return 0;

}
