#include "camera/TCamera.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "object/ObjAction.h"
#include "Ninja/ninja.h"
#include "camera/TCamLockOnCursor.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "stl/math.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "egg/EggCreature.h"
#include "camera/TCameraCtrl.h"
#include "object/TObjPlayer.h"
#include "animal/RacerMover.h"
struct DrawTask;
struct NJS_QUAD_TEXTURE;

/** Where a mark's quad goes on screen and which part of the texture it shows */
struct LockOnQuad {
	f32 mX0, mY0, mX1, mY1;
	f32 mU0, mV0, mU1, mV1;
};

extern "C" {
TArc* gIcon3dArc; // ar_icon3d.arc loaded by TSpeakCursor
}

// 0x80241620
// The locked-on mark for each kind of target in the icon texture
const LockOnUV gLockOnUV[5] = {
	{ 0.0f, 0.78125f, 0.1875f, 0.921875f },  { 0.1875f, 0.78125f, 0.375f, 0.921875f }, { 0.375f, 0.78125f, 0.5625f, 0.921875f },
	{ 0.5625f, 0.78125f, 0.75f, 0.921875f }, { 0.296875f, 0.0f, 0.59375f, 0.28125f },
};

// 0x80241670
// The free marks then from 5 the player labels
const LockOnUV gLockOnFreeUV[10] = {
	{ 0.546875f, 0.28125f, 0.703125f, 0.40625f }, { 0.546875f, 0.40625f, 0.703125f, 0.53125f },
	{ 0.546875f, 0.53125f, 0.703125f, 0.65625f }, { 0.546875f, 0.65625f, 0.703125f, 0.78125f },
	{ 0.296875f, 0.28125f, 0.453125f, 0.40625f }, { 0.703125f, 0.0f, 1.0f, 0.1875f },
	{ 0.703125f, 0.1875f, 1.0f, 0.375f },         { 0.703125f, 0.375f, 1.0f, 0.5625f },
	{ 0.703125f, 0.5625f, 1.0f, 0.75f },          { 0.0f, 0.0f, 0.0078125f, 0.0078125f },
};

// 0x8002FAFC
void LockOnMark::reset()
{
	mId    = 0xFFFF;
	mPos.x = 0.0f;
	mPos.y = 0.0f;
	mPos.z = 0.0f;
	mDist  = 0.0f;
}

typedef char TCamLockOnCursor_size_check[(sizeof(TCamLockOnCursor) == 0x12C) ? 1 : -1];

const char* gTCamLockOnCursorName = "TCamLockOnCursor";

// 0x8002FB20
TCamLockOnCursor::TCamLockOnCursor(TObject* parent)
    : TObject(parent)
{
	_00   = (u32)gTCamLockOnCursorName;
	mSize = 0x12C;
	mTex  = 0;
	if (gIcon3dArc != 0) {
		void** list = (void**)findArcExport(gIcon3dArc, "texlists");
		if (list != 0) {
			mTex = list[0];
		}
	}
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3D, 0);
	mDraw->mFlags &= ~1;
	mId = 0xFFFF;
	useNormalRange();
	init();
}

// 0x8002FC04
TCamLockOnCursor::~TCamLockOnCursor()
{
	removeDrawTask(mDraw);
}

// 0x8002FC6C
void TCamLockOnCursor::init()
{
	int i;

	mCount = 0;
	for (i = 0; i < LOCK_MARKS; i++) {
		mMark[i].reset();
	}
	mIndex = 0;
	mCurrent.reset();
	mBits  = 0;
	mBlink = 0.0f;
	enable();
}

// 0x8002FCE4
void TCamLockOnCursor::setOwner(u16 id)
{
	if (id < 4) {
		mId = id;
	}
}

// 0x8002FCF8
BOOL TCamLockOnCursor::hasMarks()
{
	return mCount > 0;
}

