#include "ui/TAeObjConsole1P.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "ui/AeArc.h"
#include "egg/EggCreature.h"
#include "ui/AeUtil.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;

// TAeObjConsole1P class at 0x800CF6FC Vtable 0x8025A624
// The destructor removes the task at mTask

typedef char TAeObjConsole1P_size_check[(sizeof(TAeObjConsole1P) == 0xC4) ? 1 : -1];

// The pulse the gauge and the egg kind take when they run low
static const AeColor sPulseA = { 1.0f, 0.98f, 0.588f, 0.588f };
static const AeColor sPulseB = { 1.0f, 0.9f, 0.196f, 0.196f };
static const u32 sConsoleBanks[3]   = { 0x16, 0x17, 0x18 };                    // palette banks
static const u32 sCellSlots[5]      = { 0x111, 0x112, 0x113, 0x114, 0x115 };  // frame slots for the cells
static const u32 sStarSlots[5]      = { 0x174, 0x175, 0x176, 0x177, 0x178 };  // star frame slots
static const u32 sStarOn[4]         = { 0x12A, 0x130, 0x136, 0x13C };         // a star got by kind
static const u32 sStarOff[4]        = { 0x12C, 0x132, 0x138, 0x13E };         // a star lost by kind
static const u32 sKindIcons[4]      = { 2, 5, 9, 0xE };                         // the egg kind icon
static const u32 sKindTimers[4]     = { 0x3C, 0x42, 0x48, 0x4E };              // the hatched egg by kind
static const int sBannerCells[17]   = { 0xAB, 0xC6, 0xC9, 0xCC, 0xCF, 0xD2, 0xD5, 0xD8, 0xDB,
                                        0xAE, 0xB1, 0xB4, 0xB7, 0xBA, 0xBD, 0xC0, 0xC3 };
static const f32 sLevelTimes[4]     = { 0.23f, 0.47f, 0.7f, 0.93f };           // egg meter time per level

static const char* sConsole1PName = "TAeObjConsole1P";

// 0x800CF420
extern "C" void loadConsole1P()
{
	loadArc("ae_main_console.arc", &sConsole1PArc);
	if (sConsole1PArc != 0) {
		AeArcBlock* block = aeArcBlock(sConsole1PArc);
		sConsole1PAe      = block->mAe;
		relocateAe(sConsole1PAe);
		sConsole1PTexList = block->mTexList;
		bindGvmTextures((u8*)block->mGvm, sConsole1PTexList);
		u8* buf = new u8[0x800];
		if (block->mPalettes != 0) {
			loadE3Palette(block->mPalettes[0], (E3Palette*)&buf, sConsoleBanks[0]);
			loadE3Palette(block->mPalettes[1], (E3Palette*)&buf, sConsoleBanks[1]);
			loadE3Palette(block->mPalettes[2], (E3Palette*)&buf, sConsoleBanks[2]);
		}
		delete[] buf;
		njSetTexture(sConsole1PTexList);
		njSetPaletteBankNum(0, sConsoleBanks[0]);
		njSetPaletteBankNum(1, sConsoleBanks[1]);
		njSetPaletteBankNum(2, sConsoleBanks[2]);
		GXInvalidateTexAll();
	}
}

// 0x800CF538
extern "C" void freeConsole1P()
{
	if (sConsole1PArc != 0) {
		releaseTextures((NJS_TEXLIST*)sConsole1PTexList);
		sConsole1PTexList = 0;
		releaseResource(sConsole1PArc);
		sConsole1PArc = 0;
		sConsole1PAe  = 0;
	}
}

// 0x800CF584
extern "C" TAeObjConsole1P* createConsole1P(TObject* parent)
{
	TAeObjConsole1P* made;

	if (sConsole1PArc == 0) {
		return 0;
	}
	made = new TAeObjConsole1P(parent, sConsole1PAe, sConsole1PTexList);
	if (made != 0) {
		made->setupSolo();
	}
	return made;
}

