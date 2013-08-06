#define INITCALLS			\
	KEEP(*(.initcall.0))			\
	KEEP(*(.initcall.1))			\
	KEEP(*(.initcall.2))			\
	KEEP(*(.initcall.3))			\
	KEEP(*(.initcall.4))			\
	KEEP(*(.initcall.5))			\
	KEEP(*(.initcall.6))			\
	KEEP(*(.initcall.7))			\
	KEEP(*(.initcall.8))			\
	KEEP(*(.initcall.9))			\
	KEEP(*(.initcall.10))			\
	KEEP(*(.initcall.11))

SECTIONS
{
	. = ALIGN(64);
	__bmc_initcalls_start = .;
	__bmc_initcalls : { INITCALLS }
	__bmc_initcalls_end = .;
}

INSERT BEFORE .rodata;
