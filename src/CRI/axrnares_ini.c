#include <sdk_old/types.h>
#include <CRI/axrna.h>
#include <CRI/adx.h>

AXRNARES_OBJ axrnares_obj[AXRNARES_MAX];
u32 axrnares_base;
u32 axrnares_size;
u32 axrnares_num;
u32 axrnares_ext_buf; /* set when the buffers were given from outside so Finish leaves ARAM alone */
u32 axrnares_init_cnt;

/* AXRNARES_Destroy inlined the DOL clears each handle in place */
static inline void axrnares_destroy(AXRNARES res)
{
	if (res == 0) {
		return;
	}
	res->used = 0;
}

// 0x80230410
void AXRNARES_Finish(void)
{
	u32 len;
	int i;

	if (--axrnares_init_cnt != 0) {
		return;
	}
	for (i = 0; i < AXRNARES_MAX; i++) {
		if (axrnares_obj[i].used == 1) {
			axrnares_destroy(&axrnares_obj[i]);
		}
	}
	memset(axrnares_obj, 0, sizeof(axrnares_obj));
	if (axrnares_ext_buf == 0) {
		ARFree(&len);
		if (len != axrnares_size) {
			axrna_call_errfn("E1090601:Free area other than ADX buffer.\n");
		}
		axrnares_num  = 0;
		axrnares_size = 0;
		axrnares_base = 0;
	}
}

// 0x8023057C
void AXRNARES_Init(void)
{
	AXRNARES res;
	u32 i;

	if (axrnares_init_cnt == 0) {
		if (axrnares_ext_buf == 0) {
			axrnares_num  = AXRNARES_MAX;
			axrnares_size = AXRNARES_SIZE;
			axrnares_base = ARAlloc(AXRNARES_SIZE);
		}
		memset(axrnares_obj, 0, sizeof(axrnares_obj));
		res = axrnares_obj;
		for (i = 0; i < axrnares_num; i++) {
			res->adr = (axrnares_base + i * AXRNARES_BUF) >> 1;
			res->len = AXRNARES_BUF / 2;
			res++;
		}
	}
	axrnares_init_cnt++;
}
