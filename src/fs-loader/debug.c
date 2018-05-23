/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "debug.h"

#include <pure64/core/string.h>

#include <stdarg.h>

#if defined PURE64_ARCH_x86_64

static void outb(unsigned short int port, unsigned char value) {
	asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static unsigned char inb(unsigned short int port) {
	unsigned char value;
	asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

#define COM1 0x03f8

void serial_putc(char c) {
	for (;;) {
		/* check if transmit is ready */
		if (inb(COM1 + 5) & 0x20)
			break;
	}
	outb(COM1, c);
}

#elif defined PURE64_ARCH_riscv64

#include "uart.h"

void serial_putc(char c) {
	uart_putc(c);
}

#else

#error "Unknown architecture"

#endif

static int pure64_isdigit(char c) {
	if ((c >= '0') && (c <= '9'))
		return 1;
	else
		return 0;
}

static void debug_putc(char c) {
	serial_putc(c);
}

const char hextable[16] = "0123456789abcdef";

/** Data type is char */
#define LENGTH_hh 0x01

/** Data type is short int */
#define LENGTH_h 0x02

/** Data type is long int */
#define LENGTH_l 0x03

/** Data type is long long int */
#define LENGTH_ll 0x04

/** Data type is intmax_t or uintmax_t */
#define LENGTH_j 0x05

/** Data type is size_t */
#define LENGTH_z 0x06

/** Data type is ptrdiff_t */
#define LENGTH_t 0x07

/** Data type is long double */
#define LENGTH_L 0x08

/** Left justify the field width. */
#define FLAG_LEFT_JUSTIFY 0x01

/** Add a '+' or '-' sign whenever
 * appropriate. */
#define FLAG_ADD_SIGN 0x02

/** If no sign is going to be printed,
 * add a space instead. */
#define FLAG_SPACE_OR_SIGN 0x04

/** For hex fields, precede the number
 * with 0, 0x, or 0X. For floating point
 * numbers, it forces a decimal point to
 * be written. */
#define FLAG_HASH_TAG 0x08

/** Print the leading zeros of the number,
 * if any. */
#define FLAG_PAD_ZERO 0x10

/** Decimal integer */
#define SPECIFIER_d 0x01

/** Decimal whole number */
#define SPECIFIER_u 0x02

/** Hexidecimal (lower case) */
#define SPECIFIER_x 0x03

/** Hexidecimal (upper case) */
#define SPECIFIER_X 0x04

/** An ASCII character */
#define SPECIFIER_c 0x05

/** A string of ASCII characters */
#define SPECIFIER_s 0x06

/** A pointer value */
#define SPECIFIER_p 0x07

struct fmt_info {
	/** A bit mask containing
	 * flags from the format string. */
	unsigned int flags;
	/** Number of characters
	 * to print. Zero means that
	 * the width was not specified. */
	unsigned int width;
	/** If non-zero, the width is
	 * given in an argument. */
	unsigned int width_arg;
	/** If non-zero, the width was
	 * set in either an argument or
	 * the format specifier. */
	unsigned int width_set;
	/** The precision of the field. */
	unsigned int precision;
	/** If non-zero, the precision is
	 * given in an argument. */
	unsigned int precision_arg;
	/** If non-zero, it means that the
	 * precision was set in either the
	 * format specifier or an argument. */
	unsigned int precision_set;
	/** A length specifier from the
	 * format string. */
	unsigned int length;
	/** If non-zero, it means that the
	 * length was set in the format string.
	 * */
	unsigned int length_set;
	/** The main specifier of the statement. */
	unsigned int specifier;
};

static void fmt_info_init(struct fmt_info *fmt_info) {
	fmt_info->flags = 0;
	fmt_info->width = 0;
	fmt_info->width_arg = 0;
	fmt_info->width_set = 0;
	fmt_info->precision = 0;
	fmt_info->precision_arg = 0;
	fmt_info->precision_set = 0;
	fmt_info->length = 0;
	fmt_info->length_set = 0;
	fmt_info->specifier = 0;
}

/** Attempts to parse a hex field
 * in a format string. If the function
 * is successfull, it returns the the
 * number of characters parsed. If it
 * fails, it returns zero.
 * */

static unsigned int fmt_parse(struct fmt_info *fmt_info, const char *fmt, va_list args) {

	unsigned int i = 0;

	if (fmt[i] != '%')
		return 0;
	else
		i++;

	/* Parse flags */

	fmt_info->flags = 0;

	while (fmt[i] != 0) {

		if (fmt[i] == '-') {
			fmt_info->flags |= FLAG_LEFT_JUSTIFY;
		} else if (fmt[i] == '+') {
			fmt_info->flags |= FLAG_ADD_SIGN;
		} else if (fmt[i] == ' ') {
			fmt_info->flags |= FLAG_SPACE_OR_SIGN;
		} else if (fmt[i] == '#') {
			fmt_info->flags |= FLAG_HASH_TAG;
		} else if (fmt[i] == '0') {
			fmt_info->flags |= FLAG_PAD_ZERO;
		} else {
			break;
		}

		i++;
	}

	/* Parse width */

	if (pure64_isdigit(fmt[i]) && (fmt[i] != '0')) {
		/* Print at least this many
		 * characters. */
		fmt_info->width = fmt[i] - '0';
		fmt_info->width_arg = 0;
		fmt_info->width_set = 1;
		i++;
	} else if (fmt[i] == '*') {
		/* The width specifier is in
		 * an argument. */
		fmt_info->width = 0;
		fmt_info->width_arg = 1;
		fmt_info->width_set = 1;
		i++;
	} else {
		/* No width specifier was given. */
		fmt_info->width = 0;
		fmt_info->width_set = 0;
	}

	/* Get the width argument. */

	if (fmt_info->width_set && fmt_info->width_arg) {
		fmt_info->width = va_arg(args, unsigned int);
	}

	/* Parse precision */

	if (fmt[i] == '.') {

		i++;

		if (pure64_isdigit(fmt[i])) {
			fmt_info->precision = fmt[i] - '0';
			fmt_info->precision_arg = 0;
			fmt_info->precision_set = 1;
			i++;
		} else if (fmt[i] == '*') {
			fmt_info->precision = 0;
			fmt_info->precision_arg = 1;
			fmt_info->precision_set = 1;
			i++;
		} else {
			/* There should have been a
			 * precision specifier here.
			 * Bail out with and return zero
			 * to indicate failure. */
			fmt_info->precision = 0;
			fmt_info->precision_arg = 0;
			fmt_info->precision_set = 0;
			return 0;
		}

		/* Get the precision argument. */

		if (fmt_info->precision_set && fmt_info->precision_arg) {
			fmt_info->precision = va_arg(args, unsigned int);
		}
	}

	/* Parse length */

	if ((fmt[i + 0] == 'h')
	 && (fmt[i + 1] == 'h')) {
		fmt_info->length = LENGTH_hh;
		fmt_info->length_set = 1;
		i += 2;
	} else if (fmt[i] == 'h') {
		fmt_info->length = LENGTH_h;
		fmt_info->length_set = 1;
		i++;
	} else if ((fmt[i + 0] == 'l')
	        && (fmt[i + 1] == 'l')) {
		fmt_info->length = LENGTH_ll;
		fmt_info->length_set = 1;
		i += 2;
	} else if (fmt[i] == 'l') {
		fmt_info->length = LENGTH_l;
		fmt_info->length_set = 1;
		i++;
	} else if (fmt[i] == 'j') {
		fmt_info->length = LENGTH_j;
		fmt_info->length_set = 1;
		i++;
	} else if (fmt[i] == 'z') {
		fmt_info->length = LENGTH_z;
		fmt_info->length_set = 1;
		i++;
	} else if (fmt[i] == 't') {
		fmt_info->length = LENGTH_t;
		fmt_info->length_set = 1;
		i++;
	} else if (fmt[i] == 'L') {
		fmt_info->length = LENGTH_L;
		fmt_info->length_set = 1;
		i++;
	} else {
		fmt_info->length = 0;
		fmt_info->length_set = 0;
	}

	/* Parse specifier */

	if ((fmt[i] == 'd') || (fmt[i] == 'i')) {
		fmt_info->specifier = SPECIFIER_d;
	} else if (fmt[i] == 'u') {
		fmt_info->specifier = SPECIFIER_u;
	} else if (fmt[i] == 'x') {
		fmt_info->specifier = SPECIFIER_x;
	} else if (fmt[i] == 'X') {
		fmt_info->specifier = SPECIFIER_X;
	} else if (fmt[i] == 'c') {
		fmt_info->specifier = SPECIFIER_c;
	} else if (fmt[i] == 's') {
		fmt_info->specifier = SPECIFIER_s;
	} else if (fmt[i] == 'p') {
		fmt_info->specifier = SPECIFIER_p;
	} else {
		/* Unsupported field specifier */
		return 0;
	}

	i++;

	return i;
}

static void debug_print_string(const char *str, unsigned int len) {

	unsigned int i;

	for (i = 0; i < len; i++)
		debug_putc(str[i]);
}

static void fmt_print_hex(const struct fmt_info *fmt_info, va_list args) {

	char str[16 + 1];
	unsigned long long int n;
	unsigned int i;
	unsigned int bits;
	unsigned int ch_count;

	if (fmt_info->length_set) {
		if (fmt_info->length == LENGTH_l) {
			bits = sizeof(unsigned long int) * 8;
			n = va_arg(args, unsigned long int);
		} else if (fmt_info->length == LENGTH_ll) {
			bits = sizeof(unsigned long long int) * 8;
			n = va_arg(args, unsigned long long int);
		} else if (fmt_info->length == LENGTH_h) {
			bits = sizeof(unsigned char);
			n = va_arg(args, unsigned int);
		} else if (fmt_info->length == LENGTH_hh) {
			bits = sizeof(unsigned short int);
			n = va_arg(args, unsigned int);
		} else {
			/* Unsupported length.
			 * Bail out before corupting
			 * the stack */
			return;
		}
	} else if (fmt_info->specifier == SPECIFIER_p) {
		bits = 64;
		n = (unsigned long long int) va_arg(args, void *);
	} else {
		bits = 32;
		n = va_arg(args, unsigned int);
	}

	if (bits == 8) {
		ch_count = 2;
		str[0] = hextable[(n >> 4) & 0xf];
		str[1] = hextable[(n >> 0) & 0xf];
	} else if (bits == 16) {
		ch_count = 4;
		str[0] = hextable[(n >> 12) & 0xf];
		str[1] = hextable[(n >> 8) & 0xf];
		str[2] = hextable[(n >> 4) & 0xf];
		str[3] = hextable[(n >> 0) & 0xf];
	} else if (bits == 32) {
		ch_count = 8;
		str[0] = hextable[(n >> 28) & 0xf];
		str[1] = hextable[(n >> 24) & 0xf];
		str[2] = hextable[(n >> 20) & 0xf];
		str[3] = hextable[(n >> 16) & 0xf];
		str[4] = hextable[(n >> 12) & 0xf];
		str[5] = hextable[(n >> 8) & 0xf];
		str[6] = hextable[(n >> 4) & 0xf];
		str[7] = hextable[(n >> 0) & 0xf];
	} else if (bits == 64) {
		ch_count = 16;
		str[0] = hextable[(n >> 60) & 0xf];
		str[1] = hextable[(n >> 56) & 0xf];
		str[2] = hextable[(n >> 52) & 0xf];
		str[3] = hextable[(n >> 48) & 0xf];
		str[4] = hextable[(n >> 44) & 0xf];
		str[5] = hextable[(n >> 40) & 0xf];
		str[6] = hextable[(n >> 36) & 0xf];
		str[7] = hextable[(n >> 32) & 0xf];
		str[8] = hextable[(n >> 28) & 0xf];
		str[9] = hextable[(n >> 24) & 0xf];
		str[10] = hextable[(n >> 20) & 0xf];
		str[11] = hextable[(n >> 16) & 0xf];
		str[12] = hextable[(n >> 12) & 0xf];
		str[13] = hextable[(n >> 8) & 0xf];
		str[14] = hextable[(n >> 4) & 0xf];
		str[15] = hextable[(n >> 0) & 0xf];
	} else {
		/* Unsupported bit count */
		return;
	}

	/* Print leading zeros or
	 * spaces first. */

	for (i = 0; i < ch_count; i++) {
		if (str[i] == '0') {
			if (fmt_info->width_set && ((ch_count - i) < fmt_info->width)) {
				if (fmt_info->flags & FLAG_PAD_ZERO)
					debug_putc('0');
				else
					debug_putc(' ');
			}
		} else {
			break;
		}
	}

	/* Check if all numbers where zero.
	 * If they were, there should be
	 * at least one zero printed to
	 * the screen. */

	if (i >= ch_count) {
		debug_putc('0');
	}

	while (i < ch_count) {
		debug_putc(str[i]);
		i++;
	}
}

static void fmt_print_string(const struct fmt_info *fmt_info, va_list args) {

	char *str;
	unsigned int len;

	str = va_arg(args, char *);

	if (fmt_info->precision_set)
		len = fmt_info->precision;
	else
		len = pure64_strlen(str);

	debug_print_string(str, len);
}

static void fmt_print(const struct fmt_info *fmt_info, va_list args) {

	if ((fmt_info->specifier == SPECIFIER_x)
	 || (fmt_info->specifier == SPECIFIER_p)) {
		fmt_print_hex(fmt_info, args);
	} else if (fmt_info->specifier == SPECIFIER_s) {
		fmt_print_string(fmt_info, args);
	}
}

void debug(const char *fmt, ...) {

	unsigned int i;
	unsigned int ret;
	struct fmt_info fmt_info;
	va_list args;

	fmt_info_init(&fmt_info);

	va_start(args, fmt);

	for (i = 0; fmt[i]; i++) {
		ret = fmt_parse(&fmt_info, &fmt[i], args);
		if (ret > 0) {
			i += ret - 1;
			fmt_print(&fmt_info, args);
		} else if ((fmt[i] == '%') && (fmt[i + 1] == '%')) {
			debug_putc('%');
			i++;
		} else {
			debug_putc(fmt[i]);
		}
	}

	va_end(args);
}
