#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyCommon.h"
#include "enemy/boss/BossParts.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "Ninja/ninja.h"
#include "engine/DrawState.h"
#include "camera/TCameraCtrl.h"
#include "enemy/EnemyFx.h"
#include "enemy/TEneDeathCrow.h"
#include "core/TMainTask.h"

typedef char TEneDeathCrow_size_check[(sizeof(TEneDeathCrow) == 0x48) ? 1 : -1];

/**
 * @size 0x30
 */
class TEneDeathCrowGen : public TObject {
public:
	TEneDeathCrowGen(TObject* parent, const Vec3f* pos);
	virtual ~TEneDeathCrowGen();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	Vec3f mPos; // _20
	int mCount; // _2C
};

typedef char TEneDeathCrowGen_size_check[(sizeof(TEneDeathCrowGen) == 0x30) ? 1 : -1];

// 0x800E8310
// With its cry
extern "C" void* createDeathCrowLoud(TObject* parent, const Vec3f* pos)
{
	return new TEneDeathCrow(parent, pos, 0);
}

// 0x800E8368
extern "C" void* createDeathCrow(TObject* parent, const Vec3f* pos)
{
	return new TEneDeathCrow(parent, pos, 1);
}

const char* gTEneDeathCrowName = "TEneDeathCrow";

// 0x800E83C0
TEneDeathCrow::TEneDeathCrow(TObject* parent, const Vec3f* pos, int quiet)
    : TObject(parent)
{
	_00       = (u32)gTEneDeathCrowName;
	mSize     = 0x48;
	mPos.x    = pos->x;
	mPos.y    = pos->y;
	mPos.z    = pos->z;
	mRotY     = (int)(65536.0f * (3.0517578125e-05f * (f32)rand()));
	f32 speed = 0.7f * (3.0517578125e-05f * (f32)rand()) + 0.1f;
	njSinCos(mRotY, &mVel.x, &mVel.z);
	mVel.x *= speed;
	mVel.y = 0.0f;
	mVel.z *= speed;
	mFrame    = 0.0f;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mLife     = 0x78;
	if (quiet == 0) {
		seStartAt(0x67, &mPos);
	}
}

// 0x800E851C
TEneDeathCrow::~TEneDeathCrow()
{
	removeDrawTask(mDrawTask);
}

// 0x800E8584
void TEneDeathCrow::draw()
{
	mLife--;
	if (mLife <= 0) {
		mFlags |= 1;
	}
	mFrame += 1.0f;
	if (mFrame > 20.0f) {
		mFrame -= 20.0f;
	}
	mVel.y += 0.03f;
	njAddVector(&mPos, &mVel);
}

