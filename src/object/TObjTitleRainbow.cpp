#include "object/TObjTitleRainbow.h"
#include "engine/DrawTask.h"
#include "egg/EggCreature.h"
#include "io/NinjaFile.h"
#include "object/ObjAction.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
#include "engine/FogBlend.h"
#include "ui/TitleObjArc.h"
#include "mode/TModeTitle.h"
struct NJS_TEXLIST;

// 0x801B4710
// loads ae_gallery.arc and is not this class

typedef char TObjTitleRainbow_size_check[(sizeof(TObjTitleRainbow) == 0x8C) ? 1 : -1];

// 0x801B42F4
// The set init pair both empty
extern "C" void initTObjTitleRainbow() { }

// 0x801B42F8
extern "C" void termTObjTitleRainbow() { }

// 0x801B42FC
extern "C" TObject* createTObjTitleRainbow(TObject* parent, SetDesc* desc)
{
	return new TObjTitleRainbow(parent, desc);
}

const char* gTObjTitleRainbowName = "TObjTitleRainbow";

// 0x801B4350
TObjTitleRainbow::TObjTitleRainbow(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjTitleRainbowName;
	mSize = 0x8C;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x2A, 0);
	NJS_OBJECT** models   = getTitleObjModels();
	mModel.setModel(models[9]);
	void** textures = (void**)getTitleObjTexLists();
	mModel.mTexList = (NJS_TEXLIST*)textures[0];
	mModel.mState   = getDefaultDrawState();
	initFromDesc(desc);
}

// 0x801B4404
TObjTitleRainbow::~TObjTitleRainbow()
{
	removeDrawTask(mDraw);
	mDraw = 0;
}

// 0x801B4480
void TObjTitleRainbow::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	f32 value = desc->getFloat(0) < -1.0f ? -1.0f : (desc->getFloat(0) > 10.0f ? 10.0f : desc->getFloat(0));
	mScale = 1.0f + value;
}

// 0x801B4528
void TObjTitleRainbow::debugPrint(void*)
{
	njPrint(0xA0017, ": SCALE(+1.0f)");
}

// 0x801B455C
void TObjTitleRainbow::draw()
{
	mModel.calcMotion();
}

// 0x801B4580
void TObjTitleRainbow::vfn_10()
{
	if (isInRange((void*)this, (const Vec3f*)&mPos, 100000.0f) != 0 && gTitleScene != 3) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x801B45EC
void TObjTitleRainbow::render()
{
	njPushMatrixEx();
	ObjDrawState state;
	state = *getDefaultDrawState();
	int alpha;
	if (debugEditActive() != 0) {
		alpha = 0xFF;
	} else {
		alpha = (int)(255.0f * (1.0f - fogBlendUpdate()));
	}
	state.mFlags |= DRAWSTATE_AMBIENT;
	state.mAmbient = (alpha & 0xFF) | 0xFFFFFF00;
	mModel.mState = &state;
	njTranslateEx(&mPos);
	gjRotateZ(mRot.z);
	gjRotateX(mRot.x);
	gjRotateY(mRot.y);
	njScale(0, mScale, mScale, mScale);
	blendModeAdd();
	mModel.draw();
	mModel.mState = getDefaultDrawState();
	njPopMatrixEx();
}
