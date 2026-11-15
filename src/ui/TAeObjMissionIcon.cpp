#include "ui/TAeObjMissionIcon.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "ui/AeArc.h"
#include "egg/EggCreature.h"
#include "ui/AeObjControl.h"
#include "ui/AeUtil.h"
#include "sound/SoundCtrl.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;

// TAeObjMissionIcon class at 0x800FD47C Vtable 0x8025CB4C
// The kind switch calls the nine setups
// The destructor calls 0x800FF764
// vtable+0x24 through the repeated 0x80100430 land inside aeSendMessage
// Kinds 0-2 count down marks 3 4 and 6 show a three-digit number 6 and 8 show a time
// Kind 8 is a gauge with three levels

extern "C" {

int missionIconSetDigits3(TAeObjMissionIcon* icon, u32 hundreds, u32 tens, u32 ones, int value, const int* cells);
}

typedef char TAeObjMissionIcon_size_check[(sizeof(TAeObjMissionIcon) == 0x7C) ? 1 : -1];

// The gauge's colours it pulses between the first two for the first second then shows the fourth for a second then the
// third
static const AeColor sGaugePulseA     = { 1.0f, 0.98f, 0.588f, 0.588f };
static const AeColor sGaugePulseB     = { 1.0f, 0.9f, 0.196f, 0.196f };
static const AeColor sGaugeLate       = { 1.0f, 0.9f, 0.9f, 0.0f };
static const AeColor sGaugeMid        = { 1.0f, 0.7843f, 0.3137f, 0.3137f };
static const u32 sMissionIconBanks[3] = { 0xFFFFFFFF, 0x14, 0x15 };     // palette banks
static const u32 sMarkSlots[4]        = { 0x101, 0x100, 0xFF, 0xFE };   // frame slots for the marks
static const u32 sFillSlots[4]        = { 0x106, 0x105, 0x104, 0x103 }; // frame slots for the fills
static const int sBigDigitCells[10]   = { 0xB0, 0xB3, 0xB6, 0xB9, 0xBC, 0xBF, 0xC2, 0xC5, 0xC8, 0xCB };
static const int sSmallDigitCells[10] = { 0xD1, 0xD4, 0xD7, 0xDA, 0xDD, 0xE0, 0xE3, 0xE6, 0xE9, 0xEC };
static const char* sMissionIconName   = "TAeObjMissionIcon";

// 0x800FD2CC
extern "C" void loadMissionIcon()
{
	sMissionIconGauge = 0;
	loadArc("ae_mission_icon.arc", &sMissionIconArc);
	if (sMissionIconArc != 0) {
		AeArcBlock* block = aeArcBlock(sMissionIconArc);
		sMissionIconAe    = block->mAe;
		relocateAe(sMissionIconAe);
		sMissionIconTexList = block->mTexList;
		bindGvmTextures((u8*)block->mGvm, sMissionIconTexList);
		u8* buf = new u8[0x800];
		if (block->mPalettes != 0) {
			loadE3Palette(block->mPalettes[0], (E3Palette*)&buf, sMissionIconBanks[1]);
			loadE3Palette(block->mPalettes[1], (E3Palette*)&buf, sMissionIconBanks[2]);
		}
		delete[] buf;
		njSetTexture(sMissionIconTexList);
		njSetPaletteBankNum(1, sMissionIconBanks[1]);
		njSetPaletteBankNum(2, sMissionIconBanks[2]);
		GXInvalidateTexAll();
	}
}

// 0x800FD3BC
extern "C" void freeMissionIcon()
{
	if (sMissionIconArc != 0) {
		releaseTextures((NJS_TEXLIST*)sMissionIconTexList);
		sMissionIconTexList = 0;
		releaseResource(sMissionIconArc);
		sMissionIconArc = 0;
		sMissionIconAe  = 0;
	}
}

