#include "engine/WaterSurface.h"
#include "object/green/TObjGreenPondScene.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"

// 0x80193478
extern "C" void bossWaterChan()
{
	static const GXColor sWhite = { 255, 255, 255, 255 }; // 0x802CBBA8
	GXColor color = sWhite;

	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanMatColor(GX_COLOR0A0, color);
}

// 0x801934D8
void GreenBossWater::beginDraw()
{
	u32 info[8];

	_nj_control_3d_flag_ |= 0x80000;
	info[4] = (u32)waterTexGen0;
	info[5] = (u32)bossWaterChan;
	gjChangeDrawObjIF((void*const*)info, (u32)0xC0);
	gjSetNumPostChans(0);
	gjSetPostChanCallback(0);
	gjSetNumPostTevStages(1);
	gjSetPostTevStageCallback((void*)waterTevStage);
	gjSetNumPostTexGens(2);
	gjSetPostTexGenCallback((void*)waterTexGen1);
}

// 0x8019355C
void GreenBossWater::endDraw()
{
	u32 info[8];

	GXSetNumIndStages(0);
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetTevDirect(GX_TEVSTAGE1);
	info[4] = 0;
	info[5] = 0;
	gjChangeDrawObjIF((void*const*)info, (u32)0xC0);
	gjSetNumPostChans(0);
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	_nj_control_3d_flag_ &= ~0x80000;
}
