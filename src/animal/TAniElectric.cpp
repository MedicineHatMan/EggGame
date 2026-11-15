#include "egg/EggCreature.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "stage/Stage.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "engine/GroundQuery.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"
#include "camera/TCameraCtrl.h"
#include "engine/Colli.h"
#include "camera/TCamPath.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "io/ResBank.h"
#include "object/ObjShadow.h"
#include "egg/TEggCreatureTypeE.h"
#include "sound/Sound3DPlay.h"
#include "engine/Color.h"
#include "animal/TAniSaruKoumori.h"
#include "animal/TAnimalRacer.h"
struct DrawStateSlot;
struct DrawState;
struct ColliShape;
struct Fx;
struct CharaColli;

typedef char TAniElectric_size_check[(sizeof(TAniElectric) == 0xE0) ? 1 : -1];

const char* gTAniElectricName = "TAniElectric";

// 0x801303A4
TAniElectric::TAniElectric(TEggCreatureTypeE* owner)
    : TObject(owner)
{
	_00       = (u32)gTAniElectricName;
	mSize     = 0xE0;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mTexList  = (u32)owner->mTexList;
	mtxIdentity(mMatrix);
	mFx     = fxSpawnFollow(1, 9, 3, mMatrix);
	mWidth  = 10.0f;
	mHeight = 20.0f;
	mFrame  = 0.0f;
	mColor  = 0xFFFFFFFF;
}

// 0x80130458
TAniElectric::~TAniElectric()
{
	if (mFx != 0) {
		fxRemove((Fx*)mFx);
	}
	removeDrawTask(mDrawTask);
}

// 0x801304D0
void TAniElectric::draw()
{
	if (mFrame < 3.75f) {
		mFrame += 0.25f;
	} else {
		mFrame = 0.0f;
	}

	f32 frame = mFrame;
	s16 row   = (int)frame;
	f32 u     = frame - (f32)row;
	f32 v     = 0.25f * (f32)row;

	EggHits hits;
	Vec3f pos = *mPosRef;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&pos, (unsigned long)0xD) != 0) {
		mGround = hits.mHits[0].mPos;
		mGround.y += 0.5f;
		mHeight = pos.y - mGround.y;
	}
	mtxSetTransV(mMatrix, &mGround);

	mVtx[0].x     = -0.5f * mWidth;
	f32 u2 = 0.25f + u;
	f32 v2 = 0.25f + v;

	mVtx[0].y     = 0.0f;
	mVtx[0].z     = 0.0f;
	mVtx[0].u     = u;
	mVtx[0].v     = v;
	mVtx[0].color = mColor;
	mVtx[1].x     = 0.5f * mWidth;
	mVtx[1].y     = 0.0f;
	mVtx[1].z     = 0.0f;
	mVtx[1].u     = u2;
	mVtx[1].v     = v;
	mVtx[1].color = mColor;
	mVtx[2].x     = -0.5f * mWidth;
	mVtx[2].y     = -mHeight;
	mVtx[2].z     = 0.0f;
	mVtx[2].u     = u;
	mVtx[2].v     = v2;
	mVtx[2].color = mColor;
	mVtx[3].x     = 0.5f * mWidth;
	mVtx[3].y     = -mHeight;
	mVtx[3].z     = 0.0f;
	mVtx[3].u     = u2;
	mVtx[3].v     = v2;
	mVtx[3].color = mColor;
}

// 0x801306D8
void TAniElectric::vfn_10()
{
}

// 0x801306DC
void TAniElectric::render()
{
	u32 state;

	getCullMode((int*)&state);
	setCullMode(0);
	blendModeAdd();
	njSetTexture((void*)mTexList);
	njSetTextureNum(0);
	njPushMatrixEx();
	njTranslateEx(mPosRef);
	gjRotateY(camGetDrawYaw());
	njDrawTexture3DEx(mVtx, 4, 1);
	njPopMatrixEx();
	blendModeAlpha();
	setCullMode(state);
}

// 0x8013075C
int TAniElectric::vfn_20()
{
	return 4;
}

// 0x80130764
const Vec3f* TAniElectric::vfn_24()
{
	return mPosRef;
}
