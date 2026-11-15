#include "object/TObjTitleFlowers.h"
#include "engine/DrawTask.h"
#include "egg/EggCreature.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
#include "ui/TitleObjArc.h"
struct NJS_TEXLIST;

typedef char TObjTitleFlowers_size_check[(sizeof(TObjTitleFlowers) == 0x8C) ? 1 : -1];

const char* gTObjTitleFlowersName = "TObjTitleFlowers";

// 0x801AD5A0
// The set init pair both empty
extern "C" void initTObjTitleFlowers() { }

// 0x801AD5A4
extern "C" void termTObjTitleFlowers() { }

// 0x801AD5A8
extern "C" TObject* createTObjTitleFlowers(TObject* parent, SetDesc* desc)
{
	return new TObjTitleFlowers(parent, desc);
}

// 0x801AD5FC
TObjTitleFlowers::TObjTitleFlowers(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjTitleFlowersName;
	mSize = 0x8C;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	NJS_OBJECT** models   = getTitleObjModels();
	mModel.setModel(models[1]);
	void** textures = (void**)getTitleObjTexLists();
	mModel.mTexList = (NJS_TEXLIST*)textures[0];
	mModel.mState   = getDefaultDrawState();
	initFromDesc(desc);
}

// 0x801AD6B0
TObjTitleFlowers::~TObjTitleFlowers()
{
	removeDrawTask(mDraw);
	mDraw = 0;
}

// 0x801AD72C
void TObjTitleFlowers::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	f32 value = desc->getFloat(0) < -1.0f ? -1.0f : (desc->getFloat(0) > 10.0f ? 10.0f : desc->getFloat(0));
	mScale = 1.0f + value;
}

// 0x801AD7D4
void TObjTitleFlowers::debugPrint(void*)
{
	njPrint(0xA0017, ": SCALE(+1.0f)");
}

// 0x801AD808
void TObjTitleFlowers::draw()
{
	mModel.calcMotion();
}

// 0x801AD82C
void TObjTitleFlowers::vfn_10()
{
	if (isInRange((void*)this, (const Vec3f*)&mPos, 100000.0f) != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x801AD88C
void TObjTitleFlowers::render()
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
