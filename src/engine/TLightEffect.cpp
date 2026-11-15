#include "enemy/boss/EneBoss.h"
#include "io/Resource.h"
#include "Ninja/ninja.h"
#include "engine/MotionPose.h"
#include "engine/Light.h"
#include "engine/TLightEffect.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "ui/AeArc.h"
#include "object/TObjEggRing.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "object/TObjEggSuit.h"
#include "object/ObjAction.h"
#include "stage/SetManager.h"
#include "engine/CharaColli.h"
#include "object/ObjCommon.h"
#include "engine/DrawTask.h"
#include "camera/TCamera.h"
#include "ui/TSimpleText.h"
#include "engine/Color.h"
#include "player/Player.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "stage/TEventCtrl.h"
#include "enemy/TEneGreenDemo.h"
#include "io/NinjaFile.h"
#include "object/ObjShadow.h"
#include "stage/EventScript.h"
#include "object/TObjLauncher.h"
#include "engine/Colli.h"
#include "engine/ActorMotion.h"
#include "camera/TCameraCtrl.h"
#include "stage/SetSlot.h"
#include "sound/SoundCtrl.h"
#include "ui/MessageTable.h"
#include "object/TObjPlayer.h"
struct Fx;
struct NJS_TEXLIST;

extern "C" {
const char* gTLightEffectName = "TLightEffect";
}

typedef char TLightEffect_size_check[(sizeof(TLightEffect) == 0x10C) ? 1 : -1];

// 0x801837E8
TLightEffect::TLightEffect(TObject* parent, void* motion, int effect, const f32* matrix, const u32* color)
    : TObject(parent)
    , mEffect(0)
    , mMotion(motion)
    , mEffectId(effect)
    , mFrame(0)
{
	_00   = (u32)gTLightEffectName;
	mSize = 0x10C;
	mtxCopy(mBase, matrix);
	mtxCopy(mMatrix, matrix);
	Vec3f pos;
	mtxGetPos(matrix, &pos);
	mLight.mFlags = 2;
	mLight.mKa[0] = 1.0f;
	mLight.mKa[1] = 0.0f;
	mLight.mKa[2] = 0.0f;
	mLight.mColor = *color;
	mLight.mPos   = pos;
}

// 0x801838C0
TLightEffect::~TLightEffect()
{
	playerRemoveLight(0, &mLight);
	fxStop((Fx*)mEffect);
}

// 0x8018395C
void TLightEffect::draw()
{
	MotionNodePose at;
	NJS_OBJECT pose;
	motionObjInit(&pose);
	motionGetNodePose(&at, &pose, mMotion, 0.5f * (f32)mFrame);
	mtxSetTrans(mMatrix, at.mPos.x, at.mPos.y, at.mPos.z);
	mtxConcat(mMatrix, mBase, mMatrix);
	Vec3f pos;
	mtxGetPos(mMatrix, &pos);
	mLight.mPos = pos;
	if (mFrame == 0x6E) {
		playerAddLight(0, &mLight);
		mEffect = fxSpawnFollow(1, 0, mEffectId, mMatrix);
	} else if (mFrame == 0x17C) {
		mFlags |= 1;
	}
	mFrame++;
}

// 0x80183A60
extern "C" int lightEffectLoad()
{
	lightEffectLoadArc();
	return 1;
}
// 0x80183A84
extern "C" void lightEffectLoadArc()
{
	loadArc("egg_suit.arc", &gEggSuitArc);
	if (gEggSuitArc != 0) {
		gEggSuitModels   = (NJS_OBJECT**)findArcExport(gEggSuitArc, "models");
		gEggSuitTexlists = (NJS_TEXLIST**)findArcExport(gEggSuitArc, "texlists");
		void** textures  = (void**)findArcExport(gEggSuitArc, "textures");
		bindGvmTextures((u8*)*textures, gEggSuitTexlists[0]);
		bindGvmTextures((u8*)*textures, gEggSuitTexlists[1]);
		bindGvmTextures((u8*)*textures, gEggSuitTexlists[2]);
	}
}

struct NJS_OBJECT;
struct SetLink;
struct NJS_TEXLIST;
struct Fx;

