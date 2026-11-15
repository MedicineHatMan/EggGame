#ifndef _TMAINMENUMODEL_H
#define _TMAINMENUMODEL_H
struct TObject;
struct Vec3f;
struct GjActor;

#include "engine/MotionPlayer.h"
struct DrawTask;
#include "stage/Stage.h"

extern "C" void objHeapFree(void*, void*);

/**
 * One model on the main menu. mKind selects the model; mAlt is that kind plus 3 and
 * is the still model drawn while mState is 0. Kinds below 3 also own a motion.
 *
 * @size 0xC0
 */
// The three rotation angles copied as one block
struct MenuModelAngle {
	int v[3];
};

class TMainMenuModel : public TObject {
public:
	TMainMenuModel(TObject* parent, const Vec3f* pos, const int* angle, int kind, int priority);
	virtual ~TMainMenuModel();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10 empty
	virtual void render(); // _1C

	void setup();            // 0x801B7CE0
	void updateMatrix();     // 0x801B7F70
	void stepMotion();       // 0x801B7FB4
	void play(s16 id);       // 0x801B8054
	void setIdle();          // 0x801B80A0
	void setState1();        // 0x801B80D8
	void setState2();        // 0x801B8110

	GjActor* mActor;         // _20 built when the model's first word has bit 0x2000
	GjActor* mActorB;        // _24 the same test on model mKind+3
	int mKind;            // _28
	int mAlt;             // _2C
	f32 mMtx[12];         // _30
	Vec3f mPos;           // _60
	MenuModelAngle mAngle; // _6C
	DrawTask* mDraw;          // _78
	MotionPlayer mMotion; // _7C
	s16 mMotionId;        // _B8
	u16 _BA;              // _BA
	int mState;           // _BC
};

extern "C" {
void freeMainMenuModel();
void loadMainMenuModel();
}

extern "C" {
TMainMenuModel* createTMainMenuModel(TObject*, const Vec3f*, const int*, int, int);
}

extern "C" {
extern const char* gTMainMenuModelName;
extern MotionEntry sMainMenuModelMotions[];
}

#endif
