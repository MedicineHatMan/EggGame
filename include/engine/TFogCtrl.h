#ifndef _ENGINE_TFOGCTRL_H_
#define _ENGINE_TFOGCTRL_H_
struct BinFile;
#include "Ninja/ninja.h"
struct DrawTask;
#include "stage/Stage.h"
struct FogBase;
struct FogBlend;
struct FogMix;
struct FogRun;
struct TFogCtrl;
struct FogRecord;

#include "core/types.h"

extern "C" {
void fogApply();
void fogLoad();
void fogOff();
void fogSetDefaultMain(int);
void fogSetDefaultOther(int);
void fogSetMain(int);
void fogSetOther(int);
void fogUnload();
}

extern "C" {
int fogRangeFlag(FogRecord*);
}

extern "C" {
void fogRequest(int, int, int, int, f32);
TFogCtrl* fogCreate();
}

/**
 * A record being run.
 *
 * @size 0x1C
 */
struct FogRun {
	void init();
	void load(int id);
	void update();

	int mId;        // _00
	u32 mColor;     // _04
	int mColorTime; // _08
	f32 mNear;      // _0C
	int mNearTime;  // _10
	f32 mFar;       // _14
	int mFarTime;   // _18
};

/**
 * The DOL has its vtable, all zeros, and no name.
 *
 * @size 0xC
 */
class FogBase {
public:
	FogBase() { }

	void init();

	int mEnabled;    // _00
	int mRangeAdjust; // _04

	// _08 = vtable
	virtual void update() = 0; // _08
	virtual void apply()  = 0; // _0C
};

/**
 * @size 0x6C
 */
class FogBlend : public FogBase {
public:
	FogBlend();

	virtual void update(); // _08
	virtual void apply();  // _0C
	virtual void init();   // _10

	void request(int id, int priority, int order, f32 rate);
	void setMain(int id);
	void setOther(int id);
	void change(int id);
	void useMain();
	void useOther();

	FogRun mFrom;   // _0C
	FogRun mTo;     // _28
	int mBlending;  // _44
	int mNext;      // _48
	f32 mRate;      // _4C
	int mPriority;  // _50
	int mOrder;     // _54
	f32 mBlend;     // _58
	f32 mStep;      // _5C
	int mMain;      // _60
	int mOther;     // _64
	int mCurrent;   // _68
};

/**
 * @size 0x48
 */
class FogMix : public FogBase {
public:
	FogMix();

	virtual void update(); // _08
	virtual void apply();  // _0C

	void init();
	void setMix(f32 mix);

	FogRun mFrom; // _0C
	FogRun mTo;   // _28
	f32 mMix;     // _44
};

/**
 * @size 0x30C
 */
class TFogCtrl : public TObject {
public:
	TFogCtrl(TObject* parent);
	virtual ~TFogCtrl();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void request(int view, int id, int priority, int order, f32 rate);
	void setMain(int id);
	void setOther(int id);

	DrawTask* mTask;           // _20
	FogBlend mBlends[4];   // _24
	FogMix mMixes[4];      // _1D4
	int mMode;             // _2F4 0 main 1 mixing 2 other
	u16 mRangeTable[10];   // _2F8
};

/**
 * A record of fog_param.bin.
 *
 * @size 0x40
 */
struct FogRecord {
	int mCurve;      // _00
	u32 mFlags;      // _04 bit 0 turns on range adjustment
	u32 mColorA;     // _08
	u32 mColorB;     // _0C
	s16 mColorWave;  // _10
	s16 mColorSpeed; // _12
	f32 mNear;       // _14
	f32 mNearSwing;  // _18
	s16 mNearWave;   // _1C
	s16 mNearSpeed;  // _1E
	f32 mFar;        // _20
	f32 mFarSwing;   // _24
	s16 mFarWave;    // _28
	s16 mFarSpeed;   // _2A
	u8 _2C[0x14];    // _2C
};

extern "C" {
extern const char* gTFogCtrlName;
extern TFogCtrl* sFogCtrlObj;
extern u32 sFogView;
extern int sFogBlending;
extern FogRecord* sFogRecords;
extern BinFile* sFogFile;
extern int gFogMain;
extern int gFogOther;
}

#endif
