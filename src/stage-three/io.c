#include "io.h"

#ifdef PURE64_BOARD_QEMU_VIRT
#include "virtio-mmio.h"
#endif

void io_init(void) {

#ifdef PURE64_BOARD_QEMU_VIRT
	virtio_mmio_init();
#endif
}