// 0x800FD408
extern "C" TAeObjMissionIcon* createMissionIcon(u32 kind)
{
	TAeObjMissionIcon* made;

	if (sMissionIconArc == 0) {
		return 0;
	}
	made = new TAeObjMissionIcon(gTL_07, sMissionIconAe, sMissionIconTexList, kind);
	aeSetMissionIcon(made);
	return made;
}

// 0x800FD47C
TAeObjMissionIcon::TAeObjMissionIcon(TObject* parent, void* data, void* assets, u32 kind)
    : TAeObject(parent, data, assets)
{
	int i;

	mKind  = kind;
	_00    = (u32)sMissionIconName;
	mSize  = 0x7C;
	_5C    = 0;
	mLayer = 0;
	for (i = 0; i < 4; i++) {
		mMarks[i] = 0;
		mFills[i] = 0;
	}
	switch (mKind) {
	case 0:
		missionIconSetup0(this);
		break;
	case 1:
		missionIconSetup1(this);
		break;
	case 2:
		missionIconSetup2(this);
		break;
	case 3:
		missionIconSetup3(this);
		break;
	case 4:
		missionIconSetup4(this);
		break;
	case 5:
		missionIconSetup5(this);
		break;
	case 6:
		missionIconSetup6(this);
		break;
	case 7:
		missionIconSetup7(this);
		break;
	case 8:
		missionIconSetup8(this);
		break;
	}
	mTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3D, 0);
	mSound = -1;
}

// 0x800FD5B0
TAeObjMissionIcon::~TAeObjMissionIcon()
{
	removeDrawTask(mTask);
	aeForgetMissionIcon(this);
}

// 0x800FD620
void TAeObjMissionIcon::draw()
{
	switch (mKind) {
	case 3:
		missionIconStepCounter(this);
		break;
	case 8:
		missionIconStepGauge(this);
		break;
	default:
		missionIconStep(this);
		break;
	}
}

// 0x800FD66C
void TAeObjMissionIcon::render()
{
	if (mLayer != 0) {
		mLayer->present();
	}
}

// 0x800FD698
extern "C" void missionIconSetup0(TAeObjMissionIcon* icon)
{
	int i;

	icon->mFrame     = 0;
	icon->mPulse     = 0.0f;
	icon->mPulseStep = 0.1f;
	icon->mCount     = 3;
	icon->mLayer     = icon->createLayer(0x110, 0);
	if (icon->mLayer != 0) {
		icon->mLayer->mBanks = sMissionIconBanks;
	}
	for (i = 0; i < 3; i++) {
		icon->mMarks[i] = icon->createLayer(0xA3, 0);
		if (icon->mMarks[i] != 0) {
			icon->mMarks[i]->mBanks = sMissionIconBanks;
		}
		icon->mFills[i] = icon->createLayer(0xAC, 0x10);
		if (icon->mFills[i] != 0) {
			icon->mFills[i]->mBanks = sMissionIconBanks;
		}
		if (icon->mFills[i] != 0) {
			((TAeClip*)icon->mFills[i])->pause();
			icon->mFills[i]->hide();
		}
	}
	missionIconBindMarks(icon, 3);
}

// 0x800FD7AC
extern "C" void missionIconSetup1(TAeObjMissionIcon* icon)
{
	int i;

	icon->mFrame     = 0;
	icon->mPulse     = 0.0f;
	icon->mPulseStep = 0.1f;
	icon->mCount     = 3;
	icon->mLayer     = icon->createLayer(0x110, 0);
	if (icon->mLayer != 0) {
		icon->mLayer->mBanks = sMissionIconBanks;
	}
	for (i = 0; i < 3; i++) {
		icon->mMarks[i] = icon->createLayer(0x95, 0);
		if (icon->mMarks[i] != 0) {
			icon->mMarks[i]->mBanks = sMissionIconBanks;
		}
		icon->mFills[i] = icon->createLayer(0x9C, 0);
		if (icon->mFills[i] != 0) {
			icon->mFills[i]->mBanks = sMissionIconBanks;
		}
	}
	missionIconBindMarks(icon, 3);
	for (i = 0; i < 3; i++) {
		if (icon->mFills[i] != 0) {
			icon->mLayer->bind(sFillSlots[i], icon->mFills[i]);
		}
	}
}

