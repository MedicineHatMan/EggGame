#include "ui/TAeObjStageSelectE3.h"
#include "object/TObjEggYolk.h"
#include "io/Arc.h"
#include "ui/AeArc.h"
#include "core/TMainTask.h"
#include "core/dolphin.h"
#include "Ninja/ninja.h"

const u32 gE3PreviewLayers[4] = { 0x17, 0x1D, 0x23, 0x29 };

TAeObjStageSelectE3* sAeStageSelectE3; // 0x802C54A0
TArc* sAeStageSelectE3Arc;             // 0x802C54A4
void* sAeStageSelectE3Ae;              // 0x802C54A8
NJS_TEXLIST* sAeStageSelectE3TexList;  // 0x802C54AC

// 0x80246EC0
// the palette bank for each palette
extern const u32 gAeStageSelectE3Banks[8] = { 16, 17, 18, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };

// 0x802C41A0
// the class name record
u32 sAeStageSelectE3Name[2] = { (u32) "TAeObjStageSelectE3", 0 };

// 0x801016CC
extern "C" void loadStageSelectE3()
{
	loadArc("ae_st_select.arc", &sAeStageSelectE3Arc);
	if (sAeStageSelectE3Arc != 0) {
		AeArcBlock* block  = aeArcBlock(sAeStageSelectE3Arc);
		sAeStageSelectE3Ae = block->mAe;
		relocateAe(sAeStageSelectE3Ae);
		sAeStageSelectE3TexList = block->mTexList;
		bindGvmTextures((u8*)block->mGvm, sAeStageSelectE3TexList);
		u8* buf = new u8[0x800];
		loadE3Palette(block->mPalettes[0], (E3Palette*)&buf, gAeStageSelectE3Banks[0]);
		loadE3Palette(block->mPalettes[1], (E3Palette*)&buf, gAeStageSelectE3Banks[1]);
		loadE3Palette(block->mPalettes[2], (E3Palette*)&buf, gAeStageSelectE3Banks[2]);
		delete[] buf;
		njSetTexture(sAeStageSelectE3TexList);
		njSetPaletteBankNum(0, gAeStageSelectE3Banks[0]);
		njSetPaletteBankNum(1, gAeStageSelectE3Banks[1]);
		njSetPaletteBankNum(2, gAeStageSelectE3Banks[2]);
		GXInvalidateTexAll();
	}
}

// 0x801017C8
extern "C" void freeStageSelectE3()
{
	if (sAeStageSelectE3Arc != 0) {
		releaseTextures((NJS_TEXLIST*)sAeStageSelectE3TexList);
		sAeStageSelectE3TexList = 0;
		releaseResource(sAeStageSelectE3Arc);
		sAeStageSelectE3Arc = 0;
		sAeStageSelectE3Ae  = 0;
	}
}

// 0x80101814
extern "C" TAeObjStageSelectE3* createStageSelectE3()
{
	if (sAeStageSelectE3Arc == 0) {
		return 0;
	}
	return sAeStageSelectE3 = new TAeObjStageSelectE3(gTL_07, sAeStageSelectE3Ae, sAeStageSelectE3TexList);
}

typedef char TAeObjStageSelectE3_size_check[(sizeof(TAeObjStageSelectE3) == 0x44) ? 1 : -1];

// 0x80101874
TAeObjStageSelectE3::TAeObjStageSelectE3(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	_00    = sAeStageSelectE3Name[0];
	mSize  = 0x44;
	mLayer = createLayer(0x81, 0x10);
	if (mLayer == 0) {
		return;
	}
	mLayer->mBanks = gAeStageSelectE3Banks;
	mPreview     = createLayer(0x17, 0);
	if (mPreview == 0) {
		return;
	}
	mPreview->mBanks = gAeStageSelectE3Banks;
	mLayer->bind(0x2F, static_cast<TAeClip*>(mPreview));
}

// 0x8010194C
TAeObjStageSelectE3::~TAeObjStageSelectE3() { }

// 0x801019A8
void TAeObjStageSelectE3::draw()
{
	if (mLayer != 0) {
		mLayer->update();
	}
	if (mPreview != 0) {
		mPreview->update();
	}
}

// 0x801019F0
void TAeObjStageSelectE3::vfn_10()
{
	if (mLayer != 0) {
		mLayer->present();
	}
}

// 0x80101A1C
// Shows row stage's preview
extern "C" void showStageSelectE3(int stage)
{
	if (sAeStageSelectE3 != 0 && sAeStageSelectE3->mPreview != 0) {
		static_cast<TAeClip&>(*sAeStageSelectE3->mPreview).reset(gE3PreviewLayers[stage], 1);
		sAeStageSelectE3->mPreview->mBanks = gAeStageSelectE3Banks;
		sAeStageSelectE3->mStage          = stage;
	}
}

// 0x80101A94
// The set hook run when the stage loads empty
extern "C" void initStageSelectE3Set()
{
}

// 0x80101A98
// The set hook run when the stage ends empty
extern "C" void termStageSelectE3Set()
{
}
