#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "sound/SeHolder.h"
#include "enemy/TEneAm02.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "enemy/boss/TEneBoss03Mole.h"
#include "enemy/ObjEnemy.h"
#include "engine/FogBlend.h"
#include "egg/EggCreature.h"
#include "object/ObjAction.h"
#include "enemy/boss/TEneBoss03_VolRock.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "engine/Colli.h"
#include "ui/AeArc.h"
#include "camera/TCameraCtrl.h"
#include "stl/math.h"
#include "object/ObjShadow.h"
#include "engine/TParticle.h"
#include "egg/TMorphEggAnim.h"
struct NJS_OBJECT;
struct GjModel;
struct ColliContact;
struct CharaColli;

// 0x80257C98) throws 50 of them at 5 a frame

const char* gTEneBoss03_VolRockName       = "TEneBoss03_VolRock";
const char* gTEneBoss03_VolRock_DummyName = "TEneBoss03_VolRock_Dummy";

extern "C" {
extern const ColliShape sBoss03_VolRockShape[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 12.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

typedef char VolFlight_size_check[(sizeof(VolFlight) == 0x68) ? 1 : -1];

typedef char TEneBoss03_VolRock_size_check[(sizeof(TEneBoss03_VolRock) == 0x298) ? 1 : -1];

/** A rock for show. @size 0x98 */
class TEneBoss03_VolRock_Dummy : public TObject {
public:
	TEneBoss03_VolRock_Dummy(TObject* parent, const Vec3f* pos, const Vec3f* target);
	virtual ~TEneBoss03_VolRock_Dummy();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void setup(const Vec3f* pos, const Vec3f* target, f32 speed, f32 gravity);

	VolFlight mFlight;   // _20
	int mState;          // _88 0 flying 1 sinking
	f32 mTime;           // _8C
	DrawTask* mDrawTask; // _90
	EnemyFx* mFx;        // _94
};

typedef char TEneBoss03_VolRock_Dummy_size_check[(sizeof(TEneBoss03_VolRock_Dummy) == 0x98) ? 1 : -1];

/** Throws the rocks. @size 0x1C8 */
class VolRockBurst : public TObject {
public:
	VolRockBurst(TObject* parent, const Vec3f* pos, int realCount);
	virtual ~VolRockBurst();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	int mSlot[50];  // _20 a shuffled order of places around the arena
	int mReal[50];  // _E8 shuffled too 1 = a real rock
	int mIndex;     // _1B0
	Vec3f mPos;     // _1B4
	int mAngle;     // _1C0 from mPos to the middle of the arena
	int mAngleBase; // _1C4
};

typedef char VolRockBurst_size_check[(sizeof(VolRockBurst) == 0x1C8) ? 1 : -1];

static inline f32 randUnit()
{
	return 3.0517578125e-05f * (f32)rand();
}

#define RAND_SIGNED() (2.0f * (3.0517578125e-05f * (f32)rand() - 0.5f))

// Not in the DOL the linker stripped it
extern "C" f32 volSignedUnit(f32 r)
{
	return 2.0f * (3.0517578125e-05f * r - 0.5f);
}

// 0x80082E20
extern "C" void volFlightInit(VolFlight* fly, const Vec3f* from, const Vec3f* to, f32 speed, f32 gravity)
{
	fly->mBody.mPos.x = from->x;
	fly->mBody.mPos.y = from->y;
	fly->mBody.mPos.z = from->z;
	fly->mSpinX       = (int)(800.0f * RAND_SIGNED());
	fly->mSpinY       = (int)(1000.0f * RAND_SIGNED());
	fly->mStart.x     = from->x;
	fly->mStart.y     = from->y;
	fly->mStart.z     = from->z;
	fly->mTarget.x    = to->x;
	fly->mTarget.y    = to->y;
	fly->mTarget.z    = to->z;
	Vec3f vel;
	f32 frames        = flatVelocityTo(from, to, &vel, speed);
	f32 up            = launchSpeedFor(frames, to->y - from->y, gravity);
	fly->mBody.mVel.x = vel.x;
	fly->mBody.mVel.y = up;
	fly->mBody.mVel.z = vel.z;
	fly->gravity()    = gravity;
}

// 0x80082F98
// Places the rock at this time and returns how far it rose
extern "C" f32 volFlightStep(VolFlight* fly, f32 time)
{
	f32 oldY = fly->mBody.mPos.y;
	gjMulAddVector(time, &fly->mBody.mPos, &fly->mStart, &fly->mBody.mVel);
	f32 fall = 0.5f * fly->gravity();
	fall *= time;
	fly->mBody.mPos.y = time * fall + fly->mBody.mPos.y;
	f32 dy            = fly->mBody.mPos.y - oldY;
	fly->mBody.mRot.x += fly->mSpinX;
	fly->mBody.mRot.y += fly->mSpinY;
	return dy;
}

// 0x8008303C
extern "C" int volFlightLanded(VolFlight* fly)
{
	if (fly->mBody.mPos.y <= fly->mTarget.y) {
		fly->mBody.mPos.y = fly->mTarget.y;
		return 1;
	}
	return 0;
}

// 0x80083064
extern "C" void volFlightApply(VolFlight* fly)
{
	njTranslateEx(&fly->mBody.mPos);
	gjRotateY(fly->mBody.mRot.y);
	gjRotateX(fly->mBody.mRot.x);
}

// 0x800830A4
// The trail effect stays 12 under the rock
extern "C" void volFlightTrail(VolFlight* fly, EnemyFx** fx)
{
	Vec3f at;
	at.x = fly->mBody.mPos.x;
	at.y = fly->mBody.mPos.y;
	at.z = fly->mBody.mPos.z;
	at.y = fly->mBody.mPos.y - 12.0f;
	if (*fx == 0) {
		*fx = (EnemyFx*)particleGenByIdDefault(&at, 1, 0x47);
	} else {
		EnemyFx* p = *fx;
		p->mPos.x  = at.x;
		p->mPos.y  = at.y;
		p->mPos.z  = at.z;
	}
}

// 0x80083128
extern "C" void volRockLoad()
{
	gVolRockList = 0;
	gRockModel   = (RockModel*)modelTEneBoss03Mole();
	gRockShape   = shapeTEneBoss03Mole();
}

// 0x8008315C
extern "C" void volRockFree()
{
	gVolRockList = 0;
	gRockModel   = 0;
	gRockShape   = 0;
}

extern "C" void lightVolRockFuse(TEneBoss03_VolRock* rock, int time);

// 0x80083170
// Sets them all off each after a time that grows with its distance from pos
extern "C" void lightVolRocksFrom(const Vec3f* pos)
{
	Vec3f delta;
	delta.x  = -pos->x;
	delta.y  = 0.0f;
	delta.z  = -pos->z;
	f32 near = vecLenXZ(&delta) - 190.0f;
	for (TEneBoss03_VolRock* rock = gVolRockList; rock != 0; rock = rock->mNext) {
		delta.x = rock->mFlight.mBody.mPos.x;
		delta.y = rock->mFlight.mBody.mPos.y;
		delta.z = rock->mFlight.mBody.mPos.z;
		njSubVector(&delta, pos);
		delta.y = 0.0f;
		f32 len = njUnitVector(&delta);
		if (len < gUnitVectorMin) {
			delta = gVecZero;
		}
		len = (len - near) / 380.0f;
		len *= 0.5f;
		lightVolRockFuse(rock, (int)(60.0f * len) + 0x48);
	}
}

// 0x800832D4
extern "C" void lightVolRocksInOrder()
{
	int time = 0;
	for (TEneBoss03_VolRock* rock = gVolRockList; rock != 0; rock = rock->mNext) {
		lightVolRockFuse(rock, time);
		time += 2;
	}
}

// 0x80083320
// Whether any rock is within radius of pos
extern "C" int volRockNear(const Vec3f* pos, f32 radius)
{
	for (TEneBoss03_VolRock* rock = gVolRockList; rock != 0; rock = rock->mNext) {
		Vec3f delta;
		delta = *rock->getPos();
		njSubVector(&delta, pos);
		if ((vecLenXZ(&delta) - radius) - 12.0f < 0.0f) {
			return 1;
		}
	}
	return 0;
}

// 0x800833F4
TEneBoss03_VolRock::TEneBoss03_VolRock(TObject* parent, const Vec3f* pos, const Vec3f* target)
    : TObjAction(parent)
{
	setup(pos, target, 4.5f, -0.12f);
}

// 0x800834A0
void TEneBoss03_VolRock::setup(const Vec3f* pos, const Vec3f* target, f32 speed, f32 gravity)
{
	_00   = (u32)gTEneBoss03_VolRockName;
	mSize = 0x298;
	setObjId(this, 0xFFFF);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	volFlightInit(&mFlight, pos, target, speed, gravity);
	mState     = 0;
	mLandFlags = 0;
	mTime      = 0.0f;
	mHotTime   = 0x1E;
	mTimer     = 0;
	mHeat      = 1.0f;
	mCarried   = 0;
	mLight.reset();
	mLitAlpha = 0xFF;
	mHitterId = 0xFFFF;
	for (int i = 0; i < 14; i++) {
		mPiecePos[i].x = 0.0f;
		mPiecePos[i].y = 0.0f;
		mPiecePos[i].z = 0.0f;
		mPieceVel[i].x = 0.0f;
		mPieceVel[i].y = 0.0f;
		mPieceVel[i].z = 0.0f;
	}
	mFade  = 0xFF;
	mFx    = 0;
	mColli = (CharaColli*)colliCreate(1, 1);
	colliCopyShapes(mColli, sBoss03_VolRockShape);
	colliSetOwner(mColli, (u16)mObjId);
	colliSetMasks(mColli, 2, 0xFF, 0, 0x14);
	colliDisable(mColli);
	colliShapeSetBit8(mColli, 0);
	shadowSetGround(&mShadow, pos, 0);
	mShadow._30 = 21.599998f;
	mShadow._34 = 21.599998f;
	shadowDisable(&mShadow);
	mNext = 0;
	if (gVolRockList == 0) {
		gVolRockList = this;
	} else {
		mNext               = gVolRockList->mNext;
		gVolRockList->mNext = this;
	}
}

// 0x8008367C
TEneBoss03_VolRock::~TEneBoss03_VolRock()
{
	TEneBoss03_VolRock* prev = 0;
	for (TEneBoss03_VolRock* rock = gVolRockList; rock != 0; prev = rock, rock = rock->mNext) {
		if (rock == this) {
			break;
		}
	}
	if (prev != 0) {
		prev->mNext = mNext;
	}
	if (gVolRockList == this) {
		gVolRockList = mNext;
	}
	if (mFx != 0) {
		mFx->mFlags |= 1;
		mFx = 0;
	}
	colliRetire(mColli);
	mColli = 0;
	removeDrawTask(mDrawTask);
	releaseObjId(this);
}

// 0x800837AC
int TEneBoss03_VolRock::onHit(const TObjAction* other, int arg, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	int took     = 0;
	if ((mState == 1 || mState == 0) && isCold() == 0) {
		hit->mFlags.clear();
		hit->mPos.x = 0.0f;
		hit->mPos.y = 0.0f;
		hit->mPos.z = 0.0f;
		if (isPlayerObj(other) != 0) {
			took         = 1;
			hit->mDamage = 0xA;
		} else if (isEggObj(other) != 0) {
			hit->mDamage = 1;
			took         = 1;
			hit->mFlags.setBit(0xD);
		}
	}
	return took;
}

// 0x80083860
// 0 = nothing 1 = hit for damage 2 = hit for none 3 = struck with flag 0xB or a touch with no hit
struct VolRockMsg {
	u16 mType; // _00
	u16 _02;
};

struct VolRockFromMsg {
	u16 mType; // _00
	u16 _02;
	u16 mFrom; // _04
	u16 _06;
};

int TEneBoss03_VolRock::checkHits()
{
	HitInfo hit;
	for (ColliContact* it = mColli->mContacts; it != 0; it = it->mNext) {
		int id = colliContactOwner(it);
		if (colliContactHasBit40(it) != 0) {
			mHitterId = id;
			return 3;
		}
		if (colliContactHasBit8(it) != 0) {
			TObjAction* obj = findObjById(id);
			if (obj != 0) {
				if ((colliContactShape(it)->mFlags & 0x400) != 0) {
					VolRockMsg msg;
					VolRockFromMsg from;
					msg.mType  = 0x74;
					from.mType = 0x2A;
					from.mFrom = mObjId;
					obj->sendToHandle((ObjMsg*)&from);
					obj->sendToHandle((ObjMsg*)&msg);
				}
				if (obj->onHit(this, colliContactTag(it), (int*)&hit) != 0) {
					if (hit.mFlags.testBit(0xB) != 0) {
						return 3;
					}
					return (hit.mDamage <= 0 ? 1 : 0) + 1;
				}
			}
		}
	}
	return 0;
}

// 0x800839B4
int TEneBoss03_VolRock::isLanded()
{
	return mState == 1;
}

// 0x800839C8
int TEneBoss03_VolRock::isBroken()
{
	return mState == 4;
}

// 0x800839DC
int TEneBoss03_VolRock::isCold()
{
	return (u8)__rlwnm(1, __cntlzw(mHotTime), 31, 31);
}

// 0x800839F4
extern "C" void lightVolRockFuse(TEneBoss03_VolRock* rock, int time)
{
	if (rock->mState == 1) {
		rock->mState         = 3;
		rock->mTimer         = time;
		rock->mPiecePos[0].x = rock->mFlight.mBody.mPos.x;
		rock->mPiecePos[0].y = rock->mFlight.mBody.mPos.y;
		rock->mPiecePos[0].z = rock->mFlight.mBody.mPos.z;
	}
}

// 0x80083A28
void TEneBoss03_VolRock::updateGround()
{
	Vec3f ground;
	Vec3f normal;
	if (probeGround(&mFlight.mBody.mPos, &ground, &normal) != 0) {
		colliEnable(mColli);
		shadowSetGround(&mShadow, &ground, &normal);
		shadowEnable(&mShadow);
	} else {
		colliDisable(mColli);
		shadowDisable(&mShadow);
	}
}

// 0x80083A9C
void TEneBoss03_VolRock::breakUp()
{
	mState = 4;
	mTimer = 0x1E;
	colliDisable(mColli);
	shadowDisable(&mShadow);
	if (mFx != 0) {
		mFx->mFlags |= 1;
		mFx = 0;
	}
	placePieces();
	seStartAt(0x4C, &mFlight.mBody.mPos);
}

// 0x80083B18
// Each piece starts where it sits in the model flying out from the middle
void TEneBoss03_VolRock::placePieces()
{
	gjPushUnitMatrix();
	volFlightApply(&mFlight);
	Vec3f centre;
	centre.x   = mFlight.mBody.mPos.x;
	centre.y   = mFlight.mBody.mPos.y;
	centre.z   = mFlight.mBody.mPos.z;
	Vec3f* pos = (Vec3f*)mPiecePos;
	Vec3f* vel = mPieceVel;
	for (int i = 0; i < 14; i++) {
		njCalcVectorEx((const Vec3f*)(gRockModel->mPieces[i] + 8), pos);
		vel->x = pos->x;
		vel->y = pos->y;
		vel->z = pos->z;
		njAddVector(pos, &centre);
		vel->y = 0.0f;
		if (njUnitVector(vel) < gUnitVectorMin) {
			vel->x = gVecZero.x;
			vel->y = gVecZero.y;
			vel->z = gVecZero.z;
		}
		vel->y = 3.5f * randUnit() + 1.5f;
		pos++;
		vel++;
	}
	njPopMatrixEx();
}

// 0x80083CA4
void TEneBoss03_VolRock::stepPieces()
{
	Vec3f* vel = mPieceVel;
	Vec3f* pos = mPiecePos;
	for (int i = 0; i < 14; i++) {
		vel->y += -0.18f;
		njAddVector(pos, vel);
		pos++;
		vel++;
	}
}

// 0x80083D14
void TEneBoss03_VolRock::draw()
{
	int hit = checkHits();
	if (isLanded() != 0 && isCold() != 0) {
		if (hit == 2) {
			breakUp();
		} else if (hit == 1) {
			spawnDropA(&mFlight.mBody.mPos);
			breakUp();
		} else if (hit == 3) {
			mCarried = (u8*)new TMorphEggAnim(this, getPos(), 1.0f);
			breakUp();
		}
	}
	switch (mState) {
	case 3:
		mTimer--;
		if (mTimer <= 0) {
			breakUp();
		}
		break;
	}
	switch (mState) {
	case 0:
		mTime += 1.0f;
		if (volFlightStep(&mFlight, mTime) < 0.0f) {
			updateGround();
			if (volFlightLanded(&mFlight) != 0) {
				mState = 1;
				mLandFlags |= 1;
				colliEnable(mColli);
				seStartAt(0x12E, &mFlight.mBody.mPos);
				if (shadowIsEnabled(&mShadow) != 0) {
					Vec3f at;
					at = mShadow.mGround;
					bindTEneBoss03Mole(&at);
				}
			}
		}
		colliPlaceShapes(mColli, &mFlight.mBody.mPos, sBoss03_VolRockShape);
		volFlightTrail(&mFlight, &mFx);
		break;
	case 2:
		if ((mCarried[0x30] & 1) != 0) {
			Vec3f at;
			at.x = mFlight.mBody.mPos.x;
			at.y = mFlight.mBody.mPos.y;
			at.z = mFlight.mBody.mPos.z;
			at.y += 12.0f;
			fogBlendHit(&at, 0, mHitterId);
			mFlags |= 1;
		}
		break;
	case 3: {
		f32 dx               = RAND_SIGNED();
		f32 dz               = RAND_SIGNED();
		f32 ox               = 0.5f * dx;
		f32 oz               = 0.5f * dz;
		mFlight.mBody.mPos.x = mPiecePos[0].x + ox;
		mFlight.mBody.mPos.z = mPiecePos[0].z + oz;
		break;
	}
	case 4:
		mTimer--;
		if (mTimer < 0) {
			mFade -= 6;
			if (mFade <= 0) {
				mFlags |= 1;
			}
		}
		stepPieces();
		break;
	}
	if ((mLandFlags & 1) != 0) {
		if (mHotTime <= 0x1E && mFx != 0) {
			mFx->mFlags |= 1;
			mFx = 0;
		}
		if (mHotTime <= 0) {
			mHeat -= 0.01f;
			if (mHeat <= 0.0f) {
				mHeat = 0.0f;
			}
		} else {
			mHotTime--;
		}
	}
}

// 0x800840AC
void TEneBoss03_VolRock::vfn_10()
{
	if (mLight.checkView(2500.0f, 14.0f, &mFlight.mBody.mPos, 0) != 0) {
		mFlags &= ~0x10;
		u32 alpha;
		int translucent = mLight.isFaded((int*)&alpha);
		if (isBroken() != 0) {
			int fade = mFade;
			translucent |= (fade < 0xFF);
			if (alpha > fade) {
				alpha = fade;
			}
		}
		mLitAlpha = alpha;
		if (translucent != 0) {
			moveDrawTask(mDrawTask, 0x22);
		} else {
			moveDrawTask(mDrawTask, 0x21);
		}
	} else {
		mFlags |= 0x10;
	}
}

// 0x80084190
static void setRockAmbient(int matSrc)
{
	GXSetChanAmbColor(GX_ALPHA0, *(GXColor*)&sVolRockColor);
	GXSetChanCtrl(GX_ALPHA0, GX_ENABLE, GX_SRC_REG, matSrc, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
}

// 0x800841EC
static void rockChanHot(int, int, int ambSrc, int matSrc)
{
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, ambSrc, matSrc, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, ambSrc, matSrc, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
}

// 0x80084250
static void rockChanHotFaded(int, int, int ambSrc, int matSrc)
{
	GXSetChanCtrl(GX_COLOR0, GX_DISABLE, ambSrc, matSrc, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	setRockAmbient(matSrc);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, ambSrc, matSrc, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
}

// 0x800842BC
static void rockChanLit(int, u8 enable, int ambSrc, int matSrc, int lights)
{
	GXSetChanCtrl(GX_COLOR0, enable, ambSrc, matSrc, lights, GX_DF_CLAMP, GX_AF_NONE);
	setRockAmbient(matSrc);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, ambSrc, matSrc, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
}

// 0x80084320
static void rockBlend(int mode, int src, int dst, int op)
{
	if (mode == 0) {
		GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
		GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
	} else {
		GXSetBlendMode(mode, src, dst, op);
	}
}

// 0x80084378
static void rockTexGen(int gen)
{
	sVolRockStage = gen & 0xFF;
	GXSetTexCoordGen2(gen & 0xFF, GX_TG_MTX2X4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
}

// 0x800843B4
static void rockTevStage(int stage)
{
	stage &= 0xFF;
	GXSetTevOrder(stage, sVolRockStage, GX_TEXMAP1, GX_COLOR_NULL);
	GXSetTevKColorSel(stage, GX_TEV_KCSEL_K0);
	GXSetTevKAlphaSel(stage, GX_TEV_KASEL_K0_A);
	GXSetTevColorIn(stage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_APREV, GX_CA_KONST, GX_CA_ZERO);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
}

static inline void setRockDrawIF(void* chan, void* blend, u32 mask)
{
	void* funcs[8];
	funcs[5] = chan;
	funcs[6] = blend;
	gjChangeDrawObjIF(funcs, mask);
}

// 0x80084470
// The first texture borrows the entry at _08 while the hot rock draws
extern "C" void beginRockHot()
{
	void* funcs[7];
	NJS_TEXLIST* list         = gRockShape[2];
	sVolRockTexWord           = list->textures[1].texaddr;
	list->textures[1].texaddr = list->textures[0].texaddr;
	njSetTexture(list);
	if (sVolRockClear != 0) {
		funcs[5] = (void*)rockChanHotFaded;
		funcs[6] = (void*)rockBlend;
		gjChangeDrawObjIF(funcs, 0x88);
	} else {
		funcs[5] = (void*)rockChanHot;
		gjChangeDrawObjIF(funcs, 0x80);
	}
}

// 0x800844FC
extern "C" void endRockHot()
{
	setRockDrawIF(0, 0, 0x88);
	NJS_TEXNAME* tex = gRockShape[2]->textures;
	tex[1].texaddr   = sVolRockTexWord;
}

// 0x80084544
// Texture 1 fades out over the cold rock as it cools
extern "C" void beginRockCooling(f32 heat)
{
	if (sVolRockClear != 0) {
		void* funcs[8];
		funcs[6] = (void*)rockBlend;
		gjChangeDrawObjIF(funcs, 8);
	}
	njSetTexture(gRockShape[2]);
	gjLoadTextureNum(0, 1, 1, 1);
	_nj_control_3d_flag_ |= 0x80000;
	gjSetNumPostChans(0);
	gjSetNumPostTexGens(1);
	gjSetNumPostTevStages(1);
	gjSetPostTexGenCallback((void*)rockTexGen);
	gjSetPostTevStageCallback((void*)rockTevStage);
	f32 level = (u8)(255.0f * heat);
	GXColor color;
	color.r = level;
	color.g = level;
	color.b = level;
	if (sVolRockClear != 0) {
		color.a = ((u8*)&sVolRockColor)[3];
	} else {
		color.a = 0xFF;
	}
	GXSetTevKColor(GX_KCOLOR0, color);
}

// 0x80084684
extern "C" void endRockCooling()
{
	_nj_control_3d_flag_ &= ~0x80000;
	gjSetNumPostChans(0);
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	gjSetPostTexGenCallback(0);
	gjSetPostTevStageCallback(0);
	_nj_control_3d_flag_ &= ~0x80000;
	if (sVolRockClear != 0) {
		void* funcs[8];
		funcs[6] = 0;
		gjChangeDrawObjIF(funcs, 8);
	}
}

// 0x80084700
extern "C" void beginRockCold()
{
	if (sVolRockClear != 0) {
		setRockDrawIF((void*)rockChanLit, (void*)rockBlend, 0x88);
	}
	njSetTexture(gRockShape[2]);
}

// 0x80084758
extern "C" void endRockCold()
{
	if (sVolRockClear != 0) {
		setRockDrawIF(0, 0, 0x88);
	}
}

// 0x80084798
void TEneBoss03_VolRock::drawModel()
{
	if (isBroken() != 0) {
		for (int i = 0; i < 14; i++) {
			njPushMatrixEx();
			njTranslateEx(&mPiecePos[i]);
			gjRotateY(mFlight.mBody.mRot.y);
			gjRotateX(mFlight.mBody.mRot.x);
			gjDrawModel((GjModel*)(*(void**)(gRockModel->mPieces[i] + 4)));
			njPopMatrixEx();
		}
	} else {
		njPushMatrixEx();
		volFlightApply(&mFlight);
		gjDrawObject((NJS_OBJECT*)gRockModel->mObject);
		njPopMatrixEx();
	}
}

// 0x80084844
void TEneBoss03_VolRock::render()
{
	if (mLitAlpha < 0xFF) {
		sVolRockClear             = 1;
		GXColor color             = *(GXColor*)&sVolRockSeeColor;
		color.a                   = mLitAlpha;
		*(GXColor*)&sVolRockColor = color;
	} else {
		sVolRockClear             = 0;
		GXColor black             = { 0, 0, 0, 0xFF };
		*(GXColor*)&sVolRockColor = black;
	}
	if (mHeat >= 1.0f) {
		beginRockHot();
		drawModel();
		endRockHot();
	} else if (mHeat > 0.0f) {
		beginRockCooling(mHeat);
		drawModel();
		endRockCooling();
	} else {
		beginRockCold();
		drawModel();
		endRockCold();
	}
	shadowSetAngle(&mShadow, 0);
}

// 0x8008494C
TEneBoss03_VolRock_Dummy::TEneBoss03_VolRock_Dummy(TObject* parent, const Vec3f* pos, const Vec3f* target)
    : TObject(parent)
{
	setup(pos, target, 4.5f, -0.12f);
}

// 0x800849B0
void TEneBoss03_VolRock_Dummy::setup(const Vec3f* pos, const Vec3f* target, f32 speed, f32 gravity)
{
	_00       = (u32)gTEneBoss03_VolRock_DummyName;
	mSize     = 0x98;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	volFlightInit(&mFlight, pos, target, speed, gravity);
	mTime  = 0.0f;
	mState = 0;
	mFx    = 0;
}

// 0x80084A60
TEneBoss03_VolRock_Dummy::~TEneBoss03_VolRock_Dummy()
{
	if (mFx != 0) {
		mFx->mFlags |= 1;
		mFx = 0;
	}
	removeDrawTask(mDrawTask);
}

// 0x80084B18
void TEneBoss03_VolRock_Dummy::draw()
{
	switch (mState) {
	case 0: {
		mTime += 1.0f;
		f32 rose = volFlightStep(&mFlight, mTime);
		if (rose < 0.0f && volFlightLanded(&mFlight) != 0) {
			mFlight.mBody.mVel.y = rose;
			mState               = 1;
		}
		break;
	}
	case 1: {
		gjScaleVector(0.2f, &mFlight.mBody.mVel, &mFlight.mBody.mVel);
		f32 vy = mFlight.mBody.mVel.y;
		if (vy >= -0.15f) {
			vy = -0.15f;
		}
		mFlight.mBody.mVel.y = vy;
		njAddVector(&mFlight.mBody.mPos, &mFlight.mBody.mVel);
		if (mFlight.mBody.mPos.y - mFlight.mTarget.y < -18.0f) {
			mFlags |= 1;
		}
		break;
	}
	}
	volFlightTrail(&mFlight, &mFx);
}

// 0x80084C20
void TEneBoss03_VolRock_Dummy::vfn_10()
{
	if (cameraIsVisible(2500.0f, 14.0f, &mFlight.mBody.mPos, 0) != 0) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x80084C80
void TEneBoss03_VolRock_Dummy::render()
{
	sVolRockClear = 0;
	beginRockHot();
	njPushMatrixEx();
	volFlightApply(&mFlight);
	gjDrawObject((NJS_OBJECT*)gRockModel->mObject);
	njPopMatrixEx();
	endRockHot();
}

// 0x80084CD4
extern "C" void swapInts(int* list, int a, int b)
{
	int swap = list[a];
	list[a]  = list[b];
	list[b]  = swap;
}

// 0x80084CF0
extern "C" void createVolRockBurst(TObject* parent, const Vec3f* pos, int realCount)
{
	new VolRockBurst(parent, pos, realCount);
	seStartAt(0x12D, pos);
}

// 0x80084D50
VolRockBurst::VolRockBurst(TObject* parent, const Vec3f* pos, int realCount)
    : TObject(parent)
{
	mPos       = *pos;
	mIndex     = 0;
	mAngleBase = (int)(65536.0f * randUnit()) / 0x18;
	Vec3f delta;
	delta.x = 0.0f;
	delta.y = 200.0f;
	delta.z = 0.0f;
	njSubVector(&delta, &mPos);
	mAngle = (int)(10430.38043493439 * (f32)atan2(delta.x, delta.z));
	if (realCount > 50) {
		realCount = 50;
	}
	for (int i = 0; i < 50; i++) {
		mSlot[i] = i;
		mReal[i] = 0;
	}
	for (int i = 0; i < realCount; i++) {
		mReal[i] = 1;
	}
	for (int i = 49; i > 0; i--) {
		swapInts(mSlot, i, (int)(49.0f * randUnit()));
		swapInts(mReal, i, (int)(49.0f * randUnit()));
	}
}

// 0x80084F60
VolRockBurst::~VolRockBurst()
{
}

// 0x80084FBC
void VolRockBurst::draw()
{
	int i;
	int start = mIndex;
	mIndex += 5;
	int end = mIndex;
	if (end > 50) {
		end = 50;
		mFlags |= 1;
	}
	for (i = start; i < end; i++) {
		f32 s;
		f32 c;
		njSinCos(mAngleBase + mSlot[i] * 0xAAA, &s, &c);
		Vec3f target;
		f32 dist;
		f32 spread;
		if (mReal[i] > 0) {
			dist     = 140.0f;
			target.x = 0.0f;
			target.y = 200.0f;
			target.z = 0.0f;
			spread   = 46.0f;
		} else {
			dist     = 270.0f;
			target.x = 0.0f;
			target.y = 180.0f;
			target.z = 0.0f;
			spread   = 60.0f;
		}
		f32 range = spread * RAND_SIGNED() + dist;
		target.x += s * range;
		target.y += 6.0f;
		target.z += c * range;
		Vec3f from;
		from.x = target.x;
		from.y = target.y;
		from.z = target.z;
		njSubVector(&from, &mPos);
		int diff = angleDiff(mAngle, (int)(10430.38043493439 * (f32)atan2(from.x, from.z)));
		njSinCos(mAngle + diff * 3, &s, &c);
		from.x = 90.0f * s;
		from.y = 0.0f;
		from.z = 90.0f * c;
		njAddVector(&from, &mPos);
		if (mReal[i] > 0) {
			new TEneBoss03_VolRock(objParent(this), &from, &target);
		} else {
			new TEneBoss03_VolRock_Dummy(objParent(this), &from, &target);
		}
	}
}

// 0x8008527C
Vec3f* TEneBoss03_VolRock::getPos()
{
	return &mFlight.mBody.mPos;
}

// 0x80085284
int TEneBoss03_VolRock::getKind()
{
	return 2;
}