// 0x800FD8E8
extern "C" void missionIconSetup2(TAeObjMissionIcon* icon)
{
	int i;

	icon->mFrame     = 0;
	icon->mPulse     = 0.0f;
	icon->mPulseStep = 0.1f;
	icon->mCount     = 4;
	icon->mLayer     = icon->createLayer(0x110, 0);
	if (icon->mLayer != 0) {
		icon->mLayer->mBanks = sMissionIconBanks;
	}
	for (i = 0; i < 4; i++) {
		icon->mMarks[i] = icon->createLayer(0x89, 0);
		if (icon->mMarks[i] != 0) {
			icon->mMarks[i]->mBanks = sMissionIconBanks;
		}
		icon->mFills[i] = icon->createLayer(0x93, 0);
		if (icon->mFills[i] != 0) {
			icon->mFills[i]->mBanks = sMissionIconBanks;
		}
		if (icon->mFills[i] != 0) {
			((TAeClip*)icon->mFills[i])->pause();
			icon->mFills[i]->hide();
		}
	}
	missionIconBindMarks(icon, 4);
}

// 0x800FD9FC
extern "C" void missionIconSetup3(TAeObjMissionIcon* icon)
{
	icon->mFrame     = 0;
	icon->mPulse     = 0.0f;
	icon->mPulseStep = 0.1f;
	icon->mCount     = 100;
	icon->mLayer     = icon->createLayer(0x110, 0);
	if (icon->mLayer != 0) {
		icon->mLayer->mBanks = sMissionIconBanks;
	}
	icon->mMarks[0] = icon->createLayer(0x77, 0);
	if (icon->mMarks[0] != 0) {
		icon->mMarks[0]->mBanks = sMissionIconBanks;
	}
	missionIconBindCounter(icon);
}

// 0x800FDAA0
extern "C" void missionIconSetup4(TAeObjMissionIcon* icon)
{
	icon->mFrame     = 0;
	icon->mPulse     = 0.0f;
	icon->mPulseStep = 0.1f;
	icon->mCount     = 8;
	icon->mLayer     = icon->createLayer(0x110, 0);
	if (icon->mLayer != 0) {
		icon->mLayer->mBanks = sMissionIconBanks;
	}
	icon->mMarks[0] = icon->createLayer(0x4A, 0);
	if (icon->mMarks[0] != 0) {
		icon->mMarks[0]->mBanks = sMissionIconBanks;
	}
	icon->mFills[0] = icon->createLayer(0x35, 0);
	if (icon->mFills[0] != 0) {
		icon->mFills[0]->mBanks = sMissionIconBanks;
	}
	missionIconBindCounter(icon);
	if (icon->mLayer != 0 && icon->mFills[0] != 0) {
		icon->mLayer->bind(0xF7, icon->mFills[0]);
	}
}

// 0x800FDBA0
extern "C" void missionIconSetup5(TAeObjMissionIcon* icon)
{
	icon->mFrame     = 0;
	icon->mPulse     = 0.0f;
	icon->mPulseStep = 0.1f;
	icon->mCount     = 0;
	icon->mLayer     = icon->createLayer(0x110, 0);
	if (icon->mLayer != 0) {
		icon->mLayer->mBanks = sMissionIconBanks;
	}
}

