#include "stage/SetManager.h"
#include "stage/SetActor.h"
#include "object/ObjAction.h"
#include "engine/DrawTask.h"
#include "engine/CharaColli.h"
#include "object/ObjArchive.h"
#include "enemy/EnemyLight.h"
#include "stage/StageObjUtil.h"
#include "egg/ObjEgg.h"
#include "engine/TParticle.h"
#include "object/TObjBlast.h"
#include "Ninja/ninja.h"
#include "sdk/stl/math.h"
#include "camera/TCamera.h"
#include "egg/EggCreature.h"
#include "engine/Colli.h"
#include "sound/Sound3DPlay.h"
#include "ui/AeArc.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "engine/GroundQuery.h"
#include "core/VecMath.h"
#include "object/TObject.h"
#include "object/TObjCannon01.h"
#include "animal/RacerMover.h"
struct TParticleGenerator;
struct _GXColor;
struct Fx;

static const GXColor sBallColor = { 0x20, 0x20, 0x20, 0xFF };

// 0x8002A6F0
TCannonBall::TCannonBall(TObject* parent, const Vec3f* pos, const Vec3f* vel, const Vec3f* target, f32 scale)
    : TObject(parent)
{
	mDraw             = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mXform.mGravity.x = 0.0f;
	mXform.mGravity.y = -CANNON_GRAVITY;
	mXform.mGravity.z = 0.0f;
	mXform.mPos       = *pos;
	mXform.mVel       = *vel;
	mXform.vfn_0C();
	mScale  = scale;
	mTarget = *target;
	mState  = STATE_FLY;
}

// 0x8002A7EC
TCannonBall::~TCannonBall()
{
	removeDrawTask(mDraw);
}

// 0x8002A87C
// Bursts on the way down through the target's height
void TCannonBall::fly()
{
	if (mXform.mPos.y <= mTarget.y && mXform.mVel.y < 0.0f) {
		mXform.mPos.y = mTarget.y;
		mState        = STATE_BURST;
	} else {
		mXform.vfn_0C();
		mXform.vfn_14();
		mXform.vfn_18();
	}
}

// 0x8002A914
void TCannonBall::burst()
{
	f32 mtx[12];

	new TObjBlast(this, &mXform.mPos, 2.5f, 1.5f, 0x19);
	mtxSetTransV(mtx, &mXform.mPos);
	fxSpawnAt(1, 8, 3, mtx);
	sePlay3D(0x21B, &mXform.mPos);
	mState = STATE_GONE;
}

// 0x8002A99C
void TCannonBall::waitGone()
{
	if (objCountChildren(this) == 0) {
		mFlags |= 1;
	}
}

// 0x8002A9DC
void TCannonBall::draw()
{
	switch (mState) {
	case STATE_FLY:
		fly();
		break;
	case STATE_BURST:
		burst();
		break;
	case STATE_GONE:
		waitGone();
		break;
	}
}

// 0x8002AA34
// Not drawn once it has burst
void TCannonBall::vfn_10()
{
	if (mState == STATE_GONE) {
		mDraw->mFlags |= 1;
	}
}

/** The ball is only ever drawn as a debug sphere */

// 0x8002AA54
void TCannonBall::render()
{
	GXColor color;
	color.r = sBallColor.r;
	color.g = sBallColor.g;
	color.b = sBallColor.b;
	color.a = sBallColor.a;

	debugSphere(&mXform.mPos, &color, 5.0f * mScale);
}
