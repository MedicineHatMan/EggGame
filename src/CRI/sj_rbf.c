#include <CRI/sj.h>
#include <CRI/adx.h>

#define SJRBF_MAX_OBJ 256

typedef struct {
	SJ_IF* vtbl;      /* 00 */
	int used;         /* 04 */
	const UUID* uuid; /* 08 */
	int num_data;     /* 0C */
	int num_free;     /* 10 */
	int free_pos;     /* 14 */
	int data_pos;     /* 18 */
	char* buf;        /* 1C */
	int bsize;        /* 20 */
	int xsize;        /* 24 */
	struct {
		int cnt[2];
	} line[2];      /* 28 */
	SJ_ERRFN errfn; /* 38 */
	void* errobj;   /* 3C */
} SJRBF_OBJ;

void SJRBF_Destroy(SJ sj);
const UUID* SJRBF_GetUuid(SJ sj);
void SJRBF_Reset(SJ sj);
void SJRBF_GetChunk(SJ sj, int id, int nbyte, SJCK* ck);
void SJRBF_UngetChunk(SJ sj, int id, SJCK* ck);
void SJRBF_PutChunk(SJ sj, int id, SJCK* ck);
int SJRBF_GetNumData(SJ sj, int id);
int SJRBF_IsGetChunk(SJ sj, int id, int nbyte, int* rbyte);
void SJRBF_EntryErrFunc(SJ sj, SJ_ERRFN func, void* obj);

