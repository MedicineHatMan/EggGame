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

typedef char TEneBoss02_Pool_size_check[(sizeof(TEneBoss02_Pool) == 0x54) ? 1 : -1];

typedef char TEneBoss02_Gate_size_check[(sizeof(TEneBoss02_Gate) == 0x54) ? 1 : -1];

// 0x8005F95C
extern "C" int growRadius(f32* radius, f32 step, f32 limit)
{
	f32 next = *radius;
	next *= next;
	next = step + next;
	next = __frsqrte(next);
	next = __fres(next);
	if (next >= limit) {
		*radius = limit;
		return 1;
	}
	*radius = next;
	return 0;
}

// 0x8005F998
extern "C" int shrinkRadius(f32* radius, f32 step)
{
	f32 next = *radius;
	next *= next;
	step = next - step;
	if (step <= 0.0f) {
		*radius = 0.0f;
		return 1;
	}
	f32 root = __frsqrte(step);
	*radius  = __fres(root);
	return 0;
}

// 0x8005F9D8
extern "C" void* createBossPool(TObject* parent, const Vec3f* pos)
{
	return new TEneBoss02_Pool(parent, pos);
}

const char* gTEneBoss02_PoolName = "TEneBoss02_Pool";

// 0x8005FA2C
TEneBoss02_Pool::TEneBoss02_Pool(TObject* parent, const Vec3f* pos)
    : TObjAction(parent)
{
	_00   = (u32)gTEneBoss02_PoolName;
	mSize = 0x54;
	setObjId(this, 0xFFFF);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mState    = 0;
	mRadius   = 2.0f;
	mAlpha    = 1.0f;
	mPos.set(pos->x, pos->y, pos->z);
	mTime    = 0;
	mSurface = createBossRipple();
	bossRippleSetPos(mSurface, pos);
}

// 0x8005FAF0
TEneBoss02_Pool::~TEneBoss02_Pool()
{
	if (mSurface != 0) {
		delete mSurface;
		mSurface = 0;
	}
	removeDrawTask(mDrawTask);
	releaseObjId(this);
}

// 0x8005FB8C
int TEneBoss02_Pool::onHit(const TObjAction* other, int arg, int* result)
{
	int hurt = 0;
	if (isPlayerObj(other) != 0) {
		HitInfo* hit = (HitInfo*)result;
		hit->mDamage = 0xA;
		hurt         = 1;
		hit->mFlags.clear();
	}
	return hurt;
}

// 0x8005FBE0
void TEneBoss02_Pool::draw()
{
	switch (mState) {
	case 0:
		if (growRadius(&mRadius, 60.0f, 30.0f) != 0) {
			mState = 1;
			mTime  = 6;
		}
		bossRippleSetRadius(mSurface, mRadius);
		break;
	case 1:
		mTime--;
		if (mTime < 0) {
			mState = 2;
		}
		break;
	case 2:
		mAlpha -= 0.03f;
		if (mAlpha <= 0.0f) {
			mAlpha = 0.0f;
			mFlags |= 1;
		}
		break;
	}
	mSurface->update();
}

// 0x8005FCD4
void TEneBoss02_Pool::vfn_10()
{
	if (cameraIsVisible(2000.0f, mRadius, &mPos, 0)) {
		setTaskHidden(mDrawTask, 0);
	} else {
		setTaskHidden(mDrawTask, 1);
	}
}

// 0x8005FD3C
void TEneBoss02_Pool::render()
{
	mSurface->draw(mAlpha);
}

