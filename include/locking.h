/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

#ifndef HC_LOCKING_H
#define HC_LOCKING_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#ifdef __cplusplus
extern "C"{
#endif
int hc_lockfile   (HCFILE *fp);
int hc_unlockfile (HCFILE *fp);
#ifdef __cplusplus
}
#endif
#endif // HC_LOCKING_H