// 0x800CF5FC
extern "C" TAeObjConsole1P* createConsole1PB(TObject* parent, int slot)
{
	TAeObjConsole1P* made;

	if (sConsole1PArc == 0) {
		return 0;
	}
	made = new TAeObjConsole1P(parent, sConsole1PAe, sConsole1PTexList);
	if (made != 0) {
		made->setupNumbered();
		made->mSlot = slot;
	}
	return made;
}

// 0x800CF67C
extern "C" TAeObjConsole1P* createConsole1PC(TObject* parent, int slot)
{
	TAeObjConsole1P* made;

	if (sConsole1PArc == 0) {
		return 0;
	}
	made = new TAeObjConsole1P(parent, sConsole1PAe, sConsole1PTexList);
	if (made != 0) {
		made->setupPlain();
		made->mSlot = slot;
	}
	return made;
}

// 0x800CF6FC
TAeObjConsole1P::TAeObjConsole1P(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	int i;

	_00    = (u32)sConsole1PName;
	mSize  = 0xC4;
	mFrame = 0;
	mOpen  = 0;
	for (i = 0; i < 5; i++) {
		mCells[i] = 0;
		mStars[i] = 0;
	}
	mEgg         = 0;
	mEggBase     = 0;
	mEggIcon     = 0;
	mEggKind     = 0;
	mEggTimer    = 0;
	mStarFrame   = 0;
	mScore       = 0;
	mBanner      = 0;
	mBannerBase  = 0;
	mBannerIcon  = 0;
	mFlags       = 0;
	mWasHatching = 0;
	mBlinkCell   = -1;
	mValue       = 0;
	mHatchTime   = 0.0f;
	mKind        = 0;
	mStarMax     = 0;
	mStarCount   = 0;
	mLevel       = 0;
	_AC          = 0;
	_B0          = 0;
	mPulse       = 0.0f;
	mPulseStep   = 0.1f;
	mMode        = 0;
	mSlot        = -1;
}

// 0x800CF7E8
TAeObjConsole1P::~TAeObjConsole1P() { removeDrawTask(mTask); }

// 0x800CF850
void TAeObjConsole1P::setupSolo()
{
	int i;

	mTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3D, 0);
	mFrame = createLayer(0x17F, 0);
	if (mFrame != 0) {
		mFrame->mBanks = sConsoleBanks;
	}
	mOpen = createLayer(0x10B, 0x10);
	if (mOpen != 0) {
		mOpen->mBanks = sConsoleBanks;
	}
	for (i = 0; i < 5; i++) {
		mCells[i] = createLayer(0xEC, 0);
		if (mCells[i] != 0) {
			mCells[i]->mBanks = sConsoleBanks;
		}
	}
	if (mFrame != 0 && mOpen != 0) {
		mFrame->bind(0x180, mOpen);
	}
	mEgg = createLayer(0x17A, 0);
	if (mEgg != 0) {
		mEgg->mBanks = sConsoleBanks;
	}
	mEggBase = createLayer(0x1E, 0);
	if (mEggBase != 0) {
		mEggBase->mBanks = sConsoleBanks;
	}
	mEggIcon = createLayer(0x18, 0);
	if (mEggIcon != 0) {
		mEggIcon->mBanks = sConsoleBanks;
	}
	mEggKind = createLayer(2, 0);
	if (mEggKind != 0) {
		mEggKind->mBanks = sConsoleBanks;
	}
	mEggTimer = createLayer(0x2E, 0x10);
	if (mEggTimer != 0) {
		mEggTimer->mBanks = sConsoleBanks;
	}
	if (mEggTimer != 0) {
		mEggTimer->setRate(0.2f);
		mEggTimer->_A8 = 0.0f;
		mEggTimer->_AC = 0;
	}
	if (mEgg != 0) {
		mEgg->attach(0x29, mEggBase, 0);
	}
	mScore = createLayer(0x184, 0x10);
	if (mScore != 0) {
		mScore->mBanks = sConsoleBanks;
	}
	mBanner = createLayer(0x17D, 4);
	if (mBanner != 0) {
		mBanner->mBanks = sConsoleBanks;
	}
	if (mBanner != 0) {
		mBanner->_A8 = 0.0f;
		mBanner->_AC = 0;
	}
	mBannerBase = createLayer(0x56, 4);
	if (mBannerBase != 0) {
		mBannerBase->mBanks = sConsoleBanks;
	}
	if (mBannerBase != 0) {
		mBannerBase->_A8 = 0.0f;
		mBannerBase->_AC = 0;
	}
	mBannerIcon = createLayer(0x5C, 4);
	if (mBannerIcon != 0) {
		mBannerIcon->mBanks = sConsoleBanks;
	}
	if (mBannerIcon != 0) {
		((TAeClip*)mBannerIcon)->pause();
	}
	if (mBanner != 0) {
		mBanner->attach(0xE5, mBannerBase, 0);
	}
	mMode = 0;
	_AC   = 0x3C;
}

