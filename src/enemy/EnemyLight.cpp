#include "camera/TCamera.h"
#include "enemy/EnemyLight.h"
#include "engine/ScrollModel.h"
#include "engine/DrawTask.h"
#include "object/ObjModel.h"
#include "io/ResBank.h"
#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/GenTex.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "core/MathUtil.h"
#include "camera/TCameraCtrl.h"
#include "egg/EggCreature.h"
#include "ui/AeArc.h"
#include "object/red/TObjRedMagmaScene.h"
#include "camera/TCamPath.h"
#include "enemy/boss/TEneBoss02_Pool.h"
struct GjActor;
struct NJS_OBJECT;
struct DrawStateSlot;

// 0x80064208
EnemyLight::EnemyLight()
{
	reset();
}

// 0x80064258
EnemyLight::~EnemyLight()
{
}

// 0x80064294
void EnemyLight::reset()
{
	int i;

	for (i = 0; i < 4; i++) {
		mFaded[i]  = 0;
		mAlpha[i]  = 0xFF;
		mView[i].x = 0.0f;
		mView[i].y = 0.0f;
		mView[i].z = 0.0f;
	}
	mFadeDist = 20.0f;
	mTimer    = 1;
}

// 0x800642EC
void EnemyLight::setFadeDist(f32 dist)
{
	mFadeDist = dist;
}

// 0x800642F4
int EnemyLight::isFaded(int* alpha)
{
	int view = camGetDrawIndex();

	if (alpha != 0) {
		*alpha = mAlpha[view];
	}
	return mFaded[view] != 0;
}

// 0x8006434C
void EnemyLight::noteView(int, int, int)
{
}

// 0x80064350
// A see-through object draws in the sorted pass back to front
void EnemyLight::sortTask(int view, int wasFaded, DrawTask* task)
{
	if (wasFaded == 0) {
		if (mFaded[view]) {
			moveDrawTask(task, 0x22);
		}
	} else if (!mFaded[view]) {
		moveDrawTask(task, 0x21);
	}
	if (mFaded[view]) {
		task->mSortKey = drawTaskDepthKey(mView[view].z);
	}
}

static inline f32 clamp01(f32 x)
{
	return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
}

// 0x800643DC
void EnemyLight::setAlpha(f32 opacity, int view, int least)
{
	least        = (u8)(int)(255.0f * clamp01(opacity)) < (u8)least ? least : (int)(255.0f * clamp01(opacity));
	mAlpha[view] = least;
	if (mAlpha[view] == 0xFF) {
		mFaded[view] = 0;
	} else {
		mFaded[view] = 3;
	}
}

// 0x80064494
int EnemyLight::checkView(f32 far, f32 radius, Vec3f* pos, Vec3f* out)
{
	int view = camGetDrawIndex();
	int visible;
	int nearFade;
	int farFade;
	f32 z;
	f32 nearZ;

	visible = cameraIsVisible(100.0f + far, radius, pos, &mView[view]);
	if (out != 0) {
		out->x = mView[view].x;
		out->y = mView[view].y;
		out->z = mView[view].z;
	}
	z        = mView[view].z;
	nearZ    = -(mFadeDist + radius);
	nearFade = z > nearZ;
	farFade  = z < -far;
	if (nearFade && mTimer) {
		setAlpha(z / nearZ, view, 0x80);
	} else if (farFade) {
		setAlpha(1.0f - (-z - far) / 100.0f, view, 0);
	} else {
		setAlpha(1.0f, view, 0);
	}
	noteView(view, nearFade, farFade);
	return visible && mAlpha[view] != 0;
}

// 0x80064608
int EnemyLight::isVisible(void* task, f32 far, Vec3f* pos, f32 radius, int out)
{
	int visible;
	int view     = camGetDrawIndex();
	int wasFaded = mFaded[view];
	visible      = checkView(far, radius, pos, (Vec3f*)out);

	if (task != 0) {
		sortTask(view, wasFaded, (DrawTask*)task);
	}
	return visible;
}

// 0x800646B8
// Keeps the view state for an object that is always drawn
void EnemyLight::place(int mode, DrawTask* task, Vec3f* pos, Vec3f* out)
{
	int view     = camGetDrawIndex();
	int wasFaded = mFaded[view];

	cameraIsVisible(100.0f, 100.0f, pos, &mView[view]);
	if (out != 0) {
		out->x = mView[view].x;
		out->y = mView[view].y;
		out->z = mView[view].z;
	}
	noteView(view, mode, 0);
	if (task != 0) {
		sortTask(view, wasFaded, task);
	}
}

// 0x80064768
extern "C" void setDrawAmbient(const EnemyLight* light, DrawState* state)
{
	int view = camGetDrawIndex();

	if (light->mFaded[view]) {
		state->mFlags |= 0x200;
		state->mAmbient = light->mAlpha[view];
	} else {
		state->mFlags &= ~0x200;
	}
}

// 0x800647D4
// The default draw state made see-through in buf when it needs to be
DrawState* EnemyLight::drawState(DrawState* buf) const
{
	DrawState* result;
	int view = camGetDrawIndex();
	result   = getDefaultDrawState();

	if (mFaded[view]) {
		*buf = *result;
		buf->mFlags |= 0x200;
		buf->mAmbient = mAlpha[view];
		result        = buf;
	}
	return result;
}

// 0x8006487C
void EnemyLight::drawObject(void* model)
{
	ObjDrawState buf;
	DrawStateSlot* slot = getDrawStateSlot();

	beginDrawState(slot, drawState(&buf));
	gjDrawObject((NJS_OBJECT*)model);
	endDrawState(slot);
}

// 0x800648E8
void EnemyLight::drawActor(void* actor)
{
	ObjDrawState buf;
	DrawStateSlot* slot = getDrawStateSlot();

	setDrawAmbient(this, &buf);
	beginDrawState(slot, drawState(&buf));
	gjDrawActor((GjActor*)actor);
	endDrawState((DrawStateSlot*)slot);
}

// 0x80064960
void EnemyLight::drawMotion(void* object, void* motion, f32 frame)
{
	ObjDrawState buf;
	DrawStateSlot* slot = getDrawStateSlot();

	beginDrawState(slot, drawState(&buf));
	gjDrawMotion((NJS_OBJECT*)object, motion, frame);
	endDrawState((DrawStateSlot*)slot);
}