/** The colours of the three lights */
static u32 sLightRed    = ColorValue(0xFF0000FF).mValue;
static u32 sLightBlue   = ColorValue(0x0000FFFF).mValue;
static u32 sLightYellow = ColorValue(0xFFFF00FF).mValue;

extern "C" {
TArc* gEggSuitArc;              // egg_suit.arc loaded by lightEffectLoadArc
NJS_OBJECT** gEggSuitModels;    // its "models"
NJS_TEXLIST** gEggSuitTexlists; // its "texlists" the ball unused the environment map
}

/** What the player's vfn_BC takes */
struct EggSuitPlayerMotions {
	int mMotions[6];
};

typedef char TObjEggSuit_size_check[(sizeof(TObjEggSuit) == 0xD4) ? 1 : -1];

static const char* sClassName = "TObjEggSuit";

// 0x80183B24
extern "C" void initTObjEggSuit()
{
	gEggSuitArc      = 0;
	gEggSuitModels   = 0;
	gEggSuitTexlists = 0;
}

// 0x80183B38
extern "C" void termTObjEggSuit()
{
	if (gEggSuitArc != 0) {
		for (int i = 0; i < 3; i++) {
			releaseTextures(gEggSuitTexlists[i]);
		}
		releaseResource(gEggSuitArc);
	}
	gEggSuitArc      = 0;
	gEggSuitModels   = 0;
	gEggSuitTexlists = 0;
}

// 0x80183BA4
extern "C" TObject* createTObjEggSuit(TObject* parent, SetDesc* desc)
{
	return new TObjEggSuit(parent, desc);
}

// 0x80183BF8
TObjEggSuit::TObjEggSuit(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
    , mMode(4)
{
	f32 mtx[12];

	mText = 0;
	_00   = (u32)sClassName;
	mSize = 0xD4;
	setObjId(this, desc->mObjId);
	mColli = colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, mObjId);
		colliSetMasks(mColli, 8, 0xFF, 0xF0, 0);
		colliShapeSetSphere(colliGetShape(mColli, 0));
		colliGetShape(mColli, 0)->mFlags = 0x400;
		colliGetShape(mColli, 0)->mSizeX = 7.5f;
	}
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mLink.get()->attach((SetDesc*)desc);
	initFromDesc((SetDesc*)desc);
	mtxSetTrans(mtx, mPos.x, 1.0f + (mPos.y - 7.5f), mPos.z);
	mEffect = fxSpawnAt(1, 8, 5, mtx);
	setMode(MODE_WAIT);
}

// 0x80183D80
TObjEggSuit::~TObjEggSuit()
{
	if (mText != 0) {
		mText->mHold = 1;
	}
	fxStop((Fx*)mEffect);
	removeDrawTask(mDraw);
	colliFree(mColli);
	notifySetLinkDeath(mLink.get(), this);
	unregisterId();
}

// 0x80183E38
// The ball floats 7.5 above its set point on a 15 x 15 shadow
void TObjEggSuit::initFromDesc(SetDesc* desc)
{
	Vec3f pos;
	SetRot rot;

	desc->getPos(&pos);
	desc->getRot(&rot);
	mEventId = desc->getInt(0);
	shadowSetGround(&mShadow, &pos, 0);
	mShadow._30 = 15.0f;
	mShadow._34 = 15.0f;
	shadowEnable(&mShadow);
	pos.y += 7.5f;
	mPos = pos;
	colliShapeSetPos(colliGetShape(mColli, 0), &pos);
	mRotY = rot.y;
	mtxSetTransV(mMtx, &pos);
}

// 0x80183F24
f32 TObjEggSuit::getRadiusV()
{
	return 7.5f;
}

// 0x80183F2C
void TObjEggSuit::debugPrint(void* arg)
{
	njPrint(0xA0013, ":EVENT ", mEventId);
}

// 0x80183F60
void TObjEggSuit::draw()
{
	if (debugEditActive() == 0) {
		runMode(mMode, PHASE_UPDATE);
		runMode(mMode, PHASE_LATE);
	}
}

