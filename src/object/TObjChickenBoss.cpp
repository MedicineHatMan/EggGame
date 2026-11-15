#include "stage/SetManager.h"
#include "stage/SetActor.h"
#include "io/Resource.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "egg/ObjEgg.h"
#include "engine/CharaColli.h"
#include "engine/MotionPlayer.h"
#include "engine/DrawTask.h"
#include "object/ObjArchive.h"
#include "camera/EventCamArc.h"
#include "stage/EventScript.h"
#include "engine/Landscape.h"
#include "engine/Color.h"
#include "engine/Viewport.h"
#include "camera/TCamera.h"
#include "ui/TSimpleText.h"
#include "engine/TParticle.h"
#include "object/TObjEggRing.h"
#include "engine/Fx.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "stage/TEventCtrl.h"
#include "sound/SoundCtrl.h"
#include "object/ObjShadow.h"
#include "object/TObjGoldenEgg.h"
#include "object/TObjChickenBoss.h"
#include "object/TObjLauncher.h"
#include "camera/TCameraCtrl.h"
#include "stage/TGameState.h"
#include "core/Pad.h"
#include "engine/FxSystem.h"
#include "sound/Sound3DPlay.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "stage/SetSlot.h"
#include "ui/MessageTable.h"
#include "core/VecMath.h"
#include "engine/TFrameBlur.h"
#include "core/TMainTask.h"
struct TParticleGenerator;
struct SetLink;

class TChickenBossDemo;

/** One 2D vertex for njDrawPolygon2DExSetData */
struct ChickenFadeVtx {
	f32 x, y, z;
	u32 col;
};

/** Message 0x70 a player asks to talk */
struct ChickenTalkMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mFrom; // _04 the player's id
};

/** Message 0x8A asks how far away it can be talked to */
struct ChickenRangeMsg {
	u16 mType;  // _00
	f32 mRange; // _04
};

enum ChickenBossState {
	CHICKEN_HATCH,    // 0 rising out of the golden egg
	CHICKEN_RISE,     // 1 the rise motion and the hatch camera path
	CHICKEN_DAYBREAK, // 2 waits for TChickenBossDemo
	CHICKEN_SCRIPT,   // 3 runs event script 0x7D1
	CHICKEN_4,        // 4
	CHICKEN_5,        // 5
	CHICKEN_6,        // 6
	CHICKEN_IDLE,     // 7 solid only while no egg ring is held
	CHICKEN_TURN,     // 8 turns to the player who asked to talk
	CHICKEN_GREET,    // 9
	CHICKEN_TALK,     // 10 shows the text until it closes
};

enum ChickenBossFlag {
	CHICKEN_DRAW_GOLDEN = 1, // draws the golden egg at mGoldenMtx
	CHICKEN_DRAW_MODEL  = 2, // draws the elder
	CHICKEN_IN_DEMO     = 4, // states 0-2
	CHICKEN_GREET_SOUND = 8, // the greeting sound has played
};

TArc* sChickenBossArc;            // 0x802C50A8 ar_event_presbyter.arc
NJS_MOTION** sChickenBossMotions; // 0x802C50AC its "motions" the elder's then the golden egg's at 9
void** sChickenBossCamPath;       // 0x802C50B0 its "event_cam" the camera object then its motion

typedef char TObjChickenBoss_size_check[(sizeof(TObjChickenBoss) == 0x298) ? 1 : -1];

const f32 gChickenBossSetRange = 1000.0f;

const char* gTObjChickenBossName = "TObjChickenBoss";

