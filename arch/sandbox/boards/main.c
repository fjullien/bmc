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

int main(void)
{
	int file;
	unsigned char buffer[20];
	int ret = 0;
	int i;

	file = open("/dev/stub0", O_RDWR);

	while (!ret)
		ret = read(file, buffer, 1);

	ret = read(file, &buffer[1], buffer[0] - 1);

	for (i = 0; i < buffer[0]; i++)
		printf("buf[%d] = %x\n", i, buffer[i]);

	buffer[0] = 4; /* len */
	buffer[1] = 0x09;
	buffer[2] = 0x11;
	buffer[3] = 0x00;
	buffer[4] = 0xAB;

	write(file, buffer, 5);

	close(file);

	test_i2c_func(5, 6);
	test1_i2c_func(5, 6);
	printf("TEST_COMMON is %d\n", TEST_COMMON);
	return 0;
}