// 0x8002FD0C
void TCamLockOnCursor::lock()
{
	if (hasMarks()) {
		mBits |= LOCK_LOCKED;
		mIndex   = 0;
		mCurrent = mMark[mIndex];
	}
}

// 0x8002FD8C
void TCamLockOnCursor::unlock()
{
	mBits &= ~LOCK_LOCKED;
	mIndex = 0;
	mCurrent.reset();
}

// 0x8002FDC8
void TCamLockOnCursor::nextMark()
{
	if (mCount > 0) {
		mIndex++;
		if (mIndex >= mCount) {
			mIndex = 0;
		}
		mCurrent = mMark[mIndex];
	}
}

// 0x8002FE34
void TCamLockOnCursor::enable()
{
	mBits |= LOCK_ENABLED;
}

// 0x8002FE44
void TCamLockOnCursor::disable()
{
	int i;

	mBits &= ~LOCK_ENABLED;
	unlock();
	mCount = 0;
	for (i = 0; i < LOCK_MARKS; i++) {
		mMark[i].mId = 0xFFFF;
	}
}

// 0x8002FEA4
void TCamLockOnCursor::setRange(f32 far, int angle, f32 near, f32 keep)
{
	mFar   = far;
	mAngle = angle;
	mNear  = near;
	mKeep  = keep;
}

// 0x8002FEB8
void TCamLockOnCursor::setHeight(f32 height)
{
	mHeight = height;
}

// 0x8002FEC0
void TCamLockOnCursor::useNormalRange()
{
	setRange(180.0f, 0x471C, 40.0f, 300.0f);
	setHeight(100.0f);
}

// 0x8002FF08
void TCamLockOnCursor::useAltRange()
{
	setRange(180.0f, 0x471C, 40.0f, 300.0f);
	setHeight(100.0f);
}

// 0x8002FF50
void TCamLockOnCursor::useUnlimitedRange()
{
	setRange(100000.0f, 0x471C, 40.0f, 100300.0f);
	setHeight(100000.0f);
}

// 0x8002FF98
u16 TCamLockOnCursor::getLockId()
{
	return mCurrent.mId;
}

// 0x8002FFA0
void TCamLockOnCursor::getCurrent(LockOnMark* out)
{
	*out = mCurrent;
}

// 0x8002FFCC
// Never its own player
BOOL TCamLockOnCursor::canTarget(TObjAction* obj)
{
	if (obj->mObjId == mId) {
		return FALSE;
	}
	return obj->canAct();
}

// 0x80030014
// Flattens dir and gives its length
BOOL TCamLockOnCursor::inRange(f32 dotMin, const Vec3f* fwd, Vec3f* dir, f32* outLen)
{
	f32 len;

	if ((f32)__fabs(dir->y) < mHeight) {
		dir->y = 0.0f;
		len    = njUnitVector(dir);
		if (len < gUnitVectorMin) {
			*dir = gVecZero;
		}
		*outLen = len;
		if (*outLen < mNear) {
			return TRUE;
		}
		return *outLen < mFar && vecDotXZ(dir, fwd) > dotMin;
	}
	return FALSE;
}

