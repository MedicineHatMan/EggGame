#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "sound/SeHolder.h"
#include "enemy/boss/TEneBoss02.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "Ninja/ninja.h"
#include "stl/math.h"
#include "object/ObjAction.h"
#include "enemy/ObjEnemy.h"
#include "enemy/boss/TEneBoss02_Pool.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "enemy/EnemyFx.h"
#include "engine/TParticle.h"
#include "enemy/boss/TEneBoss02_PoolPow.h"
#include "enemy/boss/TEneBoss02_GateShot.h"
struct CharaColli;
struct ColliShape;
struct NJS_OBJECT;

typedef char TEneBoss02_WaterShot_Visual_size_check[(sizeof(TEneBoss02_WaterShot_Visual) == 0x38) ? 1 : -1];

/**
 * @size 0x68
 */
class TEneBoss02_WaterShot : public TObjAction {
public:
	TEneBoss02_WaterShot(TObject* parent, const Vec3f* pos, const Vec3f* target);
	virtual ~TEneBoss02_WaterShot();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual Vec3f* getPos();
	virtual int getKind();
	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void onLand(); // _6C

	Vec3f mPos;      // _30
	Vec3f mTarget;   // _3C
	Vec3f mVel;      // _48
	int mTime;       // _54
	CharaColli* mColli;    // _58
	DrawTask* mDrawTask; // _5C
	TEneBoss02_WaterShot_Visual* mVisual; // _60
	int mState;      // _64 0 flying 1 landed
};

typedef char TEneBoss02_WaterShot_size_check[(sizeof(TEneBoss02_WaterShot) == 0x68) ? 1 : -1];

/**
 * @size 0x6C
 */
class TEneBoss02_WaterShot_Gate : public TEneBoss02_WaterShot {
public:
	TEneBoss02_WaterShot_Gate(TObject* parent, const Vec3f* pos, const Vec3f* target, int slot);
	virtual ~TEneBoss02_WaterShot_Gate();

	virtual void onLand();

	int mSlot; // _68
};

typedef char TEneBoss02_WaterShot_Gate_size_check[(sizeof(TEneBoss02_WaterShot_Gate) == 0x6C) ? 1 : -1];

const char* gTEneBoss02_WaterShot_VisualName = "TEneBoss02_WaterShot_Visual";
const char* gTEneBoss02_WaterShotName = "TEneBoss02_WaterShot";
const char* gTEneBoss02_WaterShot_GateName = "TEneBoss02_WaterShot_Gate";

