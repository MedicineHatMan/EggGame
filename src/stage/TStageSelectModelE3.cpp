#include "stage/TStageSelectModelE3.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "Dolphin/OS/OSCache.h"
#include "Ninja/ninja.h"
#include "object/TObjLauncher.h"
#include "egg/EggCreature.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
struct NJS_OBJECT;
struct NJS_TEXLIST;

typedef char TStageSelectModelE3_size_check[(sizeof(TStageSelectModelE3) == 0x98) ? 1 : -1];

const char* gTStageSelectModelE3Name = "TStageSelectModelE3";

// 0x80156090
// Skips the upload when enable is 0
extern "C" void loadE3Palette(u32 enable, E3Palette* pal, u32 slot)
{
	if (enable != 0) {
		njLoadPaletteData((const GjPaletteFile*)enable, (GjPalette*)pal);
		DCFlushRange(pal->mData, 0x800);
		gjSetPaletteData(slot, pal->mStart, pal->mCount, pal->mData);
		GXDrawDone();
	}
}

// 0x801560EC
// Name is ours
extern "C" void loadE3ModelArc()
{
	loadArc("menu_model.arc", &sStageSelectModelE3Arc);
	sE3AssetsData = (E3Assets*)sStageSelectModelE3Arc->mData;
	bindGvmTextures((u8*)sE3AssetsData->mGvm, (NJS_TEXLIST*)*sE3AssetsData->mTex);
}

// 0x80156134
// Name is ours
extern "C" void freeE3ModelArc()
{
	releaseTextures((NJS_TEXLIST*)*sE3AssetsData->mTex);
	releaseResource(sStageSelectModelE3Arc);
}

// 0x80156168
extern "C" TStageSelectModelE3* createTStageSelectModelE3(TObject* parent, const Vec3f* pos, int angle, int index)
{
	return new TStageSelectModelE3(parent, pos, angle, index);
}

// 0x801561CC
TStageSelectModelE3::TStageSelectModelE3(TObject* parent, const Vec3f* pos, int angle, int index)
    : TObject(parent)
{
	_70       = 0;
	_90       = 0;
	_00       = (u32)gTStageSelectModelE3Name;
	mSize     = 0x98;
	mDraw     = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mPos      = *pos;
	mAngle[0] = 0;
	mAngle[1] = 0;
	mAngle[2] = 0;
	mAngle[1] = angle;
	mIndex    = index;
	resetFrame();
}

// 0x80156284
TStageSelectModelE3::~TStageSelectModelE3()
{
	removeDrawTask(mDraw);
}

// 0x801562EC
void TStageSelectModelE3::resetFrame()
{
	updateMatrix();
	mFrame = 0.0f;
}

// 0x80156320
void TStageSelectModelE3::draw()
{
	mAngle[1] = (mAngle[1] + 0x100) & 0xFFFF;
	updateMatrix();
	mFrame += 1.0f;
	u32 count = *(u32*)((u8*)sE3AssetsData->mMotions[mIndex] + 4);
	if (mFrame > (f32)count) {
		mFrame -= (f32)count;
	}
}

// 0x801563C0
void TStageSelectModelE3::vfn_10()
{
}

// 0x801563C4
void TStageSelectModelE3::render()
{
	f32 view[12];
	mtxConcat(view, gjGetMatrix(), mMtx);
	njPushMatrix(view);
	njSetTexture(*sE3AssetsData->mTex);
	gjDrawMotion((NJS_OBJECT*)sE3AssetsData->mModels[mIndex], sE3AssetsData->mMotions[mIndex], mFrame);
	njPopMatrixEx();
}

// 0x80156440
void TStageSelectModelE3::updateMatrix()
{
	mtxSetTransV(mMtx, &mPos);
	mtxRotateZYX(mMtx, mMtx, mAngle);
}
