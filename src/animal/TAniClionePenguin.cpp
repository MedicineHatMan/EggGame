#include "animal/AniAttacks.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/Colli.h"
#include "camera/TCamPath.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
#include "engine/DrawState.h"
#include "ui/AeArc.h"
#include "io/ResBank.h"
#include "object/ObjShadow.h"
#include "egg/TEggCreatureTypeE.h"
#include "sound/Sound3DPlay.h"
#include "stl/math.h"
#include "engine/Color.h"
#include "core/MathUtil.h"
#include "engine/TParticle.h"
#include "engine/GroundQuery.h"
#include "animal/TAniClionePenguin.h"
struct DrawStateSlot;
struct DrawState;
struct ColliShape;
struct CharaColli;

extern "C" {
MotionEntry gClioneMotions[8] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, 0.0f, 0.0f, 0 }, { MOTION_LOOP, 1, -1, 0, 0.0f, 30.0f, 0.0f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 30.0f, 0.0f, 0.0f, 0 }, { MOTION_LOOP, 3, -1, 0, 0.0f, 15.0f, 0.0f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 14.0f, 0.0f, 0.0f, 0 }, { MOTION_ONCE, 5, -1, 0, 0.0f, 55.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 30.0f, 0.0f, 0.0f, 0 }, { MOTION_ONCE, 1, -1, 0, 0.0f, 30.0f, 0.0f, 0.0f, 0 },
};
ColliShape sClioneShape0[1] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
MotionEntry sClioneMotions1[11] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, 0.2f, 0.0f, 0 },     { MOTION_LOOP, 1, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 },     { MOTION_LOOP, 3, -1, 0, 0.0f, 24.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },     { MOTION_ONCE, 5, -1, 0, 0.0f, 55.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 },     { MOTION_ONCE, 1, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ MOTION_BLEND, 9, -1, 2, 0.0f, 19.0f, 0.35f, 0.075f, 0 }, { MOTION_BLEND, 8, -1, 10, 0.0f, 19.0f, 0.3f, 0.075f, 0 },
	{ MOTION_LOOP, 4, -1, 0, 0.0f, 9.0f, 0.3f, 0.0f, 0 },
};
ColliShape sClioneShape1[1] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

// What the bubble's sweep finds
struct BubbleSweep {
	int mHitWall;  // _00
	f32 mDist;     // _04
	Vec3f mNormal; // _08
};

struct BubbleSphere {
	Vec3f mPos;  // _00
	f32 mRadius; // _0C
};

typedef char TAniClionePenguin_size_check[(sizeof(TAniClionePenguin) == 0x3BC) ? 1 : -1];

inline f32 inlineSqrt(f32 x)
{
	if (x > 0.0f) {
		f64 guess           = __frsqrte(x);
		guess               = 0.5 * guess * (3.0 - x * (guess * guess));
		guess               = 0.5 * guess * (3.0 - x * (guess * guess));
		guess               = 0.5 * guess * (3.0 - x * (guess * guess));
		volatile f32 result = x * guess;
		return result;
	}
	return x;
}

inline void TAniClionePenguin::killBubble()
{
	if (mBubble != 0) {
		mBubble->mFlags |= 1;
		mBubble = 0;
	}
}

const char* gTAniClionePenguinName = "TAniClionePenguin";

// 0x80134CC4
TAniClionePenguin::TAniClionePenguin(TObject* parent, int player, int kind, const Vec3f* pos)
    : TEggCreatureTypeE(parent, player, kind, pos)
{
	_00   = (u32)gTAniClionePenguinName;
	mSize = 0x3BC;
	if (kind == 0xE) {
		mNodes = sClioneMotions1;
		mShape = sClioneShape1;
	} else {
		mNodes = gClioneMotions;
		mShape = sClioneShape0;
		m270   = sClionePenguinCallbacks;
	}
	initColli();
	mEnd.x      = 0.0f;
	mEnd.y      = 0.0f;
	mEnd.z      = 0.0f;
	_390.x      = 0.0f;
	_390.y      = 0.0f;
	_390.z      = 0.0f;
	_39C.x      = 0.0f;
	_39C.y      = 0.0f;
	_39C.z      = 0.0f;
	_3A8        = 0;
	_3AC        = 0;
	_3B0        = 0;
	mBubbleSize = 0.0f;
	mBubble     = 0;
}

// 0x80134DEC
TAniClionePenguin::~TAniClionePenguin()
{
	killBubble();
	colliFree(mColli);
	mColli = 0;
}

