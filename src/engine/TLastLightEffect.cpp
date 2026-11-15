#include "engine/TLastLightEffect.h"
struct DrawTask;
#include "Ninja/ninja.h"
#include "engine/MotionPose.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "object/TObjEggRing.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
struct Fx;

typedef char TLastLightEffect_size_check[(sizeof(TLastLightEffect) == 0x10C) ? 1 : -1];

// 0x801CEF50
TLastLightEffect::TLastLightEffect(TObject* parent, void* motion, int fx, const f32* mtx, const u32* color)
    : TObject(parent), mFx(0), mMotion(motion), mFxId(fx), mFrame(0)
{
	_00   = (u32)gTLastLightEffectName;
	mSize = 0x10C;
	mtxCopy(mBase, mtx);
	mtxCopy(mMtx, mtx);
	Vec3f pos;
	mtxGetPos(mtx, &pos);
	mLight.mFlags = 2;
	mLight.mKa[0] = 1.0f;
	mLight.mKa[1] = 0.0f;
	mLight.mKa[2] = 0.0f;
	mLight.mColor = *color;
	mLight.mPos   = pos;
}

// 0x801CF028
TLastLightEffect::~TLastLightEffect()
{
	playerRemoveLight(0, &mLight);
	fxStop((Fx*)mFx);
}

// 0x801CF0C4
void TLastLightEffect::draw()
{
	MotionNodePose at;
	NJS_OBJECT pose;
	motionObjInit(&pose);
	motionGetNodePose(&at, &pose, mMotion, 0.5f * (f32)mFrame);
	mtxSetTrans(mMtx, at.mPos.x, at.mPos.y, at.mPos.z);
	mtxConcat(mMtx, mBase, mMtx);
	Vec3f pos;
	mtxGetPos(mMtx, &pos);
	mLight.mPos = pos;
	if (mFrame == 0x70) {
		playerAddLight(0, &mLight);
		mFx = fxSpawnFollow(1, 1, mFxId, mMtx);
	} else if (mFrame == 0x14A) {
		mFlags |= 1;
	}
	mFrame++;
}

#include "enemy/boss/EneBoss.h"
#include "engine/DrawTask.h"
#include "Ninja/ninja.h"
#include "engine/MotionPose.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "egg/EggCreature.h"
#include "ui/TSuperEmblem.h"
#include "engine/DrawState.h"
#include "object/TObjEggRing.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "ui/AeArc.h"
#include "object/TObjLauncher.h"
#include "player/TPlyDemoLastSuper.h"
struct NJS_OBJECT;
struct DrawState;
struct DrawStateSlot;

/**
 * The extra stage the draw state holds.
 *
 * @size 0xC
 */
struct EmblemStage {
	u8 mTexGens;    // _00
	u8 mTevStages;  // _01
	void* mTexGen;  // _04
	void* mTev;     // _08
};

typedef char TSuperEmblem_size_check[(sizeof(TSuperEmblem) == 0x88) ? 1 : -1];

// 0x801CF1C8
TSuperEmblem::TSuperEmblem(TObject* parent, const f32* matrix)
    : TObject(parent)
{
	mTime = 0;
	_00   = (u32)gTSuperEmblemName;
	mSize = 0x88;
	mtxCopy(mBase, matrix);
	mtxCopy(mMatrix, matrix);
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
}

// 0x801CF254
TSuperEmblem::~TSuperEmblem() { removeDrawTask(mTask); }

// 0x801CF2BC
void TSuperEmblem::draw()
{
	MotionNodePose at;
	NJS_OBJECT pose;
	motionObjInit(&pose);
	motionGetNodePose(&at, &pose, *(void**)((u8*)gLastSuperExports + 0x2C), 0.5f * (f32)mTime);
	mtxSetTrans(mMatrix, at.mPos.x, at.mPos.y, at.mPos.z);
	mtxConcat(mMatrix, mBase, mMatrix);
	if (mTime == 0x70) {
		fxSpawnAt(1, 1, 0x3B, mMatrix);
		mFlags |= 1;
	}
	mTime++;
}

