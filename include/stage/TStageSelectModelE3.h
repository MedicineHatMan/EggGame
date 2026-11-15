#ifndef _TSTAGESELECTMODELE3_H
#define _TSTAGESELECTMODELE3_H
struct TArc;
#include "Ninja/ninja.h"
struct DrawTask;
struct E3Assets;
#include "core/types.h"
struct E3Palette;

#include "stage/Stage.h"

extern "C" void objHeapFree(void*, void*);

/**
 * One spinning model on the E3 stage-select screen. The arc is menu_model.arc.
 * draw() advances the Y angle by 0x100 and wraps mFrame by the motion's length
 * at +0x04. render() concatenates the camera with mMtx and draws that motion.
 *
 * @size 0x98
 */
class TStageSelectModelE3 : public TObject {
public:
	TStageSelectModelE3(TObject* parent, const Vec3f* pos, int angle, int index);
	virtual ~TStageSelectModelE3();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10 empty
	virtual void render(); // _1C

	void resetFrame();   // 0x801562EC
	void updateMatrix(); // 0x80156440

	int mIndex;      // _20 which model and motion
	f32 mMtx[12];    // _24
	Vec3f mPos;      // _54
	int mAngle[3];   // _60 Ninja angles draw() spins [1]
	DrawTask* mDraw; // _6C
	u32 _70;         // _70
	u8 _74[0x1C];    // _74
	u32 _90;         // _90
	f32 mFrame;      // _94
};

class TObject;
struct Vec3f;

extern "C" {
TStageSelectModelE3* createTStageSelectModelE3(TObject*, const Vec3f*, int, int);
void freeE3ModelArc();
void loadE3ModelArc();
}

// Palette block handed to 0x80156090
struct E3Palette {
	void* mData; // _00
	u16 mStart;  // _04
	u8 _06[4];   // _06
	s16 mCount;  // _0A
};

// Bank inside the arc at file+0x4C
struct E3Assets {
	void* _00;       // _00
	void** mModels;  // _04
	void** mMotions; // _08 each motion's length is a u32 at +0x04
	void** mTex;     // _0C
	void* mGvm;      // _10
};

extern "C" {
extern u8* gTimberBrickObj;
extern void* gSandFloorObj;
extern const char* gTStageSelectModelE3Name;
extern TArc* sStageSelectModelE3Arc;
extern E3Assets* sE3AssetsData;
}

#endif
