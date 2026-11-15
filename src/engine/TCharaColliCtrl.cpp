#include "enemy/boss/EneBoss.h"
#include "engine/TCharaColliCtrl.h"
#include "engine/CharaColli.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"

typedef char TCharaColliCtrl_size_check[(sizeof(TCharaColliCtrl) == 0x20) ? 1 : -1];

const char* gTCharaColliCtrlName = "TCharaColliCtrl";

// 0x8000A9A4
TCharaColliCtrl::TCharaColliCtrl(TObject* parent)
    : TObject(parent)
{
	_00   = (u32)gTCharaColliCtrlName;
	mSize = 0x20;
}

// 0x8000A9F0
TCharaColliCtrl::~TCharaColliCtrl()
{
	charaColliClear();
}

// 0x8000AA54
extern "C" void charaColliClear()
{
	CharaColli* node = gColliRetired->mNext;
	while (node != 0) {
		colliClear(node);
		CharaColli* cur = node;
		node            = node->mNext;
		colliLink(cur, gColliFree);
	}
}

// 0x8000AAA4
void TCharaColliCtrl::draw()
{
	charaColliClear();
	colliUpdate();
}

// 0x8000AAC8
void TCharaColliCtrl::vfn_10()
{
	if ((gColliDebug & 1) && (gColliDebug & 4)) {
		njPrintD(0x30008, gColliContactCount, 4);
	}
}