// 0x801CF384
void TSuperEmblem::render()
{
	u32 param[9];
	param[0] = 0;
	param[8] = 0;

	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	f32 matrix[12];
	PSMTXCopy(_gj_matrix_, matrix);
	matrix[3]  = 0.0f;
	matrix[7]  = 0.0f;
	matrix[11] = 0.0f;
	GXLoadTexMtxImm(matrix, GX_TEXMTX5, GX_MTX3x4);

	param[0] |= 0x2000;
	EmblemStage stage;
	stage.mTexGens   = 1;
	stage.mTevStages = 1;
	stage.mTexGen    = (void*)superEmblemTexGen;
	stage.mTev       = (void*)superEmblemTevStage;
	param[7]         = (u32)&stage;

	njSetTexture(*(void**)((u8*)gLastSuperExports + 0x34));
	gjLoadTextureNum(0, 1, 1, 1);
	DrawStateSlot* slot = getDrawStateSlot();
	njSetTexture(*(void**)((u8*)gLastSuperExports + 0x30));
	beginDrawState(slot, (const DrawState*)param);
	gjDrawObject((NJS_OBJECT*)(*(void**)((u8*)gLastSuperExports + 0x28)));
	endDrawState((DrawStateSlot*)slot);
	njPopMatrixEx();
}

