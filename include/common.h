#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/stddef.h>

#define TEST_COMMON	4

#define __TO_BE_IMPLEMENTED__	0

#ifdef DEBUG
#define pr_debug(fmt, arg...)	printf(fmt, ##arg)
#else
#define pr_debug(fmt, arg...)	do {} while(0)
#endif

#define debug(fmt, arg...)	pr_debug(fmt, ##arg)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define DIV_ROUND_UP(n,d)	(((n) + (d) - 1) / (d))

#define ARRAY_SIZE(arr)		(sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))

#define UNUSED(x) (void)(x)

#endif
