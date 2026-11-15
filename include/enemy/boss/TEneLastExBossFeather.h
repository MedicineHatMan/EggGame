#ifndef _ENEMY_BOSS_TENELASTEXBOSSFEATHER_H_
#define _ENEMY_BOSS_TENELASTEXBOSSFEATHER_H_
#include "egg/ObjEgg.h"
struct NJS_MOTION;
struct NJS_OBJECT;
struct NJS_TEXLIST;
struct DrawTask;
#include "enemy/EnemyAnim.h"
#include "engine/CharaColli.h"
#include "engine/FxHolder.h"
#include "engine/MotionPlayer.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TEneLastExBossFeather;

#include "core/types.h"

struct Vec3f;

class TEneLastExBossFeather : public TObjAction {
public:
	TEneLastExBossFeather(TObject* parent, const Vec3f* pos, const Vec3f* aim); // 0x8018A120
	virtual ~TEneLastExBossFeather();                                                // 0x8018A40C

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                                        // _0C 0x8018A86C
	virtual void vfn_10();                                      // _10 0x8018AA2C
	virtual void render();                                      // _1C 0x8018AA8C
	virtual Vec3f* getPos();                                    // _20 0x8018AAE8
	virtual int getKind();                                      // _2C 0x8018AAE0
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x8018A4F0

	void updateColli();  // 0x8018A588
	void updateShadow(); // 0x8018A5C0
	void trackNodes();   // 0x8018A624
	void calcPoints();   // 0x8018A670
	void emitTrail();    // 0x8018A6D0
	void stopTrail();    // 0x8018A77C
	void applyMatrix();  // 0x8018A7E4
	void placeBurst();   // 0x8018A82C

	enum {
		FEATHER_FLY   = 0, // flies to the aim trailing particles
		FEATHER_LAND  = 1, // the burst starts
		FEATHER_GROW  = 2, // the hit sphere swells to 15
		FEATHER_FADE  = 3, // waits then goes
	};

	SetTransform mXform;  // _30
	MotionPlayer mMotion; // _74
	EnemyAnim mClip;      // _B0
	CharaColli* mShape;         // _BC
	DrawTask* mDraw;          // _C0
	void* mAnim;          // _C4
	ObjShadow mShadow;    // _C8
	Vec3f mAim;           // _100 where it lands its height is the floor
	int mTrailKind;       // _10C
	int mTrailPhase;      // _110
	int mGlowKind;        // _114
	int mState;           // _118
	int mTimer;           // _11C
	f32 mRadius;          // _120
	HitVec mPoint[8];     // _124
	TObject* mPart[4];    // _184
	FxHolder mGlow;       // _194
	FxAttach mBurst;      // _1C8
};

extern "C" {
extern NJS_OBJECT* gLastExBossFeatherObject;
extern NJS_MOTION** gLastExBossFeatherMotion;
extern NJS_TEXLIST* gLastExBossFeatherTexList;
}

#endif
