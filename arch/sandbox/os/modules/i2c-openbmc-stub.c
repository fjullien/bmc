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
#include <linux/sched.h>
#include <linux/version.h>

static struct timer_list timer;

#define STUB_FLUSH_FIFO		0
#define STUB_FIFO_LEN		1
#define STUB_CONTROL_FROM_CDEV	2

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

struct data_packet {
	u8 len;
	u8 cmd;
	u8 buf[32];
};

#define DATA_HEADER_LEN (sizeof(u8) + sizeof(u8))

static struct stub_chip *stub_chips;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
static struct kfifo rx_fifo;
#else
static struct kfifo *rx_fifo;
#endif
struct cdev stub_cdev;
dev_t devid;
static struct class *cl;
static uint8_t tx_buffer[33];
int tx_answer_received = 0;
int cdev_ctrl = 1;

void timer_callback(unsigned long data)
{
	tx_answer_received = 1;
}

/* Return negative errno on error. */
static s32 stub_xfer(struct i2c_adapter *adap, u16 addr, unsigned short flags,
	char read_write, u8 command, int size, union i2c_smbus_data *data)
{
	s32 ret;
	int i, len;
	struct stub_chip *chip = NULL;
	struct data_packet rx_data;

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
		rx_data.len = DATA_HEADER_LEN + 1;
		rx_data.cmd = I2C_SMBUS_QUICK;
		rx_data.buf[0] = 1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
		kfifo_in(&rx_fifo, (unsigned char *)&rx_data, rx_data.len);
#else
		__kfifo_put(rx_fifo, (unsigned char *)&rx_data, rx_data.len);
#endif
		dev_dbg(&adap->dev, "smbus quick - addr 0x%02x\n", addr);
		ret = 0;
		break;

	case I2C_SMBUS_BYTE:
		if (read_write == I2C_SMBUS_WRITE) {
			chip->pointer = command;

			rx_data.len = DATA_HEADER_LEN + 1;
			rx_data.cmd = I2C_SMBUS_BYTE;
			rx_data.buf[0] = command;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
			kfifo_in(&rx_fifo, (unsigned char *)&rx_data, rx_data.len);
#else
			__kfifo_put(rx_fifo, (unsigned char *)&rx_data, rx_data.len);
#endif

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

			rx_data.len = DATA_HEADER_LEN + 2;
			rx_data.cmd = I2C_SMBUS_BYTE_DATA;
			rx_data.buf[0] = command;
			rx_data.buf[1] = data->byte;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
			kfifo_in(&rx_fifo, (unsigned char *)&rx_data, rx_data.len);
#else
			__kfifo_put(rx_fifo, (unsigned char *)&rx_data, rx_data.len);
#endif

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

			rx_data.len = DATA_HEADER_LEN + 3;
			rx_data.cmd = I2C_SMBUS_WORD_DATA;
			rx_data.buf[0] = command;
			rx_data.buf[1] = (data->word & 0xff00) >> 8;
			rx_data.buf[2] = data->word & 0xff;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
			kfifo_in(&rx_fifo, (unsigned char *)&rx_data, rx_data.len);
#else
			__kfifo_put(rx_fifo, (unsigned char *)&rx_data, rx_data.len);
#endif

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
		if (read_write == I2C_SMBUS_WRITE) {
			len = data->block[0];
			rx_data.len = DATA_HEADER_LEN + len + 1;
			rx_data.cmd = I2C_SMBUS_BLOCK_DATA;
			rx_data.buf[0] = command;
			printk("rx_data.len    = %02x\n", rx_data.len);
			printk("rx_data.cmd    = %02x\n", rx_data.cmd);
			printk("rx_data.buf[0] = %02x\n", rx_data.buf[0]);
			for (i = 0; i < len; i++) {
				printk("-- data = %02x\n", data->block[1 + i]);
				rx_data.buf[i + 1] = data->block[1 + i];
				chip->words[command + i] &= 0xff00;
				chip->words[command + i] |= data->block[1 + i];
			}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
			kfifo_in(&rx_fifo, (unsigned char *)&rx_data, rx_data.len);
#else
			__kfifo_put(rx_fifo, (unsigned char *)&rx_data, rx_data.len);
#endif

			dev_dbg(&adap->dev,
				"i2c block data - addr 0x%02x, wrote %d bytes at 0x%02x.\n",
				addr, len, command);
		} else {
			// data->block[0] gives read size
			ret = mod_timer(&timer, jiffies + msecs_to_jiffies(10000) );

			while (!tx_answer_received)
				schedule();
			tx_answer_received = 0;
			data->block[0] = tx_buffer[0];
			for (i = 0; i < data->block[0]; i++) {
				data->block[1 + i] = tx_buffer[1 + i];
			}
			dev_dbg(&adap->dev,
				"i2c block data - addr 0x%02x, read  %d bytes at 0x%02x.\n",
				addr, data->block[0], command);
		}

		ret = 0;
		break;

	case I2C_SMBUS_I2C_BLOCK_DATA:
		len = data->block[0];
		if (read_write == I2C_SMBUS_WRITE) {
			rx_data.len = DATA_HEADER_LEN + len + 1;
			rx_data.cmd = I2C_SMBUS_BLOCK_DATA;
			for (i = 0; i < len; i++) {
				rx_data.buf[i + 1] = data->block[1 + i];
				chip->words[command + i] &= 0xff00;
				chip->words[command + i] |= data->block[1 + i];
			}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
			kfifo_in(&rx_fifo, (unsigned char *)&rx_data, rx_data.len);
#else
			__kfifo_put(rx_fifo, (unsigned char *)&rx_data, rx_data.len);
#endif
			/*dev_dbg(&adap->dev,
				"i2c block data - addr 0x%02x, wrote %d bytes at 0x%02x.\n",
				addr, len, command);*/
		} else {
			for (i = 0; i < len; i++) {
				data->block[1 + i] =
					chip->words[command + i] & 0xff;
			}
			/*dev_dbg(&adap->dev,
				"i2c block data - addr 0x%02x, read  %d bytes at 0x%02x.\n",
				addr, len, command);*/
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
	if (copy_from_user(tx_buffer, data, min(ARRAY_SIZE(tx_buffer), len)))
		return -EFAULT;
	tx_answer_received = 1;
	return len;
}

static ssize_t stub_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	uint8_t buffer[32];
	int ret;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
	ret = kfifo_out(&rx_fifo, buffer, len);
#else
	ret = __kfifo_get(rx_fifo, buffer, len);
#endif
	if (copy_to_user(buf, buffer, len)) {
		/*kfree(buffer);*/
		return -EFAULT;
	}

	/*kfree(buffer);*/

	return ret;
}

static int stub_open(struct inode *inode, struct file *file)
{
	//printk("----> stub_open\n");
	return 0;
}

static int stub_release(struct inode *inode, struct file *file)
{
	//printk("----> stub_release\n");
	return 0;
}
/*
static int stub_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	switch(cmd) {
	case STUB_FLUSH_FIFO:
#if LINUX_VERSION_3_2
		kfifo_reset(&rx_fifo);
#else
		__kfifo_reset(rx_fifo);
#endif
		break;
	case STUB_FIFO_LEN:
#if LINUX_VERSION_3_2
		*(int *)arg = kfifo_size(&rx_fifo);
#else
		*(int *)arg = kfifo_len(rx_fifo);
#endif
		break;
	case STUB_CONTROL_FROM_CDEV:
		cdev_ctrl = !!arg;
		break;
	default:
		break;
	}

	return ret;
}
*/
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

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
	ret = kfifo_alloc(&rx_fifo, 1024, GFP_KERNEL);
	if (ret < 0)
		return -ENOMEM;
#else
	rx_fifo = kfifo_alloc(1024, GFP_KERNEL, NULL);
	if (!rx_fifo)
		return -ENOMEM;
#endif
	memset(tx_buffer, 0, ARRAY_SIZE(tx_buffer));

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

	setup_timer(&timer, timer_callback, 0);


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
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
	kfifo_free(rx_fifo);
	del_timer(&timer);
#endif
}

MODULE_AUTHOR("Mark M. Hoffman <mhoffman@lightlink.com>");
MODULE_DESCRIPTION("I2C stub driver");
MODULE_LICENSE("GPL");

module_init(i2c_stub_init);
module_exit(i2c_stub_exit);
