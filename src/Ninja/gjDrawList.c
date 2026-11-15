#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

typedef struct GjDrawEntry {
	s32 env;     // _00 loads the environment matrix too
	void* model; // _04
} GjDrawEntry;

typedef struct GjDrawList {
	u32 count;            // _00
	void (*draw)(void*);  // _04
	GjDrawEntry* entries; // _08
	f32 (*mtx)[3][4];     // _0C
} GjDrawList;

GjDrawEntry gjDrawEntries[10];
u8 gjDrawMatrices[0x1E0];
GjDrawList gjDrawList = { 0, 0, gjDrawEntries, (f32 (*)[3][4])gjDrawMatrices };

void gjDrawListDraw(u32 count);

// 0x80211B04
void gjDrawListBegin(s32 (*f)(GjModel*))
{
	gjDrawList.count = 0;
	gjDrawList.draw  = (void (*)(void*))f;
}

// 0x80211B1C
// Queues a model with its matrix
void gjDrawListAdd(NJS_OBJECT* obj, const f32* m)
{
	GjDrawList* list = &gjDrawList;
	u16 i            = list->count;

	GXLoadPosMtxImm(m, i * 3);
	GXLoadNrmMtxImm(m, i * 3);
	if (obj->evalflags & 0x4000) {
		PSMTXCopy(m, (f32*)list->mtx[i]);
		list->mtx[i][0][3]   = 0.0f;
		list->mtx[i][1][3]   = 0.0f;
		list->mtx[i][2][3]   = 0.0f;
		list->entries[i].env = 1;
	} else {
		list->entries[i].env = 0;
	}
	list->entries[i].model = obj->model;
	if (++list->count == 10) {
		gjDrawListDraw(10);
		list->count = 0;
	}
}

// 0x80211C24
void gjDrawListFlush(void)
{
	if (gjDrawList.count != 0) {
		gjDrawListDraw(gjDrawList.count);
	}
}

// 0x80211C58
void gjDrawListDraw(u32 count)
{
	GjDrawList* list;
	u8* m;
	GjDrawEntry* e;
	u32 id;

	list = &gjDrawList;
	e    = list->entries;
	id   = 0;
	m    = (u8*)list->mtx;

	while (count != 0) {
		GXSetCurrentMtx(id);
		if ((u32)e->env != 0) {
			GXLoadTexMtxImm((f32*)m, 0x2A, 0);
		}
		list->draw(e->model);
		count--;
		e++;
		m += 0x30;
		id += 3;
	}
	GXSetCurrentMtx(0);
}