// 0x800FDC0C
extern "C" void missionIconSetup6(TAeObjMissionIcon* icon)
{
	icon->mFrame     = 0;
	icon->mPulse     = 0.0f;
	icon->mPulseStep = 0.1f;
	icon->mCount     = 0;
	icon->mLayer     = icon->createLayer(0x110, 0);
	if (icon->mLayer != 0) {
		icon->mLayer->mBanks = sMissionIconBanks;
	}
	icon->mMarks[0] = icon->createLayer(0x4D, 0);
	if (icon->mMarks[0] != 0) {
		icon->mMarks[0]->mBanks = sMissionIconBanks;
	}
	icon->mFills[0] = icon->createLayer(0x50, 0x10);
	if (icon->mFills[0] != 0) {
		icon->mFills[0]->mBanks = sMissionIconBanks;
	}
	if (icon->mFills[0] != 0) {
		((TAeClip*)icon->mFills[0])->pause();
	}
	missionIconBindCounter(icon);
	if (icon->mLayer != 0) {
		if (icon->mFills[0] != 0) {
			icon->mLayer->bind(0x102, icon->mFills[0]);
		}
		icon->mLayer->setCell(0x108, 0xEF);
	}
}

// 0x800FDD34
extern "C" void missionIconSetup7(TAeObjMissionIcon* icon)
{
	icon->mFrame     = 0;
	icon->mPulse     = 0.0f;
	icon->mPulseStep = 0.1f;
	icon->mCount     = 0;
	icon->mLayer     = icon->createLayer(0x110, 0);
	if (icon->mLayer != 0) {
		icon->mLayer->mBanks = sMissionIconBanks;
	}
}

// 0x800FDDA0
extern "C" void missionIconSetup8(TAeObjMissionIcon* icon)
{
	f32 color[4];

	icon->mFrame     = 0;
	icon->mPulse     = 0.0f;
	icon->mPulseStep = 0.1f;
	icon->mCount     = 0;
	icon->mLayer     = icon->createLayer(0x110, 0);
	if (icon->mLayer != 0) {
		icon->mLayer->mBanks = sMissionIconBanks;
	}
	icon->mMarks[0] = icon->createLayer(0x30, 0);
	if (icon->mMarks[0] != 0) {
		icon->mMarks[0]->mBanks = sMissionIconBanks;
	}
	icon->mMarks[1] = icon->createLayer(0, 0);
	if (icon->mMarks[1] != 0) {
		icon->mMarks[1]->mBanks = sMissionIconBanks;
	}
	icon->mMarks[1]->setRate(0.05f);
	icon->mMarks[1]->_A8 = 0.0f;
	icon->mMarks[1]->_AC = 0;
	icon->mMarks[2]      = icon->createLayer(9, 0);
	if (icon->mMarks[2] != 0) {
		icon->mMarks[2]->mBanks = sMissionIconBanks;
	}
	icon->mMarks[2]->setRate(0.05f);
	icon->mMarks[2]->_A8 = 0.0f;
	icon->mMarks[2]->_AC = 0;
	icon->mMarks[3]      = icon->createLayer(0xF, 0);
	if (icon->mMarks[3] != 0) {
		icon->mMarks[3]->mBanks = sMissionIconBanks;
	}
	icon->mMarks[3]->setRate(0.05f);
	icon->mMarks[3]->_A8 = 0.0f;
	icon->mMarks[3]->_AC = 0;
	color[0]             = 1.0f;
	color[1] = color[2] = color[3] = 0.0f;
	icon->mMarks[3]->setColor(color);
	icon->mFills[0] = icon->createLayer(0x35, 0);
	if (icon->mFills[0] != 0) {
		icon->mFills[0]->mBanks = sMissionIconBanks;
	}
	missionIconSetGaugeLevel(icon, sMissionIconGauge);
	icon->mLayer->bind(0xF8, icon->mMarks[0]);
	icon->mLayer->bind(0xF6, icon->mMarks[1]);
	icon->mLayer->bind(0xF5, icon->mMarks[2]);
	icon->mLayer->bind(0xF4, icon->mMarks[3]);
	icon->mLayer->bind(0xF7, icon->mFills[0]);
	icon->mLayer->setCell(0xFA, 0xEF);
	icon->mLayer->setRate(0.05f);
	icon->mLayer->_A8 = 0.0f;
	icon->mLayer->_AC = 0;
	icon->mSound      = sePlay(0x28);
}

