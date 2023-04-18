/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

#ifndef HC_BITMAP_H
#define HC_BITMAP_H

#include <string.h>
#ifdef __cplusplus
extern "C"{
#endif
int  bitmap_ctx_init    (hashcat_ctx_t *hashcat_ctx);
void bitmap_ctx_destroy (hashcat_ctx_t *hashcat_ctx);
#ifdef __cplusplus
}
#endif
#endif // HC_BITMAP_H
