#ifndef _MSL_CONSOLE_IO_H
#define _MSL_CONSOLE_IO_H

#include "PowerPC_EABI_Support/MSL_C/MSL_Common/ansi_files.h"

#ifdef __cplusplus
extern "C" {
#endif

int __read_console(__file_handle file, char* buff, size_t* count, __ref_con ref_con);
int __write_console(__file_handle file, char* buff, size_t* count, __ref_con ref_con);
int __close_console(__file_handle file);

#ifdef __cplusplus
};
#endif

#endif
