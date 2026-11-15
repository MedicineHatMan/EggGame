#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "engine/Viewport.h"
#include "engine/TFogCtrl.h"
#include "Ninja/ninja.h"
#include "engine/Color.h"
#include "engine/FogBlend.h"
#include "object/TObjPlayer.h"
#include "core/TMainTask.h"

const char* gTFogCtrlName = "TFogCtrl";

typedef char FogRecord_size_check[(sizeof(FogRecord) == 0x40) ? 1 : -1];

typedef char FogRun_size_check[(sizeof(FogRun) == 0x1C) ? 1 : -1];

typedef char FogBlend_size_check[(sizeof(FogBlend) == 0x6C) ? 1 : -1];

typedef char FogMix_size_check[(sizeof(FogMix) == 0x48) ? 1 : -1];

typedef char TFogCtrl_size_check[(sizeof(TFogCtrl) == 0x30C) ? 1 : -1];

// 0x8004E630
extern "C" FogRecord* fogRecord(int id)
{
	if (id < 0 || id >= 0x80) {
		id = 0;
	}
	return &sFogRecords[id];
}

// 0x8004E654
// Gives 0 to 1 at a phase 1 goes up and down in a line 2 is a cosine and 3 only goes up
extern "C" f32 fogWave(int kind, int phase)
{
	f32 value;
	switch (kind) {
	case 0:
	default:
		return 0.0f;
	case 1:
		value = (int)(u16)phase;
		value *= 1.5258789e-05f;
		if (value >= 0.5f) {
			value = 1.0f - value;
		}
		value *= 2.0f;
		return value;
	case 2:
		value = njCos(phase);
		return -0.5f * (value - 1.0f);
	case 3:
		value = (int)(u16)phase;
		value *= 1.5258789e-05f;
		return value;
	}
}

// 0x8004E730
extern "C" u32 fogMixColor(u32 a, u32 b, f32 t)
{
	u32 ca = a;
	u32 cb = b;
	u32 out;
	lerpColor(t, &out, (const unsigned long*)&ca, (const unsigned long*)&cb);
	return out;
}

// 0x8004E768
extern "C" f32 fogSwing(f32 base, f32 swing, s16 kind, s16 speed, int* time)
{
	if (kind != 0) {
		*time += speed;
		f32 w = fogWave(kind, *time);
		return swing * (2.0f * (w - 0.5f)) + base;
	}
	return base;
}

// 0x8004E7E4
void FogRun::init()
{
	mId        = 0;
	mColor     = 0;
	mColorTime = 0;
	mNear      = 50.0f;
	mNearTime  = 0;
	mFar       = 3000.0f;
	mFarTime   = 0;
}

// 0x8004E810
void FogRun::load(int id)
{
	if (id < 0 || id >= 0x80) {
		id = 0;
	}
	mId               = id;
	FogRecord* record = fogRecord(mId);
	mColor            = record->mColorA;
	mColorTime        = 0;
	mNear             = record->mNear;
	mNearTime         = 0x2000;
	mFar              = record->mFar;
	mFarTime          = 0x2000;
}

// 0x8004E884
void FogRun::update()
{
	FogRecord* record = fogRecord(mId);
	int wave = record->mColorWave;
	if (wave != 0) {
		mColorTime += record->mColorSpeed;
		mColor = fogMixColor(record->mColorA, record->mColorB, fogWave(wave, mColorTime));
	} else {
		mColor = record->mColorA;
	}
	mNear = fogSwing(record->mNear, record->mNearSwing, record->mNearWave, record->mNearSpeed, &mNearTime);
	mFar  = fogSwing(record->mFar, record->mFarSwing, record->mFarWave, record->mFarSpeed, &mFarTime);
}

// 0x8004E930
void FogBase::init()
{
	mEnabled     = 1;
	mRangeAdjust = 0;
}

// 0x8004E944
void FogBlend::init()
{
	FogBase::init();
	mFrom.init();
	mTo.init();
	mBlending = 0;
	mNext     = -1;
	mRate     = 0.0f;
	mPriority = -1;
	mOrder    = -1;
	mBlend    = 0.0f;
	mStep     = 0.0f;
	mMain     = gFogMain;
	mOther    = gFogOther;
	mCurrent  = gFogMain;
}

// 0x8004E9C0
// The highest priority asked for this frame wins
void FogBlend::request(int id, int priority, int order, f32 rate)
{
	if (priority < mPriority) {
		return;
	}
	if (priority == mPriority && order <= mOrder) {
		return;
	}
	mNext     = id;
	mRate     = rate;
	mPriority = priority;
	mOrder    = order;
}

