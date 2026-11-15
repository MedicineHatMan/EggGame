#include "stage/Stage.h"
#include "object/ObjAction.h"
#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCameraCtrl.h"
#include "camera/TCamEditor.h"
#include "core/PerfMeter.h"
#include "object/ObjShadow.h"
#include "engine/Viewport.h"
#include "object/purple/TObjPurpleIceTrap.h"
#include "egg/EggCreature.h"
#include "stage/TStageOrange.h"
#include "engine/ShadowTex.h"
#include "object/TObjLauncher.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "engine/DrawTask.h"
#include "object/orange/TObjOrangeMapMirror.h"
#include "object/TObjPlayer.h"
#include "core/TMainTask.h"
#include "core/SdkCalls.h"
#include "object/orange/TObjOrangeMirrorTrap.h"
struct TObjOrangeMapMirror;
struct TObjPlayer;

typedef char TStageOrange_size_check[(sizeof(TStageOrange) == 0x30) ? 1 : -1];

/** A Vec3f set by the static initializer */
struct MirrorVec : public Vec3f {
	MirrorVec(f32 nx, f32 ny, f32 nz)
	{
		x = nx;
		y = ny;
		z = nz;
	}
};

NJS_TEXNAME sMirrorTexName;                // 0x80282798
MirrorVec sMirrorPos(0.0f, 0.0f, 0.0f);    // 0x802827A4 a point on the mirror
MirrorVec sMirrorNormal(1.0f, 0.0f, 0.0f); // 0x802827B0
NJS_TEXLIST sMirrorTexList;                // 0x802C55B8
int sMirrorCopyPending;                    // 0x802C55C0
int sMirrorReflecting;                     // 0x802C55C4
u16 sMirrorSync;                           // 0x802C55C8

// Unused the linker strips it but its 1.0 stays first in the pool ahead of the __sinit's 0.0
void mirrorResetNormal()
{
	sMirrorNormal.x = 1.0f;
}

// 0x80123310
// Shows a tree of objects again
extern "C" void objTreeShow(TObject* obj)
{
	obj->mFlags &= ~0x10;
	for (TObject* child = obj->mChild; child != 0; child = child->mNext) {
		objTreeShow(child);
	}
}

// 0x8012335C
// Hides a tree of objects from a draw pass
extern "C" void objTreeHide(TObject* obj)
{
	obj->mFlags |= 0x10;
	for (TObject* child = obj->mChild; child != 0; child = child->mNext) {
		objTreeHide(child);
	}
}

// 0x801233A8
extern "C" TStage* createOrangeStage()
{
	return new TStageOrange;
}

// 0x801233E0
TStageOrange::TStageOrange()
{
}

// 0x8012341C
void TStageOrange::loadAssets()
{
	mirrorTexInit();
	loadBalloonArc();
	rollMirrorTrapAnswers();
}

// 0x80123444
void TStageOrange::freeAssets()
{
	releaseBalloonArc();
	mirrorTexRelease();
}

// 0x80123468
void TStageOrange::vfn_14()
{
	createTObjOrangeMapMirror(gTL_05);
	makeOrangeBalloon();
}

// 0x80123490
void TStageOrange::run(u32 flags)
{
	if (flags & 1) {
		calc();
	}
	u32 draw = flags & 2;
	if (draw) {
		camSetDrawView(0);
		if (mirrorCopyPending()) {
			mirrorBeginReflect();
			drawScene();
			mirrorEndReflect();
		}
	}
	if (draw) {
		camSetDrawView(0);
		drawObjects();
		if (mirrorCopyPending()) {
			objTreeHide(gTL_01);
			objTreeHide(gTL_02);
			mirrorBeginReflect();
			runDrawTasks(0x20);
			runDrawTasks(0x2C);
			runDrawTasks(0x21);
			runDrawTasks(0xA);
			shadowAlphaOff();
			runDrawTasks(0x2D);
			sortDrawTasks(0x22);
			runDrawTasks(0x22);
			shadowAlphaOn();
			perfBegin(0, "ParticleSprite");
			spriteDrawAll();
			perfEnd();
			mirrorEndReflect();
			mirrorCopyTex();
			objTreeShow(gTL_01);
			objTreeShow(gTL_02);
		}
	}
	if (draw) {
		camSetDrawView(0);
		drawScene();
	}
	if (draw) {
		TObjAction* player = (TObjAction*)findPlayer(0);
		Vec3f pos          = *player->getPos();
		GXSetCopyClear(*(GXColor*)&sStageOrangeClearColor, 0xFFFFFF);
		shadowTexGet()->begin(&pos);
		runDrawTasks(0x1F);
		shadowTexGet()->end();
	}
	if (draw) {
		setViewport(0);
		camSetDrawView(0);
		drawObjects();
		drawEffects(1);
	}
}

