#include "engine/GrassBlade.h"
#include "object/ObjAction.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "core/MathUtil.h"
#include "camera/TCameraCtrl.h"
#include "core/VecMath.h"
struct NJS_VECTOR;

#define RANDF() ((1.0f / 32768.0f) * rand())

// A random value from lo up to hi
static inline f32 randomBetween(f32 lo, f32 hi)
{
	return (hi - lo) * RANDF() + lo;
}

// 0x80180770
extern "C" GrassBlade* createGrassBlade(f32 radius, const Vec3f* center, int index)
{
	return new GrassBlade(radius, center, index);
}

// 0x801807D0
// Somewhere within radius of the center a random heading 4 to 8 wide and 25 to 35 tall
GrassBlade::GrassBlade(f32 radius, const Vec3f* center, int index)
{
	int angle  = 65536.0f * RANDF();
	f32 dist   = randomBetween(0.0f, radius);
	f32 width  = 4.0f * RANDF() + 4.0f;
	f32 height = 10.0f * RANDF() + 25.0f;

	mPos.set(dist * njCos(angle) + center->x, center->y, dist * njSin(angle) + center->z);
	mDir        = 0;
	mTilt       = 0;
	mTiltTarget = 0;

	mBase[0].x   = -(width / 2.0f);
	mBase[0].y   = 0.0f;
	mBase[0].z   = 0.0f;
	mBase[0].u   = 0.0f;
	mBase[0].v   = 3.0f;
	mBase[0].col = 0xFF444446;
	mBase[1].x   = width / 2.0f;
	mBase[1].y   = 0.0f;
	mBase[1].z   = 0.0f;
	mBase[1].u   = 1.0f;
	mBase[1].v   = 3.0f;
	mBase[1].col = 0xFF666668;
	mBase[2].x   = 0.0f;
	mBase[2].y   = height;
	mBase[2].z   = 0.0f;
	mBase[2].u   = 0.5f;
	mBase[2].v   = 0.0f;
	mBase[2].col = 0xFFFFFFF0;
	syMemCopy(mVtx, mBase, sizeof(mBase));

	mFlags = 0;
	mIndex = index;
	readCameraAngle();
	setState(BLADE_STAND);
}

// 0x80180A10
GrassBlade::~GrassBlade()
{
}

// 0x80180A4C
int GrassBlade::isCut()
{
	return mFlags & BLADE_CUT;
}

// 0x80180A58
// Cut if inside the other's reach
int GrassBlade::cut(TObjAction* other)
{
	if ((mFlags & BLADE_CUT) == 0) {
		Vec3f from;
		from = *other->getPos();
		Vec3f pos;
		pos = mPos;
		f32 dist   = distXZ(&pos, &from);
		if (dist <= other->getRadiusV()) {
			mDir = angleToXZ(&from, &pos);
			mFlags |= BLADE_CUT;
			return 1;
		}
	}
	return 0;
}

// 0x80180B3C
// Flatten if within radius of pos
int GrassBlade::push(f32 radius, const Vec3f* pos)
{
	if ((mFlags & BLADE_CUT) != 0) {
		return 1;
	}
	if (distXZ(&mPos, pos) <= radius) {
		mDir = angleToXZ(pos, &mPos);
		mFlags |= BLADE_CUT;
		return 1;
	}
	return 0;
}

// -1..1 from rand() Its literals are pooled ahead of the 1792.0f below
inline f32 grassRandSigned()
{
	return 2.0f * (RANDF() - 0.5f);
}

// 0x80180BC4
// Lean away if the other is within 20
void GrassBlade::bend(TObjAction* other)
{
	if ((mFlags & (BLADE_CUT | BLADE_BENT)) == 0) {
		Vec3f from;
		from = *other->getPos();
		Vec3f pos;
		pos = mPos;
		from.y     = pos.y;
		if (vecDistanceSq(&pos, &from) <= 400.0f) {
			mDir        = angleToXZ(&from, &pos);
			mTiltTarget = (int)(1792.0f * grassRandSigned()) + 0x1CFF;
			mFlags |= BLADE_BENT;
		}
	}
}

// 0x80180CC8
// Lean over towards flat Returns 1 once there
int GrassBlade::fall()
{
	mTilt += 0x619;
	if (mTilt >= 0x3CFF) {
		mTilt = 0x3CFF;
		return 1;
	}
	return 0;
}

// 0x80180CF8
int GrassBlade::reset()
{
	if ((mFlags & BLADE_CUT) != 0) {
		mFlags |= BLADE_RESET;
		return 1;
	}
	return 0;
}

// 0x80180D1C
// Straighten by rate Returns 1 once upright
int GrassBlade::rise(f32 rate)
{
	mTilt = mTilt - rate;
	if (mTilt <= 0) {
		mTilt = 0;
		return 1;
	}
	return 0;
}

