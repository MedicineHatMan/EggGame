#include "player/TPlyDemoLastCtrl.h"
#include "io/Resource.h"
#include "ui/AeArc.h"
#include "player/SuperBillyRes.h"
#include "object/TObjPlayer.h"
#include "player/TPlyDemoLastLose.h"
#include "player/TPlyDemoLastSuper.h"
#include "engine/FxSystem.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;

TArc* gSuperBillyArc; // 0x802C5C78

// 0x801CEA5C
void superBillyLoad()
{
	void** lists;
	void** gvm;
	int i;

	loadArc("superbilly.arc", &gSuperBillyArc);
	if (gSuperBillyArc != NULL) {
		lists = (void**)findArcExport(gSuperBillyArc, "texlists");
		gvm   = (void**)findArcExport(gSuperBillyArc, "textures");
		for (i = 0; lists[i] != NULL; i++) {
			bindGvmTextures((u8*)*gvm, (NJS_TEXLIST*)lists[i]);
		}
		fxBankBind(1, (unsigned char*)(*(void**)findArcExport(gSuperBillyArc, "effect")));
		plyDemoLastLoseBind(gSuperBillyArc);
		plyDemoLastSuperBind(gSuperBillyArc);
	}
	playerEnsureArc(4);
}

// 0x801CEB10
void superBillyFree()
{
	void** lists;
	int i;

	playerFreeArc(4);
	if (gSuperBillyArc != NULL) {
		lists = (void**)findArcExport(gSuperBillyArc, "texlists");
		for (i = 0; lists[i] != NULL; i++) {
			releaseTextures((NJS_TEXLIST*)lists[i]);
		}
		fxBankUnbind(1, (unsigned char*)(*(void**)findArcExport(gSuperBillyArc, "effect")));
		releaseResource(gSuperBillyArc);
		gSuperBillyArc = NULL;
	}
}

// 0x801CEBA0
void spawnPlyDemoLastCtrl()
{
	new TPlyDemoLastCtrl(gTL_01);
}