// 0x800CFBDC
void TAeObjConsole1P::setupNumbered()
{
	int i;
	int j;

	mTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3C, 0);
	mFrame = createLayer(0x109, 0);
	if (mFrame != 0) {
		mFrame->mBanks = sConsoleBanks;
	}
	for (i = 0; i < 5; i++) {
		mCells[i] = createLayer(0x101, 0);
		if (mCells[i] != 0) {
			mCells[i]->mBanks = sConsoleBanks;
		}
		if (mFrame != 0 && mCells[i] != 0) {
			mFrame->bind(sCellSlots[i], mCells[i]);
		}
	}
	mEgg = createLayer(0x16, 0);
	if (mEgg != 0) {
		mEgg->mBanks = sConsoleBanks;
	}
	mEggBase = createLayer(0x23, 0);
	if (mEggBase != 0) {
		mEggBase->mBanks = sConsoleBanks;
	}
	mEggIcon = createLayer(0x18, 0);
	if (mEggIcon != 0) {
		mEggIcon->mBanks = sConsoleBanks;
	}
	mEggKind = createLayer(2, 0);
	if (mEggKind != 0) {
		mEggKind->mBanks = sConsoleBanks;
	}
	mEggTimer = createLayer(0x2E, 0x10);
	if (mEggTimer != 0) {
		mEggTimer->mBanks = sConsoleBanks;
	}
	if (mEggTimer != 0) {
		mEggTimer->setRate(0.2f);
		mEggTimer->_A8 = 0.0f;
		mEggTimer->_AC = 0;
	}
	if (mEgg != 0 && mEggBase != 0) {
		mEgg->attach(0x29, mEggBase, 0);
	}
	mBanner = createLayer(0x5A, 4);
	if (mBanner != 0) {
		mBanner->mBanks = sConsoleBanks;
	}
	if (mBanner != 0) {
		mBanner->_A8 = 0.0f;
		mBanner->_AC = 0;
		mBanner->hide();
	}
	mBannerBase = createLayer(0x56, 4);
	if (mBannerBase != 0) {
		mBannerBase->mBanks = sConsoleBanks;
	}
	if (mBannerBase != 0) {
		mBannerBase->_A8 = 0.0f;
		mBannerBase->_AC = 0;
	}
	mBannerIcon = createLayer(0x5C, 4);
	if (mBannerIcon != 0) {
		mBannerIcon->mBanks = sConsoleBanks;
	}
	if (mBannerIcon != 0) {
		((TAeClip*)mBannerIcon)->pause();
	}
	if (mBanner != 0) {
		mBanner->attach(0xE5, mBannerBase, 0);
	}
	mStarFrame = createLayer(0x172, 0x10);
	if (mStarFrame != 0) {
		mStarFrame->mBanks = sConsoleBanks;
	}
	if (mStarFrame != 0) {
		mStarFrame->cue(1);
	}
	for (j = 0; j < 5; j++) {
		mStars[j] = createLayer(0x12A, 0x10);
		if (mStars[j] != 0) {
			mStars[j]->mBanks = sConsoleBanks;
		}
	}
	mMode = 2;
}

