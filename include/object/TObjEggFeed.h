#ifndef _TOBJEGGFEED_H
#define _TOBJEGGFEED_H
struct Vec3f;
struct SetDesc;
struct TObject;
struct DrawTask;

#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "engine/MotionPlayer.h"
#include "engine/TMobileObject.h"
#include "enemy/EnemyLight.h"
#include "animal/AniRider.h"

extern "C" void objHeapFree(void*, void*);

/**
 * Food left in the stage: apple, banana, cherry, melon, pineapple, strawberry, watermelon.
 * A set record builds one, and so does the call that passes a position. It waits, then
 * either the player takes it or it flies to the point set by the last function.
 *
 * @size 0x220
 */
/** A MobileGround this object destroys through the rider destructor */
class TObjEggFeed : public TSetActor {
public:
	TObjEggFeed(TObject* parent, SetDesc* desc);
	TObjEggFeed(TObject* parent, int kind, const Vec3f* pos, int sizeKind);
	virtual ~TObjEggFeed();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void* arg);       // _34

	void place(int kind, const Vec3f* pos, const SetRot* rot, int sizeKind, int rebirth);
	void makeShape();
	void setSphere();
	void syncPos();
	void freeShape();
	void setPhase(int phase);
	void idle();
	void eaten();
	void fly();
	int inReach(Vec3f* pos, f32 radius);
	int land();
	void buildMatrix();
	void drawPass();
	void drawUnit();
	void drawModel();
	void drawShadow();
	void drawSpecial();

	DrawTask* mTask;      // _64 addDrawTask priority 0x22
	ObjModel mModel;      // _68
	ObjModel mShadow;     // _88
	f32 mMtx[12];         // _A8
	f32 mShadowMtx[12];   // _D8
	ObjDrawState mDraw;   // _108 both models point here
	MotionPlayer mMotion; // _12C
	Vec3f mSway;          // _168
	f32 mScale;           // _174 1.25 1.8 or 4
	f32 mBob;             // _178
	Vec3f mVel;           // _17C
	int mKind;            // _188 0 to 6
	int mSizeKind;        // _18C 0 normal 1 big 2 extreme
	int mRebirth;         // _190 0 off 1 on
	int mAmount;          // _194 1 5 or 100
	s16 mSpawned;         // _198 1 when built from code
	u16 _19A;             // _19A
	ObjRef mEater;        // _19C
	s16 mLife;            // _1A8 counts down 0x4B0 while it waits
	s16 mSpin;            // _1AA
	Rider mGround;        // _1AC
	s8 mShade;            // _1C0 -1 with no link otherwise climbs by 4
	u8 _1C1[3];           // _1C1
	u32 mFeedFlags;       // _1C4 1 on the ground 2 flying 4 model 8 shadow 0x10 special
	EnemyLight mLight;    // _1C8
	int mUser;            // _208 record int 3
	f32 mUserF[4];        // _20C record floats
	int mPhase;           // _21C 0 wait 1 eaten 2 flying
};

extern "C" {
void initTObjEggFeed();
void termTObjEggFeed();
}

extern "C" {
TObject* createTObjEggFeed(TObject*, SetDesc*);
}

extern "C" {
TObjEggFeed* createTObjEggFeedAt(int, const Vec3f*, u8);
}

extern "C" {
extern const char* gTObjEggFeedName;
extern char* sEggFeedOnOffNames[2];
extern char* sEggFeedKindNames[7];
extern char* sEggFeedSizeNames[3];
extern const f32 sEggFeedPositions[12];
extern const f32 sEggFeedTexCoords[8];
// Where a thrown feed flies to
struct EggFeedTarget : public Vec3f {
	EggFeedTarget(f32 tx, f32 ty, f32 tz)
	{
		x = tx;
		y = ty;
		z = tz;
	}
};
extern EggFeedTarget sEggFeedTarget;
}

#endif