// 0x800FE08C
extern "C" void missionIconStep(TAeObjMissionIcon* icon)
{
	int i;

	if (icon->mLayer != 0) {
		icon->mLayer->update();
	}
	for (i = 0; i < 4; i++) {
		if (icon->mMarks[i] != 0) {
			icon->mMarks[i]->update();
		}
		if (icon->mFills[i] != 0) {
			icon->mFills[i]->update();
		}
	}
}

// 0x800FE100
extern "C" void missionIconStepCounter(TAeObjMissionIcon* icon)
{
	if (icon->mLayer != 0) {
		icon->mLayer->update();
	}
	if (icon->mMarks[0] != 0) {
		icon->mMarks[0]->update();
		if (icon->mMarks[0]->getId() == 0x7A && icon->mMarks[0]->isDone()) {
			icon->mMarks[0]->reset(0x77, 1);
			icon->mMarks[0]->mBanks = sMissionIconBanks;
		}
	}
}

// 0x800FE194
extern "C" void missionIconStepGauge(TAeObjMissionIcon* icon)
{
	if (icon->mFrame == 0) {
		seStop(icon->mSound);
		sePlay(0x29);
	}
	AeColor color = { 1.0f, 1.0f, 1.0f, 1.0f };
	if (icon->mFrame < 60) {
		missionIconPulse(icon, 0.1f);
		lerp4f(icon->mPulse, &color.a, &sGaugePulseB.a, &sGaugePulseA.a);
	} else if (icon->mFrame < 120) {
		color.a = sGaugeMid.a;
		color.r = sGaugeMid.r;
		color.g = sGaugeMid.g;
		color.b = sGaugeMid.b;
	} else if (icon->mFrame < 180) {
		color.a = sGaugeLate.a;
		color.r = sGaugeLate.r;
		color.g = sGaugeLate.g;
		color.b = sGaugeLate.b;
	}
	if (icon->mLayer != 0) {
		icon->mLayer->setColor(&color.a);
		icon->mLayer->play();
	}
	if (icon->mMarks[0] != 0) {
		icon->mMarks[0]->update();
	}
	if (icon->mMarks[1] != 0) {
		icon->mMarks[1]->play();
	}
	if (icon->mMarks[2] != 0) {
		icon->mMarks[2]->setColor(&color.a);
		icon->mMarks[2]->play();
	}
	if (icon->mMarks[3] != 0) {
		icon->mMarks[3]->play();
	}
	if (icon->mFills[0] != 0) {
		icon->mFills[0]->update();
		if (icon->mFrame == 200) {
			icon->mFills[0]->reset(0x37, 1);
			if (icon->mFills[0] != 0) {
				icon->mFills[0]->mBanks = sMissionIconBanks;
			}
		} else if (icon->mFrame == 100) {
			icon->mFills[0]->reset(0x39, 1);
			if (icon->mFills[0] != 0) {
				icon->mFills[0]->mBanks = sMissionIconBanks;
			}
		}
	}
}

// 0x800FE374
extern "C" void missionIconBindMarks(TAeObjMissionIcon* icon, int count)
{
	int i;
	int n;

	if (icon->mLayer != 0) {
		n = count < 0 ? 0 : (count > 4 ? 4 : count);
		for (i = 0; i < n; i++) {
			icon->mLayer->bind(sMarkSlots[i], icon->mMarks[i]);
		}
	}
}

// 0x800FE414
extern "C" void missionIconBindCounter(TAeObjMissionIcon* icon)
{
	if (icon->mLayer != 0) {
		icon->mLayer->bind(0xFD, icon->mMarks[0]);
		icon->mLayer->setCell(0x10B, 0xCE);
	}
}

