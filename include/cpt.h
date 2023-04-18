/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

#ifndef HC_CPT_H
#define HC_CPT_H

#include <stdio.h>
#include <errno.h>
#include <time.h>
#ifdef __cplusplus
extern "C"{
#endif
int  cpt_ctx_init    (hashcat_ctx_t *hashcat_ctx);
void cpt_ctx_destroy (hashcat_ctx_t *hashcat_ctx);
void cpt_ctx_reset   (hashcat_ctx_t *hashcat_ctx);
#ifdef __cplusplus
}
#endif
#endif // HC_CPT_H
