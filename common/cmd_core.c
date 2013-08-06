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
#include <i2c.h>
#include <linux/i2c-dev.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <i2c.h>
#include <list.h>
#include <command.h>

LIST_HEAD(cmd_list);

int register_command(struct command *cmd)
{
	list_add_tail(&cmd->list, &cmd_list);
	return 0;
}

int prepare_answer(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen)
{
	int i;

	for (i = 0; i < rxlen; i++)
		printf("rxbuffer[%d] = %x\n", i, rxbuffer[i]);

	*txlen = 9;

	txbuffer[0] = 0x04; /* net_fn, lun */
	txbuffer[1] = 0x00; /* cmd */
	
	txbuffer[2] = 0x00; /* Completion code */
	txbuffer[3] = 0xAB;
	txbuffer[4] = 0xAB;
	txbuffer[5] = 0xAB;
	txbuffer[6] = 0xAB;
	txbuffer[7] = 0xAB;
	txbuffer[8] = 0xAB;

	return 0;
}

