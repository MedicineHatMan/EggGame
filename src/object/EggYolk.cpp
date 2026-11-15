#include "stage/Stage.h"
#include "ui/TextVec.h"
#include "engine/MotionPlayer.h"
#include "engine/DrawTask.h"
#include "engine/DrawState.h"
#include "egg/ObjEgg.h"
#include "core/Pad.h"
#include "core/TArray.h"
#include "core/TMainTask.h"
#include "ui/TSimpleText.h"
#include "camera/TCamera.h"
#include "engine/Viewport.h"
#include "engine/ShadowTex.h"
#include "ui/AeArc.h"
#include "ui/TAeObjStageSelectE3.h"
#include "stage/TStageSelectModelE3.h"
#include "object/ObjAction.h"
#include "object/TObjLauncher.h"
#include "io/Arc.h"
#include "camera/TCameraCtrl.h"
#include "ui/TAeObjMsgWindow.h"
#include "sound/SoundCtrl.h"
#include "ui/TFade.h"
#include "core/dolphin.h"
#include "object/TObjEggYolk.h"
#include "ui/TAeObject.h"
#include "core/Mem.h"
#include "engine/GroundQuery.h"
#include "Ninja/ninja.h"
struct NJS_TEXLIST;
struct GjActor;

class TArc;

NJS_OBJECT** sEggYolkModels;    // 0x802C5490
NJS_MOTION** sEggYolkMotions;   // 0x802C5494
NJS_TEXLIST** sEggYolkTexLists; // 0x802C5498

// 0x8025CC30
// Its motions played once only the first is used
MotionEntry sEggYolkMotionTable[2] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 0, -1, 0, 20.0f, 29.0f, 0.4f, 0.0f, 0 },
};

typedef char TObjEggYolk_size_check[(sizeof(TObjEggYolk) == 0x84) ? 1 : -1];

// 0x801007A4
extern "C" void loadEggYolk()
{
	if (gEggArc != 0) {
		sEggYolkModels   = (NJS_OBJECT**)findArcExport(gEggArc, "models");
		sEggYolkMotions  = (NJS_MOTION**)findArcExport(gEggArc, "motions");
		sEggYolkTexLists = (NJS_TEXLIST**)findArcExport(gEggArc, "texlists");
	}
}

// 0x802C4190
// the class name record Defined after loadEggYolk "texlists" comes first in .rodata
u32 sEggYolkName[2] = { (u32) "TObjEggYolk", 0 };

// 0x801007FC
extern "C" void freeEggYolk()
{
	sEggYolkModels   = 0;
	sEggYolkMotions  = 0;
	sEggYolkTexLists = 0;
}

// 0x80100810
extern "C" void spawnEggYolk(f32 size, const Vec3f* pos)
{
	TObjEggYolk* yolk = new TObjEggYolk(gTL_06);
	if (yolk != 0) {
		yolk->mPos   = *pos;
		yolk->mScale = 0.2f * size;
	}
}

// 0x80100898
TObjEggYolk::TObjEggYolk(TObject* parent)
    : TObject(parent)
{
	_00   = sEggYolkName[0];
	mSize = 0x84;
	mMotion.init(sEggYolkMotionTable, sEggYolkMotions);
	mNode = (GjActor*)(memAlloc(gjActorSize(sEggYolkModels[3])));
	gjCreateActor(sEggYolkModels[3], (u8*)mNode);
	mDraw  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mState = YOLK_PROBE;
	mLife  = 30;
}

// 0x80100950
TObjEggYolk::~TObjEggYolk()
{
	memFree(mNode);
	removeDrawTask(mDraw);
}

// 0x801009C0
void TObjEggYolk::draw()
{
	switch (mState) {
	case YOLK_PROBE: {
		EggHits hits;
		if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&mPos, 1) != 0 && (hits.mMask & 1) != 0) {
			f32 y = hits.mHits[0].mPos.y;
			if ((f32)__fabs(y - mPos.y) > 150.0f) {
				mState = YOLK_MISS;
			} else {
				mGroundY = 4.0f * mScale + y;
				mState   = YOLK_FALL;
			}
		} else {
			mState = YOLK_MISS;
		}
		mFallSpeed = 0.0f;
		break;
	}
	case YOLK_FALL:
		fall();
		if (mPos.y < mGroundY) {
			mPos.y = mGroundY;
			mState = YOLK_SPLASH;
			mMotion.play(1);
			mAlpha = 1.0f;
			mLife  = 30;
		}
		break;
	case YOLK_SPLASH:
		if (mMotion.isDone() && --mLife == 0) {
			mState = YOLK_FADE;
			mLife  = 30;
		}
		break;
	case YOLK_FADE:
		mAlpha = mAlpha + -mAlpha / (f32)mLife;
		if (--mLife == 0) {
			mFlags |= 1;
		}
		break;
	case YOLK_MISS:
		if (--mLife == 0) {
			mFlags |= 1;
			return;
		}
		fall();
		break;
	}
	njPushMatrixEx();
	njUnitMatrix(0);
	int cur   = mMotion.mCurrent;
	f32 frame = mMotion.mFrame;
	gjMotionActor(frame, mNode, mMotion.getMotion(cur));
	njPopMatrixEx();
	mMotion.update();
}

// 0x80100C50
void TObjEggYolk::render()
{
	if (mState != YOLK_PROBE) {
		njPushMatrixEx();
		njTranslateEx(&mPos);
		njScale(0, mScale, mScale, mScale);
		njSetTexture(sEggYolkTexLists[3]);
		ObjDrawState state;
		if (mState == YOLK_FADE) {
			state.mFlags    = DRAWSTATE_MATERIAL;
			state.mMaterial = 0xFFFFFF00 | (u8)(int)(255.0f * mAlpha);
		}
		beginDrawState(getDrawStateSlot(), &state);
		gjDrawActor(mNode);
		endDrawState(getDrawStateSlot());
		njPopMatrixEx();
	}
}