const MotionEntry sChickenBossMotionTable[] = {
	{ 1, 8, -1, 0, 0.0f, 399.0f, 1.0f, 0.0f, 0 }, { 1, 4, -1, 0, 0.0f, 60.0f, 0.25f, 0.0f, 0 },
	{ 0, 0, -1, 0, 0.0f, 30.0f, 0.3f, 0.0f, 0 },  { 1, 2, -1, 0, 0.0f, 39.0f, 0.3f, 0.0f, 0 },
	{ 0, 3, -1, 0, 0.0f, 20.0f, 0.2f, 0.0f, 0 },  { 1, 1, -1, 0, 0.0f, 60.0f, 0.2f, 0.0f, 0 },
	{ 1, 5, -1, 0, 0.0f, 20.0f, 0.2f, 0.0f, 0 },  { 0, 7, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
	{ 0, 6, -1, 0, 0.0f, 20.0f, 0.25f, 0.0f, 0 },
};

// 0x800884C0
extern "C" void initChickenBoss()
{
	loadArc("ar_event_presbyter.arc", &sChickenBossArc);
	sChickenBossMotions = (NJS_MOTION**)findArcExport(sChickenBossArc, "motions");
	sChickenBossCamPath = (void**)findArcExport(sChickenBossArc, "event_cam");
}

// 0x80088510
extern "C" void termChickenBoss()
{
	releaseResource(sChickenBossArc);
	sChickenBossArc     = 0;
	sChickenBossMotions = 0;
	sChickenBossCamPath = 0;
}

// 0x80088544
extern "C" TObject* createTObjChickenBoss(TObject* parent, SetDesc* desc)
{
	return new TObjChickenBoss(parent, desc);
}

// 0x80088598
extern "C" TObjChickenBoss* chickenBossHatch(const Vec3f* pos)
{
	return new TObjChickenBoss(gTL_05, pos);
}

// 0x800885E8
void TObjChickenBoss::initFromDesc(SetDesc* desc)
{
	int i;

	TSetActor::initFromDesc(desc);
	for (i = 0; i < 4; i++) {
		mParam[i] = desc->getInt(i);
		mValue[i] = desc->getFloat(i);
	}
	calcMatrix();
	makeShape();
}

// 0x8008867C
void TObjChickenBoss::debugPrint(void*)
{
	njPrintC(0xA0013, ":MSG ID");
}

// 0x800886A8
TObjChickenBoss::TObjChickenBoss(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
    , mFromSet(1)
{
	_00      = (u32)gTObjChickenBossName;
	mSize    = 0x298;
	mModel   = (NJS_OBJECT*)findGeObjData("MODEL_CHICKEN_BOSS");
	mTexList = (NJS_TEXLIST*)findGeObjData("TLS_MODEL_CHICKEN_BOSS");
	if (mModel == 0) {
		mModel   = getStgObjModels()[14];
		mTexList = getStgObjTextures()[13];
	}
	mDraw  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xF6, 0xF6);
	initFromDesc(desc);
	start();
}

// 0x800887D4
TObjChickenBoss::TObjChickenBoss(TObject* parent, const Vec3f* pos)
    : TSetActor(parent)
    , mFromSet(0)
{
	syMemSet(mParam, 0, sizeof(mParam));
	syMemSet(mValue, 0, sizeof(mValue));
	_00      = (u32)gTObjChickenBossName;
	mSize    = 0x298;
	mModel   = (NJS_OBJECT*)findGeObjData("MODEL_CHICKEN_BOSS");
	mTexList = (NJS_TEXLIST*)findGeObjData("TLS_MODEL_CHICKEN_BOSS");
	mPos.x   = pos->x;
	mPos.y   = pos->y;
	mPos.z   = pos->z;
	mRot.x   = 0;
	mRot.y   = 0;
	mRot.z   = 0;
	mDraw    = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mShape   = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xF6, 0xF6);
	makeShape();
	startHatch();
}

// 0x80088914
TObjChickenBoss::~TObjChickenBoss()
{
	if (mSound != 0) {
		seStop(mSound);
	}
	fxRemove(mFx);
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDraw);
}

// 0x800889C8
// a timed particle that starts after delay frames
extern "C" TParticleGenerator* chickenBossParticle(const Vec3f* pos, int id, int delay, int count)
{
	return particleGenTimedByIdDefault(pos, 0, id, delay, count);
}