// 0x800FE47C
extern "C" void missionIconPulse(TAeObjMissionIcon* icon, f32 step)
{
	if (icon->mPulseStep > 0.0f) {
		if (icon->mPulse >= 1.0f) {
			icon->mPulse     = 1.0f;
			icon->mPulseStep = -step;
		} else {
			icon->mPulse += icon->mPulseStep;
		}
	} else {
		if (icon->mPulse <= 0.0f) {
			icon->mPulse     = 0.0f;
			icon->mPulseStep = step;
		} else {
			icon->mPulse += icon->mPulseStep;
		}
	}
}

// 0x800FE4E4
extern "C" void missionIconSetGaugeLevel(TAeObjMissionIcon* icon, int level)
{
	u32 low;
	u32 mid;
	u32 high;

	switch (level) {
	case 0:
		low  = 0;
		mid  = 9;
		high = 0xF;
		break;
	case 1:
		low  = 0;
		mid  = 9;
		high = 0xF;
		break;
	case 2:
		low  = 3;
		mid  = 0xB;
		high = 0x11;
		break;
	case 3:
		low  = 6;
		mid  = 0xD;
		high = 0x13;
		break;
	}
	if (icon->mMarks[1] != 0) {
		icon->mMarks[1]->reset(low, 1);
		icon->mMarks[1]->mBanks = sMissionIconBanks;
	}
	if (icon->mMarks[2] != 0) {
		icon->mMarks[2]->reset(mid, 1);
		icon->mMarks[2]->mBanks = sMissionIconBanks;
	}
	if (icon->mMarks[3] != 0) {
		icon->mMarks[3]->reset(high, 1);
		icon->mMarks[3]->mBanks = sMissionIconBanks;
	}
	sMissionIconGauge = level;
}

// 0x800FE5F8
extern "C" int missionIconSetCount0(TAeObjMissionIcon* icon, int count)
{
	int i;
	int n;

	if (icon->mLayer != 0) {
		n = count < 0 ? 0 : (count > 3 ? 3 : count);
		if (n != icon->mCount) {
			for (i = 0; i < 3; i++) {
				if (i < 2 - n) {
					if (icon->mMarks[i] != 0) {
						icon->mMarks[i]->reset(0xA6, 0x10);
						icon->mMarks[i]->mBanks = sMissionIconBanks;
						icon->mMarks[i]->cue(1);
					}
					if (icon->mFills[i] != 0) {
						icon->mLayer->bind(sFillSlots[i], icon->mFills[i]);
						icon->mFills[i]->cue(1);
					}
				} else if (i == 2 - n) {
					if (icon->mMarks[i] != 0) {
						icon->mMarks[i]->reset(0xA6, 0x10);
						icon->mMarks[i]->mBanks = sMissionIconBanks;
					}
					if (icon->mFills[i] != 0) {
						icon->mLayer->bind(sFillSlots[i], icon->mFills[i]);
						icon->mFills[i]->wake();
					}
				}
			}
		}
		icon->mCount = n;
		return 1;
	}
	return 0;
}

// 0x800FE780
extern "C" int missionIconSetCount1(TAeObjMissionIcon* icon, int count)
{
	int i;
	int n;

	if (icon->mLayer != 0) {
		n = count < 0 ? 0 : (count > 3 ? 3 : count);
		if (n != icon->mCount) {
			for (i = 0; i < 3; i++) {
				if (i < 2 - n) {
					if (icon->mMarks[i] != 0) {
						icon->mMarks[i]->reset(0x99, 0x10);
						icon->mMarks[i]->mBanks = sMissionIconBanks;
						icon->mMarks[i]->cue(1);
					}
					if (icon->mFills[i] != 0) {
						((TAeClip*)icon->mFills[i])->pause();
						icon->mLayer->clearSlot(sFillSlots[i]);
					}
				} else if (i == 2 - n) {
					if (icon->mMarks[i] != 0) {
						icon->mMarks[i]->reset(0x99, 0x10);
						icon->mMarks[i]->mBanks = sMissionIconBanks;
					}
					if (icon->mFills[i] != 0) {
						((TAeClip*)icon->mFills[i])->pause();
						icon->mLayer->clearSlot(sFillSlots[i]);
					}
				}
			}
		}
		icon->mCount = n;
		return 1;
	}
	return 0;
}

