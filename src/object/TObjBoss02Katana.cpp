#include "object/TObjBoss02Katana.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "object/ObjModel.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "object/TObjBoss02Fence.h"
#include "egg/EggCreature.h"
#include "engine/DrawState.h"
#include "enemy/boss/EneBoss.h"
#include "Ninja/ninja.h"
#include "ui/AeArc.h"
#include "object/TObjLauncher.h"
#include "camera/TCamPath.h"
#include "engine/FrameTex.h"
struct NJS_OBJECT;
struct DrawStateSlot;

typedef char TObjBoss02Katana_size_check[(sizeof(TObjBoss02Katana) == 0xD4) ? 1 : -1];

// 0x800621F4
extern "C" TObject* createTObjBoss02Katana(TObject* parent, SetDesc* desc)
{
	return new TObjBoss02Katana(parent, desc);
}

const char* gTObjBoss02KatanaName = "TObjBoss02Katana";

// 0x80062248
TObjBoss02Katana::TObjBoss02Katana(TObject* parent, SetDesc* desc)
    : TSetObj(parent, desc)
{
	_00       = (u32)gTObjBoss02KatanaName;
	mSize     = 0xD4;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mLight.reset();
}

// 0x800622C0
TObjBoss02Katana::~TObjBoss02Katana()
{
	removeDrawTask(mDrawTask);
	mDrawTask = 0;
}

// 0x8006233C
void TObjBoss02Katana::draw()
{
	if (isInRange(this, &mTrans.mPos, 2050.0f) == 0) {
		mFlags |= 1;
	} else {
		frameTexRequestCur();
	}
}

// 0x8006238C
void TObjBoss02Katana::vfn_10()
{
	if (mLight.isVisible(mDrawTask, 2500.0f, &mTrans.mPos, 80.0f, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x800623FC
// The node callback stored at info[3] for mask 0x20
static void katanaTev(int stage, int coord, int map, int chan)
{
	GXSetTevKColorSel(stage, GX_TEV_KCSEL_K0);
	GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_KONST);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(stage, coord, map, chan);
}

static const GXColor sKatanaKColor = { 0x64, 0x64, 0x64, 0xFF };

// 0x800624B4
void TObjBoss02Katana::render()
{
	NJS_OBJECT** models    = boss02GetFenceModels();
	NJS_TEXLIST** textures = boss02GetFenceTextures();
	u32 info[8];
	DrawStateSlot* slot;

	njPushMatrixEx();
	njTranslateEx(&mTrans.mPos);
	gjRotateX(mTrans.mRot.x);
	gjRotateY(mTrans.mRot.y);
	njSetTexture(textures[0]);
	mLight.drawObject(models[1]);

	ObjDrawState state;
	slot = getDrawStateSlot();
	frameTexSetCur();
	setDrawAmbient(&mLight, &state);
	if ((state.mFlags & 0x200) != 0) {
		u32 ambient = state.mAmbient;
		beginDrawAmbient(slot, &ambient);
	}

	_nj_control_3d_flag_ |= 0x100000;
	info[3] = (u32)katanaTev;
	gjChangeDrawObjIF((void* const*)info, (u32)0x20);
	GXSetTevKColor(GX_KCOLOR0, sKatanaKColor);
	gjDrawObject((NJS_OBJECT*)models[2]);
	_nj_control_3d_flag_ &= ~0x100000;
	info[3] = 0;
	gjChangeDrawObjIF((void* const*)info, (u32)0x20);
	if ((state.mFlags & 0x200) != 0) {
		endDrawAmbient((DrawStateSlot*)slot);
	}
	njPopMatrixEx();
}