// 0x8004E9F0
void FogBlend::setMain(int id)
{
	if (mCurrent == mMain) {
		mCurrent = id;
	}
	mMain = id;
}

// 0x8004EA0C
void FogBlend::setOther(int id)
{
	if (mCurrent == mOther) {
		mCurrent = id;
	}
	mOther = id;
}

// 0x8004EA28
void FogBlend::change(int id)
{
	if (mFrom.mId == mCurrent) {
		mFrom.load(id);
	}
	if (mTo.mId == mCurrent) {
		mTo.load(id);
	}
	mCurrent = id;
}

// 0x8004EA8C
void FogBlend::useMain() { change(mMain); }

// 0x8004EAB0
void FogBlend::useOther() { change(mOther); }

// 0x8004EAD4
void FogBlend::update()
{
	if (mNext < 0) {
		mNext = mCurrent;
		mRate = 0.05f;
	}
	if (mNext >= 0) {
		int differs;
		if (mBlending == 0) {
			differs = (u8)(mNext != mFrom.mId);
		} else {
			differs = (u8)(mNext != mTo.mId);
		}
		if (differs) {
			if (mRate <= 0.0f) {
				mRate = 0.001f;
			}
			if (mRate >= 1.0f) {
				mFrom.load(mNext);
			} else {
				if (mBlending == 1) {
					mFrom = mTo;
				}
				mTo.load(mNext);
				mBlend    = 0.0f;
				mStep     = mRate;
				mBlending = 1;
			}
		}
	}
	mNext     = -1;
	mPriority = -1;
	mOrder    = -1;
	if (mBlending == 0) {
		mFrom.update();
	} else {
		mFrom.update();
		mTo.update();
		mBlend += mStep;
		if (mBlend >= 1.0f) {
			mBlending = 0;
			mFrom     = mTo;
		}
	}
}

// 0x8004ECA8
extern "C" FogRecord* fogBetween(FogRun* a, FogRun* b, int* curve, u32* color, f32* near, f32* far, f32 t)
{
	FogRecord* record;
	if (t < 0.5f) {
		record = fogRecord(a->mId);
	} else {
		record = fogRecord(b->mId);
	}
	*curve = record->mCurve;
	*color = fogMixColor(a->mColor, b->mColor, t);
	*near  = t * (b->mNear - a->mNear) + a->mNear;
	*far   = t * (b->mFar - a->mFar) + a->mFar;
	return record;
}

// 0x8004ED68
void FogBlend::apply()
{
	FogRecord* record;
	int curve;
	u32 color;
	f32 near;
	f32 far;
	if (mBlending == 0) {
		record = fogRecord(mFrom.mId);
		curve  = record->mCurve;
		color  = mFrom.mColor;
		near   = mFrom.mNear;
		far    = mFrom.mFar;
	} else {
		record = fogBetween(&mFrom, &mTo, &curve, &color, &near, &far, mBlend);
	}
	gjSetFogCurve(curve);
	gjSetFogColor(color);
	gjSetFogRange(near, far);
	fogApply();
	mRangeAdjust = fogRangeFlag(record);
}

// 0x8004EE1C
void FogMix::init()
{
	FogBase::init();
	mMix = 0.0f;
	mFrom.init();
	mTo.init();
	mFrom.load(gFogMain);
	mTo.load(gFogOther);
}

// 0x8004EE78
void FogMix::setMix(f32 mix) { mMix = mix; }

// 0x8004EE80
void FogMix::update()
{
	mFrom.update();
	mTo.update();
}

// 0x8004EEB8
void FogMix::apply()
{
	int curve;
	u32 color;
	f32 near;
	f32 far;
	FogRecord* record = fogBetween(&mFrom, &mTo, &curve, &color, &near, &far, mMix);
	gjSetFogCurve(curve);
	gjSetFogColor(color);
	gjSetFogRange(near, far);
	fogApply();
	mRangeAdjust = fogRangeFlag(record);
}

// 0x8004EF34
extern "C" void fogLoad()
{
	sFogCtrlObj = 0;
	sFogView = 0;
	sFogBlending = 0;
	gFogMain = 0;
	gFogOther = 0;
	loadSizedResource("fog_param.bin", &sFogFile);
	sFogRecords = (FogRecord*)sFogFile->mData;
}

// 0x8004EF84
extern "C" void fogUnload()
{
	releaseResource(sFogFile);
	sFogFile = 0;
	sFogRecords = 0;
	if (sFogCtrlObj != 0) {
		sFogCtrlObj->mFlags |= 1;
		sFogCtrlObj = 0;
	}
}

