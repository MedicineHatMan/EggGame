#ifndef _ANIATTACKS_H
#define _ANIATTACKS_H
struct Fx;
struct NJS_OBJECT;
struct CharaColli;

#include "egg/EggCreature.h"
struct DrawTask;
struct NJS_TEXLIST;
#include "object/ObjModel.h"
#include "stage/Stage.h"
#include "animal/AniAsset.h"

// The objects the combo animals leave behind them for the ones more than one animal or file needs

/**
 * @size 0x50
 */
class TAniBubble : public TObject {
public:
	TAniBubble(TEggCreatureTypeE* owner);
	virtual ~TAniBubble();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual int vfn_20();
	virtual const Vec3f* vfn_24();
	virtual int vfn_28();

	NJS_OBJECT* mObject;   // _20
	NJS_TEXLIST* mTexList; // _24
	const Vec3f* mPosRef;  // _28
	const int* mRotRef;    // _2C
	const f32* mSizeRef;   // _30
	DrawTask* mDrawTask;   // _34
	Vec3f mWobble;         // _38
	int mWobbleAngle;      // _44
	f32 mWarp;             // _48
	int mWarpAngle;        // _4C
};

typedef char TAniBubble_size_check[(sizeof(TAniBubble) == 0x50) ? 1 : -1];

/**
 * @size 0x13C
 */
class TAniBubbleWave : public TObject {
public:
	TAniBubbleWave(TEggCreatureTypeE* owner, const Vec3f* pos, u16 ownerId);
	virtual ~TAniBubbleWave();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void renderDisc();
	void renderWave();
	void beginScroll();
	void endScroll();
	void updateMatrix();

	void* mObjectA;          // _20
	void* mObjectB;          // _24
	NJS_TEXLIST* mTexList;   // _28
	MotionPlayer mPlayerA;   // _2C
	MotionPlayer mPlayerB;   // _68
	ObjDrawState mDrawState; // _A4
	CharaColli* mColli;      // _C8
	f32 mMatrix[12];         // _CC
	Vec3f mPos;              // _FC
	Vec3f mScale;            // _108
	f32 mRadius;             // _114
	f32 mHeight;             // _118
	f32 mFrame;              // _11C the scroll of the texture
	int mFrameWait;          // _120
	int mRise;               // _124
	int mTicks;              // _128
	int mIsDisc;             // _12C
	f32 mWarp;               // _130
	int mDiscAngle;          // _134
	DrawTask* mDrawTask;     // _138
};

typedef char TAniBubbleWave_size_check[(sizeof(TAniBubbleWave) == 0x13C) ? 1 : -1];

/**
 * @size 0x70
 */
class TAniIceSlider : public TObject {
public:
	TAniIceSlider(TEggCreatureTypeE* owner, const Vec3f* pos, const Vec3f* vel, u16 ownerId, u16 targetId);
	virtual ~TAniIceSlider();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void run();
	int touchedTarget();

	int mState;            // _20 1 once the animal lets it go
	void* mObject;         // _24
	NJS_TEXLIST* mTexList; // _28
	CharaColli* mColli;    // _2C
	Vec3f mPos;            // _30
	Vec3f mVel;            // _3C
	int mRot[3];           // _48
	u16 mTargetId;         // _54
	u16 mOwnerId;          // _56
	int mDone;             // _58
	int mTicks;            // _5C
	f32 mRange;            // _60
	f32 mTravelled;        // _64
	DrawTask* mDrawTask;   // _68
	s16 mGround;           // _6C 1 = on the ground 2 = at a drop
};

typedef char TAniIceSlider_size_check[(sizeof(TAniIceSlider) == 0x70) ? 1 : -1];

// What a rock throws out when it breaks
struct IceBurst {
	void** mModels; // _00
	void* mTexture; // _04
	int mFirst;     // _08
	int mVariants;  // _0C
	int mCount;     // _10
	f32 mScale;     // _14
	f32 mSpeed;     // _18
};

/**
 * @size 0xCC
 */
class TAniIceRock : public TObject {
public:
	TAniIceRock(TObject* parent, const Vec3f* pos, u16 ownerId);
	virtual ~TAniIceRock();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void runRise();
	void runGone();
	void burst(const Vec3f* size, const IceBurst* burst);

