#include "camera/CamArea.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "camera/TCamera.h"
#include "camera/TCamZelda5.h"
#include "object/ObjAction.h"
#include "egg/EggCreature.h"
#include "stl/math.h"
#include "camera/TCameraCtrl.h"
#include "camera/CamPad.h"
#include "Ninja/ninja.h"
#include "core/Mem.h"
#include "camera/TCamVerticalLine.h"
#include "camera/TCamWatch.h"
#include "camera/TCamPointLook.h"

struct CamPath;

static inline f32 areaSqrt(f32 x)
{
	return __fres((f32)__frsqrte(x));
}

static inline f32 areaAbs(f32 x)
{
	return (f32)__fabs(x);
}

BinFile* gCamAreaFile;
CamArea* gCamAreas;
int gCamAreaCount;

// 0x800A80D4
extern "C" void clearCamAreas()
{
	gCamAreaFile  = 0;
	gCamAreas     = 0;
	gCamAreaCount = 0;
}

// 0x800A80E8
extern "C" void loadCamAreas(const char* name)
{
	if (name == 0) {
		gCamAreas = new CamArea[64];
		CamArea area;
		area.init();
		for (int i = 0; i < 64; i++) {
			gCamAreas[i] = area;
		}
	} else {
		loadSizedResource(name, &gCamAreaFile);
		gCamAreas     = (CamArea*)gCamAreaFile->mData;
		gCamAreaCount = countCamAreas();
	}
	for (int i = 0; i < gCamAreaCount; i++) {
		gCamAreas[i].prepare();
	}
}

// 0x800A8274
extern "C" void freeCamAreas()
{
	if (gCamAreaFile == 0) {
		if (gCamAreas != 0) {
			delete[] gCamAreas;
			gCamAreas = 0;
		}
	} else {
		releaseResource(gCamAreaFile);
		gCamAreaFile = 0;
		gCamAreas    = 0;
	}
	gCamAreaCount = 0;
}

// 0x800A82D0
extern "C" CamAreaCtrl* createCamAreaCtrl()
{
	return new CamAreaCtrl;
}

// 0x800A8308
CamAreaCtrl::CamAreaCtrl()
{
	for (int p = 0; p < 4; p++) {
		for (int i = 0; i < 64; i++) {
			mState[p][i] = 0;
		}
		mCamera[p] = 0;
		mArea[p]   = -1;
	}
	mScan  = 0;
	mFirst = 1;
}

// 0x800A8374
CamAreaCtrl::~CamAreaCtrl()
{
}

// is player inside bounds
// 0x800A83B0
void CamAreaCtrl::testOuter(CamArea* area, Vec3f* pos, u8* state)
{
	if (area->mType == 6) {
		*state &= ~6;
		return;
	}
	f64 ady    = __fabs(pos->y - area->mPos.y);
	f32 dx     = pos->x - area->mPos.x;
	f32 dz     = pos->z - area->mPos.z;
	f32 height = area->mOuterHeight;
	*state &= ~1;
	f32 radius = area->mOuterRadius;
	if ((f32)ady < height) {
		dx *= dx;
		dz *= dz;
		dz += dx;
		if (areaSqrt(dz) < radius) {
			*state |= 1;
			return;
		}
	}
	*state &= ~6;
}

// player is inside the zone
// 0x800A8454
void CamAreaCtrl::testInner(CamArea* area, Vec3f* pos, u8* state)
{
	int inside = 0;
	f32 dx     = pos->x - area->mPos.x;
	f32 dy     = pos->y - area->mPos.y;
	f32 dz     = pos->z - area->mPos.z;

	switch (area->mShape) {
	case 0:
		if (areaSqrt(dx * dx + dy * dy + dz * dz) <= area->mRadius) {
			inside = 1;
		}
		break;
	case 1:
		if (areaAbs(dy) <= area->mHeight && areaSqrt(dx * dx + dz * dz) <= area->mRadius) {
			inside = 1;
		}
		break;
	case 2:
		if (areaAbs(dy) <= area->mHeight && areaAbs(area->mCos * dz + area->mSin * dx) <= area->mDepth
		    && areaAbs(area->mCos * dx - area->mSin * dz) <= area->mRadius) {
			inside = 1;
		}
		break;
	}
	// todo more enummmmmsss
	if (inside) {
		if (!(*state & 2)) {
			if (area->mSide & 1) {
				if (dx * area->mSin + dz * area->mCos <= 0.0f) {
					*state |= 4;
				} else {
					*state &= ~4;
				}
			} else {
				*state |= 4;
			}
		}
		*state |= 2;
		return;
	}
	*state &= ~6;
}

