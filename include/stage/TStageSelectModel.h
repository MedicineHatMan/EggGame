#ifndef _TSTAGESELECTMODEL_H
#define _TSTAGESELECTMODEL_H
struct Fx;

#include "stage/Stage.h"
struct DrawTask;

extern "C" void objHeapFree(void*, void*);
extern "C" char* getStageSelectMission(int index); // 0x80179EBC the mission's text 0 past the end

/**
 * One model on the stage-select screen. mIndex picks the model and motion in the
 * bank at gMenuModelSet. Index 1 also copies two nodes of that model onto mMtxA and
 * mMtxB. draw() wraps mFrame while mPlaying is set. render() lights the model and
 * draws either the motion or the still object.
 *
 * @size 0x140
 */
class TStageSelectModel : public TObject {
public:
	TStageSelectModel(TObject* parent, const Vec3f* pos, int* angle, int index, u16 priority);
	virtual ~TStageSelectModel();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	// Switches to another model and motion from its first frame
	inline void setModel(int index)
	{
		mIndex = index;
		mFrame = 0.0f;
	}

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10 empty
	virtual void render(); // _1C

	void setup();                                      // 0x801795EC
	void updateMatrix();                               // 0x80179864
	void applyNodes(struct SelectNode* node, int depth);            // 0x80179900
	void attachBurst(const f32* mtx);                  // 0x80179A28
	void spawnIndex2();                                // 0x80179A68
	void spawnIndex5();                                // 0x80179ACC
	void spawnIndex4();                                // 0x80179B30
	void spawnIndex1();                                // 0x80179B94
	void spawnIndex6();                                // 0x80179C1C
	void setMaterial(int kind);                        // 0x80179C80

	int mIndex;        // _20
	int _24;           // _24
	u32 mSaved;        // _28 the word at model+0x1AC before setMaterial changes it
	f32 mMtx[12];      // _2C
	Vec3f mPos;        // _5C
	int mAngle[3];     // _68
	f32 mScale;        // _74
	DrawTask* mDraw;       // _78
	u32 _7C;           // _7C
	u8 _80[0x1C];      // _80
	u32 _9C;           // _9C
	int mPlaying;      // _A0
	f32 mFrame;        // _A4
	f32 mRate;         // _A8
	Vec3f mLightColor; // _AC
	Vec3f mAmbient;    // _B8
	f32 mLightX;       // _C4
	f32 mLightY;       // _C8
	f32 mLightZ;       // _CC
	void* mBurst;      // _D0
	u32 _D4;           // _D4
	Fx* mFx;         // _D8
	f32 mMtxA[12];     // _DC
	Fx* mFxB;        // _10C
	f32 mMtxB[12];     // _110
};

class TObject;
struct Vec3f;

extern "C" {
TStageSelectModel* createTStageSelectModel(TObject*, const Vec3f*, const int*, int, u16);
void freeStageSelectText();
void loadStageSelectText();
}

extern "C" {
char* getStageSelectWarning(int);
}

extern "C" {
extern const char* gTStageSelectModelName;
}

#endif
