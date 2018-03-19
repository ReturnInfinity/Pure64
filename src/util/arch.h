#ifndef PURE64_ARCH_H
#define PURE64_ARCH_H

#ifdef __cplusplus
extern "C" {
#endif

/** Enumerates all of
 * the supported architectures.
 * */

enum pure64_arch {
	/** This means no architecture
	 * was specified. */
	PURE64_ARCH_NONE,
	/** This means the architecture is x86_64. */
	PURE64_ARCH_x86_64
};

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_ARCH_H */
