#include "stage/Stage.h"
#include "enemy/EnemyCommon.h"
#include "object/ObjCommon.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "Ninja/ninja.h"
#include "engine/FrameTex.h"
#include "object/ObjShadow.h"
#include "egg/ObjEgg.h"
#include "enemy/EnemyFx.h"
#include "stl/math.h"
#include "object/ObjAction.h"
#include "core/SdkCalls.h"
struct NJS_OBJECT;

// See ShadowSystem.cpp

// 0x800F1EFC
// The flat decals
void shadowDrawDecals()
{
	if (sShadowList != NULL) {
		enemyCommonSetTexture();
		ObjShadow* s;
		NJS_OBJECT* model = enemyShadowModel();
		alphaTestBegin();
		for (s = sShadowList; s != NULL; s = s->mNext) {
			if (shadowIsEnabled(s) != 0) {
				njPushMatrixEx();
				njTranslateEx(&s->mGround);
				gjRotateY(s->mTiltY);
				gjRotateX(s->mTiltX);
				gjRotateY((0x10000 - s->mTiltY) + s->mAngle);
				((void (*)(f32, f32, f32))gjTranslate)(0.0f, 0.42f, 0.0f);
				njScale(NULL, 0.1f * s->_30, 1.0f, 0.1f * s->_34);
				gjDrawObject(model);
				njPopMatrixEx();
			}
		}
		alphaTestEnd();
	}
}

// 0x800F1FD4
// The shadow volumes each drawn twice
void shadowDrawVolumes()
{
	if (sShadowList != NULL) {
		ObjShadow* s;
		shadowVolumeBegin();
		NJS_OBJECT* model = (NJS_OBJECT*)shadowGetModel();
		s                 = sShadowList;
		while (s != NULL) {
			if (shadowIsEnabled(s)) {
				njPushMatrixEx();
				njTranslateEx(&s->mGround);
				gjRotateY(s->mAngle);
				f32 x = s->_30 * 0.1f;
				f32 z;
				z = s->_34 * 0.1f;
				njScale(NULL, x, x < z ? x : z, z);
				shadowVolumeSubtract();
				gjDrawObject(model);
				shadowVolumeAdd();
				gjDrawObject(model);
				njPopMatrixEx();
			}
			s = s->mNext;
		}
		shadowVolumeEnd();
	}
}

// 0x800F2098
void shadowDrawAll()
{
	if (sShadowVolumesOn == 0) {
		shadowDrawDecals();
	} else {
		shadowDrawVolumes();
	}
}

// 0x800F20CC
ObjShadow::ObjShadow()
{
	mEnabled  = 0;
	mGround.x = 0.0f;
	mGround.y = 0.0f;
	mGround.z = 0.0f;
	mNormal.x = 0.0f;
	mNormal.y = 0.0f;
	mNormal.z = 0.0f;
	mNormal.y = 1.0f;
	mTiltX    = 0;
	mTiltY    = 0;
	mAngle    = 0;
	_30       = 10.0f;
	_34       = 10.0f;
	mNext     = NULL;
	mPrev     = NULL;
	if (sShadowList == NULL) {
		sShadowList = this;
	} else {
		mNext              = sShadowList->mNext;
		mPrev              = sShadowList;
		sShadowList->mNext = this;
		if (mNext != NULL) {
			mNext->mPrev = this;
		}
	}
}

// 0x800F2158
ObjShadow::~ObjShadow()
{
	if (mNext != NULL) {
		mNext->mPrev = mPrev;
	}
	if (mPrev != NULL) {
		mPrev->mNext = mNext;
	}
	if (sShadowList == this) {
		sShadowList = mNext;
	}
}

// 0x800F21D4
void shadowEnable(ObjShadow* shadow)
{
	shadow->mEnabled = 1;
}

// 0x800F21E0
void shadowDisable(ObjShadow* shadow)
{
	shadow->mEnabled = 0;
}

// 0x800F21EC
int shadowIsEnabled(ObjShadow* shadow)
{
	return shadow->mEnabled != 0;
}

// 0x800F2200
// Only works out the tilt again when the normal changes
void shadowSetGround(ObjShadow* shadow, const Vec3f* pos, const Vec3f* normal)
{
	if (pos != NULL) {
		shadow->mGround = *pos;
	}
	if (normal != NULL) {
		if (shadow->mNormal.x != normal->x || shadow->mNormal.y != normal->y || shadow->mNormal.z != normal->z) {
			shadow->mNormal = *normal;
			shadow->mTiltY  = (int)(10430.38043493439 * (f32)atan2(shadow->mNormal.x, shadow->mNormal.z));
			shadow->mTiltX  = (int)(10430.38043493439 * (f32)acos(shadow->mNormal.y));
		}
	}
}

// 0x800F22E0
// Takes the ground from a contact record
void shadowSetFromContact(ObjShadow* shadow, u8* contact)
{
	shadowSetGround(shadow, &((EggHit*)contact)->mPos, &((EggHit*)contact)->mNormal);
	shadowEnable(shadow);
}

// 0x800F2320
int shadowSetAngle(ObjShadow* shadow, int angle)
{
	shadow->mAngle = angle;
	return shadowIsEnabled(shadow);
}