// 0x800FE8EC
extern "C" int missionIconSetCount2(TAeObjMissionIcon* icon, int count)
{
	int i;
	int n;

	if (icon->mLayer != 0) {
		n = count < 0 ? 0 : (count > 4 ? 4 : count);
		if (n != icon->mCount) {
			for (i = 0; i < 4; i++) {
				if (i < 3 - n) {
					if (icon->mMarks[i] != 0) {
						icon->mMarks[i]->reset(0x8C, 0x10);
						icon->mMarks[i]->mBanks = sMissionIconBanks;
						icon->mMarks[i]->cue(1);
					}
					if (icon->mFills[i] != 0) {
						icon->mLayer->bind(sFillSlots[i], icon->mFills[i]);
						icon->mFills[i]->wake();
					}
				} else if (i == 3 - n) {
					if (icon->mMarks[i] != 0) {
						icon->mMarks[i]->reset(0x8C, 0x10);
						icon->mMarks[i]->mBanks = sMissionIconBanks;
					}
					if (icon->mFills[i] != 0) {
						icon->mLayer->bind(sFillSlots[i], icon->mFills[i]);
						icon->mFills[i]->wake();
					}
				}
			}
		}
		icon->mCount = n;
		return 1;
	}
	return 0;
}

// 0x800FEA70
extern "C" int missionIconSetCount7(TAeObjMissionIcon* icon, int count)
{
	return 0;
}

// 0x800FEA78
extern "C" int missionIconSetCount3(TAeObjMissionIcon* icon, int count)
{
	if (icon->mLayer != 0) {
		if (count != icon->mCount && icon->mMarks[0] != 0) {
			icon->mMarks[0]->reset(0x7A, 0x10);
			icon->mMarks[0]->mBanks = sMissionIconBanks;
		}
		icon->mCount = count;
		return 1;
	}
	return 0;
}

// 0x800FEAF8
extern "C" int missionIconSetCount6(TAeObjMissionIcon* icon, int count)
{
	if (icon->mLayer != 0) {
		if (count != icon->mCount && icon->mFills[0] != 0) {
			icon->mFills[0]->cue(0);
			icon->mFills[0]->wake();
		}
		icon->mCount = count;
		return 1;
	}
	return 0;
}

// 0x800FEB70
extern "C" int missionIconSetDigit(TAeObjMissionIcon* icon, u32 slot, int value, const int* cells)
{
	int n;

	if (icon->mLayer != 0) {
		n = value < 0 ? 0 : (value > 9 ? 9 : value);
		icon->mLayer->setCell(slot, cells[n]);
		return 1;
	}
	return 0;
}

// 0x800FEBD4
extern "C" int missionIconSetDigits2(TAeObjMissionIcon* icon, u32 tens, u32 ones, int value, const int* cells)
{
	int n;

	if (icon->mLayer != 0) {
		n = value < 0 ? 0 : (value > 99 ? 99 : value);
		icon->mLayer->setCell(tens, cells[n / 10]);
		icon->mLayer->setCell(ones, cells[n % 10]);
		return 1;
	}
	return 0;
}

// 0x800FEC7C
extern "C" int missionIconSetDigits3(TAeObjMissionIcon* icon, u32 hundreds, u32 tens, u32 ones, int value, const int* cells)
{
	int n;
	int top;

	if (icon->mLayer != 0) {
		n   = value < 0 ? 0 : (value > 999 ? 999 : value);
		top = n / 100;
		icon->mLayer->setCell(hundreds, cells[top]);
		icon->mLayer->setCell(tens, cells[(n - top * 100) / 10]);
		icon->mLayer->setCell(ones, cells[n % 10]);
		return 1;
	}
	return 0;
}