// 0x800CFFAC
void TAeObjConsole1P::setupPlain()
{
	int i;
	int j;

	mTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3C, 0);
	mFrame = createLayer(0x107, 0);
	if (mFrame != 0) {
		mFrame->mBanks = sConsoleBanks;
	}
	for (i = 0; i < 5; i++) {
		mCells[i] = createLayer(0x101, 0);
		if (mCells[i] != 0) {
			mCells[i]->mBanks = sConsoleBanks;
		}
		if (mFrame != 0 && mCells[i] != 0) {
			mFrame->bind(sCellSlots[i], mCells[i]);
		}
	}
	mEgg = createLayer(0x14, 0);
	if (mEgg != 0) {
		mEgg->mBanks = sConsoleBanks;
	}
	mEggBase = createLayer(0x23, 0);
	if (mEggBase != 0) {
		mEggBase->mBanks = sConsoleBanks;
	}
	mEggIcon = createLayer(0x18, 0);
	if (mEggIcon != 0) {
		mEggIcon->mBanks = sConsoleBanks;
	}
	mEggKind = createLayer(2, 0);
	if (mEggKind != 0) {
		mEggKind->mBanks = sConsoleBanks;
	}
	mEggTimer = createLayer(0x2E, 0x10);
	if (mEggTimer != 0) {
		mEggTimer->mBanks = sConsoleBanks;
	}
	if (mEggTimer != 0) {
		mEggTimer->setRate(0.2f);
		mEggTimer->_A8 = 0.0f;
		mEggTimer->_AC = 0;
	}
	if (mEgg != 0 && mEggBase != 0) {
		mEgg->attach(0x29, mEggBase, 0);
	}
	mBanner = createLayer(0x58, 4);
	if (mBanner != 0) {
		mBanner->mBanks = sConsoleBanks;
	}
	if (mBanner != 0) {
		mBanner->_A8 = 0.0f;
		mBanner->_AC = 0;
		mBanner->hide();
	}
	mBannerBase = createLayer(0x56, 4);
	if (mBannerBase != 0) {
		mBannerBase->mBanks = sConsoleBanks;
	}
	if (mBannerBase != 0) {
		mBannerBase->_A8 = 0.0f;
		mBannerBase->_AC = 0;
	}
	mBannerIcon = createLayer(0x5C, 4);
	if (mBannerIcon != 0) {
		mBannerIcon->mBanks = sConsoleBanks;
	}
	if (mBannerIcon != 0) {
		((TAeClip*)mBannerIcon)->pause();
	}
	if (mBanner != 0) {
		mBanner->attach(0xE5, mBannerBase, 0);
	}
	mStarFrame = createLayer(0x170, 0x10);
	if (mStarFrame != 0) {
		mStarFrame->mBanks = sConsoleBanks;
	}
	if (mStarFrame != 0) {
		mStarFrame->cue(1);
	}
	for (j = 0; j < 5; j++) {
		mStars[j] = createLayer(0x12A, 0x10);
		if (mStars[j] != 0) {
			mStars[j]->mBanks = sConsoleBanks;
		}
	}
	mMode = 2;
}

// 0x800D037C
void TAeObjConsole1P::draw()
{
	switch (mMode) {
	case 0:
		stepOpen();
		break;
	case 1:
		stepMain();
		break;
	case 2:
		stepMain();
		placeBySlot();
		break;
	}
}

// 0x800D03E8
void TAeObjConsole1P::render()
{
	if (mFrame != 0) {
		mFrame->present();
	}
	if (mEgg != 0) {
		mEgg->present();
	}
	if (mStarFrame != 0) {
		mStarFrame->present();
	}
	if (mScore != 0) {
		mScore->present();
	}
	if (mBanner != 0) {
		mBanner->present();
	}
}

// 0x800D0460
// The opening finished the cells take its place in the frame
void TAeObjConsole1P::openDone()
{
	int i;

	mFrame->clearSlot(0x180);
	for (i = 0; i < 5; i++) {
		mFrame->bind(sCellSlots[i], mCells[i]);
	}
}

