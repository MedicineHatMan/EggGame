#include "animal/AniAttacks.h"
#include "engine/Colli.h"
#include "camera/TCamPath.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
#include "engine/DrawState.h"
#include "ui/AeArc.h"
#include "object/ObjShadow.h"
#include "egg/TEggCreatureTypeE.h"
#include "core/MathUtil.h"
#include "sound/Sound3DPlay.h"
#include "engine/Color.h"
#include "animal/TAniHariAzarashi.h"
#include "animal/RacerMover.h"
struct DrawStateSlot;
struct DrawState;
struct CharaColli;

extern "C" {
// 0x802C43E8
f32 sHariAzarashiRate0 = 0.25f;
f32 sHariAzarashiRate1 = 0.27f;
f32 sHariAzarashiRate2 = 0.55f;
f32 sHariAzarashiRate3 = 0.3f;
f32 sHariAzarashiRate4 = 0.27f;

// The rates come from the writable floats above so the static initialiser fills them in
MotionEntry gHariMotions[] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, sHariAzarashiRate0, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 29.0f, sHariAzarashiRate1, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 14.0f, sHariAzarashiRate2, 0.0f, 0 },
	{ MOTION_LOOP, 3, -1, 0, 0.0f, 28.0f, sHariAzarashiRate3, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 14.0f, sHariAzarashiRate4, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 55.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 14.0f, sHariAzarashiRate2, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 29.0f, sHariAzarashiRate1, 0.0f, 0 },
};
ColliShape sHariAzarashiFirstShape[] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

typedef char TAniHariAzarashi_size_check[(sizeof(TAniHariAzarashi) == 0x3A0) ? 1 : -1];

#define KILL(obj)           \
	if ((obj) != 0) {       \
		(obj)->mFlags |= 1; \
		(obj) = 0;          \
	}

inline void TAniHariAzarashi::killAll()
{
	KILL(mSliders[0]);
	KILL(mSliders[1]);
	KILL(mSliders[2]);
	KILL(mRocks[0]);
	KILL(mRocks[1]);
	KILL(mRocks[2]);
}

extern "C" void hariBones0();
extern "C" void hariBones1();
extern "C" void hariBones2();
extern "C" void hariBones3();
extern "C" void hariBones4();

// 0x802496C0
// the bone copiers the creature runs in order null ended
void* const sHariAzarashiCallbacks[] = {
	(void*)hariBones0, (void*)hariBones1, (void*)hariBones2, (void*)hariBones3, (void*)hariBones4, 0,
};

const char* gTAniHariAzarashiName = "TAniHariAzarashi";

// 0x80135BF8
TAniHariAzarashi::TAniHariAzarashi(TObject* parent, int player, int kind, const Vec3f* pos)
    : TEggCreatureTypeE(parent, player, kind, pos)
{
	_00    = (u32)gTAniHariAzarashiName;
	mSize  = 0x3A0;
	mNodes = gHariMotions;
	mShape = sHariAzarashiFirstShape;
	initColli();
	m270        = sHariAzarashiCallbacks;
	mSliders[2] = 0;
	mSliders[1] = 0;
	mSliders[0] = 0;
	mRocks[2]   = 0;
	mRocks[1]   = 0;
	mRocks[0]   = 0;
	mSpread     = 0.0f;
}

// 0x80135CE4
TAniHariAzarashi::~TAniHariAzarashi()
{
	killAll();
	colliFree(mColli);
	mColli = 0;
}

// 0x80135E14
void TAniHariAzarashi::render()
{
	if (mMode != 8) {
		TEggCreatureTypeE::render();
		return;
	}
	setDrawAmbient(&mLight, &mDrawState);
	DrawStateSlot* slot = getDrawStateSlot();
	njPushMatrixEx();
	beginDrawState(slot, (const DrawState*)&mDrawState);
	njSetTexture(mTexList);
	shadowAlphaBegin();
	for (int i = 0; i < 3; i++) {
		njPushMatrixEx();
		njTranslateEx(&mSliders[i]->mPos);
		gjRotateY(mSliders[i]->mRot[1]);
		if (mActor != 0) {
			gjDrawActor(mActor);
		} else {
			debugWireSphere(gjGetMatrix(), &gColorBlack);
		}
		njPopMatrixEx();
	}
	shadowAlphaEnd();
	endDrawState((DrawStateSlot*)slot);
	njPopMatrixEx();
}

static inline u16 objIdOf(TObjAction* obj)
{
	return obj->mObjId;
}

