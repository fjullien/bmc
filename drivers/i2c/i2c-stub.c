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
#include <string.h>

int i2c_stub_init(struct i2c_master *master)
{
	int file;
	file = open("/dev/stub0", O_RDWR);
	if (!file)
		return -1;

	master->file = file;

	return 0;
}

int i2c_stub_get_msg(struct i2c_master *master, int *len, unsigned char *buffer)
{
	int ret;
	int bytes;
	int rxlen;

	bytes = read(master->file, &rxlen, 4);
	if (!bytes) {
		*len = 0;
		return 0;
	}

	ret = read(master->file, &buffer[0], rxlen - 4);
	if (ret <= 0) {
		*len = 0;
		return -1;
	}

	*len = rxlen - 4;

	return 0;
}

int i2c_stub_send_msg(struct i2c_master *master, int len, unsigned char *buffer)
{
	unsigned char *_buf;
	int ret;

	_buf = malloc(len + 1);
	if (!_buf)
		return -1;

	memcpy(&_buf[1], buffer, len); 
	_buf[0] = len;

	ret = write(master->file, _buf, len + 1);
	if (ret != (len + 1))
		return -1;

	return 0;
}

struct i2c_master master = {
	.get_msg = i2c_stub_get_msg,
	.send_msg = i2c_stub_send_msg,
	.init = i2c_stub_init,
};
