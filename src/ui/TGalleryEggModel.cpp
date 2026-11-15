#include "ui/TGalleryEggModel.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "ui/AeArc.h"
#include "object/TObjLauncher.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"

const char* gTGalleryEggModelName = "TGalleryEggModel";
struct NJS_TEXLIST;

// The block of gallery_egg.arc gGalleryEggs
// +0xB8 and +0xBC are two extra textures used by eggs 0x28-0x2A and 0x2C
struct GalleryEggAssets {
	int mCount;         // _00 of mTex and mGvm
	NJS_OBJECT* mModel; // _04
	void* mMotion;      // _08 length is a u32 at +0x04
	void** mTex;        // _0C
	void** mGvm;        // _10
};

TArc* gGalleryEggArc;           // 0x802C5AE0
GalleryEggAssets* gGalleryEggs; // 0x802C5AE4

// 0x801B622C
// Name is ours
extern "C" void loadGalleryEggs()
{
	loadArc("gallery_egg.arc", &gGalleryEggArc);
	gGalleryEggs = (GalleryEggAssets*)gGalleryEggArc->mData;
	for (int i = 0; i < gGalleryEggs->mCount; i++) {
		bindGvmTextures((u8*)gGalleryEggs->mGvm[i], (NJS_TEXLIST*)gGalleryEggs->mTex[i]);
	}
}

// 0x801B62A4
// Name is ours
extern "C" void freeGalleryEggs()
{
	for (int i = 0; i < gGalleryEggs->mCount; i++) {
		releaseTextures((NJS_TEXLIST*)gGalleryEggs->mTex[i]);
	}
	releaseResource(gGalleryEggArc);
}

typedef char TGalleryEggModel_size_check[(sizeof(TGalleryEggModel) == 0x60) ? 1 : -1];

// 0x801B6300
extern "C" TGalleryEggModel* createTGalleryEggModel(TObject* parent, u16 egg)
{
	return new TGalleryEggModel(parent, egg);
}

// 0x801B6354
TGalleryEggModel::TGalleryEggModel(TObject* parent, u16 egg)
    : TObject(parent)
{
	mEgg  = egg;
	_00   = (u32)gTGalleryEggModelName;
	mSize = 0x60;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	initMotion();
}

// 0x801B63CC
TGalleryEggModel::~TGalleryEggModel()
{
	removeDrawTask(mDraw);
}

// 0x801B6434
void TGalleryEggModel::initMotion()
{
	mtxIdentity(mMtx);
	mFrame = 0.0f;
	mRate  = 1.0f;
}

// 0x801B6474
void TGalleryEggModel::draw()
{
	mFrame += mRate;
	u32 count = *(u32*)((u8*)gGalleryEggs->mMotion + 4);
	if (mFrame > (f32)count) {
		mFrame -= (f32)count;
	}
}

// 0x801B64D8
void TGalleryEggModel::vfn_10()
{
}

// 0x801B64DC
// Opaque post-tev stage
extern "C" void setGalleryEggTev(u8 stage)
{
	GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(stage, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
}

// 0x801B6580
// The colour C input is the texture
extern "C" void setGalleryEggTevTex(u8 stage)
{
	GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(stage, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
}

// 0x801B6624
// Name is ours
extern "C" void setGalleryEggTexGen(u8 dst)
{
	GXSetTexCoordGen2(dst, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX5, GX_TRUE, GX_PTTEXMTX19);
}

// 0x801B665C
// kind 0 installs the opaque callback anything else the textured one
extern "C" void beginGalleryEggTex(void* tex, void* image, int kind)
{
	f32 mtx[12];
	gjLoadTexture((NJS_TEXLIST*)image, 0, 0, 0, 1);
	PSMTXCopy(_gj_matrix_, mtx);
	mtx[3]  = 0.0f;
	mtx[7]  = 0.0f;
	mtx[11] = 0.0f;
	GXLoadTexMtxImm(mtx, GX_TEXMTX5, GX_MTX3x4);
	gjSetNumPostTexGens(1);
	gjSetNumPostTevStages(1);
	if (kind != 0) {
		gjSetPostTevStageCallback(setGalleryEggTevTex);
	} else {
		gjSetPostTevStageCallback(setGalleryEggTev);
	}
	gjSetPostTexGenCallback(setGalleryEggTexGen);
	_nj_control_3d_flag_ |= 0x80000;
	njSetTexture(tex);
}

// 0x801B6724
// Name is ours
extern "C" void endGalleryEggTex()
{
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	_nj_control_3d_flag_ &= 0xFFF7FFFF;
}

// 0x801B675C
// Eggs 0x28-0x2A and 0x2C go through the post-tev path
void TGalleryEggModel::render()
{
	f32 view[12];
	mtxConcat(view, gjGetMatrix(), mMtx);
	njPushMatrix(view);

	int egg = (s16)mEgg;
	switch (egg) {
	case 0x28:
	case 0x29:
	case 0x2A: {
		void** tex = gGalleryEggs->mTex;
		beginGalleryEggTex(tex[egg], *(void**)((u8*)tex + 0xB8), 0);
		gjDrawMotion(gGalleryEggs->mModel, gGalleryEggs->mMotion, mFrame);
		endGalleryEggTex();
		break;
	}
	case 0x2C: {
		void** tex = gGalleryEggs->mTex;
		beginGalleryEggTex(tex[egg], *(void**)((u8*)tex + 0xBC), 1);
		gjDrawMotion(gGalleryEggs->mModel, gGalleryEggs->mMotion, mFrame);
		endGalleryEggTex();
		break;
	}
	default:
		njSetTexture(gGalleryEggs->mTex[egg]);
		gjDrawMotion(gGalleryEggs->mModel, gGalleryEggs->mMotion, mFrame);
		break;
	}
	njPopMatrixEx();
}
