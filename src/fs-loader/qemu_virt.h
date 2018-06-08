#ifndef PURE64_RISCV64_QEMU_VIRT_H
#define PURE64_RISCV64_QEMU_VIRT_H

#define UART0 0x10000000

#define DRAM_START 0x80000000

#define STACK_START (DRAM_START + 0x1000000)

#define STACK_END DRAM_START

#define VIRTIO_MMIO_START 0x10001000

#define VIRTIO_MMIO_SIZE 0x1000

#endif /* PURE64_RISCV64_QEMU_VIRT_H */