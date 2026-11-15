#include "object/TObjBoss02Fence.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/ResBank.h"
#include "egg/EggCreature.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"

typedef char TObjBoss02Fence_size_check[(sizeof(TObjBoss02Fence) == 0xD4) ? 1 : -1];

// 0x800683D8
extern "C" void initBlueBoss()
{
	resBankLoad(&sBoss02FenceBank, "ar_obj_blue_boss.arc");
	resBankAddTextures(&sBoss02FenceBank, 0, "obj_blue_boss.gvm");
	sBoss02FenceModels   = resBankGetModels(&sBoss02FenceBank);
	sBoss02FenceTextures = resBankGetTextures(&sBoss02FenceBank);
}

// 0x80068440
extern "C" void termBlueBoss()
{
	resBankRemoveTextures(&sBoss02FenceBank, 0);
	resBankRelease(&sBoss02FenceBank);
	sBoss02FenceModels   = 0;
	sBoss02FenceTextures = 0;
}

// 0x80068484
extern "C" NJS_OBJECT** boss02GetFenceModels()
{
	return sBoss02FenceModels;
}

// 0x8006848C
extern "C" NJS_TEXLIST** boss02GetFenceTextures()
{
	return sBoss02FenceTextures;
}

// 0x80068494
extern "C" TObject* createTObjBoss02Fence(TObject* parent, SetDesc* desc)
{
	return new TObjBoss02Fence(parent, desc);
}

// 0x800684E8
TObjBoss02Fence::TObjBoss02Fence(TObject* parent, SetDesc* desc)
    : TSetObj(parent, desc)
{
	_00       = gTObjBoss02FenceName;
	mSize     = 0xD4;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mLight.reset();
}

// 0x80068560
TObjBoss02Fence::~TObjBoss02Fence()
{
	removeDrawTask(mDrawTask);
	mDrawTask = 0;
}

// 0x800685DC
void TObjBoss02Fence::draw()
{
	if (isInRange(this, &mTrans.mPos, 2050.0f) == 0) {
		mFlags |= 1;
	}
}

// 0x80068624
void TObjBoss02Fence::vfn_10()
{
	if (mLight.isVisible(mDrawTask, 2500.0f, &mTrans.mPos, 55.0f, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80068694
void TObjBoss02Fence::render()
{
	NJS_OBJECT** models    = boss02GetFenceModels();
	NJS_TEXLIST** textures = boss02GetFenceTextures();

	njSetTexture(textures[0]);
	njPushMatrixEx();
	njTranslateEx(&mTrans.mPos);
	gjRotateY(mTrans.mRot.y);
	mLight.drawObject(models[0]);
	njPopMatrixEx();
}
