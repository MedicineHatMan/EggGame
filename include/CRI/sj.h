#include <CRI/cri_types.h>
#ifndef CRI_SJ_H
#define CRI_SJ_H

#define SJ_LIN_FREE 0
#define SJ_LIN_DATA 1
#define SJ_ERR_PRM (-3)

typedef struct {
	unsigned int Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char Data4[8];
} UUID;

typedef struct {
	char* data;
	int len;
} SJCK;

typedef void (*SJ_ERRFN)(void* obj, int ecode);

struct _sj_vtbl;
typedef struct _sj_obj {
	struct _sj_vtbl* vtbl;
} SJ_OBJ;

typedef struct _sj_vtbl {
	void (*QueryInterface)();
	void (*AddRef)();
	void (*Release)();
	void (*Destroy)(SJ sj);
	const UUID* (*GetUuid)(SJ sj);
	void (*Reset)(SJ sj);
	void (*GetChunk)(SJ sj, int id, int nbyte, SJCK* ck);
	void (*UngetChunk)(SJ sj, int id, SJCK* ck);
	void (*PutChunk)(SJ sj, int id, SJCK* ck);
	int (*GetNumData)(SJ sj, int id);
	int (*IsGetChunk)(SJ sj, int id, int nbyte, int* rbyte);
	void (*EntryErrFunc)(SJ sj, SJ_ERRFN func, void* obj);
} SJ_IF;

#define SJ_Destroy(sj) ((sj)->vtbl->Destroy((sj)))
#define SJ_GetUuid(sj) ((sj)->vtbl->GetUuid((sj)))
#define SJ_Reset(sj) ((sj)->vtbl->Reset((sj)))
#define SJ_GetChunk(sj, id, nbyte, ck) ((sj)->vtbl->GetChunk((sj), (id), (nbyte), (ck)))
#define SJ_UngetChunk(sj, id, ck) ((sj)->vtbl->UngetChunk((sj), (id), (ck)))
#define SJ_PutChunk(sj, id, ck) ((sj)->vtbl->PutChunk((sj), (id), (ck)))
#define SJ_GetNumData(sj, id) ((sj)->vtbl->GetNumData((sj), (id)))
#define SJ_IsGetChunk(sj, id, nbyte, rbyte) ((sj)->vtbl->IsGetChunk((sj), (id), (nbyte), (rbyte)))
#define SJ_EntryErrFunc(sj, f, o) ((sj)->vtbl->EntryErrFunc((sj), (f), (o)))

void SJCRS_Lock(void);
void SJCRS_Unlock(void);

void SJ_SplitChunk(SJCK* ck, int nbyte, SJCK* ck1, SJCK* ck2);
SJ SJRBF_Create(char* buf, int bsize, int xsize);

SJ SJMEM_Create(char* data, int size);
void SJUNI_Init(void);
void SJUNI_Finish(void);
void SJRBF_Init(void);
void SJRBF_Finish(void);
void SJMEM_Init(void);
void SJMEM_Finish(void);

#endif