// 0x8005FD74
extern "C" void initBossGates()
{
	sBoss02GatePlaces = new Vec3f[6];
	sBoss02GateTaken  = new int[6];
	sBoss02Gates      = new TEneBoss02_Gate*[6];
	sBoss02GateCount  = 0;
	f32 s             = 112.0f;
	f32 c             = 133.0f;
	f32 sign          = 1.0f;
	for (int i = 0; i < 6; i++) {
		njSinCos((i << 16) / 6, &s, &c);
		f32 radius = 10.5f * (3.0517578125e-05f * (f32)rand());
		radius *= sign;
		radius += 122.5f;
		s *= radius;
		c *= radius;
		sBoss02GatePlaces[i].x = s;
		sBoss02GatePlaces[i].y = 610.2f;
		sBoss02GatePlaces[i].z = c;
		sign *= -1.0f;
	}
	for (int i = 0; i < 6; i++) {
		sBoss02GateTaken[i] = 0;
		sBoss02Gates[i]     = 0;
	}
}

// 0x8005FF20
extern "C" void freeBossGates()
{
	if (sBoss02Gates != 0) {
		delete[] sBoss02Gates;
		sBoss02Gates = 0;
	}
	if (sBoss02GateTaken != 0) {
		delete[] sBoss02GateTaken;
		sBoss02GateTaken = 0;
	}
	if (sBoss02GatePlaces != 0) {
		delete[] sBoss02GatePlaces;
		sBoss02GatePlaces = 0;
	}
}

// 0x8005FF84
extern "C" void resetBossGates()
{
	for (int i = 0; i < sBoss02GateCount; i++) {
		if (sBoss02Gates[i] != 0) {
			sBoss02Gates[i]->close();
		}
		sBoss02Gates[i] = 0;
	}
	sBoss02GateCount = 0;
	for (int i = 0; i < 6; i++) {
		sBoss02GateTaken[i] = 0;
		sBoss02Gates[i]     = 0;
	}
}

// 0x80060014
extern "C" void closeBossGates()
{
	for (int i = 0; i < sBoss02GateCount; i++) {
		if (sBoss02Gates[i] != 0) {
			sBoss02Gates[i]->close();
		}
	}
}

// Stripped nothing calls it but its 72.0f a full gate's radius comes before pickGateSlot's 5.0f in the pool
extern "C" f32 bossGateFullRadius()
{
	return 72.0f;
}

// 0x8006006C
extern "C" int pickGateSlot(int taken)
{
	int found = -1;
	int start = (int)(5.0f * (3.0517578125e-05f * (f32)rand()));
	for (int i = start; i < 6; i++) {
		if (taken == sBoss02GateTaken[i]) {
			found = i;
			break;
		}
	}
	if (found == -1) {
		for (int i = 0; i < start; i++) {
			if (taken == sBoss02GateTaken[i]) {
				found = i;
				break;
			}
		}
	}
	return found;
}

// 0x80060150
extern "C" int takeFreeGateSlot(Vec3f* out)
{
	int slot = pickGateSlot(0);
	if (slot != -1) {
		sBoss02GateTaken[slot] = 1;
		*out                   = sBoss02GatePlaces[slot];
	}
	return slot;
}

// 0x800601BC
extern "C" void findTakenGateSlot(Vec3f* out)
{
	int slot = pickGateSlot(1);
	if (slot == -1) {
		out->x = 0.0f;
		out->y = 610.2f;
		out->z = 0.0f;
	} else {
		*out = sBoss02GatePlaces[slot];
	}
}

// 0x80060230
extern "C" void takeGateSlot(int slot, Vec3f* out)
{
	sBoss02GateTaken[slot] = 1;
	*out                   = sBoss02GatePlaces[slot];
}

// 0x80060268
extern "C" void freeGateSlot(int slot)
{
	sBoss02GateTaken[slot] = 0;
}

// 0x8006027C
extern "C" void removeBossGate(TEneBoss02_Gate* gate)
{
	for (int i = 0; i < sBoss02GateCount; i++) {
		if (sBoss02Gates[i] == gate) {
			sBoss02GateCount--;
			sBoss02Gates[i]                = sBoss02Gates[sBoss02GateCount];
			sBoss02Gates[sBoss02GateCount] = 0;
			return;
		}
	}
}

