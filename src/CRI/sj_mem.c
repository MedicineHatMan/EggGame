#include <CRI/sj.h>
#include <CRI/adx.h>

#define SJMEM_MAX_OBJ 32

typedef struct {
	SJ_IF* vtbl;      /* 00 */
	int used;         /* 04 */
	const UUID* uuid; /* 08 */
	int remain;       /* 0C */
	int pos;          /* 10 */
	char* data;       /* 14 */
	int size;         /* 18 */
	SJ_ERRFN errfn;   /* 1C */
	void* errobj;     /* 20 */
} SJMEM_OBJ;

void SJMEM_Destroy(SJ sj);
const UUID* SJMEM_GetUuid(SJ sj);
void SJMEM_Reset(SJ sj);
void SJMEM_GetChunk(SJ sj, int id, int nbyte, SJCK* ck);
void SJMEM_UngetChunk(SJ sj, int id, SJCK* ck);
void SJMEM_PutChunk(SJ sj, int id, SJCK* ck);
int SJMEM_GetNumData(SJ sj, int id);
int SJMEM_IsGetChunk(SJ sj, int id, int nbyte, int* rbyte);
void SJMEM_EntryErrFunc(SJ sj, SJ_ERRFN func, void* obj);

static const UUID sjmem_uuid = { 0xDD9EEE41, 0x1679, 0x11D2, { 0x93, 0x6C, 0x00, 0x60, 0x08, 0x94, 0x48, 0xBC } };

static SJ_IF sjmem_if = {
	0,
	0,
	0,
	SJMEM_Destroy,
	SJMEM_GetUuid,
	SJMEM_Reset,
	SJMEM_GetChunk,
	SJMEM_UngetChunk,
	SJMEM_PutChunk,
	SJMEM_GetNumData,
	SJMEM_IsGetChunk,
	SJMEM_EntryErrFunc,
};

static SJMEM_OBJ sjmem_obj[SJMEM_MAX_OBJ];
static int sjmem_init_cnt;

// 0x8022C5D4
static void sjmem_DefErrFunc(void* obj, int ecode)
{
	for (;;) { }
}

// 0x8022C580
void SJMEM_Init(void)
{
	if (sjmem_init_cnt == 0) {
		memset(sjmem_obj, 0, sizeof(sjmem_obj));
	}
	sjmem_init_cnt++;
}

// 0x8022C538
void SJMEM_Finish(void)
{
	if (--sjmem_init_cnt == 0) {
		memset(sjmem_obj, 0, sizeof(sjmem_obj));
	}
}

// 0x8022C410
SJ SJMEM_Create(char* data, int size)
{
	int i;
	SJMEM_OBJ* mem;

	for (i = 0; i < SJMEM_MAX_OBJ; i++) {
		if (sjmem_obj[i].used == 0) {
			break;
		}
	}
	if (i == SJMEM_MAX_OBJ) {
		return 0;
	}
	mem         = &sjmem_obj[i];
	mem->used   = 1;
	mem->vtbl   = &sjmem_if;
	mem->data   = data;
	mem->size   = size;
	mem->uuid   = &sjmem_uuid;
	mem->errfn  = sjmem_DefErrFunc;
	mem->errobj = mem;
	mem->remain = mem->size;
	mem->pos    = 0;
	return (SJ)mem;
}

// 0x8022C3D0
void SJMEM_Destroy(SJ sj)
{
	SJMEM_OBJ* mem = (SJMEM_OBJ*)sj;

	if (mem != 0) {
		memset(mem, 0, sizeof(SJMEM_OBJ));
		mem->used = 0;
	}
}

// 0x8022C3C8
const UUID* SJMEM_GetUuid(SJ sj)
{
	return ((SJMEM_OBJ*)sj)->uuid;
}

// 0x8022C3BC
void SJMEM_EntryErrFunc(SJ sj, SJ_ERRFN func, void* obj)
{
	SJMEM_OBJ* mem = (SJMEM_OBJ*)sj;

	mem->errfn  = func;
	mem->errobj = obj;
}

