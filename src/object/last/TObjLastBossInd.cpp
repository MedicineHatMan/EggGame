#include "object/last/TObjLastBossInd.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "camera/TCameraCtrl.h"
#include "enemy/EnemyFx.h"
#include "object/last/TObjLastBossEgg_Frag.h"
#include "object/last/TObjLastBossEgg.h"

extern "C" {

// TObjLastBossEgg.cpp The egg before the last boss which shows where it is until it breaks open
}

extern const f32 gLastBossIndMargin;

extern const char* gTObjLastBossIndName;

// 0x8016C998
TObjLastBossInd::TObjLastBossInd(TObject* parent, SetDesc* desc)
    : TSetObj(parent, desc)
{
	_00   = (u32)gTObjLastBossIndName;
	mSize = 0xBC;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mSurface.reset();
	initFromDesc(desc);
}

// 0x8016CA2C
TObjLastBossInd::~TObjLastBossInd()
{
	removeDrawTask(mDraw);
	mDraw = 0;
}

// 0x8016CAA8
void TObjLastBossInd::initFromDesc(SetDesc* desc)
{
	readDesc(desc);
	int kind = desc->getInt(0);
	if (kind < 0) {
		kind = 0;
	} else if (kind > gLastBossIndCount - 1) {
		kind = gLastBossIndCount - 1;
	}
	mModel = lastBossGetModel((int)gLastBossIndModels[kind]);
	GjModel* model;
	if (mModel != 0 && (model = mModel->model) != 0) {
		mRadius = gLastBossIndMargin + model->r;
	} else {
		mRadius = gLastBossIndMargin;
	}
	mSurface.readDesc(desc);
	mSurface.bindModel(mModel->model, &mTrans);
}

// 0x8016CB88
void TObjLastBossInd::draw()
{
	if (isInRange((void*)this, (const Vec3f*)&mTrans.mPos, 1000020.0f) == 0) {
		mFlags |= 1;
	} else {
		mSurface.update();
	}
}

// 0x8016CBDC
void TObjLastBossInd::debugPrint(void*)
{
	njPrintC(0xA0013, ":MDL NO");
	mSurface.debugPrint();
	mSurface.debugPrintTrans(&mTrans);
}

// 0x8016CC28
void TObjLastBossInd::vfn_10()
{
	if (cameraIsVisible(1000000.0f, mRadius, &mTrans.mPos, 0) != 0 && gLastBossIndShown != 0) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x8016CC94
void TObjLastBossInd::render()
{
	mSurface.draw(&mTrans, mModel, lastBossGetTexList(gLastBossIndTex));
}

// 0x8016CCD8
// 200 more than the model's bounding radius
extern "C" f32 lastBossModelRadius(const NJS_OBJECT* obj)
{
	if (obj != 0 && obj->model != 0) {
		return 200.0f + obj->model->r;
	}
	return 200.0f;
}

// 0x8016CD04
// From lastBossLoad
extern "C" void lastBossIndInit()
{
	gLastBossEggPos.x = 0.0f;
	gLastBossEggOpen  = 0;
	gLastBossEggPos.y = 0.0f;
	gLastBossEggPos.z = 0.0f;
}

// 0x8016CD24
extern "C" void lastBossIndExit()
{
}

// 0x8016CD28
// The flash before the egg breaks
extern "C" void lastBossEggFlash()
{
	Vec3f pos;
	pos.x = gLastBossEggPos.x;
	pos.y = gLastBossEggPos.y;
	pos.z = gLastBossEggPos.z;
	enemyFxSpawnAtPos(5, (void*)0x24, (const Vec3f*)&pos);
}

// 0x8016CD74
// Breaks the egg into its pieces the shell then each piece under it each flying away from the middle
extern "C" void lastBossEggBreak()
{
	gLastBossEggOpen = 1;
	gLastBossEggYaw  = 0x4000;
	NJS_OBJECT* egg  = lastBossGetModel(4);
	gjPushUnitMatrix();
	njTranslateEx(&gLastBossEggPos);
	gjTranslate(0.0f, -450.0f, 0.0f);
	gjRotateY(gLastBossEggYaw);
	NJS_OBJECT* piece = egg->child;
	Vec3f local;
	Vec3f world;
	Vec3f gap;
	local.x = piece->pos[0];
	local.y = piece->pos[1];
	local.z = piece->pos[2];
	njCalcPointEx(&local, &world);
	gap.x     = world.x;
	gap.y     = world.y;
	gap.z     = world.z;
	gap.x     = gap.x - gLastBossEggPos.x;
	gap.y     = gap.y - gLastBossEggPos.y;
	gap.z     = gap.z - gLastBossEggPos.z;
	Vec3f out = gap;
	spawnTObjLastBossEgg_FragShell(piece, &world, &out, gLastBossEggYaw);
	njTranslateEx((Vec3f*)piece->pos);
	for (piece = piece->child; piece != 0; piece = piece->sibling) {
		local.x = piece->pos[0];
		local.y = piece->pos[1];
		local.z = piece->pos[2];
		njCalcPointEx(&local, &world);
		gap.x      = world.x;
		gap.y      = world.y;
		gap.z      = world.z;
		gap.x      = gap.x - gLastBossEggPos.x;
		gap.y      = gap.y - gLastBossEggPos.y;
		gap.z      = gap.z - gLastBossEggPos.z;
		Vec3f away = gap;
		spawnTObjLastBossEgg_Frag(piece, &world, &away, gLastBossEggYaw);
	}
	njPopMatrixEx();
}

// 0x8016CF28
extern "C" void lastBossEggClose()
{
	gLastBossEggOpen = 0;
}
