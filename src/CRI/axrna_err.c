#include <sdk_old/types.h> /* the old SDK headers below need its macros */
#include <CRI/sj.h>
#include "Dolphin/ax.h"
#include "Dolphin/mix.h"

#include <CRI/axrna.h>
#include <CRI/adx.h>
#include <CRI/cri_std.h>

static char axrna_err_msg[0x100];
static void* axrna_err_obj;
static void (*axrna_err_func)(void* obj, char* msg);

// 0x8022EAA4
void axrna_lock(void)
{
	SJCRS_Lock();
}

// 0x8022EA84
void axrna_unlock(void)
{
	SJCRS_Unlock();
}

// 0x8022EA70
void axrna_entry_errfn(void (*func)(void*, char*), void* obj)
{
	axrna_err_func = func;
	axrna_err_obj = obj;
}

// 0x8022EA18
void axrna_call_errfn(const char* msg)
{
	strncpy(axrna_err_msg, msg, sizeof(axrna_err_msg) - 1);
	if (axrna_err_func != 0) {
		axrna_err_func(axrna_err_obj, axrna_err_msg);
	}
}
