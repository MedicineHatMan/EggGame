#ifndef _OBJECT_TOBJHELIOS_H_
#define _OBJECT_TOBJHELIOS_H_
#include "engine/DrawTask.h"
#include "engine/ModelDraw.h"
#include "engine/TLensFlare.h"
#include "stage/Stage.h"
struct TObjHelios;
struct TObject;
struct Vec3f;

#include "core/types.h"

extern "C" {
void initTObjHelios();
void lastExBossFeatherExit();
void lastExBossFeatherInit();
void termTObjHelios();
}

extern "C" {
int eventSol(f32*);
}

extern "C" {
TObjHelios* createTObjHelios(TObject*, const Vec3f*);
}

class TObjHelios : public TObject {
public:
	TObjHelios(TObject* parent, const Vec3f* pos);
	virtual ~TObjHelios();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10
	virtual void render(); // _1C

	DrawTask* mDraw;     // _20
	ModelDraw mModel;    // _24
	int mFlags;          // _40
	u8 _44[0x10];
	u32 mColor;          // _54
	u8 _58[8];
	int mZero;           // _60
	MotionTrack mTrack;  // _64
	Vec3f mPos;          // _80
	f32 mMtx[12];        // _8C
	void* mEffect;       // _BC
	TLensFlare* mFlare;  // _C0
};

#endif
