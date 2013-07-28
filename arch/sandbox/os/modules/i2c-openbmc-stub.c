/*
    i2c-stub.c - I2C/SMBus chip emulator

    Copyright (c) 2004 Mark M. Hoffman <mhoffman@lightlink.com>
    Copyright (C) 2007, 2012 Jean Delvare <khali@linux-fr.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define DEBUG 1

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/kfifo.h>

#define MAX_CHIPS 10
#define STUB_FUNC (I2C_FUNC_SMBUS_QUICK | I2C_FUNC_SMBUS_BYTE | \
		   I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA | \
		   I2C_FUNC_SMBUS_I2C_BLOCK)

static unsigned short chip_addr[MAX_CHIPS];
module_param_array(chip_addr, ushort, NULL, S_IRUGO);
MODULE_PARM_DESC(chip_addr,
		 "Chip addresses (up to 10, between 0x03 and 0x77)");

static unsigned long functionality = STUB_FUNC;
module_param(functionality, ulong, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(functionality, "Override functionality bitfield");

struct stub_chip {
	u8 pointer;
	u16 words[256];		/* Byte operations use the LSB as per SMBus
				   specification */
};

static struct stub_chip *stub_chips;

static struct __kfifo xfer_fifo;

static u16 read_buffer[256];
struct cdev stub_cdev;
dev_t devid;
static struct class *cl;

/* Return negative errno on error. */
static s32 stub_xfer(struct i2c_adapter *adap, u16 addr, unsigned short flags,
	char read_write, u8 command, int size, union i2c_smbus_data *data)
{
	s32 ret;
	int i, len;
	struct stub_chip *chip = NULL;

	printk("--- stub_xfer\n");

	/* Search for the right chip */
	for (i = 0; i < MAX_CHIPS && chip_addr[i]; i++) {
		if (addr == chip_addr[i]) {
			chip = stub_chips + i;
			break;
		}
	}
	if (!chip)
		return -ENODEV;

	switch (size) {

	case I2C_SMBUS_QUICK:
		dev_dbg(&adap->dev, "smbus quick - addr 0x%02x\n", addr);
		ret = 0;
		break;

	case I2C_SMBUS_BYTE:
		if (read_write == I2C_SMBUS_WRITE) {
			chip->pointer = command;
			dev_dbg(&adap->dev,
				"smbus byte - addr 0x%02x, wrote 0x%02x.\n",
				addr, command);
		} else {
			data->byte = chip->words[chip->pointer++] & 0xff;
			dev_dbg(&adap->dev,
				"smbus byte - addr 0x%02x, read  0x%02x.\n",
				addr, data->byte);
		}

		ret = 0;
		break;

	case I2C_SMBUS_BYTE_DATA:
		if (read_write == I2C_SMBUS_WRITE) {
			chip->words[command] &= 0xff00;
			chip->words[command] |= data->byte;
			dev_dbg(&adap->dev,
				"smbus byte data - addr 0x%02x, wrote 0x%02x at 0x%02x.\n",
				addr, data->byte, command);
		} else {
			data->byte = chip->words[command] & 0xff;
			dev_dbg(&adap->dev,
				"smbus byte data - addr 0x%02x, read  0x%02x at 0x%02x.\n",
				addr, data->byte, command);
		}
		chip->pointer = command + 1;

		ret = 0;
		break;

	case I2C_SMBUS_WORD_DATA:
		if (read_write == I2C_SMBUS_WRITE) {
			chip->words[command] = data->word;
			dev_dbg(&adap->dev,
				"smbus word data - addr 0x%02x, wrote 0x%04x at 0x%02x.\n",
				addr, data->word, command);
		} else {
			data->word = chip->words[command];
			dev_dbg(&adap->dev,
				"smbus word data - addr 0x%02x, read  0x%04x at 0x%02x.\n",
				addr, data->word, command);
		}

		ret = 0;
		break;

	case I2C_SMBUS_BLOCK_DATA:
		if (read_write == I2C_SMBUS_READ) {
			len = data->block[0] = 10;
			for (i = 0; i < len; i++)
				data->block[1 + i] = 0x11 + i;
			dev_dbg(&adap->dev,
				"i2c block data - addr 0x%02x, read %d bytes at 0x%02x.\n",
				addr, len, command);
		} else {
			len = data->block[0];
			for (i = 0; i < len; i++)
				printk("%02X ", data->block[1 + i]);
			printk("\n");
			dev_dbg(&adap->dev,
				"i2c block data - addr 0x%02x, wrote  %d bytes at 0x%02x.\n",
				addr, len, command);
		}

		ret = 0;
		break;

	case I2C_SMBUS_I2C_BLOCK_DATA:
		len = data->block[0];
		if (read_write == I2C_SMBUS_WRITE) {
			for (i = 0; i < len; i++) {
				chip->words[command + i] &= 0xff00;
				chip->words[command + i] |= data->block[1 + i];
			}
			dev_dbg(&adap->dev,
				"i2c block data - addr 0x%02x, wrote %d bytes at 0x%02x.\n",
				addr, len, command);
		} else {
			for (i = 0; i < len; i++) {
				data->block[1 + i] =
					chip->words[command + i] & 0xff;
			}
			dev_dbg(&adap->dev,
				"i2c block data - addr 0x%02x, read  %d bytes at 0x%02x.\n",
				addr, len, command);
		}

		ret = 0;
		break;



	default:
		dev_dbg(&adap->dev, "Unsupported I2C/SMBus command (0x%x)\n", size);
		ret = -EOPNOTSUPP;
		break;
	} /* switch (size) */

	return ret;
}

