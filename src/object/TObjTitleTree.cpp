#include "object/TObjTitleTree.h"
#include "engine/DrawTask.h"
#include "egg/EggCreature.h"
#include "ui/TitleObjArc.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
struct NJS_TEXLIST;

typedef char TObjTitleTree_size_check[(sizeof(TObjTitleTree) == 0xC8) ? 1 : -1];

const char* gTObjTitleTreeName = "TObjTitleTree";
const MotionEntry sTitleTreeMotions[] = {
	{ 0, 2, -1, 0, 0.0f, 40.0f, 0.1f, 0.0f, 0 },
};

// 0x801ADEC0
// The set init pair both empty
extern "C" void initTObjTitleTree() { }

// 0x801ADEC4
extern "C" void termTObjTitleTree() { }

// 0x801ADEC8
extern "C" TObject* createTObjTitleTree(TObject* parent, SetDesc* desc)
{
	return new TObjTitleTree(parent, desc);
}

// 0x801ADF1C
TObjTitleTree::TObjTitleTree(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjTitleTreeName;
	mSize = 0xC8;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mMotion.init(sTitleTreeMotions, getTitleObjMotion(1));
	mMotion.play(0);
	NJS_OBJECT** models    = getTitleObjModels();
	mModel.setModel(models[4]);
	void** textures  = (void**)getTitleObjTexLists();
	mModel.mTexList  = (NJS_TEXLIST*)textures[0];
	mModel.mState    = getDefaultDrawState();
	mModel.mMotion   = &mMotion;
	initFromDesc(desc);
}

// 0x801AE008
TObjTitleTree::~TObjTitleTree()
{
	removeDrawTask(mDraw);
	mDraw = 0;
}

// 0x801AE084
void TObjTitleTree::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	f32 value = desc->getFloat(0) < -1.0f ? -1.0f : (desc->getFloat(0) > 10.0f ? 10.0f : desc->getFloat(0));
	mScale = 1.0f + value;
}

// 0x801AE12C
void TObjTitleTree::debugPrint(void*)
{
	njPrint(0xA0017, ": SCALE(+1.0f)");
}

// 0x801AE160
void TObjTitleTree::draw()
{
	mMotion.update();
	mModel.calcMotion();
}

// 0x801AE198
void TObjTitleTree::vfn_10()
{
	if (isInRange((void*)this, (const Vec3f*)&mPos, 100000.0f) != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x801AE1F8
void TObjTitleTree::render()
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
