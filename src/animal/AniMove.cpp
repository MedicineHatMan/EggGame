#include "animal/AniMount.h"
#include "object/ObjAction.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "player/Player.h"
struct NJS_QUATERNION;

// 0x801C5C9C
AniMove::AniMove()
    : mBody(NULL)
    , mStick(0.0f)
{
}

// 0x801C5CBC
void AniMove::setBody(AniBody* body)
{
	mBody = body;
}

// 0x801C5CC4
void AniMove::setStick(f32 stick, int angle)
{
	mStick      = stick;
	mStickAngle = angle;
}

// 0x801C5CD0
int AniMove::getStick(f32* stick, int* angle)
{
	if (stick != NULL) {
		*stick = mStick;
	}
	if (angle != NULL) {
		*angle = mStickAngle;
	}
	return mStick > 0.0f;
}

// 0x801C5D08
void AniMove::step()
{
	update(1.0f / 60.0f);
	mStick = 0.0f;
}

// 0x801C5D4C
void AniMove::setRotY(int angle)
{
	Quat quat;
	njCreateQuaternion(angle, 0.0f, 1.0f, 0.0f, (NJS_QUATERNION*)&quat.w);
	mBody->mQuat = quat;
}

// 0x801C5DB0
int AniMove::getRotY() const
{
	Vec3f dir;
	Quat quat;
	f32 mtx[12];
	quat = mBody->mQuat;
	mtxFromQuat((f32*)mtx, (const f32*)&quat);
	mtxMultVecSR(mtx, &gAxisZ, &dir);
	return vecAngleXZ(&dir);
}

// 0x801C5E18
f32 aniLerp(f32 rate, f32 from, f32 to)
{
	f32 t = (1.0f / 60.0f) * rate;
	if (t > 1.0f) {
		t = 1.0f;
	}
	return from * (1.0f - t) + to * t;
}

// 0x801C5E44
int aniTurn(f32 rate, int from, int to)
{
	f32 t = (1.0f / 60.0f) * rate;
	if (t > 1.0f) {
		t = 1.0f;
	}
	return t * (f32)angleDiff(from, to) + (f32)from;
}