// 0x80088A00
void TObjChickenBoss::reset()
{
	TArc* arc;

	arc = goldenEggGetArc();
	if (arc != 0) {
		mGoldenModels   = (NJS_OBJECT**)findArcExport(arc, "models");
		mGoldenTexLists = (NJS_TEXLIST**)findArcExport(arc, "texlists");
	} else {
		mGoldenModels   = 0;
		mGoldenTexLists = 0;
	}
	if (mGoldenModels != 0) {
		mGoldenModel = mGoldenModels[0];
		calcGolden();
	} else {
		mGoldenModel = 0;
		mtxCopy(mGoldenMtx, mMatrix);
	}
	mFrameCount   = 0;
	mConversation = 0;
	mText         = 0;
	mFlags        = 0;
	mCamPath      = 0;
	_21C          = 0;
	mGoldenFrame  = 0.0f;
	EggLevel level;
	level.setType(0x34);
	level.feed(0, 0x400);
	mGoldenScale = 0.2f * level.getRadius();
	mFx          = 0;
	_28C         = 0.0f;
	calcMatrix();
	mtxCopy(mHomeMtx, mMatrix);
	mSound = -1;
	setLinkFree((SetLink*)&mLink);
	mMotion.init(sChickenBossMotionTable, sChickenBossMotions);
	mObj.setModelNodes(mModel, 0x5A);
	mObj.mTexList = mTexList;
	mObj.mState   = (DrawState*)getDefaultDrawState();
	mObj.mMotion  = &mMotion;
	mObj.calcMotion();
	calcNodes();
	shadowSetGround(&mShadow, &mPos, 0);
	mShadow._30 = 14.0f;
	mShadow._34 = 14.0f;
	shadowEnable(&mShadow);
}

// 0x80088B98
void TObjChickenBoss::start()
{
	reset();
	mFlags |= CHICKEN_DRAW_MODEL;
	setState(CHICKEN_IDLE);
	mMotion.play(4);
}

// 0x80088BF4
// the hatch event's start fades the music out and plays the hatch tune
extern "C" int chickenBossEventStart(int id)
{
	TObjChickenBoss* boss = (TObjChickenBoss*)findObjById(id);
	if (boss != 0) {
		bgmFadeTo(0.0f, 0xB4);
		boss->mSound = sePlayFor(0x29F, 0);
		return 1;
	}
	return 0;
}

// 0x80088C50
void TObjChickenBoss::startHatch()
{
	reset();
	mFlags |= CHICKEN_DRAW_GOLDEN;
	mCamPath = createCamPath(0, 3, sChickenBossCamPath[0], sChickenBossCamPath[1], 0, 0.0f, 1.0f);
	if (mCamPath != 0) {
		((TCamera*)mCamPath)->setCut();
		mCamPath->clearAutoDelete();
		mCamPath->attachToMtx();
		mtxSetTransV(mCamPath->getMtx(), &mPos);
	}
	mFx = fxSpawnFollow(1, 7, 0x27, mGoldenMtx);
	eventStartById(mObjId, chickenBossEventStart);
	setState(CHICKEN_HATCH);
}

// 0x80088D20
void TObjChickenBoss::makeShape()
{
	ColliShape shape;
	colliShapeMakeCylinder(&shape, &mPos, 1, 15.0f, 20.0f);
	shape.mFlags |= 0x200;
	colliCopyShapes(mShape, &shape);
}

// 0x80088D78
void TObjChickenBoss::calcMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
}

// 0x80088DBC
void TObjChickenBoss::calcNodes()
{
	f32 base[12];
	ObjPlace* nodes = mObj.mNodes;
	mtxSetTransV(base, &mPos);
	mtxConcat(mNodeMtx[0], base, nodes[0].mMatrix);
	mtxConcat(mNodeMtx[1], base, nodes[8].mMatrix);
	mtxConcat(mNodeMtx[2], base, nodes[38].mMatrix);
	mtxTranslate(mNodeMtx[2], mNodeMtx[2], 0.0f, 0.0f, -8.0f);
}

// 0x80088E3C
void TObjChickenBoss::calcGolden()
{
	njPushMatrix(mMatrix);
	njStartMotionObj(sChickenBossMotions[9], mGoldenFrame);
	njMotionTransformEx(mGoldenModel);
	njGetMatrix(mGoldenMtx);
	njPopMatrixEx();
}

