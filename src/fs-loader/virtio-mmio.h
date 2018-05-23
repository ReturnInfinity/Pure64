#ifndef VIRTIO_MMIO_H
#define VIRTIO_MMIO_H

#include <pure64/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct virtio_mmio_reg {
	pure64_uint32 magic;
	pure64_uint32 version;
	pure64_uint32 device_id;
	pure64_uint32 vendor_id;
	pure64_uint32 features;
	pure64_uint32 features_select;
	pure64_uint32 queue_select;
	pure64_uint32 queue_max;
	pure64_uint32 queue_size;
	pure64_uint32 queue_notify;
};

void virtio_mmio_init(void);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* VIRTIO_MMIO_H */
