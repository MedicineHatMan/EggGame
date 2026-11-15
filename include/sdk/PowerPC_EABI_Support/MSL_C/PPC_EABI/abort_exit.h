#ifndef _MSL_ABORT_EXIT_H
#define _MSL_ABORT_EXIT_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void (*__stdio_exit)(void);

void abort(void);
void _ExitProcess(void);

#ifdef __cplusplus
};
#endif

#endif