// 0x800602DC
extern "C" void rippleBossGates(const Vec3f* pos, f32 from, f32 to, f32 height)
{
	for (int i = 0; i < sBoss02GateCount; i++) {
		sBoss02Gates[i]->mSurface->ripple(from, to, height, pos);
	}
}

// 0x8006036C
extern "C" TEneBoss02_Gate* makeBossGateAt(TObject* parent, int slot)
{
	if (slot < 0 || slot >= 6 || sBoss02GateCount >= 6) {
		return 0;
	}
	Vec3f pos;
	takeGateSlot(slot, &pos);
	TEneBoss02_Gate* gate            = new TEneBoss02_Gate(parent, slot, &pos);
	sBoss02Gates[sBoss02GateCount++] = gate;
	return gate;
}

// 0x80060408
extern "C" TEneBoss02_Gate* makeBossGate(TObject* parent, Vec3f* pos)
{
	if (sBoss02GateCount >= 6) {
		return 0;
	}
	Vec3f at;
	int slot                         = takeFreeGateSlot(&at);
	TEneBoss02_Gate* gate            = new TEneBoss02_Gate(parent, slot, pos);
	sBoss02Gates[sBoss02GateCount++] = gate;
	return gate;
}

const char* gTEneBoss02_GateName = "TEneBoss02_Gate";

// 0x8006049C
TEneBoss02_Gate::TEneBoss02_Gate(TObject* parent, int slot, const Vec3f* pos)
    : TObjAction(parent)
{
	_00   = (u32)gTEneBoss02_GateName;
	mSize = 0x54;
	setObjId(this, 0xFFFF);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mState    = 0;
	mRadius   = 2.0f;
	mSlot     = slot;
	mPos      = *pos;
	_3C       = 0;
	mSurface  = createBossRipple();
	bossRippleSetPos(mSurface, &mPos);
	createPoolPowGen(objParent(this), pos, 0xC);
}

// 0x80060570
TEneBoss02_Gate::~TEneBoss02_Gate()
{
	if (mSurface != 0) {
		delete mSurface;
		mSurface = 0;
	}
	removeDrawTask(mDrawTask);
	releaseObjId(this);
	freeGateSlot(mSlot);
	removeBossGate(this);
}

// 0x8006061C
int TEneBoss02_Gate::onHit(const TObjAction* other, int arg, int* result)
{
	int hurt = 0;
	if (isPlayerObj(other) != 0) {
		HitInfo* hit = (HitInfo*)result;
		hit->mDamage = 0xA;
		hurt         = 1;
		hit->mFlags.clear();
	}
	return hurt;
}

// 0x80060670
void TEneBoss02_Gate::close()
{
	mState = 2;
}

// 0x8006067C
void TEneBoss02_Gate::draw()
{
	switch (mState) {
	case 0:
		if (growRadius(&mRadius, 60.0f, 72.0f) != 0) {
			mRadius = 72.0f;
			mState  = 1;
		}
		bossRippleSetRadius(mSurface, mRadius);
		break;
	case 2:
		if (shrinkRadius(&mRadius, 60.0f) != 0) {
			mFlags |= 1;
		}
		bossRippleSetRadius(mSurface, mRadius);
		break;
	}
	mSurface->update();
}

// 0x80060744
void TEneBoss02_Gate::vfn_10()
{
	if (cameraIsVisible(2000.0f, mRadius, &mPos, 0)) {
		setTaskHidden(mDrawTask, 0);
	} else {
		setTaskHidden(mDrawTask, 1);
	}
}

// 0x800607AC
void TEneBoss02_Gate::render()
{
	mSurface->draw(1.0f);
}

// 0x800607E0
Vec3f* TEneBoss02_Gate::getPos()
{
	return &mPos;
}

// 0x800607E8
int TEneBoss02_Gate::getKind()
{
	return 3;
}

// 0x800607F0
Vec3f* TEneBoss02_Pool::getPos()
{
	return &mPos;
}

// 0x800607F8
int TEneBoss02_Pool::getKind()
{
	return 3;
}
