#ifndef _COMMON_H
#define _COMMON_H

#define TEST_COMMON	4

/**
* container_of - cast a member of a structure out to the containing structure
* @ptr:        the pointer to the member.
* @type:       the type of the container struct this is embedded in.
* @member:     the name of the member within the struct.
*
*/
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define DIV_ROUND_UP(n,d)	(((n) + (d) - 1) / (d))

#define ARRAY_SIZE(arr)		(sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))

#define UNUSED(x) (void)(x)

#endif