// 0x800300FC
// Gathers the nearest marks in front of from and keeps the locked one while it is within mKeep
void TCamLockOnCursor::scan(const Vec3f* from, int rotY)
{
	Vec3f eye = *from;
	ObjIdIter it;
	Vec3f fwd;
	Vec3f dir;
	TObjAction* found[LOCK_MARKS];
	f32 dist[LOCK_MARKS];
	f32 dotMin;
	f32 len;
	int count;
	int i;

	if (mBits & LOCK_ENABLED) {
		njSinCos(rotY, &fwd.x, &fwd.z);
		dotMin = njCos(mAngle);
		count  = 0;
		for (i = 0; i < LOCK_MARKS; i++) {
			found[i] = 0;
			dist[i]  = mFar;
		}
		len = 0.0f;

		it = objIdIter();
		for (it.begin(); !it.atEnd(); it.next()) {
			TObjAction* obj = (TObjAction*)it.getEgg();
			if (canTarget(obj)) {
				dir = *obj->getPos();
				njSubVector(&dir, &eye);
				if (inRange(dotMin, &fwd, &dir, &len)) {
					int at;
					for (at = 0; at < count; at++) {
						if (dist[at] > len) {
							break;
						}
					}
					if (at < LOCK_MARKS) {
						int j = (count < LOCK_MARKS - 1) ? count - 1 : LOCK_MARKS - 2;
						for (; j >= at; j--) {
							dist[j + 1]  = dist[j];
							found[j + 1] = found[j];
						}
						dist[at]  = len;
						found[at] = obj;
						if (count < LOCK_MARKS) {
							count++;
						}
					}
				}
			}
		}

		mCount = count;
		for (i = 0; i < count; i++) {
			mMark[i].mPos = *found[i]->getPos();
			mMark[i].mPos.y += found[i]->vfn_40();
			mMark[i].mId = found[i]->mObjId;
		}
		for (i = count; i < LOCK_MARKS; i++) {
			mMark[i].mId = 0xFFFF;
		}

		TObjAction* locked = findObjById(mCurrent.mId);
		mCurrent.reset();
		if (locked != 0 && canTarget(locked)) {
			dir = *locked->getPos();
			njSubVector(&dir, &eye);
			len = vecLenXZ(&dir);
			if (len <= mKeep) {
				mCurrent.mId   = locked->mObjId;
				mCurrent.mPos  = *locked->getPos();
				mCurrent.mDist = len;
			}
		}
	}
}

// 0x80030498
// Follows its player with no player starts over
void TCamLockOnCursor::draw()
{
	TObjAction* owner;

	mBlink += 0.05f;
	if (mBlink >= 1.0f) {
		mBlink -= 1.0f;
	}
	owner = findObjById(mId);
	if (owner != 0) {
		Vec3f at;
		at      = *owner->getPos();
		int rot = owner->getRotY();
		scan(&at, rot);
	} else {
		init();
	}
}

