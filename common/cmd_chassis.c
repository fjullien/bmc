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
#include <stdlib.h>
#include <list.h>
#include <command.h>
#include <init.h>
#include <config.h>

#include <freeipmi/spec/ipmi-cmd-spec.h>

int do_get_chassis_capabilities(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen)
{
	int i;

	for (i = 0; i < rxlen; i++)
		printf("rxbuffer[%d] = %x\n", i, rxbuffer[i]);

	*txlen = 9;

	txbuffer[SSIF_HEADER] = RESP_NETFFN_LUN(rxbuffer[SSIF_HEADER]);
	txbuffer[CMD] = rxbuffer[CMD];
	txbuffer[COMPLETION] = 0x00;

	txbuffer[3] = CONFIG_CHASSIS_CAP_FLAG;
	txbuffer[4] = CONFIG_CHASSIS_FRU_DEV_ADDR;
	txbuffer[5] = CONFIG_CHASSIS_SDR_DEV_ADDR;
	txbuffer[6] = CONFIG_CHASSIS_SEL_DEV_ADDR;
	txbuffer[7] = CONFIG_CHASSIS_SYS_MAN_DEV_ADDR;
	txbuffer[8] = CONFIG_BRIDGE_DEV_ADDR;

	return 0;
}

int do_get_chassis_status(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen)
{
	int i;

	for (i = 0; i < rxlen; i++)
		printf("rxbuffer[%d] = %x\n", i, rxbuffer[i]);

	*txlen = 7;

	txbuffer[SSIF_HEADER] = RESP_NETFFN_LUN(rxbuffer[SSIF_HEADER]);
	txbuffer[CMD] = rxbuffer[CMD];
	txbuffer[COMPLETION] = 0x00;

	/* Current power state */
	txbuffer[3] = __TO_BE_IMPLEMENTED__;
	/* Last power event */
	txbuffer[4] = __TO_BE_IMPLEMENTED__;
	/* Misc. chassis state */
	txbuffer[5] = __TO_BE_IMPLEMENTED__;
	/* Front panel button capabilities */
	txbuffer[6] = __TO_BE_IMPLEMENTED__;

	return 0;
}

static struct command cmd_get_capabilities = {
	.name = "get-chassis-capabilities",
	.cmd_number = IPMI_CMD_GET_CHASSIS_CAPABILITIES,
	.cmd_handler = do_get_chassis_capabilities,
};

static struct command cmd_get_status = {
	.name = "get-chassis-status",
	.cmd_number = IPMI_CMD_GET_CHASSIS_STATUS,
	.cmd_handler = do_get_chassis_status,
};

int register_command_chassis(void)
{
	register_command(&cmd_get_capabilities);
	register_command(&cmd_get_status);
	return 0;
}

core_initcall(register_command_chassis);
