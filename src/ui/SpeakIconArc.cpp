#include "ui/TSpeakCursor.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "ui/AeArc.h"
#include "object/ObjAction.h"
#include "object/TObjLauncher.h"
#include "object/TObjEggRing.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "stage/TEventCtrl.h"
#include "camera/TCamLockOnCursor.h"
#include "io/NinjaFile.h"
struct NJS_TEXLIST;

// 0x8010F6B8
// Name is ours
extern "C" void loadSpeakIconArc()
{
	loadArc("ar_icon3d.arc", &gIcon3dArc);
	if (gIcon3dArc != 0) {
		sSpeakCursorTexLists = (NJS_TEXLIST**)findArcExport(gIcon3dArc, "texlists");
		void** textures = (void**)findArcExport(gIcon3dArc, "textures");
		bindGvmTextures((u8*)*textures, sSpeakCursorTexLists[0]);
	}
}

// 0x8010F724
// Name is ours
extern "C" void freeSpeakIconArc()
{
	if (gIcon3dArc != 0) {
		releaseTextures(sSpeakCursorTexLists[0]);
		sSpeakCursorTexLists = 0;
	}
	releaseResource(gIcon3dArc);
	gIcon3dArc = 0;
}
