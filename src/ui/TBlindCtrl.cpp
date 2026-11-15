#include "enemy/boss/EneBoss.h"
#include "engine/TFogCtrl.h"

const char* gTBlindCtrlName = "TBlindCtrl";

#include "ui/TBlindCtrl.h"
#include "core/TMainTask.h"

typedef char TBlindCtrl_size_check[(sizeof(TBlindCtrl) == 0x30) ? 1 : -1];

// 0x801BDFC4
extern "C" void blindCreate()
{
	TObject* parent = gTL_02;
	if (sBlindCtrlObj == 0) {
		sBlindCtrlObj = new TBlindCtrl(parent);
	}
}

// 0x801BE048
extern "C" void blindOn(int player)
{
	if (sBlindCtrlObj != 0 && player >= 0 && player < 4) {
		sBlindCtrlObj->mOn[player] = 1;
	}
}

// 0x801BE078
extern "C" void blindOff(int player)
{
	if (sBlindCtrlObj != 0 && player >= 0 && player < 4) {
		sBlindCtrlObj->mOn[player] = 0;
	}
}

// 0x801BE0A8
extern "C" int blindIsOn(int player)
{
	if (sBlindCtrlObj != 0) {
		if (player >= 0 && player < 4) {
			return sBlindCtrlObj->mOn[player];
		}
		return 0;
	}
	return 0;
}

// 0x801BE0E4
TBlindCtrl::~TBlindCtrl()
{
	if (this == sBlindCtrlObj) {
		sBlindCtrlObj = 0;
	}
}

// 0x801BE158
void TBlindCtrl::draw()
{
	for (int i = 0; i < 4; i++) {
		if (mOn[i] != 0) {
			fogRequest(i, 0x12, 2, 0xFF, 0.05f);
		}
	}
}