// 0x80088E90
int TObjChickenBoss::sendToHandle(ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x40:
		return mState == CHICKEN_IDLE ? 0x40 : 0;
	case 0x70: {
		u16 from        = ((ChickenTalkMsg*)msg)->mFrom;
		TObjAction* obj = findObjById(from);
		if (obj != 0) {
			Vec3f* at  = obj->getPos();
			mFaceAngle = (u16)angleToXZ(&mPos, at);
		} else {
			return 0;
		}
		if (eventStartByIds(mObjId, from) == 0) {
			return 0;
		}
		mTalker.setId(from);
		setState(CHICKEN_TURN);
		return 1;
	}
	case 0x8A:
		((ChickenRangeMsg*)msg)->mRange = 20.0f;
		return 1;
	}
	return 0;
}

// 0x80088F94
void TObjChickenBoss::draw()
{
	if ((isInRange(this, &mPos, 800.0f) || eventIsBusy()) && !objDebugEnabled(this)) {
		switch (mState) {
		case CHICKEN_HATCH:
			stateHatch();
			break;
		case CHICKEN_RISE:
			stateRise();
			break;
		case CHICKEN_DAYBREAK:
			stateDaybreak();
			break;
		case CHICKEN_SCRIPT:
			stateScript();
			break;
		case CHICKEN_IDLE:
			stateIdle();
			break;
		case CHICKEN_4:
			state4();
			break;
		case CHICKEN_5:
			state5();
			break;
		case CHICKEN_6:
			state6();
			break;
		case CHICKEN_TURN:
			stateTurn();
			break;
		case CHICKEN_GREET:
			stateGreet();
			break;
		case CHICKEN_TALK:
			stateTalk();
			break;
		}
		mObj.calcMotion();
		mFrameCount++;
	}
}

// 0x800890A4
int TObjChickenBoss::setState(int state)
{
	switch (state) {
	case CHICKEN_HATCH:
		mMotion.play(0);
		break;
	case CHICKEN_IDLE:
		mMotion.playBlend(4, 0.05f);
		break;
	case CHICKEN_TURN:
		mMotion.playBlend(7, 0.05f);
		break;
	case CHICKEN_GREET:
		mMotion.playBlend(6, 0.05f);
		break;
	case CHICKEN_TALK: {
		mText = createSimpleTextPlain(getMessage(mParam[0]));
		if (mText == 0) {
			return 0;
		}
		Vec3f focus = mPos;
		focus.y += 40.0f;
		mConversation = createTCamConversation(0, 2, &focus, -0xAAA, 0x1555);
		mMotion.playBlend(8, 0.05f);
		break;
	}
	}
	switch (state) {
	case CHICKEN_HATCH:
	case CHICKEN_RISE:
	case CHICKEN_DAYBREAK:
		mFlags |= CHICKEN_IN_DEMO;
		break;
	default:
		mFlags &= ~CHICKEN_IN_DEMO;
		break;
	}
	mState = state;
	return 1;
}

// 0x80089200
void TObjChickenBoss::stateHatch()
{
	TParticleGenerator* gen;

	mGoldenFrame += 1.0f;
	if ((int)(mGoldenFrame + 0.5f) == 100) {
		fxStop(mFx);
		mFx = 0;
		bgmStop();
		mFlags &= ~CHICKEN_DRAW_GOLDEN;
		mFlags |= CHICKEN_DRAW_MODEL;
		gen          = chickenBossParticle(&gVecZero, 0x46, 0x56, 1);
		gen->mAttach = mNodeMtx[0];
		gen          = chickenBossParticle(&gVecZero, 0x17C, 0x2B, 1);
		gen->mAttach = mNodeMtx[1];
		gen          = chickenBossParticle(&gVecZero, 0x17C, 0x44, 1);
		gen->mAttach = mNodeMtx[1];
		gen          = chickenBossParticle(&gVecZero, 0x17E, 1, 0x14);
		gen->mAttach = mNodeMtx[1];
		gen          = chickenBossParticle(&gVecZero, 0x17F, 0xF0, 1);
		gen->mAttach = mNodeMtx[2];
		gen          = chickenBossParticle(&gVecZero, 0x17F, 0x10E, 1);
		gen->mAttach = mNodeMtx[2];
		gen          = chickenBossParticle(&gVecZero, 0x17F, 0x131, 1);
		gen->mAttach = mNodeMtx[2];
		gen          = chickenBossParticle(&gVecZero, 0x195, 0xBE, 0xB4);
		gen->mAttach = mNodeMtx[1];
		fxSpawnAt(1, 7, 0x21, mGoldenMtx);
		setState(CHICKEN_RISE);
	}
	calcGolden();
}