// 0x8022C3A8
void SJMEM_Reset(SJ sj)
{
	SJMEM_OBJ* mem = (SJMEM_OBJ*)sj;

	mem->remain = mem->size;
	mem->pos    = 0;
}

// 0x8022C34C
int SJMEM_GetNumData(SJ sj, int id)
{
	SJMEM_OBJ* mem = (SJMEM_OBJ*)sj;

	if (id == SJ_LIN_DATA) {
		return mem->remain;
	}
	if (id == SJ_LIN_FREE) {
		return 0;
	}
	if (mem->errfn != 0) {
		mem->errfn(mem->errobj, SJ_ERR_PRM);
	}
	return 0;
}

// 0x8022C264
void SJMEM_GetChunk(SJ sj, int id, int nbyte, SJCK* ck)
{
	SJMEM_OBJ* mem = (SJMEM_OBJ*)sj;

	SJCRS_Lock();
	if (id == SJ_LIN_FREE) {
		ck->len  = 0;
		ck->data = 0;
	} else if (id == SJ_LIN_DATA) {
		ck->len  = (mem->remain < nbyte) ? mem->remain : nbyte;
		ck->data = mem->data + mem->pos;
		mem->pos += ck->len;
		mem->remain -= ck->len;
	} else {
		ck->len  = 0;
		ck->data = 0;
		if (mem->errfn != 0) {
			mem->errfn(mem->errobj, SJ_ERR_PRM);
		}
	}
	SJCRS_Unlock();
}

// 0x8022C1D0
void SJMEM_PutChunk(SJ sj, int id, SJCK* ck)
{
	SJMEM_OBJ* mem = (SJMEM_OBJ*)sj;

	if (ck->len <= 0 || ck->data == 0) {
		return;
	}
	SJCRS_Lock();
	if ((unsigned int)id > SJ_LIN_DATA) {
		ck->len  = 0;
		ck->data = 0;
		if (mem->errfn != 0) {
			mem->errfn(mem->errobj, SJ_ERR_PRM);
		}
	}
	SJCRS_Unlock();
}

// 0x8022C0A0
void SJMEM_UngetChunk(SJ sj, int id, SJCK* ck)
{
	SJMEM_OBJ* mem = (SJMEM_OBJ*)sj;
	int pos;
	int remain;

	if (ck->len <= 0 || ck->data == 0) {
		return;
	}
	SJCRS_Lock();
	if (id == SJ_LIN_FREE) {
		if (mem->errfn != 0) {
			mem->errfn(mem->errobj, SJ_ERR_PRM);
		}
	} else if (id == SJ_LIN_DATA) {
		pos         = (mem->pos - ck->len > 0L) ? mem->pos - ck->len : 0L;
		mem->pos    = pos;
		remain      = mem->remain + ck->len;
		mem->remain = (mem->size < remain) ? mem->size : remain;
		if (pos != ck->data - mem->data) {
			if (mem->errfn != 0) {
				mem->errfn(mem->errobj, SJ_ERR_PRM);
			}
		}
	} else {
		ck->len  = 0;
		ck->data = 0;
		if (mem->errfn != 0) {
			mem->errfn(mem->errobj, SJ_ERR_PRM);
		}
	}
	SJCRS_Unlock();
}

// 0x8022BFE8
int SJMEM_IsGetChunk(SJ sj, int id, int nbyte, int* rbyte)
{
	SJMEM_OBJ* mem = (SJMEM_OBJ*)sj;
	int n;

	SJCRS_Lock();
	if (id == SJ_LIN_FREE) {
		n = 0;
	} else if (id == SJ_LIN_DATA) {
		n = (mem->remain < nbyte) ? mem->remain : nbyte;
	} else {
		n = 0;
		if (mem->errfn != 0) {
			mem->errfn(mem->errobj, SJ_ERR_PRM);
		}
	}
	*rbyte = n;
	SJCRS_Unlock();
	if (n == nbyte) {
		return 1;
	}
	return 0;
}
