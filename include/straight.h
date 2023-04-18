/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

#ifndef HC_STRAIGHT_H
#define HC_STRAIGHT_H

#include <string.h>

#define INCR_DICTS 1000
#ifdef __cplusplus
extern "C"{
#endif
int  straight_ctx_update_loop (hashcat_ctx_t *hashcat_ctx);
int  straight_ctx_init        (hashcat_ctx_t *hashcat_ctx);
void straight_ctx_destroy     (hashcat_ctx_t *hashcat_ctx);
#ifdef __cplusplus
}
#endif
#endif // HC_STRAIGHT_H
