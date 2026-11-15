#include "enemy/boss/TEneOrangeBossBall.h"
#include "engine/CharaColli.h"
#include "Ninja/ninja.h"
#include "sound/SeHolder.h"
#include "enemy/ObjEnemy.h"
#include "enemy/boss/TEneBoss03_VolRock.h"
#include "enemy/boss/TEneOrangeBoss.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "engine/Colli.h"
#include "enemy/EnemyFx.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"
#include "object/ObjAction.h"
#include "object/ObjShadow.h"
#include "engine/TParticle.h"
#include "animal/RacerMover.h"
#include "enemy/boss/TEneOrangeBossPunch.h"
struct CharaColli;
struct NJS_OBJECT;
struct DrawTask;

static NJS_OBJECT* sBallModel;    // 0x802C55A8
static NJS_TEXLIST* sBallTexList; // 0x802C55AC
static int sNextBallColor;        // 0x802C55B0

// 0x8024882C
// A sphere of 5
const char* sOrangeBallName = "TEneOrangeBossBall";

static const ColliShape sBallShape = { { 0.0f, 0.0f, 0.0f }, 0, 0, 0, 5.0f, 0.0f, 0.0f, 0, COLLISHAPE_SPHERE, 0 };

// 0x8024885C
// Red blue and yellow
static const f32 sBallColors[3 * 3] = {
	1.0f, 0.0f, 0.4f, // r g b
	0.0f, 0.2f, 1.0f, 1.0f, 1.0f, 0.0f,
};
static const int sBallTrail[3]  = { 0x48, 0x49, 0x4A }; // 0x80248880 particle bank 1
static const int sBallSparks[3] = { 0x4D, 0x4E, 0x4C }; // 0x8024888C particle bank 1
static const int sBallBurst[3]  = { 7, 9, 3 };          // 0x80248898 effect bank 5

// 0x80122234
extern "C" void orangeBallLoad()
{
	sBallModel     = modelAtTEneOrangeBoss(0);
	sBallTexList   = texListTEneOrangeBoss();
	sNextBallColor = 0;
}

// 0x8012226C
extern "C" void orangeBallRelease()
{
	sBallModel   = 0;
	sBallTexList = 0;
}

typedef char TEneOrangeBossBall_size_check[(sizeof(TEneOrangeBossBall) == 0xA4) ? 1 : -1];

// 0x8012227C
TEneOrangeBossBall::TEneOrangeBossBall(TObject* parent, const Vec3f* pos)
    : TObjAction(parent)
{
	_00   = (u32)sOrangeBallName;
	mSize = 0xA4;
	setObjId(this, 0xFFFF);
	mDraw    = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mPos     = *pos;
	mVel.x   = 0.0f;
	mVel.y   = 0.0f;
	mVel.z   = 0.0f;
	mGravity = 0.0f;
	mState   = BALL_HELD;
	mRadius  = 5.0f;
	mTimer   = 0;
	mColor   = sNextBallColor;
	if (++sNextBallColor >= 3) {
		sNextBallColor = 0;
	}
	for (int i = 0; i < 2; i++) {
		mParticles[i] = NULL;
	}
	mShape = colliCreate(1, 1);
	colliCopyShapes((CharaColli*)mShape, &sBallShape);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 2, 255, 0, 0x16);
	colliDisable((CharaColli*)mShape);
	colliShapeSetBit8((CharaColli*)mShape, 0);
	mShadow._30 = 9.0f;
	mShadow._34 = 9.0f;
	shadowEnable(&mShadow);
	placeShadow();
}

// 0x801223F4
TEneOrangeBossBall::~TEneOrangeBossBall()
{
	stopParticles();
	colliRetire((CharaColli*)mShape);
	mShape = 0;
	removeDrawTask(mDraw);
	releaseObjId(this);
}

// 0x80122484
int TEneOrangeBossBall::onHit(const TObjAction* other, int, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	int hurt     = 0;
	if (mState == BALL_FLYING || mState == BALL_SWELL) {
		hit->mFlags.clear();
		hit->mPos.x = 0.0f;
		hit->mPos.y = 0.0f;
		hit->mPos.z = 0.0f;
		if (isPlayerObj(other)) {
			hurt         = 1;
			hit->mDamage = 10;
		} else if (isEggObj(other)) {
			hit->mDamage = 1;
			hurt         = 1;
			hit->mFlags.setBit(0xD);
		}
	}
	return hurt;
}

// 0x8012252C
void TEneOrangeBossBall::placeColli()
{
	ColliShape* shape = colliGetShapes(mShape);
	colliShapeSetPos(shape, &mPos);
	colliShapeSetRadius(shape, mRadius);
}

// 0x80122574
// The shadow lies on the floor at y 50
void TEneOrangeBossBall::placeShadow()
{
	Vec3f ground;
	ground.x = mPos.x;
	ground.y = mPos.y;
	ground.z = mPos.z;
	ground.y = 50.0f;
	Vec3f up;
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;
	shadowSetGround(&mShadow, &ground, &up);
}

// 0x801225D8
void TEneOrangeBossBall::keepParticles()
{
	particleKeepAt((void**)&mParticles[0], &mPos, 1, sBallTrail[mColor]);
	if (mParticles[0] != NULL) {
		mParticles[0]->setScale(8.0f);
	}
	particleKeepAt((void**)&mParticles[1], &mPos, 1, sBallSparks[mColor]);
}

// 0x8012265C
void TEneOrangeBossBall::stopParticles()
{
	for (int i = 1; i >= 0; i--) {
		if (mParticles[i] != NULL) {
			mParticles[i]->mFlags |= 1;
			mParticles[i] = NULL;
		}
	}
}