// Drops the zone's camera and gives the held camera its settings back
// 0x800A85FC
void CamAreaCtrl::leave(int player)
{
	if (mCamera[player] != 0) {
		mCamera[player]->mFlags |= 1;
		mCamera[player] = 0;
	}
	int area = mArea[player];
	if (area >= 0) {
		TCamera* held = camGetCamera(player);
		if (held != 0) {
			if (gCamAreas[area].mType == 1) {
				((TCamZelda5*)held)->resetCurveLift();
				((TCamZelda5*)held)->resetChaseOffset();
				((TCamZelda5*)held)->resetChaseSpeed();
			}
			((TCamera*)held)->setBlendStep(gCamAreas[area].mFarBack);
		}
	}
	mArea[player] = -1;
}

// 0x800A86AC
void CamAreaCtrl::enter(int player, int area)
{
	if (area < 0) {
		leave(player);
		return;
	}
	CamArea* zone;
	TCamera* made = 0;
	if (mArea[player] == area) {
		return;
	}
	leave(player);
	zone = &gCamAreas[area];
	Vec3f pos;
	Vec3f dir;
	switch (zone->mType) {
	case 1:
		applySettings(player, zone);
		break;
	case 2:
		zone->getPoint(&pos);
		made = createTCamPointLook(player, 1, &pos, zone->mLook.mDistance);
		if (made != 0) {
			((TCamera*)made)->setLift(zone->mLook.mNear);
		}
		break;
	case 3:
		zone->getLine(&pos, &dir);
		made = createTCamVerticalLine(player, 1, &pos, &dir);
		if (made != 0) {
			((TCamera*)made)->setLift(zone->mLook.mNear);
		}
		break;
	case 4:
		zone->getWatch(&pos);
		made = createTCamWatch(player, 1, &pos);
		if (made != 0) {
			((TCamera*)made)->setLift(zone->mLook.mNear);
		}
		break;
	case 5:
		zone->getPath(&pos, &dir);
		made = createTCamFix(player, 1, &pos, &dir);
		break;
	case 6:
	default:
		zone->getPath(&pos, &dir);
		made = createTCamFix(player, 1, &pos, &dir);
		break;
	}
	mCamera[player] = made;
	mArea[player]   = area;
	if (mCamera[player] != 0) {
		mCamera[player]->setTarget(player);
		((TCamera*)mCamera[player])->setBlendStep(zone->mFar);
	}
}

// Settings for the held camera instead of a camera of the zone's own
// 0x800A8884
void CamAreaCtrl::applySettings(int player, CamArea* area)
{
	TCamera* held = camGetCamera(player);
	if (held == 0) {
		return;
	}
	if (area->mFlags & 0x20) {
		if (area->mFlags & 2) {
			((TCamZelda5*)held)->setCurveHeight(area->mParam[0]);
		}
		if (area->mFlags & 4) {
			((TCamZelda5*)held)->setChaseBase(area->mParam[1], area->mParam[2]);
		}
		if (area->mFlags & 8) {
			((TCamZelda5*)held)->setChaseBaseSpeed(area->mParam[3]);
		}
		return;
	}
	((TCamera*)held)->setBlendStep(area->mFar);
	if (area->mFlags & 2) {
		((TCamZelda5*)held)->setCurveLift(area->mParam[0]);
	} else {
		((TCamZelda5*)held)->resetCurveLift();
	}
	if (area->mFlags & 4) {
		((TCamZelda5*)held)->setChaseOffset(area->mParam[1], area->mParam[2]);
	} else {
		((TCamZelda5*)held)->resetChaseOffset();
	}
	if (area->mFlags & 8) {
		((TCamZelda5*)held)->setChaseSpeed(area->mParam[3]);
	} else {
		((TCamZelda5*)held)->resetChaseSpeed();
	}
	if (area->mFlags & 0x10) {
		((TCamZelda5*)held)->resetChase((int)(182.04445f * area->mParam[4]));
	}
}

// A zone marked 1 lets go of the player while they ride
// 0x800A89B4
void CamAreaCtrl::checkRide(int player)
{
	int area = mArea[player];
	if (area >= 0 && mCamera[player] != 0 && ((TCamera*)mCamera[player])->isPicked() != 0 && (gCamAreas[area].mFlags & 1)) {
		CamPath* path = camGetPath(player);
		if (path != 0 && (camPadHardPressed(path) != 0 || camPadStickMoved(path) != 0)) {
			leave(player);
			mState[player][area] &= ~4;
		}
	}
}

