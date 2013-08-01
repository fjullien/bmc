#ifndef _I2C_H
#define _I2C_H

#include <sys/types.h>

#define TEST_I2C	1

struct i2c_master {
	int (*get_msg)(struct i2c_master *master, int *len, unsigned char *buffer);
	int (*send_msg)(struct i2c_master *master, int len, unsigned char *buffer);
	int (*init)(struct i2c_master *master);
	int file;
};

int test_i2c_func(int a, int b);
int test1_i2c_func(int a, int b);

#endif