// After the names their strings come first in .rodata
extern "C" {
extern const ColliShape sBoss02_PoolShape[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 7.5f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

// 0x80060800
TEneBoss02_WaterShot_Visual::TEneBoss02_WaterShot_Visual(TObject* parent, const Vec3f* pos)
    : TObject(parent)
{
	_00   = (u32)gTEneBoss02_WaterShot_VisualName;
	mSize = 0x38;
	mPos  = *pos;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mFx  = 0;
	mFx2 = 0;
}

// 0x80060890
TEneBoss02_WaterShot_Visual::~TEneBoss02_WaterShot_Visual()
{
	removeDrawTask(mDrawTask);
	if (mFx2 != 0) {
		mFx2->mFlags |= 1;
		mFx2 = 0;
	}
	if (mFx != 0) {
		mFx->mFlags |= 1;
		mFx = 0;
	}
}

// 0x80060938
void TEneBoss02_WaterShot_Visual::setPos(const Vec3f* pos)
{
	mPos = *pos;
}

// 0x80060954
void TEneBoss02_WaterShot_Visual::draw()
{
	particleKeepAt((void**)&mFx, &mPos, 1, 0x46);
	particleKeepAt((void**)&mFx2, &mPos, 1, 0x44);
}

// 0x800609A4
void TEneBoss02_WaterShot_Visual::vfn_10()
{
	if (cameraIsVisible(2000.0f, 9.5f, &mPos, 0) != 0) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x80060A04
void TEneBoss02_WaterShot_Visual::render()
{
	njSetTexture(sBoss02ShotTextures);
	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjDrawObject(sBoss02ShotModel);
	njPopMatrixEx();
}

// 0x80060A4C
extern "C" void setWaterShotAssets(NJS_OBJECT* model, NJS_TEXLIST* textures)
{
	sBoss02ShotModel = model;
	sBoss02ShotTextures = textures;
}

// 0x80060A58
extern "C" void clearWaterShotAssets()
{
	sBoss02ShotModel = 0;
	sBoss02ShotTextures = 0;
}

// 0x80060A68
extern "C" void* createWaterShot(TObject* parent, const Vec3f* pos, const Vec3f* target)
{
	return new TEneBoss02_WaterShot(parent, pos, target);
}

// 0x80060AC4
TEneBoss02_WaterShot::TEneBoss02_WaterShot(TObject* parent, const Vec3f* pos, const Vec3f* target)
    : TObjAction(parent)
{
	_00   = (u32)gTEneBoss02_WaterShotName;
	mSize = 0x68;
	setObjId(this, 0xFFFF);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	TEneBoss02_WaterShot_Visual* visual = new TEneBoss02_WaterShot_Visual(this, pos);
	mVisual = visual;
	mState  = 0;
	mPos    = *pos;
	mTarget = *target;
	mTime   = 0;
	aimWaterShot(pos, target, &mVel, 0.1f, 1.5f);
	mVel.y += 0.1f;
	njSubVector(&mPos, &mVel);
	mColli = colliCreate(1, 1);
	colliCopyShapes((CharaColli*)mColli, (const ColliShape*)sBoss02_PoolShape);
	colliSetOwner(mColli, (u16)mObjId);
	colliSetMasks((CharaColli*)mColli, 4, 0xFF, 0, 6);
	colliShapeSetBit8((CharaColli*)mColli, 0);
	colliShapeSetTag(mColli, 0, 0);
}

// 0x80060C3C
TEneBoss02_WaterShot::~TEneBoss02_WaterShot()
{
	if (mVisual != 0) {
		mVisual->mFlags |= 1;
		mVisual = 0;
	}
	colliRetire((CharaColli*)mColli);
	mColli = 0;
	removeDrawTask(mDrawTask);
	releaseObjId(this);
}

// 0x80060CDC
extern "C" void aimWaterShot(const Vec3f* from, const Vec3f* to, Vec3f* out, f32 gravity, f32 rise)
{
	Vec3f delta;
	delta = *to;
	njSubVector(&delta, from);
	f32 frames = ceil(quadRoot(gravity, gravity - 2.0f * rise, 2.0f * delta.y));
	delta.y = 0.0f;
	f32 dist = njUnitVector(&delta);
	if (dist < gUnitVectorMin) {
		delta = gVecZero;
	}
	f32 speed = dist / frames;
	out->x = delta.x * speed;
	out->y = rise;
	out->z = delta.z * speed;
}

// 0x80060DC8
int TEneBoss02_WaterShot::onHit(const TObjAction* other, int arg, int* result)
{
	int hurt = 0;
	HitInfo* hit = (HitInfo*)result;
	if (isPlayerObj(other) != 0) {
		hit->mDamage = 0xA;
		hurt = 1;
		hit->mFlags.clear();
		hit->mPos.x = 0.0f;
		hit->mPos.y = 0.0f;
		hit->mPos.z = 0.0f;
	} else if (isEggObj(other) != 0) {
		hit->mDamage = 1;
		hurt = 1;
		hit->mFlags.clear();
		hit->mFlags.setBit(0xD);
		hit->mPos.x = 0.0f;
		hit->mPos.y = 0.0f;
		hit->mPos.z = 0.0f;
	}
	return hurt;
}

// 0x80060E74
void TEneBoss02_WaterShot::onLand()
{
	createBossPool(objParent(this), &mPos);
	noOp__9TObjEnemyFv(4);
	seStartAtFar(0x115, &mPos);
}

// 0x80060EBC
void TEneBoss02_WaterShot::draw()
{
	switch (mState) {
	case 0:
		mVel.y -= 0.1f;
		njAddVector(&mPos, &mVel);
		colliPlaceShapes(mColli, &mPos, (const ColliShape*)sBoss02_PoolShape);
		mVisual->setPos(&mPos);
		if (mPos.y <= 610.2f) {
			mPos.y = 610.2f;
			mState = 1;
			mTime  = 3;
			onLand();
			colliShapeSetRadius(colliGetShapes(mColli), 11.25f);
			particleOnceByIdDefault(&mPos, 1, 0x4B);
			particleOnceByIdDefault(&mPos, 1, 0x4E);
			if (mVisual != 0) {
				mVisual->mFlags |= 1;
				mVisual = 0;
			}
		}
		break;
	case 1:
		mTime--;
		if (mTime <= 0) {
			mFlags |= 1;
		}
		break;
	}
}

// 0x80060FF0
void TEneBoss02_WaterShot::vfn_10()
{
	if (cameraIsVisible(2000.0f, 9.5f, &mPos, 0)) {
		setTaskHidden(mDrawTask, 0);
	} else {
		setTaskHidden(mDrawTask, 1);
	}
}

// 0x80061058
static void drawNothing(const Vec3f*)
{
}

// 0x8006105C
void TEneBoss02_WaterShot::render()
{
	drawNothing(&mPos);
}

// 0x80061080
extern "C" void* createWaterShotGate(TObject* parent, const Vec3f* pos, const Vec3f* target, int slot)
{
	return new TEneBoss02_WaterShot_Gate(parent, pos, target, slot);
}

// 0x800610E4
TEneBoss02_WaterShot_Gate::TEneBoss02_WaterShot_Gate(TObject* parent, const Vec3f* pos, const Vec3f* target, int slot)
    : TEneBoss02_WaterShot(parent, pos, target)
{
	_00   = (u32)gTEneBoss02_WaterShot_GateName;
	mSize = 0x6C;
	mSlot = slot;
}

// 0x80061138
TEneBoss02_WaterShot_Gate::~TEneBoss02_WaterShot_Gate()
{
}

// 0x80061194
void TEneBoss02_WaterShot_Gate::onLand()
{
	makeBossGateAt(objParent(this), mSlot);
	noOp__9TObjEnemyFv(5);
	seStartAtFar(0x116, &mPos);
}

// 0x800611DC
Vec3f* TEneBoss02_WaterShot::getPos()
{
	return &mPos;
}

// 0x800611E4
int TEneBoss02_WaterShot::getKind()
{
	return 3;
}