	AniAsset mArc;           // _20
	void** mAssets;          // _38
	NJS_OBJECT* mObject;     // _3C
	NJS_TEXLIST* mTexList;   // _40
	ObjDrawState mDrawState; // _44
	int mState;              // _68
	Vec3f mPos;              // _6C
	f32 mRise;               // _78
	int mRotY;               // _7C
	f32 mScale;              // _80
	int mTimer;              // _84
	int mDone;               // _88
	int mHolderId;           // _8C
	DrawTask* mDrawTask;     // _90
	CharaColli* mColli;      // _94
	ColliShape mShapeData;   // _98
	u32 mFlags2;             // _C8 1 = breaks by itself 2 = tells mHolderId when it does
};

typedef char TAniIceRock_size_check[(sizeof(TAniIceRock) == 0xCC) ? 1 : -1];

/**
 * @size 0xC0
 */
class TAniFlushFlowerBomb : public TObject {
public:
	TAniFlushFlowerBomb(TObject* owner, u16 ownerId, const Vec3f* pos, int fxBank, ColliShape* shape);
	virtual ~TAniFlushFlowerBomb();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual int vfn_20();
	virtual const Vec3f* vfn_24();

	void runFly();
	void runGrow();
	void killFx();

	Vec3f mPos;  // _20
	Vec3f mVel;  // _2C
	f32 mDrag;   // _38
	f32 _3C;     // _3C
	f32 mGrowth; // _40
	int mTimer;  // _44
	int mState;  // _48
	u8 _4C[4];
	CharaColli* mColli;    // _50
	ColliShape mShapeData; // _54
	Fx* mFx;               // _84
	f32 mFxMatrix[12];     // _88
	int mFxBank;           // _B8
	int mDone;             // _BC
};

typedef char TAniFlushFlowerBomb_size_check[(sizeof(TAniFlushFlowerBomb) == 0xC0) ? 1 : -1];

/**
 * @size 0x40
 */
class TAniAImageHomingBomb : public TObject {
public:
	TAniAImageHomingBomb(TObject* owner, const Vec3f* pos, u16 ownerId);
	virtual ~TAniAImageHomingBomb();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void render();
	virtual int vfn_20();
	virtual const Vec3f* vfn_24();

	CharaColli* mColli; // _20
	Vec3f mPos;         // _24
	f32 mRadius;        // _30
	f32 mGrowth;        // _34
	int mTimer;         // _38
	u8 _3C[4];
};

typedef char TAniAImageHomingBomb_size_check[(sizeof(TAniAImageHomingBomb) == 0x40) ? 1 : -1];

/**
 * @size 0x40
 */
class TAniBigTackleBomb : public TObject {
public:
	TAniBigTackleBomb(TObject* owner, const Vec3f* pos, u16 ownerId);
	virtual ~TAniBigTackleBomb();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void render();
	virtual int vfn_20();
	virtual const Vec3f* vfn_24();

	CharaColli* mColli; // _20
	Vec3f mPos;         // _24
	f32 mRadius;        // _30
	f32 mGrowth;        // _34
	int mTimer;         // _38
	u8 _3C[4];
};

typedef char TAniBigTackleBomb_size_check[(sizeof(TAniBigTackleBomb) == 0x40) ? 1 : -1];

/**
 * @size 0x40
 */
class TAniBoomerangCurveBomb : public TObject {
public:
	TAniBoomerangCurveBomb(TObject* owner, const Vec3f* pos, u16 ownerId);
	virtual ~TAniBoomerangCurveBomb();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void render();
	virtual int vfn_20();
	virtual const Vec3f* vfn_24();

	CharaColli* mColli; // _20
	Vec3f mPos;         // _24
	f32 mRadius;        // _30
	f32 mGrowth;        // _34
	int mTimer;         // _38
	u8 _3C[4];
};

typedef char TAniBoomerangCurveBomb_size_check[(sizeof(TAniBoomerangCurveBomb) == 0x40) ? 1 : -1];

/**
 * @size 0x40
 */
class TAniLowFlyingBomb : public TObject {
public:
	TAniLowFlyingBomb(TObject* owner, const Vec3f* pos, u16 ownerId);
	virtual ~TAniLowFlyingBomb();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void render();
	virtual int vfn_20();
	virtual const Vec3f* vfn_24();

	CharaColli* mColli; // _20
	Vec3f mPos;         // _24
	f32 mRadius;        // _30
	f32 mGrowth;        // _34
	int mTimer;         // _38
	u8 _3C[4];
};

typedef char TAniLowFlyingBomb_size_check[(sizeof(TAniLowFlyingBomb) == 0x40) ? 1 : -1];

#endif