// 0x80134E7C
// Kind 0x13 draws a reflection over the actor except while it stands still
void TAniClionePenguin::render()
{
	if (mAsset.mLink.mList == 0) {
		return;
	}
	if (mMotion == 0) {
		return;
	}
	if (mKind != 0x13) {
		TEggCreatureTypeE::render();
		return;
	}
	if (mMode == 1 && mStep == 0) {
		TEggCreatureTypeE::render();
		return;
	}
	setDrawAmbient(&mLight, &mDrawState);
	DrawStateSlot* slot = getDrawStateSlot();
	njPushMatrixEx();
	njMultiMatrix(0, mNodeMatrix);
	beginDrawState(slot, (const DrawState*)&mDrawState);
	njSetTexture(((void***)gAnimalCommonModel)[4][1]);
	gjLoadTextureNum(0, 0, 0, 1);
	f32 m[12];
	PSMTXCopy(_gj_matrix_, m);
	m[3]  = 0.0f;
	m[7]  = 0.0f;
	m[11] = 0.0f;
	GXLoadTexMtxImm(m, GX_TEXMTX5, GX_MTX3x4);
	gjSetNumPostTexGens(1);
	gjSetNumPostTevStages(1);
	gjSetPostTevStageCallback((void*)modelPostTevAdd);
	gjSetPostTexGenCallback((void*)modelPostTexGenNormal);
	_nj_control_3d_flag_ |= 0x80000;
	njSetTexture(mTexList);
	beginTexAnim();
	shadowAlphaBegin();
	if (mActor != 0) {
		gjDrawActor(mActor);
	} else {
		debugWireSphere(gjGetMatrix(), &gColorBlack);
	}
	shadowAlphaEnd();
	endTexAnim();
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	_nj_control_3d_flag_ &= ~0x80000;
	endDrawState((DrawStateSlot*)slot);
	njPopMatrixEx();
}

// 0x8013500C
void TAniClionePenguin::onPhase(int phase)
{
	if (phase == 0) {
		setMotionSpeed(1.0f);
		playMotion(3, 1, 0.075f);
		eggTypeStopAllFx(this);
		startFollowers();
		mTimer    = 0x3C;
		mAniFlags = 0;
		mStep     = 0xA;

		u8 shape[0x30];
		if ((int)mParams->mValues[4] == 0) {
			colliShapeMakeSphere((ColliShape*)shape, &gVecZero, 1, 5.0f);
		} else {
			colliShapeMakeCylinder((ColliShape*)shape, &gVecZero, 1, 5.0f, 5.0f);
		}
		colliCopyShapes(mColli, (const ColliShape*)shape);
		colliShapeSetBit8(mColli, 0);
		colliSetMasks(mColli, 0x20, 0xFF, 0xFF, 0x10);

		TAniBubble* bubble = new TAniBubble(this);
		mBubble            = bubble;
		mBubble->mPosRef   = &mPos;
		mBubble->mRotRef   = mRot;
		mBubble->mSizeRef  = &mBubbleSize;
		sePlay3D(0x1B5, &mPos);

		f32 rise  = mParams->mValues[7];
		f32 speed = mParams->mValues[8];
		if ((int)mTarget.getId() != 0xFFFF) {
			f32 up   = -rise / -0.0522667f;
			f32 high = rise * up + -0.0261333f * up * up + mPos.y - mTargetPos.y;
			if (high > 0.0f) {
				up += inlineSqrt(2.0f * high / 0.0522667f);
				f32 flat = njDistanceP2P(&mPos, &mTargetPos) / up;
				gjScaleVector(flat, &mDir, &mDir);
				mDir.y = rise;
			} else {
				gjScaleVector(speed, &mDir, &mDir);
				mDir.y = rise;
			}
		} else {
			gjScaleVector(speed, &mDir, &mDir);
			mDir.y = rise;
		}
		mVel        = mDir;
		mEnd.x      = 0.0f;
		mEnd.y      = 0.0f;
		mEnd.z      = 0.0f;
		_390.x      = 0.0f;
		_390.y      = 0.0f;
		_390.z      = 0.0f;
		_39C.x      = 0.0f;
		_39C.y      = 0.0f;
		_39C.z      = 0.0f;
		_3A8        = 0;
		_3AC        = 0;
		_3B0        = 0;
		mBubbleSize = 5.0f;
		mSubMode    = 5;
	} else if (phase == 2) {
		switch (mStep) {
		case 0xA:
			runRise();
			break;
		case 0xB:
			runEnd();
			break;
		}
	} else if (phase == 1) {
		if (mKind == 0xE) {
			mShape = sClioneShape1;
		} else {
			mShape = sClioneShape0;
		}
		colliCopyShapes(mColli, mShape);
		colliShapeClearBit8(mColli, 0);
		colliEnable(mColli);
		colliSetMasks(mColli, 0x20, 0, 0, 0x10);
		killBubble();
		mPos = mEnd;
		eggTypeStopAllFx(this);
		clearExtraFx();
	}
}

// 0x801357AC
// Called for each face the sphere touches
static int onSweepFace(void* a, void* b, BubbleSweep* sweep, f32 dist)
{
	if (sweep == 0) {
		return 0;
	}
	if (dist < sweep->mDist && dist > 0.0f) {
		Vec3f normal;
		meshTriNormal((MeshColli*)a, (int)b, &normal);
		sweep->mNormal = normal;
		if (normal.y < 0.5f) {
			sweep->mHitWall = 1;
		}
		sweep->mDist = dist;
	}
	return 1;
}

