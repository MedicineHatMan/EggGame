#include "object/yellow/TObjYellowTornadoScene.h"
#include "camera/TCamera.h"
#include "engine/Landscape.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "stl/math.h"
#include "animal/AniMount.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"
#include "stage/TEventCtrl.h"
struct Fx;

extern "C" {

extern const int sTornadoRails[] = {
	2, 3, 4, 5, 6,
};
extern const int sTornadoEffects[] = {
	2, 84,
};
}

typedef char TObjYellowTornadoScene_size_check[(sizeof(TObjYellowTornadoScene) == 0x88) ? 1 : -1];

// 0x80189908
extern "C" TObjYellowTornadoScene* createTObjYellowTornadoScene(TObject* parent, int path, int kind)
{
	return new TObjYellowTornadoScene(parent, path, kind);
}

const char* gTObjYellowTornadoSceneName = "TObjYellowTornadoScene";

// 0x80189964
TObjYellowTornadoScene::TObjYellowTornadoScene(TObject* parent, int path, int kind)
    : TObject(parent)
{
	mFx    = 0;
	mKind  = kind;
	mSwing = 0.0f;
	mAngle = 0;
	mScale.x = 0.01f;
	mScale.y = 0.01f;
	mScale.z = 0.01f;
	_00      = (u32)gTObjYellowTornadoSceneName;
	mSize    = 0x88;

	int index = path < 0 ? 0 : (path > 4 ? 4 : path);
	RailPath* rail = railFind(sTornadoRails[index]);
	if (rail != 0) {
		mRail.setPath(rail);
	}
	updatePlace();
}

// 0x80189A34
TObjYellowTornadoScene::~TObjYellowTornadoScene()
{
	fxStop((Fx*)mFx);
}

// 0x80189AA8
void TObjYellowTornadoScene::spawnFx()
{
	int which;
	if (mKind < 0) {
		which = 0;
	} else {
		which = 1;
		if (mKind <= 1) {
			which = mKind;
		}
	}
	mFx = (Fx*)fxSpawnFollow(1, 8, sTornadoEffects[which], mMatrix);
}

// 0x80189B14
void TObjYellowTornadoScene::updatePlace()
{
	Vec3f pos;
	mRail.getPos(&pos);

	f64 s = __fabs(njSin(mAngle >> 3));
	mSwing = 50.0f * (f32)s;
	Vec3f swing;
	swing.set(0.0f, 0.0f, mSwing);
	vecRotateY(&swing, &swing, mAngle);
	vecAdd(&pos, &pos, &swing);

	mtxSetTransV(mMatrix, &pos);
	mtxScaleV(mMatrix, mMatrix, &mScale);
	mAngle += 0x400;
}

// 0x80189BC4
void TObjYellowTornadoScene::draw()
{
	if (eventIsBusy() != 0 || getLandscapeBlend() != 0.0f) {
		if (mFx != 0) {
			fxStop(mFx);
			mFx = 0;
		}
	} else if (mFx == 0) {
		spawnFx();
		mScale.set(0.01f, 0.01f, 0.01f);
	}

	if (mFx == 0) {
		return;
	}
	mRail.advance(1.0f);
	vecScale(1.02f, &mScale, &mScale);
	mScale.x = mScale.x > 1.0f ? 1.0f : mScale.x;
	mScale.y = mScale.y > 1.0f ? 1.0f : mScale.y;
	mScale.z = mScale.z > 1.0f ? 1.0f : mScale.z;
	updatePlace();
}

// 0x80189CCC
void TObjYellowTornadoScene::vfn_10()
{
}
