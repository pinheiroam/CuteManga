/* Stub for Nintendo Switch (devkitA64 has no asm/hwcap.h).
 * Linux aarch64 HWCAP bits so lzmasdk/CpuArch.c compiles.
 * Switch (ARMv8) has ASIMD (NEON), CRC32, SHA1, SHA2, AES. */
#ifndef CUTEMANGA_UNARR_STUB_HWCAP_H
#define CUTEMANGA_UNARR_STUB_HWCAP_H

/* aarch64 HWCAP bits (match Linux uapi asm/hwcap.h) */
#define HWCAP_ASIMD   (1 << 1)
#define HWCAP_CRC32   (1 << 7)
#define HWCAP_SHA1    (1 << 2)
#define HWCAP_SHA2    (1 << 3)
#define HWCAP_AES     (1 << 4)

/* ARM 32-bit HWCAP2 (unarr may use for MY_CPU_ARM) */
#define HWCAP2_NEON   (1 << 0)

#endif
