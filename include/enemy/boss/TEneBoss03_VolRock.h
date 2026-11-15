#ifndef _ENEMY_BOSS_TENEBOSS03_VOLROCK_H_
#define _ENEMY_BOSS_TENEBOSS03_VOLROCK_H_
#include "Ninja/ninja.h"
struct RockModel;
#include "egg/ObjEgg.h"
#include "enemy/EnemyLight.h"
#include "enemy/TEneAm02.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TEneBoss03_VolRock;
struct VolFlight;

#include "core/types.h"
#include "enemy/boss/Ballistic.h"

class TObject;
struct Vec3f;

extern "C" {
void createVolRockBurst(TObject*, const Vec3f*, int);
void volRockLoad();
void volRockFree();
void lightVolRocksFrom(const Vec3f*);
void lightVolRocksInOrder();
int volRockNear(const Vec3f*, f32);
}

/** A thrown rock's motion. @size 0x68 */
struct VolFlight {
	f32& gravity() { return mBody.mForce.y; }

	SetTransform mBody; // _00 mVel is the speed gravity at _20
	int mSpinX;         // _44
	int mSpinY;         // _48
	u8 _4C[4];
	Vec3f mStart;  // _50
	Vec3f mTarget; // _5C
};

/** A rock that lands and can be hit. @size 0x298 */
class TEneBoss03_VolRock : public TObjAction {
public:
	TEneBoss03_VolRock(TObject* parent, const Vec3f* pos, const Vec3f* target);
	virtual ~TEneBoss03_VolRock();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual Vec3f* getPos();
	virtual int getKind();
	virtual int onHit(const TObjAction* other, int arg, int* result);

	void setup(const Vec3f* pos, const Vec3f* target, f32 speed, f32 gravity);
	int checkHits();
	int isLanded();
	int isBroken();
	int isCold();
	void updateGround();
	void breakUp();
	void placePieces();
	void stepPieces();
	void drawModel();

	TEneBoss03_VolRock* mNext; // _30
	VolFlight mFlight;        // _34
	int mState;                // _9C 0 flying 1 landed 2 waiting 3 on its fuse 4 broken
	u32 mLandFlags;            // _A0 bit 0 has landed
	f32 mTime;                 // _A4 frames in the air
	f32 mHeat;                 // _A8 1 down to 0 once it starts to cool
	int mHotTime;              // _AC from landing to the start of cooling
	int mTimer;                // _B0
	ObjShadow mShadow;         // _B4
	CharaColli* mColli;              // _EC
	DrawTask* mDrawTask;           // _F0
	u8* mCarried;              // _F4
	EnemyLight mLight;         // _F8
	u32 mLitAlpha;             // _138
	u16 mHitterId;             // _13C
	HitVec mPiecePos[14];      // _140 The first holds the rest place while the fuse shakes it
	HitVec mPieceVel[14];      // _1E8
	int mFade;                 // _290 the pieces' alpha
	EnemyFx* mFx;              // _294
};

/** The rock's model _08 the whole object _0C its 14 pieces */
struct RockModel {
	u8 _00[8];
	void* mObject; // _08
	u8* mPieces[14];  // _0C Each _04 the model _08 where it sits
};

extern "C" {
extern const char* gTEneBoss03_VolRockName;
extern const char* gTEneBoss03_VolRock_DummyName;
extern u32 sVolRockSeeColor;
extern RockModel* gRockModel;
extern NJS_TEXLIST** gRockShape;
extern TEneBoss03_VolRock* gVolRockList;
extern int sVolRockStage;
extern GjTexMem* sVolRockTexWord;
extern int sVolRockClear;
extern u32 sVolRockColor;
}

#endif
