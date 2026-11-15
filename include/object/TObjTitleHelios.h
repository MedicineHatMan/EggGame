#ifndef _TOBJTITLEHELIOS_H
#define _TOBJTITLEHELIOS_H

#include "engine/DrawTask.h"
#include "engine/MotionPlayer.h"
#include "egg/ObjEgg.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"

extern "C" void objHeapFree(void*, void*);

/**
 * The title-screen sun. Built without a set record. It rides path 1, and the path speed depends on gTitleScene:
 * 2000, 4000 or 6000 for 0, 1 and 2. Mode 3 hides it. Mode 4 hides it for the first 600 frames. Otherwise it turns
 * to face the camera. The lens flare's alpha tracks one minus the landscape blend, and drops to 0 in mode 3.
 *
 * @size 0x13C
 */
class TObjTitleHelios : public TSetActor {
public:
	TObjTitleHelios(TObject* parent);
	virtual ~TObjTitleHelios();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void vfn_74();                    // _74

	void setup();           // ours
	void ride();            // ours
	void setPace(int pace); // ours path speed is pace times 10/9

	DrawTask* mDraw;      // _64
	ObjModel mModel;      // _68
	MotionPlayer mMotion; // _88
	EggCollide mPath;     // _C4 path 1
	f32 mMatrix[12];      // _E0
	int _110;             // _110 stored 0
	u8 _114[0x10];        // _114
	u32 mColor;           // _124 the flare tint
	u8 _128[8];           // _128
	int _130;             // _130 stored 0
	void* mFlare;         // _134
	int mTimer;           // _138 counts in mode 4
};

class TObject;

extern "C" {
TObjTitleHelios* createTObjTitleHelios(TObject*);
}

extern "C" {
extern const char* gTObjTitleHeliosName;
extern const MotionEntry sTitleHeliosMotions[];
}

#endif