// test outer bounds then the zones each player is near and moves each player to the highest zone they may be picked for
// 0x800A8A74
void CamAreaCtrl::update()
{
	if (gCamAreas == 0) {
		return;
	}

	Vec3f pos[4];
	int alive[4];
	int id = 0;
	for (int i = 0; i < 4; id++, i++) {
		TObjAction* obj = findObjById(id);
		if (obj != 0) {
			pos[i]   = *obj->getPos();
			alive[i] = 1;
		} else {
			alive[i] = 0;
		}
	}

	int first;
	int last;
	if (mFirst != 0) {
		last  = gCamAreaCount;
		first = 0;
	} else {
		first = mScan;
		last  = first + 8;
		if (last > gCamAreaCount) {
			last  = gCamAreaCount;
			mScan = 0;
		} else {
			mScan = last;
		}
	}

	for (int p = 0; p < 4; p++) {
		if (alive[p] != 0) {
			for (int i = first; i < last; i++) {
				testOuter(&gCamAreas[i], &pos[p], &mState[p][i]);
			}
			for (int i = 0; i < gCamAreaCount; i++) {
				if (mState[p][i] & 1) {
					testInner(&gCamAreas[i], &pos[p], &mState[p][i]);
				}
			}
			int best = -1;
			int pick = -1;
			for (int i = 0; i < gCamAreaCount; i++) {
				if ((mState[p][i] & 4) && best < gCamAreas[i].mPriority) {
					best = gCamAreas[i].mPriority;
					pick = i;
				}
			}
			enter(p, pick);
		}
		checkRide(p);
	}
	mFirst = 0;
}

// 0x800A8CA4
extern "C" int countCamAreas()
{
	int count = 0;
	for (int i = 0; i < 64; i++) {
		if (gCamAreas[i].isEnd()) {
			break;
		}
		count++;
	}
	return count;
}

// 0x800A8D00
void CamArea::init()
{
	mType     = 0;
	mPriority = 0;
	mFar      = 1.0f;
	mFarBack  = 1.0f;
	for (int i = 0; i < 8; i++) {
		mParam[i] = 0.0f;
	}
	mPos.x       = 0.0f;
	mPos.y       = 0.0f;
	mPos.z       = 0.0f;
	mAngle       = 0;
	mShape       = 0;
	mRadius      = 0.0f;
	mHeight      = 0.0f;
	mDepth       = 0.0f;
	mSide        = 0;
	mOuterRadius = 0.0f;
	mOuterHeight = 0.0f;
	mSin         = 0.0f;
	mCos         = 1.0f;
	for (int i = 0; i < 7; i++) {
		_64[i] = 0;
	}
}

// 0x800A8D94
int CamArea::isEnd()
{
	return (bool)(mType == 0);
}

// 0x800A8DA4
void CamArea::prepare()
{
	f32 radius = 0.0f;
	f32 height = radius;
	switch (mShape) {
	case 0:
		radius = mRadius;
		height = radius;
		break;
	case 1:
		radius = mRadius;
		height = mHeight;
		break;
	case 2:
		radius = mRadius;
		height = mHeight;
		radius = areaSqrt(mDepth * mDepth + radius * radius);
		break;
	}
	mOuterRadius = 50.0f + radius;
	mOuterHeight = 50.0f + height;
	njSinCos(mAngle, &mSin, &mCos);
}

// 0x800A8E4C
void CamArea::getPoint(Vec3f* out) const
{
	*out = mLook.mAt;
}

// 0x800A8E68
void CamArea::getLine(Vec3f* pos, Vec3f* line) const
{
	pos->x  = mParam[1];
	pos->y  = 0.0f;
	pos->z  = mParam[2];
	line->x = 0.0f;
	line->y = mParam[3];
	line->z = mParam[4];
}

// 0x800A8E98
void CamArea::getWatch(Vec3f* out) const
{
	*out = mLook.mAt;
}

// 0x800A8EB4
void CamArea::getPath(Vec3f* pos, Vec3f* dir) const
{
	*pos = mPath.mPos;
	*dir = mPath.mDir;
}

// test opening Players past the second share its start
// 0x800A8EE8
extern "C" int findCamStart(int player, Vec3f* pos, Vec3f* dir)
{
	if (gCamAreas != 0) {
		int which = player < 0 ? 0 : (player > 1 ? 1 : player);
		for (int i = 0; i < gCamAreaCount; i++) {
			if (gCamAreas[i].mType == 6 && which == gCamAreas[i].mPriority) {
				gCamAreas[i].getPath(pos, dir);
				return 1;
			}
		}
	}
	pos->x = 0.0f;
	pos->y = 0.0f;
	pos->z = 0.0f;
	dir->x = 0.0f;
	dir->y = 0.0f;
	dir->z = 0.0f;
	dir->y = 10.0f;
	dir->z = -30.0f;
	return 0;
}