// 0x800E860C
void TEneDeathCrow::vfn_10()
{
	if (cameraIsVisible(800.0f, 10.0f, &mPos, 0) != 0) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x800E866C
void TEneDeathCrow::render()
{
	enemyCommonSetEye(0);
	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjRotateY(mRotY);
	njScale(0, 2.0f, 2.0f, 2.0f);
	u8* motion = (u8*)enemyCommonMotion();
	drawMotionDefault(enemyCrowModel(), *(void**)(motion + 0x14), mFrame);
	njPopMatrixEx();
	enemyCommonEndEye();
}

// 0x800E86E4
extern "C" void* createDeathCrowGen(TObject* parent, const Vec3f* pos)
{
	return new TEneDeathCrowGen(parent, pos);
}

// 0x800E8738
// A generator at each part in a model's list whose height offset by base falls within a band
extern "C" void spawnDeathCrowsOnParts(NJS_OBJECT* model, f32 base, f32 high, f32 low)
{
	f32 bottom;
	f32 top = high + base;
	bottom  = low + base;
	Vec3f at;
	Vec3f origin;
	origin.x = model->pos[0];
	origin.y = model->pos[1];
	origin.z = model->pos[2];
	for (NJS_OBJECT* part = model->child; part != 0; part = part->sibling) {
		at.x = part->pos[0];
		at.y = part->pos[1];
		at.z = part->pos[2];
		njAddVector(&at, &origin);
		if (bottom <= at.y && at.y < top) {
			createDeathCrowGen(gTL_TIMESTOP, &at);
		}
	}
}

const char* gTEneDeathCrowGenName = "TEneDeathCrowGen";

// 0x800E87F8
TEneDeathCrowGen::TEneDeathCrowGen(TObject* parent, const Vec3f* pos)
    : TObject(parent)
{
	_00    = (u32)gTEneDeathCrowGenName;
	mSize  = 0x30;
	mPos.x = pos->x;
	mPos.y = pos->y;
	mPos.z = pos->z;
	mCount = 0;
}

// 0x800E8868
TEneDeathCrowGen::~TEneDeathCrowGen()
{
}

// -1..1 from rand() Its literals are pooled where it is defined ahead of draw's 5.0f
inline f32 deathCrowRandSigned()
{
	return 2.0f * (3.0517578125e-05f * (f32)rand() - 0.5f);
}

// 0x800E88C4
void TEneDeathCrowGen::draw()
{
	if (mCount % 5 == 0) {
		Vec3f at;
		at.x = 5.0f * deathCrowRandSigned();
		at.y = 0.0f;
		at.z = 5.0f * deathCrowRandSigned();
		njAddVector(&at, &mPos);
		createDeathCrow(objParent(this), &at);
	}
	mCount++;
	if (mCount >= 5) {
		mFlags |= 1;
	}
}

// 0x800E89C8
BossSuit::BossSuit()
{
	mAlpha = 0;
	_20 = mSinkTo = mSinkFrom = mSinkRate = mSinkStart = mHeight = mFadeRate = mFadeStart = 0.0f;
	mFxKind                                                                               = 1;
	for (int i = 0; i < 2; i++) {
		mFx[i]   = 0;
		mFxId[i] = 0;
	}
}

// 0x800E8A20
BossSuit::~BossSuit()
{
	bossSuitClearFx(this);
}

// 0x800E8A68
extern "C" void bossSuitSetFade(f32 start, f32 end, BossSuit* suit)
{
	suit->mFadeStart = start;
	suit->mFadeRate  = 1.0f / (end - start);
}

// 0x800E8A80
// 1 once the fade has begun
extern "C" int bossSuitFade(f32 time, BossSuit* suit)
{
	f32 t = suit->mFadeRate * (time - suit->mFadeStart);
	if (t < 0.0f) {
		suit->mAlpha = 0xFF;
	} else if (t >= 1.0f) {
		suit->mAlpha = 0;
	} else {
		t            = 1.0f - t;
		suit->mAlpha = (u8)(int)(255.0f * t);
	}
	return suit->mAlpha != 0xFF;
}

// 0x800E8B04
extern "C" void bossSuitSetSink(f32 start, f32 end, f32 from, f32 to, f32 extra, BossSuit* suit)
{
	suit->mHeight    = from;
	suit->mSinkStart = start;
	suit->mSinkRate  = 1.0f / (end - start);
	suit->mSinkFrom  = from;
	suit->mSinkTo    = to;
	suit->_20        = extra;
}

// 0x800E8B2C
// 1 once the sink has begun
extern "C" int bossSuitSink(f32 time, BossSuit* suit, const Vec3f* pos, void* model)
{
	int started = 1;
	f32 t       = suit->mSinkRate * (time - suit->mSinkStart);
	if (t <= 0.0f) {
		started       = 0;
		suit->mHeight = suit->mSinkFrom;
		bossSuitClearFx(suit);
	} else {
		if (t >= 1.0f) {
			suit->mHeight = suit->mSinkTo;
			bossSuitClearFx(suit);
		} else {
			f32 high      = suit->mHeight;
			suit->mHeight = suit->mSinkTo * t + suit->mSinkFrom * (1.0f - t);
			if (pos != 0) {
				if (model != 0) {
					spawnDeathCrowsOnParts((NJS_OBJECT*)model, pos->y, high, suit->mHeight);
				}
				bossSuitKeepFx(suit, pos);
			}
		}
	}
	return started;
}

// 0x800E8BFC
extern "C" void bossSuitClearFx(BossSuit* suit)
{
	for (int i = 1; i >= 0; i--) {
		if (suit->mFx[i] != 0) {
			suit->mFx[i]->mFlags |= 1;
			suit->mFx[i] = 0;
		}
	}
}

// 0x800E8C38
extern "C" void bossSuitSetFx(BossSuit* suit, int kind, int idA, int idB)
{
	suit->mFxKind  = kind;
	suit->mFxId[0] = idA;
	suit->mFxId[1] = idB;
}

// 0x800E8C48
extern "C" void bossSuitKeepFx(BossSuit* suit, const Vec3f* pos)
{
	Vec3f at;
	at = *pos;
	at.y += suit->mHeight;
	particleKeepAt((void**)&suit->mFx[0], &at, suit->mFxKind, suit->mFxId[0]);
	particleKeepAt((void**)&suit->mFx[1], &at, suit->mFxKind, suit->mFxId[1]);
}

// 0x800E8CBC
// 1 while either part has begun
extern "C" int bossSuitUpdate(f32 time, BossSuit* suit, const Vec3f* pos, void* model)
{
	int fade = bossSuitFade(time, suit);
	int sink = bossSuitSink(time, suit, pos, model);
	u8 on    = 0;
	if (fade != 0 || sink != 0) {
		on = 1;
	}
	return on;
}

// 0x800E8D30
extern "C" int bossSuitUpdateNoFx(f32 time, BossSuit* suit)
{
	int fade = bossSuitFade(time, suit);
	int sink = bossSuitSink(time, suit, 0, 0);
	u8 on    = 0;
	if (fade != 0 || sink != 0) {
		on = 1;
	}
	return on;
}