// 0x80135EEC
void TAniHariAzarashi::onPhase(int phase)
{
	if (phase == 0) {
		setMotionSpeed(1.0f);
		playMotion(3, 1, 0.075f);
		eggTypeStopAllFx(this);
		startFollowers();
		mTimer    = 0;
		mAniFlags = 0;
		mStep     = 0xE;
		colliDisable(mColli);
		mSpread = 0.0f;
		for (s16 i = 0; i < 3; i++) {
			TAniIceSlider* slider = new TAniIceSlider(this, &mPos, &mDir, objIdOf(this), mTarget.getId());
			mSliders[i]           = slider;
			mSliders[i]->mRange   = mParams->mValues[3];
		}
		mSubMode = 5;
	} else if (phase == 2) {
		switch (mStep) {
		case 0xE:
			runSpread();
			break;
		case 0xF:
			runSlide();
			break;
		case 0x10:
			runRocks();
			break;
		}
	} else if (phase == 1) {
		colliEnable(mColli);
		killAll();
		eggTypeStopAllFx(this);
		clearExtraFx();
	}
}

// 0x8013611C
void TAniHariAzarashi::runSpread()
{
	f32 a[12];
	f32 b[12];
	f32 c[12];
	Vec3f at = mSliders[0]->mPos;
	int rotY = mSliders[0]->mRot[1];
	mtxIdentity(a);
	mtxTranslateV(a, a, &at);
	mtxRotateY(a, a, rotY);
	mtxTranslate((float*)b, (const float*)a, -mSpread, 0.0f, 0.0f);
	mtxTranslate((float*)c, (const float*)a, mSpread, 0.0f, 0.0f);
	Vec3f left;
	Vec3f right;
	mtxMultVec(b, &gVecZero, &left);
	mtxMultVec(c, &gVecZero, &right);
	mSliders[1]->mPos = left;
	mSliders[2]->mPos = right;

	if (mSpread < 20.0f) {
		mSpread += 2.0f;
		return;
	}

	Vec3f dir;
	dir = mSliders[0]->mVel;
	if (njUnitVector(&dir) < gUnitVectorMin) {
		dir = gVecZero;
	}
	mtxIdentity(a);
	mtxRotateY(b, a, 0xE38);
	mtxRotateY(c, a, -0xE38);
	Vec3f dirLeft;
	Vec3f dirRight;
	mtxMultVecSR(b, &dir, &dirLeft);
	mtxMultVecSR(c, &dir, &dirRight);
	mSliders[0]->mVel   = dir;
	mSliders[1]->mVel   = dirLeft;
	mSliders[2]->mVel   = dirRight;
	mSliders[0]->mState = 1;
	mSliders[1]->mState = 1;
	mSliders[2]->mState = 1;
	mStep               = 0xF;
	sePlay3D(0x1B1, &mPos);
}

static inline void iceRockSetHolder(u16 id, TAniIceRock* rock)
{
	rock->mFlags2 |= 2;
	rock->mHolderId = id;
}

// 0x80136358
void TAniHariAzarashi::runSlide()
{
	mPos = mSliders[0]->mPos;
	if (mSliders[0]->mDone == 0 && mSliders[1]->mDone == 0 && mSliders[2]->mDone == 0) {
		return;
	}
	for (int i = 0; i < 3; i++) {
		TAniIceRock* rock = new TAniIceRock(this, &mSliders[i]->mPos, mObjId);
		mRocks[i]         = rock;
		if (mRocks[i] != 0) {
			mRocks[i]->mScale = 3.0f;
			mRocks[i]->mRise  = -40.0f;
			mRocks[i]->mFlags2 |= 1;
			if (i == 0) {
				iceRockSetHolder(mHolderId, mRocks[i]);
			}
		}
	}
	mStep = 0x10;
}

// 0x80136474
// The game tests rock 0 twice here and never rock 1
void TAniHariAzarashi::runRocks()
{
	if (mRocks[0]->mDone != 0 && mRocks[0]->mDone != 0 && mRocks[2]->mDone != 0) {
		KILL(mRocks[0]);
		KILL(mRocks[1]);
		KILL(mRocks[2]);
		endAttack();
	}
}

// 0x80136514
int TAniHariAzarashi::onHit(const TObjAction* other, int, int* result)
{
	return fillHit(other, result, 5);
}

static int sBone; // 0x802C5690

// 0x80136588
void TAniHariAzarashi::resetBones()
{
	sBone = 0;
}

// 0x80136594
extern "C" void hariBones0()
{
	copyAnimalBones(2, 0, sBone);
	sBone++;
}

// 0x8013662C
extern "C" void hariBones1()
{
	copyAnimalBones(2, 1, sBone);
	switch (sBone) {
	case 0x2C:
		njGetMatrix(gAnimalExtraMatrices[0]);
		break;
	case 0x1E:
		njGetMatrix(gAnimalExtraMatrices[1]);
		break;
	}
	sBone++;
}

// 0x80136700
extern "C" void hariBones2()
{
	copyAnimalBones(2, 2, sBone);
	switch (sBone) {
	case 0x2C:
		njGetMatrix(gAnimalExtraMatrices[0]);
		break;
	case 0x1E:
		njGetMatrix(gAnimalExtraMatrices[1]);
		break;
	}
	sBone++;
}

// 0x801367D4
extern "C" void hariBones3()
{
	copyAnimalBones(2, 3, sBone);
	sBone++;
}

// 0x8013686C
extern "C" void hariBones4()
{
	copyAnimalBones(2, 4, sBone);
	sBone++;
}
