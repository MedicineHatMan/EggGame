#include "enemy/boss/EneBoss.h"

#include "core/TPeriVibrate.h"
#include "core/TMainTask.h"

typedef char TPeriVibrate_size_check[(sizeof(TPeriVibrate) == 0x500) ? 1 : -1];

// 0x80031F14
extern "C" void vibeUnpack(VibePattern* pattern, VibeRecord* record)
{
	int i;
	u8* out         = pattern->mSteps;
	pattern->mValueA = (record->mHeader & 0xFF00) >> 8;
	pattern->mValueB = record->mHeader & 0xFF;
	for (i = 0; i < 2; i++) {
		u32 bits = record->mBits[i];
		for (int j = 0; j < 0x10; j++) {
			*out++ = bits >> 30;
			bits <<= 2;
		}
	}
}

// 0x80031F70
extern "C" TPeriVibrate* periVibrateCreate()
{
	if (sPeriVibrateObj == 0) {
		sPeriVibrateObj = new TPeriVibrate(gTL_02);
	}
	return sPeriVibrateObj;
}

const char* gTPeriVibrateName = "TPeriVibrate";

// 0x80031FC0
TPeriVibrate::TPeriVibrate(TObject* parent)
    : TObject(parent)
{
	_00   = (u32)gTPeriVibrateName;
	mSize = 0x500;
	for (int i = 0; i < 4; i++) {
		mPorts[i].mOn      = 0;
		mPorts[i].mPattern = 0;
		mPorts[i].mStep    = 0;
		mPorts[i].mTime    = 0;
	}
	for (int i = 0; i < 0x20; i++) {
		vibeUnpack(&mPatterns[i], &sVibePatterns[i]);
	}
}

// 0x8003207C
TPeriVibrate::~TPeriVibrate()
{
	if (sPeriVibrateObj == this) {
		sPeriVibrateObj = 0;
	}
}

// 0x800320F0
extern "C" int periVibrateExists() { return sPeriVibrateObj != 0; }

// 0x80032104
void TPeriVibrate::play(int port, int pattern)
{
	mPorts[port].mOn      = 1;
	mPorts[port].mStep    = 0;
	mPorts[port].mTime    = 0;
	mPorts[port].mPattern = pattern;
}

// 0x80032128
extern "C" void periVibratePlay(int port, int pattern)
{
	if (periVibrateExists()) {
		sPeriVibrateObj->play(port, pattern);
	}
}

// 0x80032170
void TPeriVibrate::draw() { }