// 0x800D04E0
void TAeObjConsole1P::eggBaseDone()
{
	mEggBase->reset(0x23, 1);
	if (mEggBase != 0) {
		mEggBase->mBanks = sConsoleBanks;
	}
	if (mEgg != 0) {
		mEgg->_A8 = sLevelTimes[mLevel];
		mEgg->_AC = 0;
	}
}

// 0x800D0564
void TAeObjConsole1P::showHatch()
{
	mEggBase->reset(0x21, 1);
	if (mEggBase != 0) {
		mEggBase->mBanks = sConsoleBanks;
	}
	if (mHatchTime == 1.0f) {
		mEggTimer->reset(sKindTimers[mKind], 1);
		mEggTimer->mBanks = sConsoleBanks;
		if (mEggIcon->getId() != 0x1B) {
			mEggIcon->reset(0x1B, 1);
			mEggIcon->mBanks = sConsoleBanks;
		}
		mEgg->clearSlots();
		mEgg->attach(0x29, mEggBase, 0);
		mEgg->attach(0x2B, mEggIcon, 0);
		mEgg->attach(0x2A, mEggTimer, 0);
	} else {
		mEggTimer->reset(0x2E, 0x10);
		mEggTimer->mTime  = mHatchTime;
		mEggTimer->_AC    = 0;
		mEggTimer->mBanks = sConsoleBanks;
		if (mEggIcon->getId() != 0x18) {
			mEggIcon->reset(0x18, 1);
			mEggIcon->mBanks = sConsoleBanks;
		}
		mEgg->clearSlots();
		mEgg->attach(0x29, mEggBase, 0);
		mEgg->attach(0x2B, mEggIcon, 0);
		mEgg->attach(0x2C, mEggKind, 0);
		mEgg->attach(0x2A, mEggTimer, 0);
	}
	mEggKind->reset(sKindIcons[mKind], 1);
	if (mEggKind != 0) {
		mEggKind->mBanks = sConsoleBanks;
	}
}

// 0x800D07EC
void TAeObjConsole1P::hideHatch()
{
	mEggBase->reset(0x23, 1);
	if (mEggBase != 0) {
		mEggBase->mBanks = sConsoleBanks;
	}
	mEgg->clearSlots();
	mEgg->attach(0x29, mEggBase, 0);
}

// 0x800D0870
void TAeObjConsole1P::stepOpen()
{
	if (mFrame != 0) {
		mFrame->update();
	}
	if (mOpen != 0) {
		mOpen->update();
		if (mOpen->isDone()) {
			openDone();
			mMode = 1;
		}
	}
	if (mEgg != 0) {
		mEgg->update();
	}
	if (mEggBase != 0) {
		mEggBase->update();
		if (mEggBase->isDone()) {
			eggBaseDone();
		}
	}
}

