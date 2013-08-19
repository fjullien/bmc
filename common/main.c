/*
 * main.c
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
#include <command.h>
#include <init.h>

extern initcall_t __bmc_initcalls_start[], __bmc_early_initcalls_end[],
		  __bmc_initcalls_end[];

extern struct i2c_master master;

int main(void)
{
	int rxlen = 0;
	int txlen = 0;
	unsigned char rxbuffer[256];
	unsigned char txbuffer[256];
	int i;

	initcall_t *initcall;
	int result;

	for (initcall = __bmc_initcalls_start;
			initcall < __bmc_initcalls_end; initcall++) {
		/*printf("initcall-> %pS\n", *initcall);*/
		result = (*initcall)();
		/*printf("initcall<- %pS (%d)\n", *initcall, result);*/
	}

	master.init(&master);

	while (!rxlen) {
		master.get_msg(&master, &rxlen, rxbuffer);
		sleep(1);
	}

	process_command(rxbuffer, rxlen, txbuffer, &txlen);

	master.send_msg(&master, txlen, txbuffer);

	return 0;
}
