#ifndef _CHASSIS_H
#define _CHASSIS_H

#include <stdint.h>

#define PROVIDES_INTRUSION_SENSOR	0x01
#define PROVIDES_FRONT_PANEL_LOCKOUT	0x02
#define PROVIDES_DIAGNOSTIC_INTERRUPT	0x04
#define PROVIDES_POWER_INTERLOCK	0x08

struct chassis_ops {
	int (*chassis_control)(uint8_t control);
	int (*chassis_identify)(int nb_param, uint8_t interval, uint8_t force);
};

struct chassis_status {
	uint8_t power_state;
	uint8_t last_power_event;
	uint8_t chassis_state;
	uint8_t button;
	int identify_interval;
	int restart_cause;
	uint32_t hour_counter;
};

int get_chassis_status(struct chassis_status *state);

int register_chassis_ops(struct chassis_ops *chassis_ops);

#endif