// 0x8004EFD0
extern "C" TFogCtrl* fogCreate()
{
	if (sFogCtrlObj == 0) {
		sFogCtrlObj = new TFogCtrl(gTL_02);
	}
	return sFogCtrlObj;
}

// 0x8004F020
extern "C" void fogSetDefaultMain(int id) { gFogMain = id; }

// 0x8004F028
extern "C" void fogSetDefaultOther(int id) { gFogOther = id; }

// 0x8004F030
TFogCtrl::TFogCtrl(TObject* parent)
    : TObject(parent)
{
	_00   = (u32)gTFogCtrlName;
	mSize = 0x30C;
	for (int i = 0; i < 4; i++) {
		FogBlend* blend = &mBlends[i];
		blend->init();
		mMixes[i].init();
	}
	mMode = 0;
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x20, 0);
}

// 0x8004F11C
FogMix::FogMix() { }

// 0x8004F138
FogBlend::FogBlend() { }

// 0x8004F154
TFogCtrl::~TFogCtrl()
{
	removeDrawTask(mTask);
	if (sFogCtrlObj == this) {
		sFogCtrlObj = 0;
	}
}

// 0x8004F1D0
extern "C" void fogRequest(int view, int id, int priority, int order, f32 rate)
{
	if (sFogCtrlObj != 0) {
		sFogCtrlObj->request(view, id, priority, order, rate);
	}
}

// 0x8004F21C
void TFogCtrl::request(int view, int id, int priority, int order, f32 rate)
{
	if (view >= 0 && view < 4) {
		mBlends[view].request(id, priority, order, rate);
	}
}

// 0x8004F268
extern "C" void fogSetMain(int id)
{
	if (sFogCtrlObj != 0) {
		sFogCtrlObj->setMain(id);
	}
}

// 0x8004F29C
void TFogCtrl::setMain(int id)
{
	for (int i = 0; i < 4; i++) {
		mBlends[i].setMain(id);
	}
}

// 0x8004F2F0
extern "C" void fogSetOther(int id)
{
	if (sFogCtrlObj != 0) {
		sFogCtrlObj->setOther(id);
	}
}

// 0x8004F324
void TFogCtrl::setOther(int id)
{
	for (int i = 0; i < 4; i++) {
		mBlends[i].setOther(id);
	}
}

// 0x8004F378
extern "C" int fogRangeFlag(FogRecord* record) { return record->mFlags & 1; }

// 0x8004F384
extern "C" void fogApply()
{
	if (sFogBlending != 0) {
		gjEnableFog();
	} else {
		gjDisableFog();
	}
	gjSetFog();
}

// 0x8004F3BC
extern "C" void fogOff()
{
	gjDisableFog();
	gjSetFog();
}

// 0x8004F3E0
void TFogCtrl::draw()
{
	FogMix* fog;
	int old;
	f32 mix = fogBlendUpdate();
	old = mMode;
	if (mix <= 0.0f) {
		mMode = 0;
	} else if (mix >= 1.0f) {
		mMode = 2;
	} else {
		mMode = 1;
	}
	if (mMode == 1) {
		for (int i = 0; i < 4; i++) {
			fog = &mMixes[i];
			fog->setMix(mix);
			fog->update();
		}
	}
	if (old != mMode) {
		if (mMode == 0) {
			for (int i = 0; i < 4; i++) {
				mBlends[i].useMain();
			}
		} else if (mMode == 2) {
			for (int i = 0; i < 4; i++) {
				mBlends[i].useOther();
			}
		}
	}
	for (int i = 0; i < 4; i++) {
		FogBlend* blend = &mBlends[i];
		blend->update();
	}
}

// 0x8004F548
void TFogCtrl::vfn_10() { }

// 0x8004F54C
void TFogCtrl::render()
{
	int view = playerGetDrawView();
	if ((u16)view >= 4) {
		view = 3;
	}
	FogBase* fog;
	if (mMode == 1) {
		fog = &mMixes[(u16)view];
	} else {
		fog = &mBlends[(u16)view];
	}
	f32 x, y, width, height;
	getViewport(&x, &y, &width, &height);
	GXInitFogAdjTable((_GXFogAdjTable*)mRangeTable, (u16)width, (const float*)&gProjection);
	GXSetFogRangeAdj(fog->mRangeAdjust, (u16)(0.5f * width + x), (_GXFogAdjTable*)mRangeTable);
	sFogView = (u16)view;
	sFogBlending = fog->mEnabled;
	fog->apply();
}
