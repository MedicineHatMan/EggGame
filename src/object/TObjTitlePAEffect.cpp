#include "object/TObjTitlePAEffect.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "camera/TCameraCtrl.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "stl/math.h"
#include "object/TObjEggRing.h"
#include "core/VecMath.h"
#include "mode/TModeTitle.h"
struct Fx;

typedef char TObjTitlePAEffect_size_check[(sizeof(TObjTitlePAEffect) == 0x9C) ? 1 : -1];

// 0x801AE33C
// The set init pair both empty
extern "C" void initTObjTitlePAEffect() { }

// 0x801AE340
extern "C" void termTObjTitlePAEffect() { }

// 0x801AE344
extern "C" TObject* createTObjTitlePAEffect(TObject* parent, SetDesc* desc)
{
	return new TObjTitlePAEffect(parent, desc);
}

const char* gTObjTitlePAEffectName = "TObjTitlePAEffect";

// 0x801AE398
TObjTitlePAEffect::TObjTitlePAEffect(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjTitlePAEffectName;
	mSize = 0x9C;
	mtxIdentity(mMatrix);
	if (gTitleScene == 3) {
		mFx = fxSpawnFollow(1, 8, 0, mMatrix);
	} else {
		mFx = fxSpawnFollow(1, 8, 1, mMatrix);
	}
}

// 0x801AE42C
TObjTitlePAEffect::~TObjTitlePAEffect()
{
	fxStop((Fx*)mFx);
}

// 0x801AE494
void TObjTitlePAEffect::draw()
{
	Vec3f cam;
	Vec3f at;
	Vec3f dir;
	camGetAt(0, &cam);
	if (gTitleScene == 3) {
		mtxSetTrans((float*)mMatrix, cam.x, cam.y, cam.z);
		return;
	}

	dir.x = 1.0f;
	dir.y = 0.0f;
	dir.z = 0.0f;
	if (vecNormalize(&dir, &dir) == 0.0f) {
		dir.x = 1.0f;
		dir.y = 0.0f;
		dir.z = 0.0f;
	}

	f32 sin0, cos0, sin1, cos1;
	int around = (int)(10430.38043493439 * (f64)(f32)atan2(dir.y, dir.z));
	njSinCos(around, &sin0, &cos0);
	int lift = (int)(10430.38043493439 * (f64)(f32)atan2(dir.x, dir.y * sin0 + dir.z * cos0));
	njSinCos(lift, &sin1, &cos1);

	f32 mtx[12];
	mtx[0]  = cos1;
	mtx[1]  = 0.0f;
	mtx[2]  = sin1;
	mtx[4]  = -sin0 * sin1;
	mtx[5]  = cos0;
	mtx[6]  = sin0 * cos1;
	mtx[8]  = -cos0 * sin1;
	mtx[9]  = -sin0;
	mtx[10] = cos0 * cos1;

	vecScaleAdd(-100.0f, &at, &dir, &cam);
	mtx[3]  = at.x;
	mtx[7]  = 50.0f + at.y;
	mtx[11] = 100.0f + at.z;
	mtxCopy(mMatrix, mtx);
}