// 0x800893AC
void TObjChickenBoss::stateRise()
{
	switch ((int)(mMotion.mFrame + 0.5f)) {
	case 0x2B:
	case 0x44:
		break;
	case 0xF0:
	case 0x10E:
		padVibStart(0, (int)this, 0xFF, 0, 0x1B);
		frameBlurStart(10, 0.8f);
		break;
	case 0x131:
		padVibStart(0, (int)this, 0xFF, 0, 0x1C);
		frameBlurStart(10, 0.8f);
		break;
	}
	if (mMotion.isDone() && mCamPath->isDone()) {
		mCamPath->mFlags |= 1;
		mDemo = createChickenBossDemo(this, 0);
		if (mDemo != 0) {
			setState(CHICKEN_DAYBREAK);
		} else {
			eventEnd();
			setState(CHICKEN_IDLE);
		}
	}
	mMotion.update();
	calcNodes();
}

// 0x800894F0
void TObjChickenBoss::stateDaybreak()
{
	if (mDemo->isDone()) {
		if (mDemo != 0) {
			mDemo->mFlags |= 1;
			mDemo = 0;
		}
		seStop(mSound);
		mSound = -1;
		eventEnd();
		setState(CHICKEN_SCRIPT);
	}
}

// 0x80089568
void TObjChickenBoss::stateScript()
{
	eventScriptRequest(0x7D1);
	setState(CHICKEN_IDLE);
}

// 0x800895A4
void TObjChickenBoss::stateIdle()
{
	if (EggRingContent::getHeldCount() != 0) {
		colliDisable(mShape);
	} else {
		colliEnable(mShape);
	}
	mMotion.update();
}

// 0x800895F4
void TObjChickenBoss::state4()
{
}

// 0x800895F8
void TObjChickenBoss::state5()
{
}

// 0x800895FC
void TObjChickenBoss::state6()
{
}

// 0x80089600
void TObjChickenBoss::stateTalk()
{
	ObjMsg done;
	ObjMsg end;
	ObjMsg resume;

	if (mText != 0 && mText->mState == 3) {
		mText->mHold       = 1;
		mText              = 0;
		TObjAction* talker = mTalker.get();
		if (talker != 0) {
			done.mType = 0x30;
			talker->sendToHandle(&done);
		}
		if (mConversation != 0) {
			mConversation->mFlags |= 1;
			mConversation = 0;
		}
		eventEnd();
		end.mType = 0x8D;
		sendGameMsg(&end);
		resume.mType = 0x94;
		sendGameMsg(&resume);
		setState(CHICKEN_IDLE);
	}
	mMotion.update();
}

// 0x800896D4
void TObjChickenBoss::stateTurn()
{
	int angle = angleApproach(mRot.y, mFaceAngle, 0x200);
	if (angle == mFaceAngle) {
		setState(CHICKEN_GREET);
	}
	mRot.y = angle;
	mMotion.update();
	calcMatrix();
}

// 0x8008973C
void TObjChickenBoss::stateGreet()
{
	if (mMotion.isDone()) {
		mFlags &= ~CHICKEN_GREET_SOUND;
		setState(CHICKEN_TALK);
	} else {
		mMotion.update();
	}
	if (!(mFlags & CHICKEN_GREET_SOUND) && (int)(mMotion.mFrame + 0.5f) == 8) {
		sePlay3D(0x64, &mPos);
		mFlags |= CHICKEN_GREET_SOUND;
	}
}