// 0x800D0918
void TAeObjConsole1P::stepMain()
{
	int i;
	int j;
	AeColor cellColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	AeColor kindColor = { 1.0f, 1.0f, 1.0f, 1.0f };

	if (mValue != 0 && (mValue <= 20 || mKind > 1)) {
		pulse(0.1f);
		if (mValue <= 20) {
			lerp4f(mPulse, &cellColor.a, &sPulseB.a, &sPulseA.a);
		}
		if (mKind > 1) {
			lerp4f(mPulse, &kindColor.a, &sPulseB.a, &sPulseA.a);
		}
	}
	if (mFrame != 0) {
		mFrame->update();
	}
	for (i = 0; i < 5; i++) {
		if (mCells[i] != 0) {
			mCells[i]->update();
			mCells[i]->setColor(&cellColor.a);
		}
	}
	if ((mFlags & 1) && mCells[mBlinkCell]->isDone()) {
		mFlags &= ~1;
		if (mCells[mBlinkCell]->getId() == 0xF0) {
			consoleSetGauge(mCells, mValue);
			mCells[mBlinkCell]->reset(0xF6, 1);
		} else {
			consoleSetGauge(mCells, mValue);
			mCells[mBlinkCell]->reset(0x101, 1);
		}
		for (i = 0; i < 5; i++) {
			if (mCells[i] != 0) {
				mCells[i]->mBanks = sConsoleBanks;
			}
		}
	}
	if (mEgg != 0) {
		mEgg->_A8 = sLevelTimes[mLevel];
		if (mEgg->play()) {
			if (mFlags & 0x40) {
				mFlags &= ~0x40;
				if (mLevel == 3) {
					mEgg->mTime = 0.7f;
				}
			} else {
				mLevel = 0;
				mEgg->cue(0);
			}
		}
	}
	if (mEggBase != 0) {
		mEggBase->update();
	}
	if (mEggIcon != 0) {
		mEggIcon->update();
	}
	if (mEggKind != 0) {
		mEggKind->update();
		if (mKind > 1) {
			mEggKind->setColor(&kindColor.a);
		}
	}
	if (mWasHatching != 0) {
		if (!(mFlags & 2)) {
			hideHatch();
		}
	} else if (mFlags & 2) {
		showHatch();
	}
	if (mEggTimer != 0) {
		if (mEggTimer->getId() == 0x2E) {
			mEggTimer->_A8 = mHatchTime;
			mEggTimer->play();
			if (mEggTimer->isDone()) {
				mEggTimer->reset(sKindTimers[mKind], 1);
				if (mEggTimer != 0) {
					mEggTimer->mBanks = sConsoleBanks;
				}
				if (mEggIcon->getId() != 0x1B) {
					mEggIcon->reset(0x1B, 1);
					mEggIcon->mBanks = sConsoleBanks;
				}
				if (mEgg != 0) {
					mEgg->clearSlot(0x2C);
				}
			}
		} else {
			mEggTimer->update();
		}
	}
	if (mFlags & 2) {
		mWasHatching = 1;
	} else {
		mWasHatching = 0;
	}
	mFlags &= ~2;
	if (mStarFrame != 0) {
		mStarFrame->update();
	}
	for (j = 0; j < 5; j++) {
		if (mStars[j] != 0) {
			mStars[j]->update();
		}
	}
	mFlags &= ~4;
	if (mScore != 0) {
		mScore->update();
	}
	if (mBanner != 0) {
		mBanner->play();
	}
	if (mBannerBase != 0) {
		mBannerBase->play();
	}
	if (mBannerIcon != 0) {
		mBannerIcon->update();
	}
}

// 0x800D0DB0
void TAeObjConsole1P::placeBySlot()
{
	switch (mSlot) {
	case -1:
		break;
	case 0:
		place(15.0f, 21.0f);
		break;
	case 1:
		place(15.0f, 250.0f);
		break;
	case 2:
		place(5.0f, 16.0f);
		break;
	case 3:
		place(330.0f, 16.0f);
		break;
	case 4:
		place(5.0f, 245.0f);
		break;
	case 5:
		place(330.0f, 245.0f);
		break;
	}
}

// 0x800D0E50
void TAeObjConsole1P::pulse(f32 step)
{
	if (mPulseStep > 0.0f) {
		if (mPulse >= 1.0f) {
			mPulse     = 1.0f;
			mPulseStep = -step;
		} else {
			mPulse += mPulseStep;
		}
	} else {
		if (mPulse <= 0.0f) {
			mPulse     = 0.0f;
			mPulseStep = step;
		} else {
			mPulse += mPulseStep;
		}
	}
}

// 0x800D0EB8
void TAeObjConsole1P::place(f32 x, f32 y)
{
	if (mFrame != 0) {
		mFrame->setTrans(x, y);
	}
	if (mEgg != 0) {
		mEgg->setTrans(x, y);
	}
	if (mStarFrame != 0) {
		mStarFrame->setTrans(x, y);
	}
	if (mBanner != 0) {
		mBanner->setTrans(x, y);
	}
}

// 0x800D0F60
void TAeObjConsole1P::setValue(int value)
{
	int i;

	if (!(mFlags & 1)) {
		mValue = value;
		consoleSetGauge(mCells, mValue);
		for (i = 0; i < 5; i++) {
			if (mCells[i] != 0) {
				mCells[i]->mBanks = sConsoleBanks;
			}
		}
	}
}

