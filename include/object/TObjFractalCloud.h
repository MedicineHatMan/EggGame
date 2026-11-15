#ifndef _OBJECT_TOBJFRACTALCLOUD_H_
#define _OBJECT_TOBJFRACTALCLOUD_H_
#include "Ninja/ninja.h"
#include "engine/DrawTask.h"
#include "engine/NoiseTex.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct FractalPuff;
struct SetDesc;
struct TObjFractalCloud;
struct TObject;

#include "core/types.h"

extern "C" {
void initTObjFractalCloud();
void termTObjFractalCloud();
}

extern "C" {
TObject* createTObjFractalCloud(TObject*, SetDesc*);
}

/**
 * The cloud's look: its noise texture and how the octaves of it are layered. The name is ours.
 *
 * @size 0x34
 */
struct FractalPuff {
	FractalPuff();  // 0x80148014
	~FractalPuff(); // 0x80148068
	bool build();   // 0x801480C0

	void setOctaves(int count);        // 0x80148134 1 to 4
	void fill();                      // 0x80148160
	void scroll();                    // 0x801481A8
	void setColor(u8 r, u8 g, u8 b, u8 a); // 0x8014822C
	void draw();                      // 0x80148240

	NoiseTex* mTex;   // _00
	f32 _04;          // _04
	f32 _08;          // _08
	f32 mSpeedX;      // _0C X-SPD
	f32 mSpeedY;      // _10 Y-SPD
	f32 mFreq;        // _14 SCALE the noise's frequency
	u8 mOctaves;      // _18
	u8 mThreshold;    // _19 alpha below this is cut
	GXColor mColor;   // _1A
	f32 mScrollU;     // _20 -1 to 1
	f32 mScrollV;     // _24
	f32 mDU;          // _28 scroll per frame
	f32 mDV;          // _2C
	f32 mRepeat;      // _30 how many times the texture fits across the sheet
};

class TObjFractalCloud : public TSetActor {
public:
	TObjFractalCloud(TObject* parent, SetDesc* desc);
	virtual ~TObjFractalCloud();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual void init();                      // _74

	DrawTask* mDraw;   // _64
	FractalPuff mPuff; // _68
	f32 mScale;        // _9C SIZE half the sheet's width
	f32 mMtx[12];      // _A0
};

#endif
