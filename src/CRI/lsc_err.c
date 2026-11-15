#include <CRI/lsc.h>
#include <core/types.h>
#include <stdarg.h>
#include <CRI/cri_std.h>

static char lsc_err_msg[0x100];
static void* lsc_err_obj;
static void (*lsc_err_func)(void*, char*);

// 0x8022B170
void LSC_EntryErrFunc(void (*func)(void*, char*), void* obj)
{
	if (func == 0) {
		lsc_err_func = 0;
		lsc_err_obj = 0;
		return;
	}
	lsc_err_func = func;
	lsc_err_obj = obj;
}

// 0x8022B0B0
void LSC_CallErrFunc(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf(lsc_err_msg, fmt, args);
	va_end(args);
	if (lsc_err_func != 0) {
		lsc_err_func(lsc_err_obj, lsc_err_msg);
	}
}
