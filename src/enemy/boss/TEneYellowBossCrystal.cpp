#include "enemy/boss/TEneYellowBossCrystal.h"
#include "enemy/boss/TEnePurpleBoss.h"
#include "engine/CharaColli.h"
#include "sound/SeHolder.h"
#include "enemy/boss/TEneYellowBoss.h"
#include "enemy/EnemyFx.h"
#include "object/ObjShadow.h"
#include "enemy/ObjEnemy.h"
#include "ui/AeUtil.h"
#include "core/MathUtil.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyTexFx.h"
#include "enemy/EnemyGround.h"
#include "engine/MotionPose.h"
#include "object/ObjAction.h"
#include "engine/Colli.h"
#include "engine/DrawTask.h"
#include "ui/AeArc.h"
#include "egg/EggCreature.h"
#include "stl/math.h"
#include "Ninja/ninja.h"
#include "enemy/EnemyHit.h"
#include "enemy/boss/TEneYellowBossPurple.h"
struct Vec3f;
struct GjActor;

struct CrystalMsg : public ObjMsg {
	u16 _02;
	u16 mFrom; // _04 the toucher's id
};

// 0x8014F238
TEneYellowBossCrystal::TEneYellowBossCrystal(TObject* parent)
    : TObjAction(parent)
{
	setObjId(this, 0xFFFF);
	motionCtrlSetPlayer(&mClip, &mMotion);
	shadowSetGround(&mShadow, &mXform.mPos, 0);
	mShadow._30 = 10.0f;
	mShadow._34 = 10.0f;
	shadowDisable(&mShadow);
	mSight.reset();
	mSight.mHeightLimit = 1000.0f;
	mLookPos.x = 0.0f;
	mLookPos.y = 0.0f;
	mLookPos.z = 0.0f;
	mLookAngle = 0;
	touchListInit(&mTouch);
	mCrystalFlags = 0;
	mAction       = -1;
	mNextAction   = -1;
}

// 0x8014F33C
TEneYellowBossCrystal::~TEneYellowBossCrystal() { releaseObjId(this); }

// 0x8014F3F8
void TEneYellowBossCrystal::look()
{
	if (mSight.findInRange(100000000.0f, &mXform.mPos)) {
		mLookPos.x = mSight.mTarget.x;
		mLookPos.y = mSight.mTarget.y;
		mLookPos.z = mSight.mTarget.z;
		mLookAngle = mSight.mAngle;
	}
}

// 0x8014F464
void TEneYellowBossCrystal::startAction(int action)
{
	mAction     = action;
	mNextAction = -1;
	runAction(mAction, 0);
}

// 0x8014F4A4
void TEneYellowBossCrystal::setNext(int action) { mNextAction = action; }

// 0x8014F4AC
void TEneYellowBossCrystal::clearNext() { mNextAction = -1; }

// 0x8014F4B8
void TEneYellowBossCrystal::checkAction() { runAction(mAction, 1); }

// 0x8014F4EC
void TEneYellowBossCrystal::switchAction()
{
	if (mNextAction != -1) {
		runAction(mAction, 3);
		mAction = mNextAction;
		runAction(mAction, 0);
	}
}

// 0x8014F55C
void TEneYellowBossCrystal::stepAction() { runAction(mAction, 2); }

// 0x8014F590
// Tells what newly touches colli that it did with a spark and a sound at the touch
void crystalTouch(CharaColli* colli, EnemyTouchList* list, f32 radius)
{
	EnemyTouchList now;
	u32 id;
	ColliContact* contact;
	ColliShape* shape;

	touchListInit(&now);
	if (colli != 0) {
		for (contact = colli->mContacts; contact != 0; contact = contact->mNext) {
			if (colliContactHasBit8(contact)) {
				id = colliContactOwner(contact);
				touchListAdd(&now, id);
				if (!touchListHas(list, id)) {
					if (colliContactShape(contact)->mFlags & 0x400) {
						TObjAction* obj = findObjById(id);
						if (obj != 0) {
							CrystalMsg msg;
							msg.mType = 0x2A;
							msg.mFrom = colliGetOwner(colli);
							obj->sendToHandle(&msg);
						}
					}
					shape = colliGetShape(colli, contact->mShape);
					if (shape != 0) {
						Vec3f at;
						at.set(shape->mPos.x, shape->mPos.y, shape->mPos.z);
						enemyFxSparks(radius, &contact->mPos, &at);
						seStartAtFar(0x155, &contact->mPos);
					}
				}
			}
		}
	}
	*list = now;
}