// 0x80180D7C
// Lean towards mTiltTarget Returns 1 once there
int GrassBlade::bendOver()
{
	mTilt += 0x399;
	if (mTilt >= mTiltTarget) {
		mTilt = mTiltTarget;
		return 1;
	}
	return 0;
}

// 0x80180DAC
void GrassBlade::readCameraAngle()
{
	mCameraAngle = camGetDrawYaw();
}

// 0x80180DDC
// Has the camera turned more than 0x400 since the last read
int GrassBlade::hasCameraTurned()
{
	int angle = camGetDrawYaw();
	int diff  = mCameraAngle - angle;
	if (diff < -0x400 || diff > 0x400) {
		mCameraAngle = angle;
		return 1;
	}
	return 0;
}

// 0x80180E30
void GrassBlade::placeAll(GrassVtx* src, GrassVtx* dst, const Vec3f* pos, int angX, int angY, int angZ)
{
	f32 mtx[12];

	njUnitMatrix(mtx);
	njRotateZXY(mtx, angX, angY, angZ);
	for (int i = 0; i < 3; i++) {
		placeVtx(mtx, &src[i], &dst[i], pos);
	}
}

// 0x80180EBC
void GrassBlade::placeTip(GrassVtx* src, GrassVtx* dst)
{
	f32 mtx[12];

	njUnitMatrix(mtx);
	njRotateZXY(mtx, mTilt, mDir, 0);
	placeVtx(mtx, &src[2], &dst[2], &mPos);
}

// 0x80180F20
void GrassBlade::placeBase(GrassVtx* src, GrassVtx* dst)
{
	f32 mtx[12];

	njUnitMatrix(mtx);
	njRotateZXY(mtx, 0, mCameraAngle, 0);
	placeVtx(mtx, &src[0], &dst[0], &mPos);
	placeVtx(mtx, &src[1], &dst[1], &mPos);
}

// 0x80180F9C
void GrassBlade::placeVtx(f32* mtx, GrassVtx* src, GrassVtx* dst, const Vec3f* pos)
{
	Vec3f v;

	syMemCopy(&v, src, sizeof(Vec3f));
	njCalcVector(mtx, (const Vec3f*)(const NJS_VECTOR*)&v, (Vec3f*)(NJS_VECTOR*)&v);
	njAddVector(&v, pos);
	syMemCopy(dst, &v, sizeof(Vec3f));
}

// 0x80181008
void GrassBlade::setState(int state)
{
	mState = state;
	runState(mState, BLADE_ENTER);
}

// 0x80181034
void GrassBlade::exitState()
{
	runState(mState, BLADE_EXIT);
}

// 0x8018105C
void GrassBlade::changeState(int state)
{
	exitState();
	setState(state);
}

// 0x80181098
void GrassBlade::update()
{
	runState(mState, BLADE_UPDATE);
	runState(mState, BLADE_CHECK);
}

// 0x801810DC
void GrassBlade::runState(int state, int event)
{
	switch (state) {
	case BLADE_STAND:
		stateStand(event);
		break;
	case BLADE_SWAY:
		stateSway(event);
		break;
	case BLADE_FALL:
		stateFall(event);
		break;
	case BLADE_LYING:
		stateLying(event);
		break;
	case BLADE_REGROW:
		stateRegrow(event);
		break;
	case BLADE_BEND:
		stateBend(event);
		break;
	}
}

// 0x80181170
void GrassBlade::stateStand(int event)
{
	switch (event) {
	case BLADE_ENTER:
		readCameraAngle();
		placeAll(mBase, mVtx, &mPos, 0, mCameraAngle, 0);
		break;
	case BLADE_CHECK:
		changeState(BLADE_SWAY);
		break;
	case BLADE_UPDATE:
		break;
	}
}

// 0x801811E4
// One frame of an ease in and out over frames amount in all
int GrassBlade::swayStep(int frame, int frames, int amount)
{
	return (amount / frames) * (1.0f - njCos(frame * 0xFFFF / frames));
}

