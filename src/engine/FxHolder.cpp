#include "engine/FxHolder.h"
#include "egg/EggCreature.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
struct Fx;

// 0x8018938C
FxMtx::FxMtx()
{
	mtxIdentity(m[0]);
}

// 0x801893BC
void FxMtx::setPos(const Vec3f* pos)
{
	mtxSetTransV(m[0], pos);
}

// 0x801893DC
void FxMtx::setPosRot(const Vec3f* pos, int rotX, int rotY)
{
	mtxSetTransV(m[0], pos);
	mtxRotateY(m[0], m[0], rotY);
	mtxRotateX(m[0], m[0], rotX);
}

// 0x80189430
void FxMtx::setPosRotScale(const Vec3f* pos, int rotX, int rotY, f32 sx, f32 sy, f32 sz)
{
	mtxSetTransV(m[0], pos);
	mtxRotateY(m[0], m[0], rotY);
	mtxRotateX(m[0], m[0], rotX);
	mtxScale(sx, sy, sz, m[0], m[0]);
}

// 0x801894B8
FxHolder::FxHolder()
{
	mEffect = 0;
}

// 0x801894F0
FxHolder::~FxHolder()
{
	fxHolderStop(this);
}

// 0x80189538
extern "C" void fxHolderStop(FxHolder* holder)
{
	if (holder->mEffect != 0) {
		fxStop((Fx*)holder->mEffect);
		holder->mEffect = 0;
	}
}

// 0x80189578
void FxHolder::start(int bank, int id)
{
	if (mEffect == 0) {
		mEffect = fxSpawnFollow(1, bank, id, (f32*)this);
	}
}

// 0x801895BC
void FxHolder::start(int bank, int id, const Vec3f* pos)
{
	start(bank, id);
	if (mEffect != 0) {
		mMtx.setPos(pos);
	}
}

// 0x80189604
void FxHolder::start(int bank, int id, const Vec3f* pos, int rotX, int rotY)
{
	start(bank, id);
	if (mEffect != 0) {
		mMtx.setPosRot(pos, rotX, rotY);
	}
}

// 0x8018965C
FxAttach::FxAttach()
{
	mActive = 0;
}

// 0x80189694
FxAttach::~FxAttach()
{
	stop();
}

// 0x801896DC
void FxAttach::stop()
{
	if (mActive != 0) {
		mActive = 0;
		fxRemoveOwner(this);
	}
}

// 0x80189710
void FxAttach::start(int bank, int id)
{
	if (mActive == 0) {
		Fx* fx = (Fx*)fxSpawnFollow(1, bank, id, (f32*)this);
		if (fx != 0) {
			mActive = 1;
			fxSetOwner(fx, (void*)this);
		}
	}
}