// 0x80122698
void TEneOrangeBossBall::setPos(const Vec3f* pos)
{
	mPos = *pos;
}

// 0x801226B4
// Flat speed to cover the distance in 60 frames 4 up and the gravity that brings it down on target
void TEneOrangeBossBall::throwAt(const Vec3f* target)
{
	Vec3f dir;
	dir.x = target->x;
	dir.y = target->y;
	dir.z = target->z;
	njSubVector(&dir, &mPos);
	dir.y    = 0.0f;
	f32 dist = njUnitVector(&dir);
	if (dist < gUnitVectorMin) {
		dir = gVecZero;
	}
	f32 speed = dist / 60.0f;
	mVel.x    = dir.x * speed;
	mVel.z    = dir.z * speed;
	mVel.y    = 4.0f;
	mGravity  = gravityInSteps(60, mVel.y, target->y - mPos.y);
	mState    = BALL_FLYING;
	colliEnable(mShape);
	placeColli();
}

// 0x80122798
void TEneOrangeBossBall::draw()
{
	switch (mState) {
	case BALL_HELD:
		placeShadow();
		keepParticles();
		break;
	case BALL_FLYING:
		mVel.y += mGravity;
		njAddVector(&mPos, &mVel);
		keepParticles();
		if (mPos.y <= 50.0f) {
			mPos.y = 50.0f;
			mState = BALL_LANDED;
			mTimer = 8;
			shadowDisable(&mShadow);
			stopParticles();
			colliShapeSetBit10(mShape, 0);
			particleOnceByIdDefault(&mPos, 1, 0x52);
			fxSpawnFlat(5, (void*)sBallBurst[mColor], &mPos);
		}
		placeColli();
		placeShadow();
		break;
	case BALL_LANDED:
		mTimer--;
		if (mTimer <= 0) {
			mState = BALL_SWELL;
			seStartAt(0x148, &mPos);
		}
		break;
	case BALL_SWELL:
		mRadius += 2.5384614f;
		if (mRadius >= 33.0f) {
			mRadius = 33.0f;
			mFlags |= 1;
		}
		placeColli();
		break;
	}
}

// 0x80122914
// Hidden while off camera
void TEneOrangeBossBall::vfn_10()
{
	if (cameraIsVisible(2500.0f, 7.0f, &mPos, NULL)) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x80122974
// A swelling ball is only its shape
void TEneOrangeBossBall::render()
{
	if (mState != BALL_SWELL) {
		drawBall();
	}
}

// 0x801229A0
// In its colour through the constant material with the environment matrix
void TEneOrangeBossBall::drawBall()
{
	njSetTexture(sBallTexList);
	_nj_control_3d_flag_ |= 0x100010;
	f32 argb[4];
	int i   = mColor * 3;
	argb[0] = 1.0f;
	int j   = i++;
	argb[1] = sBallColors[j];
	j       = i++;
	argb[2] = sBallColors[j];
	argb[3] = sBallColors[i];
	njSetConstantMaterial(argb);
	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjDrawObject(sBallModel);
	njPopMatrixEx();
	_nj_control_3d_flag_ &= ~0x100010;
}

const char* gTEneOrangeBossPunchName = "TEneOrangeBossPunch";

struct ColliShape;
struct CharaColli;

extern "C" {
}

typedef char TEneOrangeBossPunch_size_check[(sizeof(TEneOrangeBossPunch) == 0x50) ? 1 : -1];

extern const char* gTEneOrangeBossPunchName;

// 0x80122A58
TEneOrangeBossPunch::TEneOrangeBossPunch(TObject* parent, const Vec3f* from, const Vec3f* to)
    : TObjAction(parent)
{
	_00   = (u32)gTEneOrangeBossPunchName;
	mSize = 0x50;
	mFrom = *from;
	mTo   = *to;
	mTo.y = 0.5f;
	mLife = 30;
	setObjId(this, 0xFFFF);
	mShape = colliCreate(1, 1);
	colliSetOwner((CharaColli*)mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 2, 255, 0, 0x16);
	colliShapeMakeSphere(colliGetShapes((CharaColli*)mShape), &mFrom, 0, 300.0f);
	colliEnable((CharaColli*)mShape);
	colliShapeSetBit10((CharaColli*)mShape, 0);
	colliShapeSetBit8((CharaColli*)mShape, 0);
}

// 0x80122B68
TEneOrangeBossPunch::~TEneOrangeBossPunch()
{
	colliRetire((CharaColli*)mShape);
	mShape = 0;
	releaseObjId(this);
}

// 0x80122BE0
// Hurts a player it touches then ends
int TEneOrangeBossPunch::onHit(const TObjAction* other, int, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	if (isPlayerObj(other)) {
		hit->mFlags.clear();
		hit->mPos.x  = 0.0f;
		hit->mPos.y  = 0.0f;
		hit->mPos.z  = 0.0f;
		hit->mDamage = 20;
		mLife        = 0;
		return 1;
	}
	return 0;
}

// 0x80122C50
void TEneOrangeBossPunch::draw()
{
	mLife--;
	if (mLife <= 0) {
		mFlags |= 1;
	}
}

// 0x80122C78
Vec3f* TEneOrangeBossPunch::getPos()
{
	return &mFrom;
}

// 0x80122C80
int TEneOrangeBossPunch::getKind()
{
	return 2;
}

// 0x80122C88
// The ball's which come after the punch
Vec3f* TEneOrangeBossBall::getPos()
{
	return &mPos;
}

// 0x80122C90
int TEneOrangeBossBall::getKind()
{
	return 2;
}