// 0x800897DC
// hides it while an event runs or while its shape is off screen
void TObjChickenBoss::vfn_10()
{
	if (eventIsBusy()) {
		mDraw->mFlags &= ~1;
		return;
	}
	ColliShape* shape = colliGetShape(mShape, 0);
	Vec3f center;
	center.set(shape->mPos.x, shape->mPos.y, shape->mPos.z);
	if (cameraIsVisible(1000.0f, 1.5f * colliGetShape(mShape, 0)->mSizeY, &center, 0)) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x8008989C
void TObjChickenBoss::render()
{
	if (mFlags & CHICKEN_DRAW_MODEL) {
		shadowAlphaBegin();
		njPushMatrixEx();
		njMultiMatrix(0, mMatrix);
		mObj.draw();
		njPopMatrixEx();
		shadowAlphaEnd();
	}
	if (mFlags & CHICKEN_DRAW_GOLDEN) {
		drawGolden();
	}
}

// 0x80089908
// the draw object's texture slot always texture 0
extern "C" void chickenBossTexNum(int, int map, int slot, int flags)
{
	gjLoadTextureNum(0, map, slot, flags);
}

// 0x8008992C
// the extra stage that adds the sheen
extern "C" void chickenBossTev(int stage)
{
	u8 s = stage;
	GXSetTevColorIn(s, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
	GXSetTevColorOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(s, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(s, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
}

// 0x800899D0
extern "C" void chickenBossTexGen(u8 stage)
{
	GXSetTexCoordGen2(stage, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX5, GX_TRUE, GX_PTTEXMTX19);
}

// 0x80089A08
// the golden egg he hatches from drawn the way TObjGoldenEgg draws itself
void TObjChickenBoss::drawGolden()
{
	njSetTexture(mGoldenTexLists[0]);
	gjLoadTextureNum(0, 0, 0, 0);
	gjLoadTexture(mGoldenTexLists[2], 0, 0, 0, 1);
	njPushMatrixEx();
	njMultiMatrix(0, mGoldenMtx);
	njScale(0, mGoldenScale, mGoldenScale, mGoldenScale);

	void* funcs[8];
	f32 tex[12];
	funcs[7] = (void*)chickenBossTexNum;
	gjChangeDrawObjIF(funcs, 0x10);
	PSMTXCopy(_gj_matrix_, tex);
	tex[3]  = 0.0f;
	tex[7]  = 0.0f;
	tex[11] = 0.0f;
	GXLoadTexMtxImm(tex, GX_TEXMTX5, GX_MTX3x4);
	gjSetNumPostTexGens(1);
	gjSetNumPostTevStages(1);
	gjSetPostTevStageCallback((void*)chickenBossTev);
	gjSetPostTexGenCallback((void*)chickenBossTexGen);
	_nj_control_3d_flag_ |= 0x80000;
	shadowAlphaBegin();
	gjDrawObject(mGoldenModel);
	shadowAlphaEnd();
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	_nj_control_3d_flag_ &= ~0x80000;
	funcs[7] = 0;
	gjChangeDrawObjIF(funcs, 0x10);
	njPopMatrixEx();
}

// 0x80089B4C
extern "C" TChickenBossDemo* createChickenBossDemo(TObject* parent, int autoDelete)
{
	return new TChickenBossDemo(parent, autoDelete);
}

// 0x80089BA0
TChickenBossDemo::TChickenBossDemo(TObject* parent, int autoDelete)
    : TObject(parent)
{
	mFx         = 0;
	mPhase      = 0;
	mAutoDelete = autoDelete;
	mtxIdentity(mMtx);
	mDraw           = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3A, 0);
	mDraw->mSortKey = drawTaskDepthKey(0.0f);
	mPath           = eventCamArcPlayNamed(1.0f, 0.0f, "daybreak");
	if (mPath != 0) {
		((TCamera*)mPath)->setCut();
	}
}

// 0x80089C50
TChickenBossDemo::~TChickenBossDemo()
{
	fxRemove(mFx);
	if (mPath != 0) {
		mPath->mFlags |= 1;
		mPath = 0;
	}
	removeDrawTask(mDraw);
}

// 0x80089CE0
int TChickenBossDemo::isDone()
{
	if (mPath != 0) {
		return mPath->isDone();
	}
	return 1;
}

// 0x80089D14
void TChickenBossDemo::draw()
{
	if ((mPath != 0 && mPath->isDone()) || mPath == 0) {
		if (mAutoDelete != 0) {
			mFlags |= 1;
			return;
		}
		return;
	}
	switch ((int)(mPath->getFrame() + 0.5f)) {
	case 0:
		mPhase = 0;
		mFade  = 0.0f;
		setLandscapeBlend(0.0f, 0xF0);
		if (mFx == 0) {
			mFx = fxSpawnFollow(1, 7, 0x16, mMtx);
		}
		break;
	case 0x7D:
		mPhase = 1;
		mFade  = 0.0f;
		setLandscapeBlend(1.0f, 0);
		setLandscapeBlend(0.0f, 0xF0);
		break;
	case 0x10E:
		mPhase = 2;
		mFade  = 0.0f;
		setLandscapeBlend(1.0f, 0);
		setLandscapeBlend(0.0f, 0xF0);
		fxRemove(mFx);
		mFx = 0;
		break;
	}
	mFade += 1.0f / 240.0f;
	if (mFade > 1.0f) {
		mFade = 1.0f;
	}
	CamView* view = camGetDrawView();
	if (view != 0) {
		Vec3f pos = view->mPos;
		vecScaleAdd(70.0f, &pos, &view->mFront, &pos);
		vecScaleAdd(50.0f, &pos, &view->mSide, &pos);
		vecScaleAdd(35.0f, &pos, &view->mUp, &pos);
		mtxSetTransV(mMtx, &pos);
	}
}

// 0x80089EF0
void TChickenBossDemo::vfn_10()
{
}

static inline u32 chickenARGB(u32 rgba)
{
	return (rgba << 24) | (rgba >> 8);
}

static inline u32 chickenRGBA(u32 r, u32 g, u32 b, u32 a)
{
	return (r << 24) | (g << 16) | (b << 8) | a;
}

// 0x80089EF4
// a wash over the screen that rises and falls with mFade
void TChickenBossDemo::render()
{
	f32 x;
	f32 y;
	f32 w;
	f32 h;
	u32 inner;
	u32 outer;
	f32 t;

	getViewport(&x, &y, &w, &h);
	static ColorValue sFrom(chickenRGBA(0, 0, 0, 0xFF));
	static ColorValue sTo(chickenRGBA(0xFF, 0xFF, 0xFF, 0xFF));
	if (mFade < 0.5f) {
		t = 2.0f * mFade;
	} else {
		t = 2.0f * (1.0f - mFade);
	}
	lerpColor(0.1f * t, &inner, &sFrom.mValue, &sTo.mValue);
	lerpColor(0.8f * t, &outer, &sFrom.mValue, &sTo.mValue);
	ChickenFadeVtx quad[4] = {
		{ 0.0f, 0.0f, 0.5f, 0 },
		{ 0.0f, 0.0f, 0.5f, 0 },
		{ 0.0f, 0.0f, 0.5f, 0 },
		{ 0.0f, 0.0f, 0.5f, 0 },
	};
	quad[0].x   = x;
	quad[0].y   = y + h;
	quad[0].col = chickenARGB(inner);
	quad[1].x   = x;
	quad[1].y   = y;
	quad[1].col = chickenARGB(outer);
	quad[2].x   = x + w;
	quad[2].y   = y + h;
	quad[2].col = chickenARGB(inner);
	quad[3].x   = x + w;
	quad[3].y   = y;
	quad[3].col = (mPhase == 2) ? chickenARGB(outer) : chickenARGB(inner);
	njColorBlendingMode(0, 10);
	njColorBlendingMode(1, 10);
	pushCullMode(0);
	njDrawPolygon2DExStart(1);
	njDrawPolygon2DExSetData((const NJS_POLYGON_VTX*)quad, (s32)4);
	njDrawPolygon2DExEnd();
	popCullMode();
	njColorBlendingMode(0, 8);
	njColorBlendingMode(1, 6);
}

// 0x8008A0C4
extern "C" void camLinkInit(CamLink* link, int look)
{
	link->mPrev  = 0;
	link->mNext  = 0;
	link->mLook  = look;
	link->mFlags = 0;
}

// 0x8008A0DC
extern "C" int camLinkIsAbove(const CamLink* link, const CamLink* other)
{
	return (u8)(link->mLook >= other->mLook);
}

// 0x8008A0FC
extern "C" int camLinkIsPicked(const CamLink* link)
{
	return link->mFlags & 1;
}

// 0x8008A108
extern "C" int camLinkIsEntered(const CamLink* link)
{
	return link->mFlags & 2;
}

// 0x8008A114
extern "C" void camLinkSetCut(CamLink* link)
{
	link->mFlags |= 8;
}

// 0x8008A124
extern "C" int camLinkIsCut(const CamLink* link)
{
	return link->mFlags & 0x10;
}

// 0x8008A130
// marks and returns the link with the highest look
extern "C" CamLink* camLinkPickBest(CamLink* head)
{
	CamLink* link;
	CamLink* best;

	if (head == 0) {
		return 0;
	}
	best = head;
	head->mFlags &= ~1;
	for (link = head->mNext; link != 0; link = link->mNext) {
		link->mFlags &= ~1;
		if (camLinkIsAbove(link, best)) {
			best = link;
		}
	}
	best->mFlags |= 1;
	return best;
}

// 0x8008A1BC
extern "C" void camAimClear(CamAim* aim)
{
	aim->mHead    = 0;
	aim->mCurrent = 0;
	aim->mFlags   = 0;
}

// 0x8008A1D0
// adds the link at the end
extern "C" void camAimAdd(CamAim* aim, CamLink* link)
{
	CamLink* last;

	aim->mFlags |= 1;
	link->mNext = 0;
	if (aim->mHead == 0) {
		aim->mHead  = link;
		link->mPrev = 0;
		return;
	}
	for (last = aim->mHead; last->mNext != 0; last = last->mNext) { }
	last->mNext = link;
	link->mPrev = last;
}

// 0x8008A220
extern "C" void camAimRemove(CamAim* aim, CamLink* link)
{
	aim->mFlags |= 1;
	if (link->mPrev != 0) {
		link->mPrev->mNext = link->mNext;
	}
	if (link->mNext != 0) {
		link->mNext->mPrev = link->mPrev;
	}
	if (aim->mHead == link) {
		aim->mHead = link->mNext;
	}
	if (aim->mCurrent == link) {
		if (aim->mCurrent->mFlags & 8) {
			aim->mFlags |= 4;
		} else {
			aim->mFlags &= ~4;
		}
		aim->mCurrent = 0;
	}
	camLinkInit(link, 0);
}

// 0x8008A2CC
extern "C" void camAimUpdate(CamAim* aim)
{
	CamLink* link;
	CamLink* best;

	if (aim->mFlags & 2) {
		aim->mFlags &= ~2;
		for (link = aim->mHead; link != 0; link = link->mNext) {
			link->mFlags &= ~0x16;
		}
	}
	if (aim->mFlags & 1) {
		aim->mFlags &= ~1;
		best = camLinkPickBest(aim->mHead);
		if (best != aim->mCurrent) {
			aim->mFlags |= 2;
			if (aim->mCurrent != 0) {
				aim->mCurrent->mFlags |= 4;
			}
			if (best != 0) {
				best->mFlags |= 2;
				if (aim->mFlags & 4) {
					aim->mFlags &= ~4;
					best->mFlags |= 0x10;
				}
			}
		}
		aim->mCurrent = best;
	}
}

// 0x8008A3B0
extern "C" TCamFix* createTCamFix(int player, int look, const Vec3f* at, const Vec3f* eye)
{
	return new TCamFix(gTL_01, player, look, at, eye);
}
