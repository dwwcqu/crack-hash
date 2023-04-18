/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

#ifndef HC_OUTFILE_CHECK_H
#define HC_OUTFILE_CHECK_H

#include <unistd.h>
#include <errno.h>

#define OUTFILES_DIR "outfiles"
#ifdef __cplusplus
extern "C"{
#endif
HC_API_CALL void *thread_outfile_remove (void *p);

int  outcheck_ctx_init    (hashcat_ctx_t *hashcat_ctx);
void outcheck_ctx_destroy (hashcat_ctx_t *hashcat_ctx);
#ifdef __cplusplus
}
#endif
#endif // HC_OUTFILE_CHECK_H