// 0x801353F0
void TAniClionePenguin::runRise()
{
	f32 size = mBubbleSize;
	f32 most = mParams->mValues[5];
	if (size < most) {
		mBubbleSize = size + 2.5f;
	} else {
		mBubbleSize = most;
	}
	size                           = mBubbleSize;
	colliGetShapes(mColli)->mSizeX = size;

	mVel.y += -0.0522667f;
	BubbleSphere sphere;
	sphere.mPos    = mPos;
	sphere.mRadius = mBubbleSize;
	BubbleSweep sweep;
	sweep.mHitWall  = 0;
	sweep.mDist     = 9999.9f;
	sweep.mNormal.x = 0.0f;
	sweep.mNormal.y = 0.0f;
	sweep.mNormal.z = 0.0f;
	int landed      = 0;
	if (groundQuerySphere((const void*)&sphere, (void*)onSweepFace, (void*)&sweep, (void*)(-1)) != 0) {
		Vec3f dir  = mVel;
		f32 length = njUnitVector(&dir);
		if (length < 1e-6f) {
			dir = gVecZero;
		}
		if (sweep.mHitWall != 0) {
			Vec3f normal = sweep.mNormal;
			vecReflect(&dir, &normal, &dir);
			Vec3f out;
			gjScaleVector(length, &out, &dir);
			mVel = out;
		} else if (sweep.mDist < -mVel.y) {
			mVel.y = -sweep.mDist;
			landed = 1;
		}
	}

	if (landed != 0) {
		njAddVector(&mPos, &mVel);
		mEnd   = mPos;
		mVel.z = 0.0f;
		mVel.x = 0.0f;
		colliDisable(mColli);
		particleOnceByIdDefault(&mEnd, 0, 0x98);
		new TAniBubbleWave(this, &mEnd, mObjId);
		notifyHolder();
		sePlay3D(0x1B6, &mPos);
		mStep = 0xB;
	} else {
		mRot[1] = (int)(10430.38043493439 * (f32)atan2(mVel.x, mVel.z));
		njAddVector(&mPos, &mVel);
		if (mPos.y < -1000.0f) {
			mEnd = mPos;
		}
	}
}

// 0x801356AC
void TAniClionePenguin::runEnd()
{
	mVel.y += -0.0522667f;
	njAddVector(&mPos, &mVel);
	if (mTimer == 0) {
		endAttack();
	} else if (mTimer == 0x1E) {
		mBubble->mFlags |= 0x10;
	}
	mTimer--;
}

// 0x8013572C
int TAniClionePenguin::onHit(const TObjAction* other, int, int* result)
{
	return fillHit(other, result, 7);
}

static int sBone; // 0x802C5688

// 0x801357A0
void TAniClionePenguin::resetBones()
{
	sBone = 0;
}

// 0x8013584C
extern "C" void clioneBones0()
{
	copyAnimalBones(0, 0, sBone);
	sBone++;
}

// 0x801358E4
extern "C" void clioneBones1()
{
	copyAnimalBones(0, 1, sBone);
	switch (sBone) {
	case 0x1B:
		njGetMatrix(gAnimalExtraMatrices[0]);
		break;
	case 0x18:
		njGetMatrix(gAnimalExtraMatrices[1]);
		break;
	}
	sBone++;
}

// 0x801359B8
extern "C" void clioneBones2()
{
	copyAnimalBones(0, 2, sBone);
	switch (sBone) {
	case 0x1B:
		njGetMatrix(gAnimalExtraMatrices[0]);
		break;
	case 0x18:
		njGetMatrix(gAnimalExtraMatrices[1]);
		break;
	}
	sBone++;
}

// 0x80135A8C
extern "C" void clioneBones3()
{
	copyAnimalBones(0, 3, sBone);
	sBone++;
}

// 0x80135B24
extern "C" void clioneBones4()
{
	copyAnimalBones(0, 4, sBone);
	sBone++;
}

// 0x80135BBC
// Part of the unit's static setup the node table takes these from .sdata
extern "C" void initClionePenguinNodes()
{
	f32 a;
	a = sClionePenguinRate0;
	f32 c;
	f32 b;
	gClioneMotions[0].mRate = a;
	b                       = sClionePenguinRate1;
	c                       = sClionePenguinRate2;
	f32 e;
	gClioneMotions[1].mRate = b;
	gClioneMotions[2].mRate = c;
	gClioneMotions[3].mRate = sClionePenguinRate3;
	e                       = sClionePenguinRate4;
	gClioneMotions[4].mRate = e;
	gClioneMotions[6].mRate = c;
	gClioneMotions[7].mRate = b;
}
