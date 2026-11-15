#ifndef _ENEMY_TENEGREENDEMO_H_
#define _ENEMY_TENEGREENDEMO_H_
#include "camera/TCamera.h"
struct DrawTask;
#include "egg/ObjEgg.h"
#include "enemy/EnemyAnim.h"
#include "enemy/boss/EneBoss.h"
#include "engine/FxHolder.h"
#include "engine/MotionPlayer.h"
#include "object/ObjCommon.h"
#include "stage/Stage.h"
struct TEneGreenDemo;

#include "core/types.h"

extern "C" {
void greenDemoBankExit();
void greenDemoBankInit();
int greenDemoBankLoad(void*);
void greenDemoSpawn();
}

/**
 * @size 0x138
 */
class TEneGreenDemo : public TObject {
public:
	TEneGreenDemo(TObject* parent);
	virtual ~TEneGreenDemo();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void startNodeCapture();
	void endNodeCapture();
	void pose();
	void placeNodes();
	void applyMatrix();

	Vec3f mPos;           // _20
	u8 _2C[4];            // _2C
	HitVec mNodes[5];     // _30
	EnemyAnim mAnim;      // _6C
	MotionPlayer mPlayer; // _78
	void* mPose;          // _B4
	DrawTask* mDrawTask;      // _B8
	ObjShadow mShadow;    // _BC
	TCamPath* mScreen;   // _F4
	TObject* mSavedParent;  // _F8
	int mState;           // _FC 0 waiting 1 playing 2 done
	EnemyFxBase* mFx;     // _100
	FxHolder mVoice;      // _104
};

extern "C" {
extern const char* gTEneGreenDemoName;
}

#endif
