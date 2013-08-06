/*
 * cmd_chassis.c
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
#include <init.h>

int do_get_chassis_capabilities(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen)
{
	return 0;
}

int do_get_chassis_status(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen)
{
	return 0;
}

static struct command cmd_get_capabilities = {
	.name = "get-chassis-capabilities",
	.cmd_number = 0,
	.cmd_handler = do_get_chassis_capabilities,
};

static struct command cmd_get_status = {
	.name = "get-chassis-status",
	.cmd_number = 1,
	.cmd_handler = do_get_chassis_status,
};

int register_command_chassis(void)
{
	register_command(&cmd_get_capabilities);
	register_command(&cmd_get_status);
	return 0;
}

core_initcall(register_command_chassis);
