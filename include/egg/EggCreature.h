#ifndef _EGGCREATURE_H
#define _EGGCREATURE_H
struct DrawStateSlot;
struct ColliShape;
class TParticleGenerator;
class TEggContentsIntro;
struct GjActor;
struct DrawState;
struct Fx;
struct CharaColli;

#include "object/ObjAction.h"
struct NJS_TEXLIST;
#include "engine/DrawTask.h"
#include "egg/TEggCreature.h"
#include "engine/CharaColli.h"
#include "egg/ObjEgg.h"
#include "engine/MotionPlayer.h"
#include "object/ObjModel.h"
#include "enemy/EnemyLight.h"
#include "sound/SoundHandle.h"
#include "animal/AniAsset.h"
#include "engine/FxSystem.h"
#include "animal/RacerMover.h"
#include "engine/Colli.h"
#include "core/PlacementNew.h"

class TCamPath;

// The base of the combo animals

// A jump an animal makes the motion the frame it leaves the ground and the frame it lands
struct AniLeap {
	int mMotion;  // _00
	int mTakeoff; // _04
	int mLand;    // _08
};

// A texture swap while a motion is between two frames done by aniTexSwapBegin
struct AniTexAnim { // size 0x14
	int mMotion; // _00 the motion's index in the model
	int mFrom;   // _04
	int mTo;     // _08
	int mA;      // _0C
	int mB;      // _10
};

// Something to set off when the animal's motion passes a frame
struct AniEvent {
	int mFrame;  // _00
	int mEffect; // _04 the last argument of particleOnceById
	u8 _08[4];
	int mBone;   // _0C the slot in gAnimalBoneMatrices or -1 for the root
};

// The events of one motion
struct AniEventSet {
	int mCount;          // _00
	AniEvent** mEvents;  // _04
};

// An animal's param file from its ani_param arc
struct AniParams {
	u32 mDrawKey;           // _00 the draw task's key once hatched
	int mReturnStyle;       // _04 how it comes back to its holder 3 walks 4 hops 5 runs 6 jumps
	f32 mWalkRate;          // _08 the motion speed while it walks
	f32 mBaseScale;         // _0C
	f32 mAccel;             // _10
	f32 mTopSpeed;          // _14
	f32* mValues;           // _18 read by each animal in its own way
	AniLeap* mHatchJump;    // _1C out of the egg
	AniLeap* mJump;         // _20 onto a ledge
	int mTexAnimCount;      // _24
	AniTexAnim** mTexAnims; // _28
	AniEventSet** mEventSets; // _2C by motion the first five
	f32* mCamera;           // _30 the hatch camera its height then where it looks from
};

class TEggCreatureTypeE;
struct AniList;

extern "C" {
extern NJS_MATRIX gAnimalExtraMatrices[5]; // more matrices some bones write
void njDrawTexture3DEx(const void*, int, int);
void gjSetPostTevStageCallback(void*);
void gjSetPostTexGenCallback(void*);
extern Vec3f gVecZero; // all zero
extern AniParams* gAnimalParams[]; // the param file of each kind
extern u8 gAnimalBoneMatrices[];  // the matrices the bones write 0x30 each
extern u8** gAnimalCommonModel;
extern u32 _nj_control_3d_flag_;
}

class TMorphEggAnim2;

/**
 * Modes, by setMode: 0 load the model, 1 hatch, 2 free, 3 greet its holder, 4 follow, 5 come back, 6 jump, 7 dash at
 * a target, 8 attack (onPhase), 9 vanish. Each runs in phases: 0 on entry, 1 on exit, 2 each frame.
 *
 * @size 0x384
 */
class TEggCreatureTypeE : public TEggCreature {
public:
	enum {
		MODE_LOAD   = 0,
		MODE_HATCH  = 1,
		MODE_FREE   = 2,
		MODE_GREET  = 3,
		MODE_FOLLOW = 4,
		MODE_RETURN = 5,
		MODE_JUMP   = 6,
		MODE_DASH   = 7,
		MODE_ATTACK = 8,
		MODE_VANISH = 9,
	};

	TEggCreatureTypeE(TObject* parent, int player, int kind, const Vec3f* pos); // 0x80099BA8
	virtual ~TEggCreatureTypeE();                                            // 0x80099C9C