static const char* const volatile sj_build = "\nSJ/GC Ver.6.10 Build:Mar  3 2003 16:46:58\n";
static const UUID sjrbf_uuid               = { 0x3B9A9E81, 0x0DBB, 0x11D2, { 0xA6, 0xBF, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

static SJ_IF sjrbf_if = {
	0,
	0,
	0,
	SJRBF_Destroy,
	SJRBF_GetUuid,
	SJRBF_Reset,
	SJRBF_GetChunk,
	SJRBF_UngetChunk,
	SJRBF_PutChunk,
	SJRBF_GetNumData,
	SJRBF_IsGetChunk,
	SJRBF_EntryErrFunc,
};

static SJRBF_OBJ sjrbf_obj[SJRBF_MAX_OBJ];
static int sjrbf_init_cnt;

// 0x8022CEF4
static void sjrbf_DefErrFunc(void* obj, int ecode)
{
	for (;;) { }
}

// 0x8022CE94
void SJRBF_Init(void)
{
	sj_build;
	if (sjrbf_init_cnt == 0) {
		memset(sjrbf_obj, 0, sizeof(sjrbf_obj));
	}
	sjrbf_init_cnt++;
}

// 0x8022CE4C
void SJRBF_Finish(void)
{
	if (--sjrbf_init_cnt == 0) {
		memset(sjrbf_obj, 0, sizeof(sjrbf_obj));
	}
}

// 0x8022CCBC
SJ SJRBF_Create(char* buf, int bsize, int xsize)
{
	int i;
	SJRBF_OBJ* rbf;

	SJCRS_Lock();
	for (i = 0; i < SJRBF_MAX_OBJ; i++) {
		if (sjrbf_obj[i].used == 0) {
			break;
		}
	}
	if (i == SJRBF_MAX_OBJ) {
		rbf = 0;
	} else {
		rbf                 = &sjrbf_obj[i];
		rbf->used           = 1;
		rbf->vtbl           = &sjrbf_if;
		rbf->buf            = buf;
		rbf->bsize          = bsize;
		rbf->xsize          = xsize;
		rbf->uuid           = &sjrbf_uuid;
		rbf->errfn          = sjrbf_DefErrFunc;
		rbf->errobj         = rbf;
		rbf->num_data       = 0;
		rbf->num_free       = rbf->bsize;
		rbf->free_pos       = 0;
		rbf->data_pos       = 0;
		rbf->line[0].cnt[0] = 0;
		rbf->line[0].cnt[1] = 0;
		rbf->line[1].cnt[0] = 0;
		rbf->line[1].cnt[1] = 0;
	}
	SJCRS_Unlock();
	return (SJ)rbf;
}

// 0x8022CC6C
void SJRBF_Destroy(SJ sj)
{
	SJRBF_OBJ* rbf = (SJRBF_OBJ*)sj;

	SJCRS_Lock();
	if (rbf != 0) {
		memset(rbf, 0, sizeof(SJRBF_OBJ));
		rbf->used = 0;
	}
	SJCRS_Unlock();
}

// 0x8022CC64
const UUID* SJRBF_GetUuid(SJ sj)
{
	return ((SJRBF_OBJ*)sj)->uuid;
}

// 0x8022CC58
void SJRBF_EntryErrFunc(SJ sj, SJ_ERRFN func, void* obj)
{
	SJRBF_OBJ* rbf = (SJRBF_OBJ*)sj;

	rbf->errfn  = func;
	rbf->errobj = obj;
}

// 0x8022CC2C
void SJRBF_Reset(SJ sj)
{
	SJRBF_OBJ* rbf = (SJRBF_OBJ*)sj;

	rbf->num_data       = 0;
	rbf->num_free       = rbf->bsize;
	rbf->free_pos       = 0;
	rbf->data_pos       = 0;
	rbf->line[0].cnt[0] = 0;
	rbf->line[0].cnt[1] = 0;
	rbf->line[1].cnt[0] = 0;
	rbf->line[1].cnt[1] = 0;
}

// 0x8022CBD0
int SJRBF_GetNumData(SJ sj, int id)
{
	SJRBF_OBJ* rbf = (SJRBF_OBJ*)sj;

	if (id == SJ_LIN_DATA) {
		return rbf->num_data;
	}
	if (id == SJ_LIN_FREE) {
		return rbf->num_free;
	}
	if (rbf->errfn != 0) {
		rbf->errfn(rbf->errobj, SJ_ERR_PRM);
	}
	return 0;
}

// 0x8022CA1C
void SJRBF_GetChunk(SJ sj, int id, int nbyte, SJCK* ck)
{
	SJRBF_OBJ* rbf = (SJRBF_OBJ*)sj;
	int room;

	SJCRS_Lock();
	if (id == SJ_LIN_FREE) {
		room          = rbf->xsize + (rbf->bsize - rbf->free_pos);
		ck->len       = (rbf->num_free < room) ? rbf->num_free : room;
		ck->len       = (ck->len < nbyte) ? ck->len : nbyte;
		ck->data      = rbf->buf + rbf->free_pos;
		rbf->free_pos = (rbf->free_pos + ck->len) % rbf->bsize;
		rbf->num_free -= ck->len;
		rbf->line[0].cnt[0] += ck->len;
	} else if (id == SJ_LIN_DATA) {
		room          = rbf->xsize + (rbf->bsize - rbf->data_pos);
		ck->len       = (rbf->num_data < room) ? rbf->num_data : room;
		ck->len       = (ck->len < nbyte) ? ck->len : nbyte;
		ck->data      = rbf->buf + rbf->data_pos;
		rbf->data_pos = (rbf->data_pos + ck->len) % rbf->bsize;
		rbf->num_data -= ck->len;
		rbf->line[1].cnt[0] += ck->len;
	} else {
		ck->len  = 0;
		ck->data = 0;
		if (rbf->errfn != 0) {
			rbf->errfn(rbf->errobj, SJ_ERR_PRM);
		}
	}
	SJCRS_Unlock();
}

// 0x8022C8C0
void SJRBF_PutChunk(SJ sj, int id, SJCK* ck)
{
	SJRBF_OBJ* rbf = (SJRBF_OBJ*)sj;
	int ofst;
	int over;

	if (ck->len <= 0 || ck->data == 0) {
		return;
	}
	SJCRS_Lock();
	if (id == SJ_LIN_DATA) {
		rbf->num_data += ck->len;
		ofst = ck->data - rbf->buf;
		if (ofst < rbf->xsize) {
			over = rbf->xsize - ofst;
			if (ck->len < over) {
				over = ck->len;
			}
			/* Mirror into the extra area past the end */
			memcpy(&rbf->buf[rbf->bsize + ofst], ck->data, over);
		}
		ofst = ck->len + (ck->data - rbf->buf);
		if (ofst > rbf->bsize) {
			over = ofst - rbf->bsize;
			if (ck->len < over) {
				over = ck->len;
			}
			memcpy(rbf->buf, rbf->buf + (ofst - over), over);
		}
		rbf->line[1].cnt[1] += ck->len;
	} else if (id == SJ_LIN_FREE) {
		rbf->num_free += ck->len;
		rbf->line[0].cnt[1] += ck->len;
	} else {
		ck->len  = 0;
		ck->data = 0;
		if (rbf->errfn != 0) {
			rbf->errfn(rbf->errobj, SJ_ERR_PRM);
		}
	}
	SJCRS_Unlock();
}

// 0x8022C714
void SJRBF_UngetChunk(SJ sj, int id, SJCK* ck)
{
	SJRBF_OBJ* rbf = (SJRBF_OBJ*)sj;
	int pos;

	if (ck->len <= 0 || ck->data == 0) {
		return;
	}
	SJCRS_Lock();
	if (id == SJ_LIN_FREE) {
		pos = (rbf->free_pos + rbf->bsize - ck->len) % rbf->bsize;
		if (pos == (ck->data - rbf->buf) % rbf->bsize) {
			rbf->free_pos = pos;
			rbf->num_free += ck->len;
		} else if (rbf->errfn != 0) {
			rbf->errfn(rbf->errobj, SJ_ERR_PRM);
		}
		rbf->line[0].cnt[0] -= ck->len;
	} else if (id == SJ_LIN_DATA) {
		pos = (rbf->data_pos + rbf->bsize - ck->len) % rbf->bsize;
		if (pos == (ck->data - rbf->buf) % rbf->bsize) {
			rbf->data_pos = pos;
			rbf->num_data += ck->len;
		} else if (rbf->errfn != 0) {
			rbf->errfn(rbf->errobj, SJ_ERR_PRM);
		}
		rbf->line[1].cnt[0] -= ck->len;
	} else {
		ck->len  = 0;
		ck->data = 0;
		if (rbf->errfn != 0) {
			rbf->errfn(rbf->errobj, SJ_ERR_PRM);
		}
	}
	SJCRS_Unlock();
}

// 0x8022C608
int SJRBF_IsGetChunk(SJ sj, int id, int nbyte, int* rbyte)
{
	SJRBF_OBJ* rbf = (SJRBF_OBJ*)sj;
	int room;

	SJCRS_Lock();
	if (id == SJ_LIN_FREE) {
		room = rbf->xsize + (rbf->bsize - rbf->free_pos);
		if (rbf->num_free < room) {
			room = rbf->num_free;
		}
		id = (room < nbyte) ? room : nbyte;
	} else if (id == SJ_LIN_DATA) {
		room = rbf->xsize + (rbf->bsize - rbf->data_pos);
		if (rbf->num_data < room) {
			room = rbf->num_data;
		}
		id = (room < nbyte) ? room : nbyte;
	} else {
		id = 0;
		if (rbf->errfn != 0) {
			rbf->errfn(rbf->errobj, SJ_ERR_PRM);
		}
	}
	*rbyte = id;
	SJCRS_Unlock();
	if (id == nbyte) {
		return 1;
	}
	return 0;
}

// 0x8022C600
char* SJRBF_GetBufPtr(SJ sj)
{
	return ((SJRBF_OBJ*)sj)->buf;
}

// 0x8022C5F8
int SJRBF_GetBufSize(SJ sj)
{
	return ((SJRBF_OBJ*)sj)->bsize;
}

// 0x8022C5F0
int SJRBF_GetXtrSize(SJ sj)
{
	return ((SJRBF_OBJ*)sj)->xsize;
}

// 0x8022C5D8
int SJRBF_GetCnt(SJ sj, int id, int no)
{
	SJRBF_OBJ* rbf = (SJRBF_OBJ*)sj;
	return rbf->line[id].cnt[no];
}