// 0x80183FBC
// Not drawn while out of view
void TObjEggSuit::vfn_10()
{
	if (!cameraIsVisible(1000.0f, 7.5f, &mPos, 0)) {
		mDraw->mFlags |= 1;
	} else {
		mDraw->mFlags &= ~1;
	}
}

// 0x80184024
void TObjEggSuit::runMode(int mode, int phase)
{
	switch (mode) {
	case MODE_WAIT:
		modeWait(phase);
		break;
	case MODE_TOUCHED:
		modeTouched(phase);
		break;
	case MODE_DEMO:
		modeDemo(phase);
		break;
	case MODE_END:
		modeEnd(phase);
		break;
	}
}

// 0x80184094
void TObjEggSuit::setMode(int mode)
{
	if (mMode != -1) {
		runMode(mMode, PHASE_LEAVE);
	}
	if (mode != -1) {
		runMode(mode, PHASE_ENTER);
	}
	mMode = mode;
}

// 0x801840F4
// Sends 0x80 to whatever touches it
void TObjEggSuit::modeWait(int phase)
{
	if (phase == PHASE_UPDATE) {
		ObjMsg msg;
		ColliContact* contact;

		msg.mType = 0x80;
		for (contact = mColli->mContacts; contact != 0; contact = contact->mNext) {
			if (!(colliContactShape(contact)->mFlags & 0x10) && sendMsgById(colliContactOwner(contact), &msg)) {
				fxStop(mEffect);
				mEffect = 0;
				colliDisable(mColli);
				setMode(MODE_TOUCHED);
				return;
			}
		}
	}
}

// 0x80184198
void TObjEggSuit::modeTouched(int phase)
{
	if (phase == PHASE_UPDATE) {
		if (eventStartById(mObjId, 0)) {
			setMode(MODE_DEMO);
		}
	}
}

// 0x801841E8
// The camera follows the event's path and three lights fly off the ball
void TObjEggSuit::modeDemo(int phase)
{
	if (phase == PHASE_ENTER) {
		EggSuitEvent* event;
		f32 mtx[12];
		TObjPlayer* player;

		event  = (EggSuitEvent*)findArcExport(gEggSuitArc, "event");
		mEvent = event;
		mFrame = 0;
		sePlayFor(0x2BB, 0);
		mCam = createCamPath(0, 2, event->mCamObject, event->mCamMotion, 0, 0.0f, 0.5f);
		mCam->attachToMtx();
		mtxSetTrans(mtx, mPos.x, mPos.y - 7.5f, mPos.z);
		mtxCopy(mCam->getMtx(), mtx);
		new TLightEffect(this, event->mLightMotions[0], 0x40, mtx, &sLightRed);
		new TLightEffect(this, event->mLightMotions[1], 0x44, mtx, &sLightBlue);
		new TLightEffect(this, event->mLightMotions[2], 0x48, mtx, &sLightYellow);
		player = findPlayer(0);
		if (player != 0) {
			EggSuitPlayerMotions motions;

			motions.mMotions[0] = 0;
			motions.mMotions[1] = 0;
			motions.mMotions[2] = 0;
			motions.mMotions[3] = 0;
			motions.mMotions[4] = 0;
			motions.mMotions[5] = 0;
			motions.mMotions[0] = event->mPlayerMotions[0];
			motions.mMotions[1] = event->mPlayerMotions[1];
			motions.mMotions[2] = event->mPlayerMotions[2];
			motions.mMotions[4] = event->mPlayerMotions[3];
			motions.mMotions[3] = event->mPlayerMotions[4];
			((TChrPlayer*)player)->vfn_BC(motions.mMotions, mtx);
			eventHideObj((u8*)player);
		}
	} else if (phase == PHASE_LEAVE) {
		ObjMsg msg;

		if (mText != 0) {
			mText->mHold = 1;
			mText        = 0;
		}
		mCam->mFlags |= 1;
		msg.mType = 0x79;
		sendMsgById(0, &msg);
		eventShowId(0);
		eventEnd();
	} else if (phase == PHASE_UPDATE) {
		if (mText != 0 && mText->mState == 3) {
			setMode(MODE_END);
		}
	} else if (phase == PHASE_LATE) {
		if (mFrame < 0x70) {
			MotionNodePose pose;
			NJS_OBJECT obj;

			motionObjInit(&obj);
			motionGetNodePose(&pose, &obj, mEvent->mSuitMotion, 0.5f * (f32)mFrame);
			mtxSetTrans(mMtx, mPos.x, mPos.y - 7.5f, mPos.z);
			mtxTranslate((float*)mMtx, (const float*)mMtx, pose.mPos.x, pose.mPos.y, pose.mPos.z);
			mtxScale(pose.mScale.x, pose.mScale.y, pose.mScale.z, mMtx, mMtx);
		} else if (mFrame == 0x70) {
			Vec3f pos;
			f32 mtx[12];

			mtxGetPos(mMtx, &pos);
			mtxSetTransV(mtx, &pos);
			fxSpawnAt(1, 0, 0x3B, mtx);
			mFlags |= 0x10;
			shadowDisable(&mShadow);
		} else if (mFrame == 0x20A) {
			mText = createSimpleTextPlain(getMessage(0xE));
		}
		if (mFrame < 1000) {
			mFrame++;
		}
	}
}

