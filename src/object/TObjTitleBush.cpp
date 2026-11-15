#include "object/TObjTitleBush.h"
#include "engine/DrawTask.h"
#include "egg/EggCreature.h"
#include "ui/TitleObjArc.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
struct NJS_TEXLIST;

typedef char TObjTitleBush_size_check[(sizeof(TObjTitleBush) == 0xC8) ? 1 : -1];

const char* gTObjTitleBushName        = "TObjTitleBush";
const MotionEntry sTitleBushMotions[] = {
	{ 0, 0, -1, 0, 0.0f, 10.0f, 0.1f, 0.0f, 0 },
};

// 0x801ACE58
// The set init pair both empty
extern "C" void initTObjTitleBush()
{
}

// 0x801ACE5C
extern "C" void termTObjTitleBush()
{
}

// 0x801ACE60
extern "C" TObject* createTObjTitleBush(TObject* parent, SetDesc* desc)
{
	return new TObjTitleBush(parent, desc);
}

// 0x801ACEB4
TObjTitleBush::TObjTitleBush(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjTitleBushName;
	mSize = 0xC8;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mMotion.init(sTitleBushMotions, getTitleObjMotion(1));
	mMotion.play(0);
	NJS_OBJECT** models = getTitleObjModels();
	mModel.setModel(models[2]);
	void** textures = (void**)getTitleObjTexLists();
	mModel.mTexList = (NJS_TEXLIST*)textures[0];
	mModel.mState   = getDefaultDrawState();
	mModel.mMotion  = &mMotion;
	initFromDesc(desc);
}

// 0x801ACFA0
TObjTitleBush::~TObjTitleBush()
{
	removeDrawTask(mDraw);
	mDraw = 0;
}

// 0x801AD01C
void TObjTitleBush::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	f32 value = desc->getFloat(0) < -1.0f ? -1.0f : (desc->getFloat(0) > 10.0f ? 10.0f : desc->getFloat(0));
	mScale    = 1.0f + value;
}

// 0x801AD0C4
void TObjTitleBush::debugPrint(void*)
{
	njPrint(0xA0017, ": SCALE(+1.0f)");
}

// 0x801AD0F8
void TObjTitleBush::draw()
{
	mMotion.update();
	mModel.calcMotion();
}

// 0x801AD130
void TObjTitleBush::vfn_10()
{
	if (isInRange((void*)this, (const Vec3f*)&mPos, 100000.0f) != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x801AD190
void TObjTitleBush::render()
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