	virtual void draw();             // _0C
	virtual void vfn_10();           // _10
	virtual void render();           // _1C
	virtual Vec3f* getPos() { return &mPos; }    // _20
	virtual int getRotY() { return mRot[1]; }    // _28
	virtual void vfn_70();           // _70 message 0x44 it is called on to attack
	virtual int vfn_74();            // _74
	virtual void vfn_78();           // _78 message 0x46 its holder lets it go
	virtual void vfn_7C() { vanish(); } // _7C
	virtual void vfn_80(const void* body); // _80 message 0x23 thrown
	virtual int vfn_84() { return mKind; }
	virtual int vfn_88() { return mFree; }
	virtual void vfn_8C(u32 flags) { mAniFlags |= flags; }
	virtual void vfn_90(u32 flags) { mAniFlags &= ~(unsigned int)flags; } // an int-sized complement the DOL loads into r5
	virtual void resetBones() { }        // _98
	virtual void onPhase(int phase) { }  // _9C 0 starts the attack 1 ends it 2 runs it
	virtual void vfn_A0();               // _A0 places the followers and sets off the motion's events

	void init();
	void vanish();
	void attachToHolder();
	void checkContacts();
	int touchedTarget();
	void pushOut();
	void setHolder(u16 id);
	void endAttack();
	void setMode(int mode);
	void runMode(int mode, int phase);
	void modeLoad(int phase);
	void modeHatch(int phase);
	void modeFree(int phase);
	void modeGreet(int phase);
	void modeFollow(int phase);
	void modeReturn(int phase);
	void modeJump(int phase);
	void modeDash(int phase);
	void modeVanish(int phase);
	void hatchOpen();
	void hatchFall();
	void hatchIntro();
	void makeEggModel();
	void makeAnimalModel();
	void makeHatchCamera();
	void makeIntro();
	void runWander();
	void wanderRest();
	void wanderLand();
	void wanderWalk();
	void returnWalk();
	void returnHop();
	void returnRun();
	void returnJump();
	void runJump();
	void pickWanderGoal();
	void walkToGoal();
	void updateShadow();
	void setMotionSpeed(f32 speed);
	void playMotion(int motion, int events, f32 blend);
	void beginTexAnim();
	void endTexAnim();
	void updateMatrix();
	void updateMotion();
	void poseActor();
	void clearExtraFx();
	void startFollowers();
	void addFollower(Vec3f* pos, int id, s16 slot);
	void runBounce();
	void startBounce();
	void checkLedge();
	void aimJump(Vec3f* vel, const Vec3f* from, const Vec3f* to);
	void startDash();
	void jump(const Vec3f* vel, AniLeap* desc);
	void notifyHolder();
	int hasGround(const Vec3f* pos);

	// These repeat in every animal
	inline void initColli();
	inline void makeAttackShape(int sphereFlag, int cylinderFlag);
	inline int fillHit(const TObjAction* other, int* result, int bit);
	inline int fillHitParam(const TObjAction* other, int* result, int slot);
	inline void killFx();

	const MotionEntry* motionEntry() const { return &((const MotionEntry*)mNodes)[mPlayer.mCurrent]; }

	int mKind;            // _4C
	s16 mCount;           // _50 attacks left
	u8 _52[2];
	AniParams* mParams;   // _54
	s16 mMode;            // _58
	s16 mSubMode;         // _5A the mode before this one
	s16 mStep;            // _5C
	s16 mMotionId;        // _5E
	u32 mFree;            // _60 1 while it has no holder
	RacerMover mMover;    // _64 moves it toward a target
	Vec3f mSpawnPos;      // _C4
	Vec3f mStartPos;      // _D0
	f32 mNodeMatrix[12];  // _DC
	Vec3f mPos;           // _10C
	Vec3f mVel;           // _118
	Rot3i mRot;           // _124
	Vec3f mScale;         // _130
	Vec3f mPush;          // _13C how far what it touches pushes it this frame
	ObjRef mHolderRef;    // _148
	ObjRef mTarget;       // _154
	Vec3f mTargetPos;     // _160
	Vec3f mGoal;          // _16C where a walk or a dash ends
	Vec3f mDir;           // _178
	f32 _184;             // _184
	f32 _188;             // _188
	AniAsset mAsset;      // _18C attached once mLink.mList is set
	void* mMotion;        // _1A4 the AniModel export
	GjActor* mActor;         // _1A8
	u32 _1AC;             // _1AC
	NJS_TEXLIST* mTexList;       // _1B0
	const void* mNodes;   // _1B4 the motion table 0x1C each
	MotionPlayer mPlayer; // _1B8
	f32 mMotionSpeed;     // _1F4
	CharaColli* mColli;         // _1F8
	const ColliShape* mShape; // _1FC
	u32 mHitValue;        // _200
	void* mPolyAnim;      // _204
	int mFxBank;          // _208
	Fx* mFx;            // _20C
	f32 mFxMatrix[12];    // _210
	u8 _240[4];
	SoundHandle mSound;   // _244
	const void* m270;     // _270
	TParticleGenerator* mFollowers[10]; // _274 effects kept at places on the model
	s8 mFollowerCount;    // _29C
	u8 _29D;
	s16 mEventCount;      // _29E
	const AniEvent* mEvents; // _2A0
	AniTexAnim* mTexAnims;   // _2A4
	s8 mTexAnimCount;     // _2A8
	u8 _2A9[3];
	AniLeap* mJump;       // _2AC
	int mFirstJoin;       // _2B0 cleared once it has told the game it joined
	int mEvent;           // _2B4
	TCamPath* mEggModel;  // _2B8
	TObject* mHatchCamera; // _2BC
	TMorphEggAnim2* mMorph; // _2C0
	TEggContentsIntro* mIntro; // _2C4
	ObjShadow mShadow;    // _2C8
	DrawTask* mLightTask; // _300
	EnemyLight mLight;    // _304
	ObjDrawState mDrawState; // _344
	int mTimer;           // _368
	int mFaded;           // _36C
	f32 mBounceBase;      // _370
	f32 mBounceAmp;       // _374
	int mBounceAngle;     // _378
	int mBounceSpeed;     // _37C
	u32 mAniFlags;        // _380 1 in the air 2 hold the motion 4 on the ground 8 hit a wall 0x10 at a ledge
	                      // 0x20 hold the rate 0x40 settled 0x100 joined a player 0x200 bounced 0x400 jump up 0x4000 gone
	                      // 0x8000 no events
};

