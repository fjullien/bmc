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
#include <chassis.h>

#include <freeipmi/spec/ipmi-cmd-spec.h>

/* Chassis current power state */
static struct chassis_status chassis_status;
/* Platform specific chassis command handler */
struct chassis_ops *platform_chassis_ops;

int get_chassis_status(struct chassis_status *state)
{
	state = &chassis_status;
	return 0;
}

int register_chassis_ops(struct chassis_ops *chassis_ops)
{
	platform_chassis_ops = chassis_ops;
	return 0;
}

int do_get_chassis_capabilities(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen)
{
	int i;
	for (i = 0; i < rxlen; i++)
		printf("rxbuffer[%d] = %x\n", i, rxbuffer[i]);

	*txlen = 9;

	SET_COMPLETION_OK(txbuffer, rxbuffer);

	txbuffer[3] = CONFIG_CHASSIS_CAP_FLAG;
	txbuffer[4] = CONFIG_CHASSIS_FRU_DEV_ADDR;
	txbuffer[5] = CONFIG_CHASSIS_SDR_DEV_ADDR;
	txbuffer[6] = CONFIG_CHASSIS_SEL_DEV_ADDR;
	txbuffer[7] = CONFIG_CHASSIS_SYS_MAN_DEV_ADDR;
	txbuffer[8] = CONFIG_BRIDGE_DEV_ADDR;

	return COMMAND_OK;
}

int do_get_chassis_status(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen)
{
	int i;
	for (i = 0; i < rxlen; i++)
		printf("rxbuffer[%d] = %x\n", i, rxbuffer[i]);

	*txlen = 7;

	SET_COMPLETION_OK(txbuffer, rxbuffer);

	/* Current power state */
	txbuffer[3] = chassis_status.power_state;
	/* Last power event */
	txbuffer[4] = chassis_status.last_power_event;
	/* Misc. chassis state */
	txbuffer[5] = chassis_status.chassis_state;
	/* Front panel button capabilities */
	txbuffer[6] = chassis_status.button;

	return COMMAND_OK;
}

int do_chassis_control(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen)
{
	int i;
	for (i = 0; i < rxlen; i++)
		printf("rxbuffer[%d] = %x\n", i, rxbuffer[i]);

	if (platform_chassis_ops && platform_chassis_ops->chassis_control) {
		platform_chassis_ops->chassis_control(rxbuffer[REQUEST_BYTE_1]);
		SET_COMPLETION_NO_DATA(txlen, txbuffer, rxbuffer, BMC_IPMI_COMMAND_OK);
	} else 
		return NO_PLATFORM_HANDLER;

	return COMMAND_OK;
}

int do_chassis_identify(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen)
{
	int i;
	for (i = 0; i < rxlen; i++)
		printf("rxbuffer[%d] = %x\n", i, rxbuffer[i]);

	if (rxlen >= 3)
		chassis_status.identify_interval = rxbuffer[REQUEST_BYTE_1];

	SET_COMPLETION_NO_DATA(txlen, txbuffer, rxbuffer, BMC_IPMI_COMMAND_OK);
	return COMMAND_OK;
}

int do_get_restart_cause(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen)
{
	int i;
	for (i = 0; i < rxlen; i++)
		printf("rxbuffer[%d] = %x\n", i, rxbuffer[i]);

	*txlen = 5;

	SET_COMPLETION_OK(txbuffer, rxbuffer);

	/* Restart cause */
	txbuffer[3] = chassis_status.restart_cause;
	/* Channel number */
	/* TODO: fix this */
	txbuffer[4] = 0;

	return COMMAND_OK;
}

int do_get_hour_counter(unsigned char *rxbuffer, int rxlen, unsigned char *txbuffer, int *txlen)
{
	int i;
	for (i = 0; i < rxlen; i++)
		printf("rxbuffer[%d] = %x\n", i, rxbuffer[i]);

	*txlen = 8;

	SET_COMPLETION_OK(txbuffer, rxbuffer);

	/* Minutes per count */
	txbuffer[3] = CONFIG_MINUTES_PER_COUNT;
	/* Counter reading byte 0 (LSB) */
	txbuffer[4] = chassis_status.hour_counter & 0xff;
	/* Counter reading byte 1 */
	txbuffer[5] = (chassis_status.hour_counter) >> 8 & 0xff;
	/* Counter reading byte 2 */
	txbuffer[6] = (chassis_status.hour_counter) >> 16 & 0xff;
	/* Counter reading byte 3 (MSB) */
	txbuffer[7] = (chassis_status.hour_counter) >> 24 & 0xff;

	return COMMAND_OK;
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

static struct command cmd_chassis_control = {
	.name = "chassis-control",
	.cmd_number = IPMI_CMD_CHASSIS_CONTROL,
	.cmd_handler = do_chassis_control,
};

static struct command cmd_chassis_identify = {
	.name = "chassis-identify",
	.cmd_number = IPMI_CMD_CHASSIS_IDENTIFY,
	.cmd_handler = do_chassis_identify,
};

static struct command cmd_get_restart_cause = {
	.name = "get-system-restart-cause",
	.cmd_number = IPMI_CMD_GET_SYSTEM_RESTART_CAUSE,
	.cmd_handler = do_get_restart_cause,
};

static struct command cmd_get_hour_counter = {
	.name = "get-power-on-hours-counter",
	.cmd_number = IPMI_CMD_GET_POWER_ON_HOURS_COUNTER,
	.cmd_handler = do_get_hour_counter,
};

int register_command_chassis(void)
{
	register_command(&cmd_get_capabilities);
	register_command(&cmd_get_status);
	register_command(&cmd_chassis_control);
	register_command(&cmd_chassis_identify);
	register_command(&cmd_get_restart_cause);
	register_command(&cmd_get_hour_counter);

	return 0;
}

core_initcall(register_command_chassis);