// 0x800FED50
// message 0xBE
extern "C" int missionIconOnCount(TAeObjMissionIcon* icon, MissionIconMsg* msg)
{
	int result;

	switch (icon->mKind) {
	case 0:
		result = missionIconSetCount0(icon, msg->mValue);
		break;
	case 1:
		result = missionIconSetCount1(icon, msg->mValue);
		break;
	case 2:
		result = missionIconSetCount2(icon, msg->mValue);
		break;
	case 3:
		missionIconSetCount3(icon, msg->mValue);
		result = missionIconSetDigits3(icon, 0x10C, 0x10D, 0x10E, msg->mValue, sBigDigitCells);
		break;
	case 4:
		result = missionIconSetDigits3(icon, 0x10C, 0x10D, 0x10E, msg->mValue, sBigDigitCells);
		break;
	case 6:
		missionIconSetCount6(icon, msg->mValue);
		result = missionIconSetDigits3(icon, 0x10C, 0x10D, 0x10E, msg->mValue, sBigDigitCells);
		break;
	case 7:
		result = missionIconSetCount7(icon, msg->mValue);
		break;
	default:
		result = 0;
		break;
	}
	return result;
}

// 0x800FEE4C
// message 0xBF minutes then seconds
extern "C" int missionIconOnTime(TAeObjMissionIcon* icon, MissionIconMsg* msg)
{
	switch (icon->mKind) {
	case 6:
		icon->mFrame = msg->mSub + msg->mValue * 60;
		missionIconSetDigit(icon, 0x107, msg->mValue, sSmallDigitCells);
		missionIconSetDigits2(icon, 0x109, 0x10A, msg->mSub, sSmallDigitCells);
		return 1;
	case 8:
		icon->mFrame = msg->mSub + msg->mValue * 60;
		missionIconSetDigit(icon, 0xF9, msg->mValue, sSmallDigitCells);
		missionIconSetDigits2(icon, 0xFB, 0xFC, msg->mSub, sSmallDigitCells);
		return 1;
	}
	return 0;
}

// 0x800FEF3C
// message 0xC0
extern "C" int missionIconOnC0(TAeObjMissionIcon* icon, s16* code)
{
	return 0;
}

// 0x800FEF44
// message 0xC1
extern "C" int missionIconOnRate(TAeObjMissionIcon* icon, MissionIconRateMsg* msg)
{
	f32 time;

	switch (icon->mKind) {
	case 8:
		time = icon->mLayer->mEnd;
		time = (1.0f - msg->mRate) * time;
		if (icon->mLayer != 0) {
			icon->mLayer->_A8 = time;
		}
		if (icon->mMarks[1] != 0) {
			icon->mMarks[1]->_A8 = time;
		}
		if (icon->mMarks[2] != 0) {
			icon->mMarks[2]->_A8 = time;
		}
		if (icon->mMarks[3] != 0) {
			icon->mMarks[3]->_A8 = time;
		}
		break;
	case 7:
		break;
	}
	return 1;
}

// 0x800FEFB4
// message 0xC2
extern "C" int missionIconOnShow(TAeObjMissionIcon* icon, s16* code)
{
	if (icon->mLayer != 0) {
		icon->mLayer->show();
	}
	return 1;
}

// 0x800FEFE8
// message 0xC3
extern "C" int missionIconOnHide(TAeObjMissionIcon* icon, s16* code)
{
	if (icon->mLayer != 0) {
		icon->mLayer->hide();
	}
	return 1;
}

// 0x800FF01C
int TAeObjMissionIcon::vfn_20(s16* code)
{
	switch (*(u16*)code) {
	case 0xBE:
		return missionIconOnCount(this, (MissionIconMsg*)code);
	case 0xBF:
		return missionIconOnTime(this, (MissionIconMsg*)code);
	case 0xC0:
		return missionIconOnC0(this, code);
	case 0xC1:
		return missionIconOnRate(this, (MissionIconRateMsg*)code);
	case 0xC2:
		return missionIconOnShow(this, code);
	case 0xC3:
		return missionIconOnHide(this, code);
	}
	return 0;
}