inline void TEggCreatureTypeE::killFx()
{
	if (mFx != 0) {
		fxRemove(mFx);
		mFx = 0;
	}
}

// The first shape and no masks until the attack starts
inline void TEggCreatureTypeE::initColli()
{
	mColli = colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, mObjId);
		colliSetMasks(mColli, 0x20, 0, 0, 0x10);
		colliCopyShapes(mColli, mShape);
		colliPlaceShapes(mColli, &mPos, (const ColliShape*)mShape);
	}
}

// Value 4 of the params picks a sphere or a cylinder sized by values 5 and 6
inline void TEggCreatureTypeE::makeAttackShape(int sphereFlag, int cylinderFlag)
{
	u8 shape[0x30];
	f32* values = mParams->mValues;
	if ((int)values[4] == 0) {
		colliShapeMakeSphere((ColliShape*)shape, &gVecZero, sphereFlag, values[5]);
	} else {
		colliShapeMakeCylinder((ColliShape*)shape, &gVecZero, cylinderFlag, values[5], values[6]);
	}
	colliCopyShapes(mColli, (const ColliShape*)shape);
	colliShapeSetBit8(mColli, 0);
	colliSetMasks(mColli, 0x20, 0xFF, 0xFF, 0x10);
}

// Not its own holder Damage is int 1 of the params
inline int TEggCreatureTypeE::fillHit(const TObjAction* other, int* result, int bit)
{
	if (other->mObjId == mHolderId) {
		return 0;
	}
	resetHitInfo((HitInfo*)result);
	result[0] = ((int*)mParams->mValues)[1];
	((HitInfo*)result)->mFlags.setBit(bit);
	result[5] = mHitValue;
	return 1;
}

inline int TEggCreatureTypeE::fillHitParam(const TObjAction* other, int* result, int slot)
{
	if (other->mObjId == mHolderId) {
		return 0;
	}
	resetHitInfo((HitInfo*)result);
	result[0] = ((int*)mParams->mValues)[1];
	((HitInfo*)result)->mFlags.setBit(((int*)mParams->mValues)[slot]);
	result[5] = mHitValue;
	return 1;
}

typedef char TEggCreatureTypeE_size_check[(sizeof(TEggCreatureTypeE) == 0x384) ? 1 : -1];

struct BoneEntry {
	u8 _00[8];
	int mBone; // _08
	int mSlot; // _0C
};

struct BoneList {
	int mCount;           // _00
	BoneEntry** mEntries; // _04
};

// Each animal's model calls functions like this as it walks a section and they keep the matrices of the bones it names
#define copyAnimalBones(animal, section, bone) { \
	u8* table; \
	int count; \
	BoneEntry* entries; \
	int i; \
	BoneList* list = (*(BoneList***)((u8*)gAnimalParams[animal] + 0x2C))[section]; \
	table          = gAnimalBoneMatrices; \
	count          = list->mCount; \
	entries        = *list->mEntries; \
	for (i = 0; i < count; i++) { \
		if (bone == entries[i].mBone) { \
			njGetMatrix((f32*)(table + entries[i].mSlot * 0x30)); \
		} \
	} \
}

#endif