// 0x8018125C
// Rest lean one way and come back and again
void GrassBlade::stateSway(int event)
{
	int turned = hasCameraTurned();

	switch (event) {
	case BLADE_ENTER:
		readCameraAngle();
		placeAll(mBase, mVtx, &mPos, 0, mCameraAngle, 0);
		mTilt  = 0;
		mStep  = 0;
		mFrame = 0;
		break;
	case BLADE_CHECK:
		if ((mFlags & BLADE_CUT) != 0) {
			changeState(BLADE_FALL);
		} else if ((mFlags & BLADE_BENT) != 0) {
			changeState(BLADE_BEND);
		}
		break;
	case BLADE_UPDATE:
		if (turned != 0) {
			placeBase(mBase, mVtx);
			break;
		}
		switch (mStep) {
		case 0: {
			mFrame = 0;
			int frames = 120.0f * RANDF();
			if (frames >= 120) {
				frames = 119;
			}
			mFrames = frames;
			mTilt   = 0;
			mStep   = 1;
			break;
		}
		case 1:
			mFrame++;
			if (mFrame >= mFrames) {
				mFrame     = 0;
				int frames = 20.0f * RANDF();
				if (frames >= 20) {
					frames = 19;
				}
				mFrames    = frames + 40;
				int amount = 512.0f * RANDF();
				if (amount >= 0x200) {
					amount = 0x1FF;
				}
				mAmount = amount + 0x200;
				mStep   = 2;
			}
			break;
		case 2:
			mFrame++;
			mTilt += swayStep(mFrame, mFrames, mAmount);
			if (mFrame >= mFrames) {
				mFrame = mFrames;
				mStep  = 3;
			}
			placeTip(mBase, mVtx);
			break;
		case 3:
			mFrame--;
			mTilt -= swayStep(mFrame, mFrames, mAmount);
			if (mFrame <= 0) {
				mStep = 0;
			}
			placeTip(mBase, mVtx);
			break;
		}
		break;
	case BLADE_EXIT:
		break;
	}
}

// 0x80181548
void GrassBlade::stateFall(int event)
{
	switch (event) {
	case BLADE_ENTER:
		mStep = 0;
		mTilt = 0;
		break;
	case BLADE_CHECK:
		if (mStep == 1) {
			changeState(BLADE_LYING);
		}
		break;
	case BLADE_UPDATE:
		placeAll(mBase, mVtx, &mPos, mTilt, mDir, 0);
		switch (mStep) {
		case 0:
			if (fall() != 0) {
				mStep = 1;
			}
			break;
		case 1:
			break;
		}
		break;
	case BLADE_EXIT:
		break;
	}
}

// 0x80181608
void GrassBlade::stateLying(int event)
{
	switch (event) {
	case BLADE_ENTER:
		placeAll(mBase, mVtx, &mPos, mTilt, mDir, 0);
		break;
	case BLADE_CHECK:
		if ((mFlags & BLADE_RESET) != 0) {
			changeState(BLADE_REGROW);
		}
		break;
	case BLADE_UPDATE:
		break;
	case BLADE_EXIT:
		mFlags &= ~BLADE_CUT;
		break;
	}
}

// 0x80181690
// Stands back up placed every other frame
void GrassBlade::stateRegrow(int event)
{
	switch (event) {
	case BLADE_ENTER:
		mStep  = 0;
		mFrame = 0;
		mTilt  = 0x3CFF;
		break;
	case BLADE_CHECK:
		if (mStep == 1) {
			changeState(BLADE_SWAY);
		}
		break;
	case BLADE_UPDATE:
		mFrame++;
		if ((mIndex + mFrame) % 2 == 0) {
			placeAll(mBase, mVtx, &mPos, mTilt, mDir, 0);
		}
		switch (mStep) {
		case 0:
			if (rise(1561.0f) != 0) {
				mStep = 1;
			}
			break;
		case 1:
			break;
		}
		break;
	case BLADE_EXIT:
		mFlags &= ~BLADE_RESET;
		break;
	}
}

// 0x801817A0
// Lean over then come back
void GrassBlade::stateBend(int event)
{
	switch (event) {
	case BLADE_ENTER:
		mStep = 0;
		mTilt = 0;
		break;
	case BLADE_CHECK:
		if ((mFlags & BLADE_CUT) != 0) {
			changeState(BLADE_FALL);
		} else if (mStep == 2) {
			changeState(BLADE_SWAY);
		}
		break;
	case BLADE_UPDATE:
		placeTip(mBase, mVtx);
		switch (mStep) {
		case 0:
			if (bendOver() != 0) {
				mStep = 1;
			}
			break;
		case 1:
			if (rise(921.0f) != 0) {
				mStep = 2;
			}
			break;
		case 2:
			break;
		}
		break;
	case BLADE_EXIT:
		mTilt = 0;
		mFlags &= ~BLADE_BENT;
		break;
	}
}

// 0x801818AC
void GrassBlade::draw()
{
	njDrawTexture3DExSetData((const NJS_TEXTURE_VTX*)mVtx, 3);
}

// 0x801818D4
// Draw with the alpha of every corner replaced then put the colors back
void GrassBlade::drawFaded(u32 alpha)
{
	u32 col[3];
	int i;

	for (i = 0; i < 3; i++) {
		col[i]     = mVtx[i].col;
		mVtx[i].col = alpha + (col[i] & 0xFFFFFF);
	}
	njDrawTexture3DExSetData((const NJS_TEXTURE_VTX*)mVtx, 3);
	for (i = 0; i < 3; i++) {
		mVtx[i].col = col[i];
	}
}
