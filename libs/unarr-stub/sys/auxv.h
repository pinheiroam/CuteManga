/* Stub for Nintendo Switch (devkitA64 has no sys/auxv.h).
 * Provides getauxval/AT_HWCAP so lzmasdk/CpuArch.c compiles.
 * Switch (ARMv8) has NEON, CRC32, SHA1, SHA2, AES; we report all supported. */
#ifndef CUTEMANGA_UNARR_STUB_AUXV_H
#define CUTEMANGA_UNARR_STUB_AUXV_H

#define AT_HWCAP  16
#define AT_HWCAP2 26

static inline unsigned long getauxval(unsigned long type)
{
	(void)type;
	/* Report all features supported (Switch is ARMv8). */
	return (unsigned long)-1;
}

#endif
