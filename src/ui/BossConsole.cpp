#include "ui/TAeObjBossConsole.h"
#include "io/ResManager.h"
#include "core/dolphin.h"
#include "egg/EggCreature.h"
#include "ui/TAeObject.h"
#include "ui/AeArc.h"
#include "core/Mem.h"
#include "core/TMainTask.h"
#include "ui/BossConsole.h"
struct NJS_TEXLIST;
const char* gTAeObjBossConsoleName = "TAeObjBossConsole";

struct BossConsoleArcData {
	void* mAe;             // _00
	NJS_TEXLIST* mTexList; // _04
	u8* mTextures;         // _08
	u32* mPalettes;        // _0C two
};

struct E3PaletteBuf {
	u8* mData;
};

const u32 sBossConsoleBanks[2] = { 20, 21 }; // 0x802C9290

// 0x800F2344
extern "C" void bossConsoleLoad(const char* stage)
{
	if (strcmp(stage, "blue_boss") == 0) {
		sBossConsoleOpen  = 5;
		sBossConsoleClose = 7;
	} else if (strcmp(stage, "red_boss") == 0) {
		sBossConsoleOpen  = 0x4E;
		sBossConsoleClose = 0x50;
	} else if (strcmp(stage, "purple_boss") == 0) {
		sBossConsoleOpen  = 0x43;
		sBossConsoleClose = 0x45;
	} else if (strcmp(stage, "orange_boss") == 0) {
		sBossConsoleOpen  = 0x38;
		sBossConsoleClose = 0x3A;
	} else if (strcmp(stage, "yellow_boss") == 0) {
		sBossConsoleOpen  = 0x59;
		sBossConsoleClose = 0x5B;
	} else if (strcmp(stage, "green_boss") == 0) {
		sBossConsoleOpen  = 0x10;
		sBossConsoleClose = 0x12;
	} else if (strcmp(stage, "last_boss") == 0) {
		sBossConsoleOpen  = 0x1B;
		sBossConsoleClose = 0x1D;
	} else if (strcmp(stage, "last_boss2") == 0) {
		sBossConsoleOpen  = 0x29;
		sBossConsoleClose = 0x2B;
	}
	loadArc("ae_boss_console.arc", &sBossConsoleArc);
	if (sBossConsoleArc != 0) {
		BossConsoleArcData* data = (BossConsoleArcData*)sBossConsoleArc->mData;
		sBossConsoleAe           = data->mAe;
		relocateAe(sBossConsoleAe);
		sBossConsoleTexList = data->mTexList;
		bindGvmTextures(data->mTextures, sBossConsoleTexList);
		E3PaletteBuf buf;
		buf.mData = new u8[0x800];
		if (data->mPalettes != 0) {
			loadE3Palette(data->mPalettes[0], (E3Palette*)&buf, sBossConsoleBanks[0]);
			loadE3Palette(data->mPalettes[1], (E3Palette*)&buf, sBossConsoleBanks[1]);
		}
		delete[] buf.mData;
		njSetTexture(sBossConsoleTexList);
		njSetPaletteBankNum(0, sBossConsoleBanks[0]);
		njSetPaletteBankNum(1, sBossConsoleBanks[1]);
		GXInvalidateTexAll();
	}
	sBossConsole = 0;
}

// 0x800F255C
extern "C" void bossConsoleFree()
{
	if (sBossConsole != 0) {
		sBossConsole->mFlags |= 1;
		sBossConsole = 0;
	}
	if (sBossConsoleArc != 0) {
		releaseTextures((NJS_TEXLIST*)sBossConsoleTexList);
		sBossConsoleTexList = 0;
		releaseResource(sBossConsoleArc);
		sBossConsoleArc = 0;
		sBossConsoleAe  = 0;
	}
}

// 0x800F25C8
// Makes the console once later calls return it again
extern "C" TAeObjBossConsole* bossConsoleGet()
{
	if (sBossConsoleArc == 0) {
		return 0;
	}
	if (sBossConsole == 0) {
		sBossConsole = new TAeObjBossConsole(gTL_07, sBossConsoleAe, sBossConsoleTexList);
	}
	return sBossConsole;
}
