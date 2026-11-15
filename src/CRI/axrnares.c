#include <sdk_old/types.h>
#include <CRI/axrna.h>
#include <CRI/adx.h>

extern AXRNARES_OBJ axrnares_obj[AXRNARES_MAX];

// 0x80230308
AXRNARES AXRNARES_Create(void)
{
	AXRNARES res;
	int i;

	for (i = 0; i < AXRNARES_MAX; i++) {
		if (axrnares_obj[i].used == 0) {
			break;
		}
	}
	if (i == AXRNARES_MAX) {
		axrna_call_errfn("E1070313:Not enough RNARES handle.\n");
		return 0;
	}
	res       = &axrnares_obj[i];
	res->used = 1;
	return res;
}

// 0x802302F4
void AXRNARES_Destroy(AXRNARES res)
{
	if (res == 0) {
		return;
	}
	res->used = 0;
}

// 0x802302DC
u32 AXRNARES_GetAdr(AXRNARES res)
{
	if (res == 0) {
		return 0;
	}
	return res->adr;
}

// 0x802302C4
u32 AXRNARES_GetLen(AXRNARES res)
{
	if (res == 0) {
		return 0;
	}
	return res->len;
}

// 0x802302A4
void AXRNA_EntryErrFunc(void (*func)(void*, char*), void* obj)
{
	axrna_entry_errfn(func, obj);
}