// 0x801CF484
extern "C" void superEmblemTevStage(u8 stage)
{
	GXSetTevOrder(stage, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
	GXSetTevColorIn(stage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_HALF, GX_CC_ZERO);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

// 0x801CF528
extern "C" void superEmblemTexGen(u8 gen) { GXSetTexCoordGen2(gen, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX5, GX_TRUE, GX_PTTEXMTX19); }

#include "player/TPlyDemoLastSuper.h"
#include "io/Resource.h"
#include "camera/EventCamArc.h"
#include "camera/TCamera.h"
#include "player/Player.h"
#include "engine/TLastLightEffect.h"
#include "stage/TEventCtrl.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "object/TObjEggRing.h"
#include "sound/SoundCtrl.h"
#include "ui/MessageTable.h"
#include "object/TObjPlayer.h"
#include "ui/TSuperEmblem.h"
struct Fx;

struct TObjPlayer;

typedef char TPlyDemoLastSuper_size_check[(sizeof(TPlyDemoLastSuper) == 0x6C) ? 1 : -1];
typedef char TSuperEmblem_size_check[(sizeof(TSuperEmblem) == 0x88) ? 1 : -1];

// 0x801CF560
// superBillyLoad hands over the archive
void plyDemoLastSuperBind(TArc* arc)
{
	gLastSuperExports = (LastSuperExport*)findArcExport(arc, "event_super");
}

// 0x801CF58C
TPlyDemoLastSuper::TPlyDemoLastSuper(TObject* parent)
    : TObject(parent)
{
	mTimer = 0;
	mEvent = 0;
	mText  = 0;
	mPart  = 0;
	_00    = gTPlyDemoLastSuperName;
	mSize  = 0x6C;
	mStep  = &TPlyDemoLastSuper::stepOpen;
}

// 0x801CF60C
TPlyDemoLastSuper::~TPlyDemoLastSuper()
{
	if (mEvent != 0) {
		mEvent->mFlags |= 1;
	}
	if (mText != 0) {
		mText->mHold = 1;
	}
	fxStop((Fx*)mPart);
}

// 0x801CF6A0
void TPlyDemoLastSuper::draw()
{
	(this->*mStep)();
}

// 0x801CF6C8
TChrPlayer* TPlyDemoLastSuper::player()
{
	return (TChrPlayer*)findPlayer(0);
}

// 0x801CF6EC
void TPlyDemoLastSuper::stepOpen()
{
	ObjMsg msg87;
	ObjMsg restart;
	TChrPlayer* ply;

	if (eventStartById(0, 0) == 0) {
		return;
	}
	sePlayFor(0x2B2, 0);
	mText     = createSimpleTextPlain(getMessage(0));
	restart.mType = 0x82;
	sendMsgById(0, &restart);
	msg87.mType = 0x87;
	sendMsgById(0, &msg87);
	mEvent = eventCamArcPlay(1.0f, 0.0f, 2);
	mEvent->attachToMtx();
	ply = player();
	mtxSetTransV(mMtx, ply->getPos());
	ply = player();
	mtxRotateY(mMtx, mMtx, ply->getRotY());
	mtxCopy(mEvent->getMtx(), mMtx);
	mStep = &TPlyDemoLastSuper::stepBurst;
}

// 0x801CF804
void TPlyDemoLastSuper::stepBurst()
{
	int motions[6]; // the motions vfn_C8 plays
	TChrPlayer* ply;

	if (mText != 0 && mText->mState != 3) {
		return;
	}
	// The store runs when mText is null too
	mText->mHold = 1;
	mText        = 0;
	mEvent->mFlags |= 1;
	mEvent = createCamPath(0, 2, gLastSuperExports->mCamObject, gLastSuperExports->mCamMotion, 0, 0.0f, 0.5f);
	mEvent->attachToMtx();
	mtxCopy(mEvent->getMtx(), mMtx);
	new TLastLightEffect(this, gLastSuperExports->mLights[0], 0x40, mMtx, &sLastSuperColor0);
	new TLastLightEffect(this, gLastSuperExports->mLights[1], 0x44, mMtx, &sLastSuperColor1);
	new TLastLightEffect(this, gLastSuperExports->mLights[2], 0x48, mMtx, &sLastSuperColor2);
	new TSuperEmblem(this, mMtx);
	motions[0] = 0;
	motions[1] = 0;
	motions[2] = 0;
	motions[3] = 0;
	motions[4] = 0;
	motions[5] = 0;
	motions[0] = gLastSuperExports->mMotions[0];
	motions[1] = gLastSuperExports->mMotions[1];
	motions[2] = gLastSuperExports->mMotions[2];
	motions[4] = gLastSuperExports->mMotions[3];
	motions[3] = gLastSuperExports->mMotions[4];
	ply     = player();
	ply->vfn_C8(motions, mMtx);
	sePlayFor(0x2BE, 0);
	mStep = &TPlyDemoLastSuper::stepPlay;
}

// 0x801CF9E8
void TPlyDemoLastSuper::stepPlay()
{
	TChrPlayer* ply;
	f32 mtx[12];

	if (mTimer == 0x104) {
		ply = (TChrPlayer*)findPlayer(0);
		mtxSetTransV(mtx, ply->getPos());
		mPart = (Fx*)fxSpawnAt(1, 1, 0x39, mtx);
	} else if (mTimer == 0x118) {
		ply = (TChrPlayer*)findPlayer(0);
		ply->vfn_D4();
	} else if (mTimer == 0x12C) {
		fxStop(mPart);
		mPart = 0;
	} else if (mTimer == 0x1B8) {
		ply = (TChrPlayer*)findPlayer(0);
		ply->vfn_9C(0xD, 0);
	} else if (mTimer == 0x20A) {
		mText = createSimpleTextPlain(getMessage(1));
		mStep = &TPlyDemoLastSuper::stepClose;
		return;
	}
	mTimer++;
}

// 0x801CFB0C
void TPlyDemoLastSuper::stepClose()
{
	ObjMsg msg;

	if (mText != 0 && mText->mState != 3) {
		return;
	}
	mText->mHold = 1;
	mText        = 0;
	mEvent->mFlags |= 1;
	mEvent    = 0;
	msg.mType = 0x79;
	sendMsgById(0, &msg);
	eventEnd();
	mFlags |= 1;
}

LastSuperExport* gLastSuperExports; // 0x802C5C88

static inline u32 superColor(u32 c) { return c; }

// The __sinit at 0x801CFB94 fills these
u32 sLastSuperColor0 = superColor(0xFF0000FF);
u32 sLastSuperColor1 = superColor(0x0000FFFF);
u32 sLastSuperColor2 = superColor(0xFFFF00FF);
