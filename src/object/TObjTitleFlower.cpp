#include "object/TObjTitleFlower.h"
#include "engine/DrawTask.h"
#include "egg/EggCreature.h"
#include "ui/TitleObjArc.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
struct NJS_TEXLIST;

typedef char TObjTitleFlower_size_check[(sizeof(TObjTitleFlower) == 0xC8) ? 1 : -1];

const char* gTObjTitleFlowerName = "TObjTitleFlower";
const MotionEntry sTitleFlowerMotions[] = {
	{ 0, 1, -1, 0, 0.0f, 10.0f, 0.1f, 0.0f, 0 },
};

// 0x801AD1FC
// The set init pair both empty
extern "C" void initTObjTitleFlower() { }

// 0x801AD200
extern "C" void termTObjTitleFlower() { }

// 0x801AD204
extern "C" TObject* createTObjTitleFlower(TObject* parent, SetDesc* desc)
{
	return new TObjTitleFlower(parent, desc);
}

// 0x801AD258
TObjTitleFlower::TObjTitleFlower(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjTitleFlowerName;
	mSize = 0xC8;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mMotion.init(sTitleFlowerMotions, getTitleObjMotion(1));
	mMotion.play(0);
	NJS_OBJECT** models    = getTitleObjModels();
	mModel.setModel(models[3]);
	void** textures  = (void**)getTitleObjTexLists();
	mModel.mTexList  = (NJS_TEXLIST*)textures[0];
	mModel.mState    = getDefaultDrawState();
	mModel.mMotion   = &mMotion;
	initFromDesc(desc);
}

// 0x801AD344
TObjTitleFlower::~TObjTitleFlower()
{
	removeDrawTask(mDraw);
	mDraw = 0;
}

// 0x801AD3C0
void TObjTitleFlower::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	f32 value = desc->getFloat(0) < -1.0f ? -1.0f : (desc->getFloat(0) > 10.0f ? 10.0f : desc->getFloat(0));
	mScale = 1.0f + value;
}

// 0x801AD468
void TObjTitleFlower::debugPrint(void*)
{
	njPrint(0xA0017, ": SCALE(+1.0f)");
}

// 0x801AD49C
void TObjTitleFlower::draw()
{
	mMotion.update();
	mModel.calcMotion();
}

// 0x801AD4D4
void TObjTitleFlower::vfn_10()
{
	if (isInRange((void*)this, (const Vec3f*)&mPos, 100000.0f) != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x801AD534
void TObjTitleFlower::render()
{
	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjRotateZ(mRot.z);
	gjRotateX(mRot.x);
	gjRotateY(mRot.y);
	njScale(0, mScale, mScale, mScale);
	mModel.draw();
	njPopMatrixEx();
}