// 0x80184530
// Waits for the event script then hands over to the green demo and goes for good
void TObjEggSuit::modeEnd(int phase)
{
	if (phase == PHASE_UPDATE) {
		if (eventScriptRequest(mEventId)) {
			setLinkForever((SetLink*)&mLink);
			greenDemoSpawn();
			mFlags |= 1;
		}
	}
}

// 0x80184588
// The model's own texture load always texture 0
static void suitLoadTexture(int, int map, int a, int b)
{
	gjLoadTextureNum(0, map, a, b);
}

// 0x801845AC
// A post stage that adds half the environment map
static void suitPostTev(u8 tev)
{
	int stage = tev;
	GXSetTevKColorSel(stage, GX_TEV_KCSEL_1_2);
	GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(stage, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
}

// 0x8018465C
// The environment map's coordinates from the normals
static void suitPostTexGen(u8 coord)
{
	GXSetTexCoordGen2(coord, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX5, GX_TRUE, GX_PTTEXMTX19);
}

// 0x80184694
// A wire sphere until the arc is in
void TObjEggSuit::render()
{
	f32 mtx[12];
	f32 env[12];
	void* funcs[8];

	mtxFromView(mtx, mMtx);
	if (gEggSuitArc == 0) {
		mtxScale(7.5f, 7.5f, 7.5f, mtx, mtx);
		debugWireSphere(mtx, &gColorBlack);
		return;
	}
	mtxScale(1.5f, 1.5f, 1.5f, mtx, mtx);
	njSetTexture(gEggSuitTexlists[0]);
	gjLoadTextureNum(0, 0, 0, 0);
	gjLoadTexture(gEggSuitTexlists[2], 0, 0, 0, 1);
	njPushMatrix(mtx);
	PSMTXCopy(_gj_matrix_, env);
	env[3]  = 0.0f;
	env[7]  = 0.0f;
	env[11] = 0.0f;
	GXLoadTexMtxImm(env, GX_TEXMTX5, GX_MTX3x4);
	funcs[7] = (void*)suitLoadTexture;
	gjChangeDrawObjIF(funcs, 0x10);
	gjSetNumPostTexGens(1);
	gjSetNumPostTevStages(1);
	gjSetPostTevStageCallback((void*)suitPostTev);
	gjSetPostTexGenCallback((void*)suitPostTexGen);
	_nj_control_3d_flag_ |= 0x80000;
	shadowAlphaBegin();
	gjDrawObject(gEggSuitModels[0]);
	shadowAlphaEnd();
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	u32 flags;
	flags    = _nj_control_3d_flag_;
	funcs[7] = 0;
	flags &= ~0x80000;
	_nj_control_3d_flag_ = flags;
	gjChangeDrawObjIF(funcs, 0x10);
	njPopMatrixEx();
}

// 0x80184810
int TObjEggSuit::getKind()
{
	return 3;
}

// 0x80184818
Vec3f* TObjEggSuit::getPos()
{
	return &mPos;
}

// 0x80184820
SetLink* TObjEggSuit::getLink()
{
	return mLink.get();
}
