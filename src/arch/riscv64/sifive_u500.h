#ifndef PURE64_RISCV64_SIFIVE_U500_H
#define PURE64_RISCV64_SIFIVE_U500_H

#define UART0 0x13000

#define DRAM_START 0x80000000

#define STACK_START (DRAM_START + 0x1000)

#define STACK_END DRAM_START

const unsigned long long int stack_start;

const unsigned long long int stack_end;

#endif /* PURE64_RISCV64_SIFIVE_U500_H */
