#ifndef PURE64_RISCV_UART_H
#define PURE64_RISCV_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initializes the default UART channel.
 * */

void uart_init(void);

/** Writes an ASCII string to the UART channel.
 * @param msg The string to send.
 * @param len The number of bytes in the string.
 * */

void uart_write_ascii(const char *msg, unsigned int len);

/** Writes a null-terminated ASCII
 * string tothe UART channel.
 * @param msg The string to send.
 * This parameter must be null terminated.
 * */

void uart_write_asciiz(const char *msg);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_RISCV_UART_H */
