#ifndef _TOBJLASTBOSSIND_H
#define _TOBJLASTBOSSIND_H

#include "stage/SetManager.h"
struct DrawTask;
#include "engine/ScrollModel.h"
#include "Ninja/ninja.h"

extern "C" {
void objHeapFree(void*, void*);
extern void* gObjHeap;

extern int* gLastBossIndModels; // 0x802C5840 model numbers the markers pick from
extern int gLastBossIndCount;   // 0x802C5844
extern int gLastBossIndTex;     // 0x802C5848 the texture list the markers draw with
extern int gLastBossIndShown;   // 0x802C584C markers hide while this is 0

void lastBossLoad();                                            // 0x8016C7E8 obj_last_boss
void lastBossFree();                                            // 0x8016C820
void lastExBossLoad();                                          // 0x8016C844 obj_last_ex_boss
void lastExBossFree();                                          // 0x8016C878
NJS_OBJECT* lastBossGetModel(int no);                           // 0x8016C898
NJS_TEXLIST* lastBossGetTexList(int no);                        // 0x8016C8A8
void lastBossIndShow();                                         // 0x8016C980
void lastBossIndHide();                                         // 0x8016C98C
TObject* createTObjLastBossInd(TObject* parent, SetDesc* desc); // 0x8016C92C
}

class TObjLastBossInd : public TSetObj {
public:
	TObjLastBossInd(TObject* parent, SetDesc* desc);
	virtual ~TObjLastBossInd();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	ScrollModel mSurface; // _90
	NJS_OBJECT* mModel;   // _B0
	f32 mRadius;          // _B4
	DrawTask* mDraw;      // _B8
};

typedef char TObjLastBossInd_size_check[(sizeof(TObjLastBossInd) == 0xBC) ? 1 : -1];

extern "C" {
void lastBossEggBreak();
void lastBossEggClose();
void lastBossEggFlash();
void lastBossIndExit();
void lastBossIndInit();
}

extern "C" {
f32 lastBossModelRadius(const NJS_OBJECT*);
}

extern "C" {
extern int gLastBossEggOpen;
}

#endif
