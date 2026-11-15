#include "object/orange/TObjOrangeBossFence.h"
#include "engine/DrawTask.h"
#include "io/ResBank.h"
#include "egg/EggCreature.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"

extern "C" {

NJS_OBJECT** gOrangeBossObjModels;    // 0x802C5620
NJS_TEXLIST** gOrangeBossObjTextures; // 0x802C5624
}

typedef char TObjOrangeBossFence_size_check[(sizeof(TObjOrangeBossFence) == 0xD4) ? 1 : -1];

const char* gTObjOrangeBossFenceName = "TObjOrangeBossFence";

// 0x80129F7C
extern "C" void loadOrangeBossObj()
{
	resBankLoad(&gOrangeBossObjBank, "ar_obj_orange_boss.arc");
	resBankAddTextures(&gOrangeBossObjBank, 0, "obj_orange_boss.gvm");
	gOrangeBossObjModels   = resBankGetModels(&gOrangeBossObjBank);
	gOrangeBossObjTextures = resBankGetTextures(&gOrangeBossObjBank);
}

// 0x80129FE4
extern "C" void releaseOrangeBossObj()
{
	resBankRemoveTextures(&gOrangeBossObjBank, 0);
	resBankRelease(&gOrangeBossObjBank);
	gOrangeBossObjModels   = 0;
	gOrangeBossObjTextures = 0;
}

// 0x8012A028
extern "C" TObject* createTObjOrangeBossFence(TObject* parent, SetDesc* desc)
{
	return new TObjOrangeBossFence(parent, desc);
}

// 0x8012A07C
TObjOrangeBossFence::TObjOrangeBossFence(TObject* parent, SetDesc* desc)
    : TSetObj(parent, desc)
{
	_00       = (u32)gTObjOrangeBossFenceName;
	mSize     = 0xD4;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mLight.reset();
	mLight.setFadeDist(-80.0f);
}

// 0x8012A100
TObjOrangeBossFence::~TObjOrangeBossFence()
{
	removeDrawTask(mDrawTask);
	mDrawTask = 0;
}

// 0x8012A17C
void TObjOrangeBossFence::draw()
{
	if (!isInRange(this, &mTrans.mPos, 1020.0f)) {
		mFlags |= 1;
	}
}

// 0x8012A1C4
void TObjOrangeBossFence::vfn_10()
{
	int visible = mLight.isVisible(mDrawTask, 1000.0f, &mTrans.mPos, 150.0f, 0);
	if (visible != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8012A234
void TObjOrangeBossFence::render()
{
	njSetTexture(*gOrangeBossObjTextures);
	njPushMatrixEx();
	njTranslateEx(&mTrans.mPos);
	gjRotateY(mTrans.mRot.y);
	mLight.drawObject(*gOrangeBossObjModels);
	njPopMatrixEx();
}
