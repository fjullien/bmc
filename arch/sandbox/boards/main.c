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

extern struct i2c_master master;

int main(void)
{
	int len = 0;
	unsigned char buffer[256];
	int i;

	master.init(&master);

	while (!len)
		master.get_msg(&master, &len, buffer);

	for (i = 0; i < len; i++)
		printf("buffer[%d] = %x\n", i, buffer[i]);

	buffer[0] = 9;    /* len */
	buffer[1] = 0x04; /* net_fn, lun */
	buffer[2] = 0x00; /* cmd */
	
	buffer[3] = 0x00; /* Completion code */
	buffer[4] = 0xAB;
	buffer[5] = 0xAB;
	buffer[6] = 0xAB;
	buffer[7] = 0xAB;
	buffer[8] = 0xAB;
	buffer[9] = 0xAB;

	master.send_msg(&master, 9, buffer);

}