// 0x800D0FD4
void TAeObjConsole1P::setHatch(f32 time, s8 kind)
{
	mHatchTime = time;
	mKind      = kind < 0 ? 0 : (kind > 3 ? 3 : kind);
	mFlags |= 2;
}

// 0x800D1010
void TAeObjConsole1P::setStars(u16 count, u16 kind)
{
	int i;

	if (mStarFrame != 0) {
		if (mStarMax < count) {
			mStarMax = count;
		}
		mStarCount = count;
		u16 k      = kind > 3 ? 0 : kind;
		if (mStarFrame->getId() != 0x182) {
			for (i = 0; i < mStarMax; i++) {
				if (i < mStarCount) {
					if (mStars[i] != 0) {
						mStars[i]->reset(sStarOn[k], 0x10);
						mStars[i]->mBanks = sConsoleBanks;
						mStarFrame->attach(sStarSlots[i], mStars[i], 0);
					}
				} else if (mStars[i] != 0 && mStars[i]->getId() == sStarOn[k]) {
					mStars[i]->reset(sStarOff[k], 0x10);
					mStars[i]->mBanks = sConsoleBanks;
					mStarFrame->attach(sStarSlots[i], mStars[i], 0);
				}
			}
		}
	}
}

// 0x800D118C
void TAeObjConsole1P::hitValue(int value)
{
	mValue     = value;
	mBlinkCell = value / 20;
	consoleSetBlink(mCells[mBlinkCell], value);
	if (mCells[mBlinkCell] != 0) {
		mCells[mBlinkCell]->mBanks = sConsoleBanks;
	}
	mFlags |= 1;
}

// 0x800D1208
void TAeObjConsole1P::raiseLevel()
{
	mLevel++;
	mLevel = mLevel > 3 ? 3 : mLevel;
	mFlags |= 0x40;
}

// 0x800D123C
void TAeObjConsole1P::setScore(int a, int b, int tenths)
{
	if (mScore != 0) {
		aeClipSetDigits2(mScore, a, 0x46, 0x14F, 0x150);
		aeClipSetDigits2(mScore, b, 0x46, 0x152, 0x153);
		aeClipSetDigits2(mScore, (int)(0.1f * tenths), 0x46, 0x155, 0x156);
		mScore->setFrame(0x151, 0x51);
		mScore->setFrame(0x154, 0x52);
	}
}

// 0x800D130C
void TAeObjConsole1P::showBanner(int which)
{
	f32 target = 2.4f;
	if (which == -1) {
		target = 0.0f;
	}
	if (mBanner != 0) {
		mBanner->mPlayFlags &= ~0x500;
		mBanner->cue(0);
		mBanner->_A8 = target;
		mBanner->_AC = 1;
		mBanner->show();
	}
	if (mBannerBase != 0) {
		mBannerBase->mPlayFlags &= ~0x500;
		mBannerBase->cue(0);
		mBannerBase->_A8 = target;
		mBannerBase->_AC = 1;
		mBannerBase->show();
	}
	if (which != -1 && mBanner != 0) {
		mBanner->setCell(0xE6, sBannerCells[which]);
		mBanner->setCell(0xE7, sBannerCells[which]);
		if (mBannerIcon != 0) {
			mBannerIcon->reset(0x5C, 4);
			mBannerIcon->mBanks = sConsoleBanks;
			mBanner->attach(0xE8, mBannerIcon, 0);
		}
	}
}

// 0x800D1484
void TAeObjConsole1P::hideBanner()
{
	if (mBanner != 0) {
		mBanner->mPlayFlags &= ~0x500;
		mBanner->_A8 = 5.0f;
	}
	if (mBannerBase != 0) {
		mBannerBase->mPlayFlags &= ~0x500;
		mBannerBase->_A8 = 5.0f;
	}
	if (mBanner != 0 && mBannerIcon != 0) {
		mBannerIcon->reset(0x62, 4);
		mBannerIcon->mBanks = sConsoleBanks;
		mBanner->attach(0xE8, mBannerIcon, 0);
	}
}