// 0x8014F6E4
int crystalBreakFx(f32 radius, const Vec3f* pos)
{
	Vec3f at;
	f32 scale = radius / 40.0f;

	at.x = pos->x;
	at.y = pos->y;
	at.z = pos->z;
	at.y += 0.2f;
	fxSpawnScaled(scale, scale, 1.0f, 5, (void*)10, (const Vec3f*)&at, -0x4000, 0);
	seStartAtFar(0x15B, pos);
	return 60;
}

// 0x8014F768
void TEneYellowBossCrystal::hide()
{
	mCrystalFlags |= 0xA;
	shadowDisable(&mShadow);
}

// 0x8014F79C
void TEneYellowBossCrystal::waitBreak(int* timer, f32* time)
{
	(*timer)--;
	if (*timer <= 0) {
		mCrystalFlags &= ~2;
		shadowEnable(&mShadow);
		*time -= 1.0f;
		if (*timer == 0) {
			seStartAtFar(0x15C, &mXform.mPos);
		}
	}
	bossSuitUpdateNoFx(*time, &mSuit);
}

// 0x8014F82C
void TEneYellowBossCrystal::show()
{
	mCrystalFlags &= ~0xA;
	shadowEnable(&mShadow);
}

// 0x8014F864
f32 TEneYellowBossCrystal::startSink(f32 from, f32 to, f32 extra)
{
	bossSuitSetFade(30.0f, 55.0f, &mSuit);
	bossSuitSetSink(0.0f, 60.0f, from, to, extra, &mSuit);
	mCrystalFlags |= 1;
	seStartAtFar(0x15D, &mXform.mPos);
	return 60.0f;
}

// 0x8014F910
void TEneYellowBossCrystal::endSink() { mCrystalFlags &= ~1; }

// 0x8014F920
int TEneYellowBossCrystal::isSinking() { return mCrystalFlags & 1; }

// 0x8014F92C
void TEneYellowBossCrystal::cull(DrawTask* task, f32 far, f32 radius, Vec3f* pos, int arg)
{
	if (mCrystalFlags & 2) {
		mFlags |= 0x10;
	} else if (!mLight.isVisible(task, far, pos, radius, arg)) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
		if (mCrystalFlags & 1) {
			moveDrawTask(task, 0x22);
		}
	}
}

// 0x8014F9B8
static void crystalNoTexture() { }

// 0x8014F9BC
static void crystalTexGen() { GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTTEXMTX0); }

// 0x8014F9F4
static void crystalTevOrder(int stage)
{
	GXSetTevOrder(stage, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GXSetTevOp(stage, GX_MODULATE);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

// 0x8014FA6C
void crystalTexScreen()
{
	f32 mtx[3][4];

	texFxScreenMtx(mtx[0]);
	GXLoadTexMtxImm(mtx[0], GX_TEXMTX0, GX_MTX3x4);
	mtxSetScale((float*)mtx[0], 3.0f, 3.0f, 1.0f);
	GXLoadTexMtxImm(mtx[0], 0x40, GX_MTX3x4);
}

// 0x8014FAC4
void crystalBeginDraw()
{
	void* funcs[8];

	njSetTexture(texture2TEneYellowBoss());
	gjLoadTextureNum(0, 1, 1, 0);
	funcs[4] = (void*)crystalTexGen;
	funcs[3] = (void*)crystalTevOrder;
	funcs[7] = (void*)crystalNoTexture;
	gjChangeDrawObjIF(funcs, 0x70);
}

// 0x8014FB2C
void crystalSetTex(int angle) { crystalTexScreen(); }

// 0x8014FB4C
void crystalEndDraw()
{
	void* funcs[8];

	funcs[4] = 0;
	funcs[3] = 0;
	funcs[7] = 0;
	gjChangeDrawObjIF(funcs, 0x70);
}

// 0x8014FB84
void crystalSuitBegin(f32 rise, BossSuit* suit)
{
	f32 height = suit->mHeight;

	gjSetBlendOverride(1, 4, 5, 3);
	_nj_control_3d_flag_ |= 0x8000;
	texFxSuitLoadTex(1);
	texFxSuitScroll(height + rise, height, 1);
	texFxSuitBegin(0, suit->mAlpha);
}