static u32 stub_func(struct i2c_adapter *adapter)
{
	return STUB_FUNC & functionality;
}

static const struct i2c_algorithm smbus_algorithm = {
	.functionality	= stub_func,
	.smbus_xfer	= stub_xfer,
};

static struct i2c_adapter stub_adapter = {
	.owner		= THIS_MODULE,
	.class		= I2C_CLASS_HWMON | I2C_CLASS_SPD,
	.algo		= &smbus_algorithm,
	.name		= "SMBus stub driver",
};

static ssize_t stub_write(struct file *file, const char __user *data, size_t len, loff_t *ppos)
{
	printk("----> stub_write\n");
	return len;
}

static ssize_t stub_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	printk("----> stub_read\n");
	return len;
}

static ssize_t stub_open(struct inode *inode, struct file *file)
{
	printk("----> stub_open\n");
	return 0;
}

static int stub_release(struct inode *inode, struct file *file)
{
	printk("----> stub_release\n");
	return 0;
}

static const struct file_operations stub_cdev_fops = {
	.owner = THIS_MODULE,
	.write = stub_write,
	.read = stub_read,
	.open = stub_open,
	.release = stub_release,
	/*.ioctl = stub_ioctl,*/
};

static int __init i2c_stub_init(void)
{
	int i, ret;

	if (!chip_addr[0]) {
		pr_err("i2c-stub: Please specify a chip address\n");
		return -ENODEV;
	}

	for (i = 0; i < MAX_CHIPS && chip_addr[i]; i++) {
		if (chip_addr[i] < 0x03 || chip_addr[i] > 0x77) {
			pr_err("i2c-stub: Invalid chip address 0x%02x\n",
			       chip_addr[i]);
			return -EINVAL;
		}

		pr_info("i2c-stub: Virtual chip at 0x%02x\n", chip_addr[i]);
	}

	ret = kfifo_alloc(xfer_fifo, 1024, GFP_KERNEL);
	if (ret)
		return ret;

	/* Allocate memory for all chips at once */
	stub_chips = kzalloc(i * sizeof(struct stub_chip), GFP_KERNEL);
	if (!stub_chips) {
		pr_err("i2c-stub: Out of memory\n");
		return -ENOMEM;
	}

	cl = class_create(THIS_MODULE, "i2cstub");
	alloc_chrdev_region(&devid, 0, 1, "i2cstub");

	cdev_init(&stub_cdev, &stub_cdev_fops);
	cdev_add(&stub_cdev, devid, 1);

	device_create(cl, NULL, devid, NULL, "stub0");

	ret = i2c_add_adapter(&stub_adapter);
	if (ret)
		kfree(stub_chips);
	return ret;
}

static void __exit i2c_stub_exit(void)
{
	i2c_del_adapter(&stub_adapter);
	cdev_del(&stub_cdev);
	device_destroy(cl, devid);
	class_destroy(cl);
	unregister_chrdev_region(devid, 1);
	kfree(stub_chips);
	kfifo_free(xfer_fifo);
}

MODULE_AUTHOR("Mark M. Hoffman <mhoffman@lightlink.com>");
MODULE_DESCRIPTION("I2C stub driver");
MODULE_LICENSE("GPL");

module_init(i2c_stub_init);
module_exit(i2c_stub_exit);