// 0x8012364C
TStageOrange::~TStageOrange()
{
}

// 0x801236A4
// Makes the 320x240 texture the reflection is copied into
extern "C" void mirrorTexInit()
{
	NJS_TEXINFO info;
	sMirrorTexList.textures  = &sMirrorTexName;
	sMirrorTexList.nbTexture = 1;
	njSetTextureInfo(&info, 0, 4, 320, 240);
	njSetTextureName(&sMirrorTexName, (void*)&info, (void*)0x101D, (u32)0x8800000);
	njLoadTexture(&sMirrorTexList);
	sMirrorCopyPending = 0;
	sMirrorReflecting  = 0;
}

// 0x8012371C
extern "C" void mirrorTexRelease()
{
	njReleaseTexture(&sMirrorTexList);
	sMirrorCopyPending = 0;
	sMirrorReflecting  = 0;
}

// 0x8012374C
extern "C" void mirrorSetPlane(const Vec3f* pos, const Vec3f* normal)
{
	sMirrorPos.x    = pos->x;
	sMirrorPos.y    = pos->y;
	sMirrorPos.z    = pos->z;
	sMirrorNormal.x = normal->x;
	sMirrorNormal.y = normal->y;
	sMirrorNormal.z = normal->z;
}

// 0x80123788
extern "C" void mirrorRequestCopy()
{
	sMirrorCopyPending = 1;
}

// 0x80123794
extern "C" int mirrorCopyPending()
{
	return sMirrorCopyPending;
}

// 0x8012379C
// 1 while the reflected pass draws
extern "C" int mirrorIsReflecting()
{
	return sMirrorReflecting;
}

// 0x801237A4
// Not written extern "C" the header declares it so its struct copy defers the body and only a plain definition
// dispatches it in place
void mirrorBeginReflect()
{
	Mtx34 m;
	f32 reflect[12];
	sMirrorReflecting = 1;
	setViewport(5);
	m = *(Mtx34*)gjGetMatrix();
	PSMTXReflect(reflect, &sMirrorPos, &sMirrorNormal);
	mtxConcat(m.m, m.m, reflect);
	setCullMode(0x18000000);
	GXSetCullMode(GX_CULL_FRONT);
	njPushMatrix(m.m);
}

// 0x80123838
extern "C" void mirrorEndReflect()
{
	njPopMatrixEx();
	setCullMode(0x10000000);
	GXSetCullMode(GX_CULL_BACK);
	sMirrorReflecting = 0;
}

// 0x80123870
// Waits for the reflected pass to finish drawing then copies it into the mirror texture
extern "C" void mirrorCopyTex()
{
	if (sMirrorCopyPending != 0) {
		GXSetDrawSync(++sMirrorSync);
		while (sMirrorSync != GXReadDrawSync()) { }
		GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
		GXSetTexCopySrc(0, 0, 320, 240);
		GXSetTexCopyDst(320, 240, GX_TF_RGB565, GX_FALSE);
		GXCopyTex(sMirrorTexList.textures->texaddr->hdr->info.image, GX_TRUE);
		GXInvalidateTexAll();
		sMirrorCopyPending = 0;
	}
}

// 0x80123918
extern "C" void mirrorSetTexture()
{
	njSetTexture(&sMirrorTexList);
}

// 0x8012393C
// is the static initializer for sMirrorPos and sMirrorNormal
