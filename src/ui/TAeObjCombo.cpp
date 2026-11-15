#include "ui/TAeObjCombo.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "ui/AeArc.h"
#include "egg/EggCreature.h"
#include "ui/AeObjControl.h"
#include "sound/SoundCtrl.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;

// TAeObjCombo class at 0x801CA93C Vtable 0x8026A690
// The loader is 0x801CA7B8 The destructor calls 0x800FF820

extern "C" {
extern const u32 sAeComboPalettes[2] = {
	0x0000001D,
	0x0000001E,
}; // palette banks
}

typedef char TAeObjCombo_size_check[(sizeof(TAeObjCombo) == 0x58) ? 1 : -1];

// 0x801CA7B8
extern "C" void loadCombo()
{
	loadArc(sAeComboArcName, &sComboArc);
	if (sComboArc != 0) {
		AeArcBlock* block = aeArcBlock(sComboArc);
		sComboAe          = block->mAe;
		relocateAe(sComboAe);
		sComboTexList = block->mTexList;
		bindGvmTextures((u8*)block->mGvm, sComboTexList);
		u8* buf = new u8[0x800];
		if (block->mPalettes != 0) {
			loadE3Palette(block->mPalettes[0], (E3Palette*)&buf, sAeComboPalettes[0]);
			loadE3Palette(block->mPalettes[1], (E3Palette*)&buf, sAeComboPalettes[1]);
		}
		delete[] buf;
		njSetTexture(sComboTexList);
		njSetPaletteBankNum(0, sAeComboPalettes[0]);
		njSetPaletteBankNum(1, sAeComboPalettes[1]);
		GXInvalidateTexAll();
	}
}

// 0x801CA888
extern "C" void freeCombo()
{
	if (sComboArc != 0) {
		releaseTextures((NJS_TEXLIST*)sComboTexList);
		sComboTexList = 0;
		releaseResource(sComboArc);
		sComboArc = 0;
		sComboAe  = 0;
	}
}

// 0x801CA8D4
extern "C" TAeObjCombo* createCombo(TObject* parent)
{
	TAeObjCombo* made;

	if (sComboArc == 0) {
		return 0;
	}
	made = new TAeObjCombo(parent, sComboAe, sComboTexList);
	return made;
}

extern const char* gTAeObjComboName;

// 0x801CA93C
TAeObjCombo::TAeObjCombo(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	int i;

	_00    = (u32)gTAeObjComboName;
	mSize  = 0x58;
	mTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3D, 0);
	mFrame = createLayer(0x85, 0);
	if (mFrame != 0) {
		mFrame->mBanks = sAeComboPalettes;
	}
	mRank = createLayer(0x4C, 4);
	if (mRank != 0) {
		mRank->mBanks = sAeComboPalettes;
	}
	mWord = createLayer(0x5A, 4);
	if (mWord != 0) {
		mWord->mBanks = sAeComboPalettes;
	}
	for (i = 0; i < 3; i++) {
		mCounts[i] = createLayer(0x58, 4);
		if (mCounts[i] != 0) {
			mCounts[i]->mBanks = sAeComboPalettes;
		}
	}
	mNextCount = 0;
	mFrameTime = 0;
}

// 0x801CAA6C
TAeObjCombo::~TAeObjCombo()
{
	removeDrawTask(mTask);
	aeForgetCombo(this);
}

// 0x801CAADC
void TAeObjCombo::draw()
{
	int i;

	if (mFrame != 0 && mFrame->play() != 0) {
		mFrameTime = mFrameTime + 1;
		if (mFrameTime == 0x1E) {
			mFrame->_A8 = 1.65f;
			mFrameTime  = 0;
		}
	}
	if (mRank != 0) {
		mRank->update();
	}
	if (mWord != 0) {
		mWord->update();
	}
	for (i = 0; i < 3; i++) {
		if (mCounts[i] != 0) {
			mCounts[i]->play();
		}
	}
}

// 0x801CAB94
void TAeObjCombo::render()
{
	int i;

	if (mFrame != 0) {
		mFrame->present();
	}
	for (i = mNextCount; i < 3; i++) {
		if (mCounts[i] != 0) {
			mCounts[i]->present();
		}
	}
	for (i = 0; i < mNextCount; i++) {
		if (mCounts[i] != 0) {
			mCounts[i]->present();
		}
	}
}

static inline int clamp(int x, int lo, int hi)
{
	return x < lo ? lo : (x > hi ? hi : x);
}

// The clips and sounds showRank picks from at 0x8024E2D8
static const u32 sRankAnims[5]  = { 0x4C, 0x4C, 0x4E, 0x51, 0x55 };                         // by rank 1 to 4
static const u32 sWordAnims[9]  = { 0x7D, 0x5C, 0x5A, 0x7F, 0x5E, 0x60, 0x68, 0x79, 0x6F }; // by word
static const u32 sRankSounds[4] = { 0x3D, 0x3E, 0x3F, 0x3F };                               // by rank from 1

// 0x801CAC30
void TAeObjCombo::showRank(int rank, int word)
{
	rank = clamp(rank, 1, 4);
	word = clamp(word, 0, 9);
	if (mRank != 0) {
		if (word >= 5) {
			rank = 4;
		}
		mRank->reset(sRankAnims[rank], 4);
		mRank->mBanks = sAeComboPalettes;
	}
	if (mWord != 0) {
		mWord->reset(sWordAnims[word], 4);
		mWord->mBanks = sAeComboPalettes;
	}
	if (mFrame != 0) {
		mFrame->clearSlots();
		mFrame->attach(0x85, mRank, 0);
		mFrame->attach(0x86, mWord, 0);
		mFrame->cue(0);
		mFrame->_A8 = 1.3f;
		mFrame->_AC = 1;
	}
	if (word != 4) {
		sePlay(sRankSounds[rank - 1]);
	}
}

// 0x801CADB4
void TAeObjCombo::showCount(int count)
{
	int i;
	int digits[2];
	int* d = digits;

	count = clamp(count, 0, 99);
	if (mCounts[mNextCount] != 0) {
		mCounts[mNextCount]->clearSlots();
		d[1] = count / 10;
		d[0] = count % 10;
		if (d[1] == 0) {
			mCounts[mNextCount]->setFrame(0x83, d[0] + 0xE);
		} else {
			mCounts[mNextCount]->setFrame(0x82, d[0] + 0x18);
			mCounts[mNextCount]->setFrame(0x81, d[1] + 0x18);
		}
	}
	for (i = 0; i < 3; i++) {
		if (mCounts[i] != 0) {
			if (i == mNextCount) {
				mCounts[i]->cue(0);
				mCounts[i]->show();
				mCounts[i]->_A8 = 0.35f;
			} else {
				mCounts[i]->_A8 = 1.0f;
			}
		}
	}
	mNextCount++;
	if (mNextCount == 3) {
		mNextCount = 0;
	}
	sePlay(0x31);
}

// 0x801CAF4C
void TAeObjCombo::fadeCounts()
{
	int i;

	for (i = 0; i < 3; i++) {
		if (mCounts[i] != 0) {
			mCounts[i]->_A8 = 1.0f;
		}
	}
}

const char* gTAeObjComboName = "TAeObjCombo";