// 0x80030558
void TCamLockOnCursor::vfn_10()
{
	if (mId == (u16)playerGetDrawView()) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x800305B0
// Into the view on a unit scale
BOOL TCamLockOnCursor::project(f32 maxY, const Vec3f* pos, Vec3f* out)
{
	Vec3f view;

	njCalcPointEx(pos, &view);
	if (view.z < 0.0f) {
		f32 inv = -1.0f / view.z;
		f32 a   = __fabs(view.y);
		f32 y   = a * inv;
		if (y > maxY) {
			y = maxY;
		}
		if (view.y < 0.0f) {
			y *= -1.0f;
		}
		out->x = view.x * inv;
		out->y = y;
		out->z = 1.0f;
		return TRUE;
	}
	return FALSE;
}

// 0x80030668
int TCamLockOnCursor::markKind(u16 id)
{
	if (id > 3) {
		return 4;
	}
	return id;
}

// 0x80030680
// A player's mark with their label above it
void TCamLockOnCursor::drawPlayerMark(const Vec3f* at, int kind, BOOL locked)
{
	LockOnQuad quad;
	const LockOnUV* uv;
	f32 size;

	if (locked) {
		uv   = &gLockOnUV[kind];
		size = gLockOnBigSize.mW;
	} else {
		uv   = &gLockOnFreeUV[kind];
		size = gLockOnBigSize.mW;
	}
	quad.mX0 = at->x - 0.5f * size;
	quad.mY0 = at->y - (10.0f + size);
	quad.mX1 = quad.mX0 + size;
	quad.mY1 = quad.mY0 + size;
	quad.mU0 = uv->mU0;
	quad.mV0 = uv->mV0;
	quad.mU1 = uv->mU1;
	quad.mV1 = uv->mV1;
	njDrawQuadTexture((const NJS_QUAD_TEXTURE*)&quad, 0.8f);

	uv       = &gLockOnFreeUV[5 + kind];
	quad.mX0 = at->x - 28.5f;
	quad.mY0 = at->y - 76.0f;
	quad.mX1 = quad.mX0 + 57.0f;
	quad.mY1 = quad.mY0 + 36.0f;
	quad.mU0 = uv->mU0;
	quad.mV0 = uv->mV0;
	quad.mU1 = uv->mU1;
	quad.mV1 = uv->mV1;
	njDrawQuadTexture((const NJS_QUAD_TEXTURE*)&quad, 0.8f);
}

// 0x800307B0
// Anything else's mark
void TCamLockOnCursor::drawOtherMark(const Vec3f* at, int kind, BOOL locked)
{
	LockOnQuad quad;
	const LockOnUV* uv;
	f32 size;
	f32 lift;

	if (locked) {
		size = gLockOnBigSize.mH;
		uv   = &gLockOnUV[kind];
		lift = 12.0f;
	} else {
		size = gLockOnSmallSize.mH;
		uv   = &gLockOnFreeUV[kind];
		lift = 12.0f;
	}
	quad.mX0 = at->x - 0.5f * size;
	lift += at->y - size;
	quad.mY0 = lift;
	quad.mX1 = quad.mX0 + size;
	quad.mY1 = quad.mY0 + size;
	quad.mU0 = uv->mU0;
	quad.mV0 = uv->mV0;
	quad.mU1 = uv->mU1;
	quad.mV1 = uv->mV1;
	njDrawQuadTexture((const NJS_QUAD_TEXTURE*)&quad, 0.8f);
}

// 0x8003086C
void TCamLockOnCursor::drawMark(const Vec3f* at, u16 id, BOOL locked)
{
	int kind = markKind(id);

	if (kind == 4) {
		drawOtherMark(at, kind, locked);
	} else {
		drawPlayerMark(at, kind, locked);
	}
}

// 0x800308D4
// The free marks farthest first then the locked one over them
void TCamLockOnCursor::render()
{
	int count = mCount;
	CamView* view;
	f32 halfW;
	f32 halfH;
	f32 scale;
	f32 maxY;
	Vec3f at;
	Vec3f scr;
	int i;

	view = camGetDrawView();
	if (view != 0) {
		halfW = 0.5f * _gj_rmode_->fbWidth;
		scale = halfW / view->mSlopeX;
		halfH = 0.5f * _gj_rmode_->xfbHeight;
		maxY  = (halfH - 50.0f) / scale;
		if (mTex != 0) {
			njSetTexture(mTex);
		}
		blendModeAlpha();
		GXSetAlphaCompare(GX_ALWAYS, 0x7F, GX_AOP_OR, GX_LEQUAL, 0xFF); // always, or at most 0xFF;
		njQuadTextureStart(1);
		njSetQuadTexture(0, 0xFFFFFFFF);

		for (i = count - 1; i >= 0; i--) {
			LockOnMark* mark = &mMark[i];
			if (mark->mId != mCurrent.mId && mark->mId != mId && project(maxY, &mark->mPos, &scr)) {
				scr.x *= scale;
				scr.y *= scale;
				scr.x = scr.x + halfW;
				scr.y = halfH - scr.y;
				drawMark(&scr, mark->mId, FALSE);
			}
		}

		if ((mBits & LOCK_LOCKED) && mCurrent.mId != mId) {
			TObjAction* obj = findObjById(mCurrent.mId);
			if (obj != 0 && obj->canAct()) {
				at = *obj->getPos();
				at.y += obj->vfn_40();
				if (project(maxY, &at, &scr)) {
					scr.x *= scale;
					scr.y *= scale;
					scr.x = scr.x + halfW;
					scr.y = halfH - scr.y;
					drawMark(&scr, mCurrent.mId, TRUE);
				}
			}
		}
		njQuadTextureEnd();
	}
}
