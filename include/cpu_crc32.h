/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

#ifndef HC_CPU_CRC32_H
#define HC_CPU_CRC32_H
#ifdef __cplusplus
extern "C"{
#endif
#include <stdio.h>
#include <errno.h>

int cpu_crc32 (const char *filename, u8 *keytab, const size_t keytabsz);
u32 cpu_crc32_buffer (const u8 *buf, const size_t length);
#ifdef __cplusplus
}
#endif
#endif // HC_CPU_CRC32_H
