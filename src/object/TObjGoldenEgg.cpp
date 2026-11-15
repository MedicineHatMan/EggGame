#include "object/ObjAction.h"

const char* gGoldenEggModeNames[2] = { "NORMAL", "PRISON" }; // for debugPrint
struct DrawTask;
#include "io/Resource.h"
#include "egg/ObjEgg.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "engine/TMobileObject.h"
#include "engine/TParticle.h"
#include "object/TObjGoldenEgg.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "io/NinjaFile.h"
#include "ui/AeArc.h"
#include "object/ObjShadow.h"
#include "object/TObjLauncher.h"
#include "engine/Colli.h"
#include "engine/FxSystem.h"
#include "sound/Sound3DPlay.h"
#include "camera/TCameraCtrl.h"
#include "stage/TGameState.h"
#include "object/TObjEggRing.h"
#include "core/MathUtil.h"
#include "egg/EggCreature.h"
#include "engine/GroundQuery.h"
#include "Ninja/ninja.h"
#include "stage/TStageScore.h"
#include "object/TObjKey00.h"
#include "stage/SetSlot.h"
#include "object/TObjChickenBoss.h"
#include "egg/EggLevel.h"
#include "ui/HatchNotice.h"
#include "engine/ActorMotion.h"
#include "object/TObjPlayer.h"
#include "core/TMainTask.h"
#include "core/VecMath.h"
#include "enemy/boss/EneBoss.h"
#include "camera/TCamera.h"
#include "engine/Viewport.h"
#include "sound/SoundCtrl.h"
#include "engine/TFrameBlur.h"
#include "egg/TGoldenEggBreak.h"
struct TObjChickenBoss;
struct NJS_TEXLIST;
struct NJS_OBJECT;
struct SetLink;
struct Fx;

extern "C" {

extern const MotionEntry sGoldenEggHatchMotions[] = { { 0, 1, -1, 0, 0.0f, 15.0f, 0.21f, 0.0f, 0 } };
const char* gTObjGoldenEggName                    = "TObjGoldenEgg";

TArc* sGoldenEggArc;
NJS_OBJECT** sGoldenEggModels;
NJS_TEXLIST** sGoldenEggTexLists;
NJS_OBJECT** sEggModels;  // 0x802C51E4 the plain egg's models for the hatch
NJS_MOTION** sEggMotions; // 0x802C51E8 the plain egg's motions
}

// What the hatch motion's walk fills in the matrix of its second node
struct GoldenEggWalk {
	int mCount;   // _00
	f32 mMtx[12]; // _04
};

typedef char TObjGoldenEgg_size_check[(sizeof(TObjGoldenEgg) == 0x314) ? 1 : -1];

// 0x800BEF78
extern "C" TArc* goldenEggGetArc()
{
	return sGoldenEggArc;
}

// 0x800BEF80
TObjGoldenEgg::TObjGoldenEgg(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
    , mState(GOLDEN_EGG_NONE)
    , mOwnerId(0xFFFF)
    , mAuraRoll(this)
    , mAuraSlow(this)
{
	_00   = (u32)gTObjGoldenEggName;
	mSize = 0x314;
	mMode = 0;
	setObjId(this, desc->mObjId);
	mShape = (CharaColli*)colliCreate(1, 1);
	if (mShape != 0) {
		colliSetOwner(mShape, (u16)mObjId);
		colliSetMasks(mShape, 8, 0xF3, 0xF0, 0);
		colliShapeSetSphere(colliGetShape(mShape, 0));
		colliGetShape(mShape, 0)->mFlags = 0x42C;
	}
	mDraw             = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mBody.mBounce     = 0.0f;
	mRoll.mOwner      = (TObjEgg*)this;
	mRoll.mPath       = &mCollide;
	mRoll.mBody       = &mBody;
	mHandle.mOwner    = this;
	mSparkle          = particleGenById(this, &gVecZero, 0, 0x181);
	mSparkle->mAttach = mRotMatrix;
	mSparkle->setScale(10.0f);
	mAuraRoll.mBody = &mBody;
	mAuraSlow.mBody = &mBody;
	mLevel.setType(0x34);
	applySize();
	mLink.get()->attach((SetDesc*)desc);
	initFromDesc((SetDesc*)desc);
	updateMatrix();
	if (mLink.get()->getData() == 0) {
		if (mMode == 0) {
			setState(GOLDEN_EGG_IDLE);
		} else if (mMode == 1) {
			setState(GOLDEN_EGG_ASLEEP);
		}
	} else {
		setState(GOLDEN_EGG_IDLE);
	}
}

// 0x800BF248
TObjGoldenEgg::~TObjGoldenEgg()
{
	runState(mState, EGG_EXIT);
	removeDrawTask(mDraw);
	colliFree(mShape);
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	unregisterId();
}

// 0x800BF370
extern "C" TObject* createTObjGoldenEgg(TObject* parent, SetDesc* desc)
{
	if (keyBitIsSet(500) == 0) {
		return new TObjGoldenEgg(parent, desc);
	}
	return (TObject*)createTObjChickenBoss(parent, desc);
}

// 0x800BF3E4
// Run when a stage starts takes the plain egg's models and motions for the hatch
extern "C" void initGoldenEgg()
{
	sGoldenEggArc    = 0;
	sGoldenEggModels = 0;
	if (gEggArc != 0) {
		sEggModels  = (NJS_OBJECT**)findArcExport(gEggArc, "models");
		sEggMotions = (NJS_MOTION**)findArcExport(gEggArc, "motions");
	}
}

// 0x800BF434
extern "C" void termGoldenEgg()
{
	if (sGoldenEggArc != 0) {
		for (int i = 0; i < 3; i++) {
			releaseTextures(sGoldenEggTexLists[i]);
		}
		releaseResource(sGoldenEggArc);
	}
	sGoldenEggArc    = 0;
	sGoldenEggModels = 0;
	sEggModels       = 0;
	sEggMotions      = 0;
}

// 0x800BF4A4
extern "C" void loadGoldenEgg()
{
	loadArc("egg_gold.arc", &sGoldenEggArc);
	if (sGoldenEggArc != 0) {
		sGoldenEggModels   = (NJS_OBJECT**)findArcExport(sGoldenEggArc, "models");
		sGoldenEggTexLists = (NJS_TEXLIST**)findArcExport(sGoldenEggArc, "texlists");
		void** textures    = (void**)findArcExport(sGoldenEggArc, "textures");
		bindGvmTextures((u8*)*textures, sGoldenEggTexLists[0]);
		bindGvmTextures((u8*)*textures, sGoldenEggTexLists[1]);
		bindGvmTextures((u8*)*textures, sGoldenEggTexLists[2]);
	}
}

// 0x800BF544
// the event command load_gold
extern "C" int eventCmdLoadGold(void*)
{
	loadGoldenEgg();
	return 1;
}

// 0x800BF568
extern "C" int goldenEggOnMessage(TObjGoldenEgg* egg, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x01:
		return egg->msgPush(msg);
	case 0x18:
		return egg->msgFeed(msg);
	case 0x2A:
		return egg->msgRoll(msg);
	case 0x76:
		return egg->msgGetLevel(msg);
	case 0x40:
		return egg->msgAbilities(msg);
	case 0x41:
		return egg->msgGetFlags(msg);
	case 0x43:
		return egg->msgGetVel(msg);
	case 0x1002:
		return egg->msgWake(msg);
	case 0x64:
		return egg->msgRollBy(msg);
	case 0x69:
		return egg->msgRelease(msg);
	case 0x6A:
		return egg->msgTake(msg);
	case 0x6B:
		return egg->msgThrow(msg);
	case 0x71:
		return egg->msgSlam(msg);
	}
	return 0;
}

// 0x800BF694
int TObjGoldenEgg::sendToHandle(ObjMsg* msg)
{
	return mHandle.send(msg);
}

// 0x800BF6B8
void TObjGoldenEgg::initFromDesc(SetDesc* desc)
{
	Vec3f pos;
	SetRot rot;

	desc->getPos(&pos);
	desc->getRot(&rot);
	pos.y += mBody.mRadius;
	setPos(&pos);
	colliShapeSetPos(colliGetShape(mShape, 0), &pos);
	mRotY = rot.y;
	mMode = desc->getInt(1);
	if (mMode < 0) {
		mMode = 0;
	} else if (mMode > 1) {
		mMode = 1;
	}
}

// 0x800BF794
void TObjGoldenEgg::debugPrint(void*)
{
	njPrint(0xA0014, ": %s", gGoldenEggModeNames[mMode]);
}

// 0x800BF7D8
void TObjGoldenEgg::setPos(const Vec3f* pos)
{
	f32 z;
	f32 y;
	f32 x;
	x            = pos->x;
	y            = pos->y;
	z            = pos->z;
	mPos.x       = x;
	mPos.y       = y;
	mPos.z       = z;
	mBody.mPos.x = x;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
}

// 0x800BF800
void TObjGoldenEgg::draw()
{
	if (debugEditActive() != 0) {
		updateMatrix();
		return;
	}
	if ((mEggFlags.mBits & 8) == 0) {
		if (mPos.y < -40.0f) {
			die();
			return;
		}
		if (mEggFlags.mBits & 4) {
			die();
			return;
		}
		if (mRoll.getMask() & 0x100) {
			die();
			return;
		}
		if (eggFirstDamage((void*)mShape, (EggContact*)mShape->mContacts, 0, (void*)this) != 0) {
			die();
			return;
		}
	}

	mGrow.tick();
	if (mGrow.isActive() != 0) {
		setRadius(mGrow.getTarget());
	}
	runState(mState, EGG_UPDATE);
	if (mFlags & 1) {
		return;
	}
	runState(mState, EGG_DRAW);
	mPos.x = mBody.mPos.x;
	mPos.y = mBody.mPos.y;
	mPos.z = mBody.mPos.z;
	colliShapeSetPos(colliGetShape(mShape, 0), &mPos);

	EggHits hits;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&mPos, -1) != 0 && (hits.mMask & 1) != 0) {
		shadowSetGround(&mShadow, &hits.mHits[0].mPos, &hits.mHits[0].mNormal);
		shadowEnable(&mShadow);
	} else {
		shadowDisable(&mShadow);
	}
	mGlow.update();
}

// 0x800BF9B4
void TObjGoldenEgg::vfn_10()
{
	if (cameraIsVisible(1000.0f, mBody.mRadius, &mPos, 0) == 0) {
		mDraw->mFlags |= 1;
	} else {
		mDraw->mFlags &= ~1;
	}
}

// 0x800BFA1C
void TObjGoldenEgg::render()
{
	f32 mtx[12];
	mtxConcat(mtx, gjGetMatrix(), mMatrix);
	if (sGoldenEggArc == 0) {
		mtxScale(5.0f, 5.0f, 5.0f, mtx, mtx);
		u32 color = 0xFFFF00FF;
		debugWireSphere(mtx, &color);
		return;
	}

	njSetTexture(sGoldenEggTexLists[0]);
	gjLoadTextureNum(0, 0, 0, 0);
	gjLoadTexture(sGoldenEggTexLists[2], 0, 0, 0, 1);
	njPushMatrix(mtx);

	void* funcs[8];
	f32 tex[12];
	funcs[7] = (void*)goldenEggTexNum;
	gjChangeDrawObjIF(funcs, 0x10);
	PSMTXCopy(_gj_matrix_, tex);
	tex[3]  = 0.0f;
	tex[7]  = 0.0f;
	tex[11] = 0.0f;
	GXLoadTexMtxImm(tex, GX_TEXMTX5, GX_MTX3x4);
	gjSetNumPostTexGens(1);
	gjSetNumPostTevStages(1);
	gjSetPostTevStageCallback((void*)goldenEggTev);
	gjSetPostTexGenCallback((void*)goldenEggTexGen);
	_nj_control_3d_flag_ |= 0x80000;
	shadowAlphaBegin();
	gjDrawObject(sGoldenEggModels[0]);
	shadowAlphaEnd();
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	_nj_control_3d_flag_ &= ~0x80000;
	funcs[7] = 0;
	gjChangeDrawObjIF(funcs, 0x10);
	njPopMatrixEx();
}

// 0x800BFB9C
int TObjGoldenEgg::onHit(const TObjAction* other, int arg, int* result)
{
	int ret;

	if (other->mObjId == mHolderId) {
		return 0;
	}
	switch (mState) {
	case GOLDEN_EGG_ROLLED:
	case GOLDEN_EGG_BOUNCE:
	case GOLDEN_EGG_SLOW:
		((HitInfo*)result)->_14 = (u32)mThrowTag;
		ret                     = mRoll.hitWhileRolling(result, mLevel.getPower());
		break;
	case GOLDEN_EGG_HELD: {
		TMobileObject* holder = mHandle.mHolder;
		ret                   = holder->takeHit((HitInfo*)result, mLevel.getPower());
		break;
	}
	case GOLDEN_EGG_SLAM:
		((HitInfo*)result)->_14 = (u32)mThrowTag;
		ret                     = mRoll.hitWhileHeld(result, mLevel.getPower());
		break;
	default:
		return 0;
	}
	if (other->_00 != (u32)gTObjEggName) {
		return ret;
	}
	if (mLevel.mSize > ((TObjEgg*)other)->mLevel.mSize) {
		((HitInfo*)result)->mDamage = mLevel.getPower();
		((HitInfo*)result)->mFlags.setBit(0xD);
		return 1;
	}
	return 0;
}

// 0x800BFCE4
void TObjGoldenEgg::runState(int state, int phase)
{
	switch (state) {
	case GOLDEN_EGG_IDLE:
		stateIdle(phase);
		break;
	case GOLDEN_EGG_HELD:
		stateHeld(phase);
		break;
	case GOLDEN_EGG_SLAM:
		stateSlam(phase);
		break;
	case GOLDEN_EGG_HATCH:
		stateHatch(phase);
		break;
	case GOLDEN_EGG_ROLLED:
		stateRolled(phase);
		break;
	case GOLDEN_EGG_THROWN:
		stateThrown(phase);
		break;
	case GOLDEN_EGG_ASLEEP:
		stateAsleep(phase);
		break;
	case GOLDEN_EGG_BOUNCE:
		stateBounce(phase);
		break;
	case GOLDEN_EGG_SLOW:
		stateSlow(phase);
		break;
	}
}

// 0x800BFD88
void TObjGoldenEgg::setState(int state)
{
	if (mState != GOLDEN_EGG_NONE) {
		runState(mState, EGG_EXIT);
	}
	if (state != GOLDEN_EGG_NONE) {
		runState(state, EGG_ENTER);
	}
	mState = state;
}

// 0x800BFDE8
int TObjGoldenEgg::isHolder(ObjMsg* msg)
{
	return ((EggThrowMsg*)msg)->mHolder == mHolderId;
}

// 0x800BFE00
int TObjGoldenEgg::isHandleBusy()
{
	return 0;
}

// 0x800BFE08
int TObjGoldenEgg::takeBy(ObjMsg* message)
{
	EggHoldMsg* msg = (EggHoldMsg*)message;
	if (mHandle.mHolder == 0 && mRoll.isGrounded() != 0) {
		msg->mHandle    = &mHandle;
		msg->mPos       = mBody.mPos;
		msg->mRot       = mBody.mRot;
		msg->mRadius    = mBody.mRadius;
		mOwnerId        = msg->mHolder;
		mHandle.mDamage = mLevel.getPower();
		mHolderId       = msg->mHolder;
		setKeyBit(400);
		return 1;
	}
	return 0;
}

// 0x800BFEC8
void TObjGoldenEgg::clearHolder()
{
	mOwnerId = 0xFFFF;
}

// 0x800BFED8
void TObjGoldenEgg::onSizeChange(int size)
{
	f32 radius = mBody.mRadius;
	mGrow.start(radius, mLevel.getRadius());
	if (mHandle.mHolder != 0) {
		mHandle.mDamage = mLevel.getPower();
	}
	if (size == mLevel.getMaxSize()) {
		sePlay3D(0x2D, &mPos);
		mGlow.startFull(mModelMatrix);
	} else {
		sePlay3D(0x87, &mPos);
		mGlow.startGrow(mModelMatrix);
	}
}

// 0x800BFF8C
void TObjGoldenEgg::applySize()
{
	setRadius(mLevel.getRadius());
}

// 0x800BFFC4
void TObjGoldenEgg::setRadius(f32 radius)
{
	mBody.mRadius = radius;
	if (mHandle.mHolder != 0) {
		mHandle.mHolder->mBody.mRadius = radius;
	}
	colliGetShape(mShape, 0)->mSizeX = radius;
	f32 size                         = 2.0f * mBody.mRadius;
	mShadow._30                      = size;
	mShadow._34                      = size;
}

// 0x800C0038
void TObjGoldenEgg::updateMatrix()
{
	Vec3f pos;
	EggRot rot;
	f32 base[12];
	f32 grow[12];

	pos       = mBody.mPos;
	rot       = mBody.mRot;
	f32 scale = mBody.mRadius / 10.0f;
	mtxSetTransV(base, &pos);
	mtxConcatQuat((f32*)mRotMatrix, (const f32*)base, (const f32*)&rot);
	mtxScale(scale, scale, scale, mModelMatrix, base);
	getGrowScale(grow);
	mtxConcat(mMatrix, mRotMatrix, grow);
}

// 0x800C0104
void TObjGoldenEgg::getGrowScale(f32* mtx)
{
	if (mGrow.isActive() != 0) {
		mGrow.getScale(mtx);
		f32 scale = 0.2f;
		mtxScale(scale, scale, scale, mtx, mtx);
	} else {
		f32 scale = 0.2f * mBody.mRadius;
		mtxSetScale(mtx, scale, scale, scale);
	}
}

// 0x800C0184
int TObjGoldenEgg::checkHatchTouch()
{
	int size = mLevel.mSize;
	if (size == mLevel.getMaxSize() && eggAnyTouch((void*)mShape, (EggContact*)mShape->mContacts, 0) != 0) {
		colliDisable(mShape);
		return 1;
	}
	return 0;
}

// 0x800C01F0
int TObjGoldenEgg::checkThrown()
{
	void* target;
	u8 point[0xC];

	if (mRoll.findPath(&target, point) != 0) {
		mRoll.attachPath(target, point);
		return 1;
	}
	return 0;
}

// 0x800C024C
void TObjGoldenEgg::die()
{
	ObjMsg msg;
	msg.mType = 0x93;
	sendGameMsg(&msg);
	setLinkDone((SetLink*)&mLink);
	mFlags |= 1;
}

// 0x800C0298
void TObjGoldenEgg::stateHeld(int phase)
{
	Vec3f pos;
	EggRot rot;

	if (phase == EGG_UPDATE) {
	} else if (phase == EGG_DRAW) {
		eggPushHeld((void*)mShape, (EggContact*)mShape->mContacts, 0, mHandle.mHolder);
		TMobileObject* holder = mHandle.mHolder;
		pos                   = holder->mBody.mPos;
		rot.mAxis             = holder->mBody.mRot.mAxis;
		f32 angle             = holder->mBody.mRot.mW;
		setPos(&pos);
		mBody.mRot.mW    = angle;
		mBody.mRot.mAxis = rot.mAxis;
		updateMatrix();
		mRoll.checkBumps();
	}
}

// 0x800C0364
void TObjGoldenEgg::stateRolled(int phase)
{
	if (phase == EGG_ENTER) {
		mTimer = *(u16*)((u8*)playerGetParams(0) + 0x18);
		mAuraRoll.start();
	} else if (phase == EGG_EXIT) {
		mAuraRoll.stop();
	} else if (phase == EGG_UPDATE) {
		if (mRoll.isRollDone() != 0) {
			setState(GOLDEN_EGG_IDLE);
		} else if (checkThrown() != 0) {
			setState(GOLDEN_EGG_THROWN);
		} else if (mRoll.hitWall() != 0) {
			mRoll.rebound();
			setState(GOLDEN_EGG_BOUNCE);
		} else if (goldenEggTick(&mTimer) != 0) {
			mRoll.stopRolling();
			setState(GOLDEN_EGG_SLOW);
		}
	} else if (phase == EGG_DRAW) {
		mRoll.pushOff(mShape, mShape->mContacts, 0);
		mRoll.moveRolling();
		updateMatrix();
		mAuraRoll.update();
	}
}

// 0x800C0490
void TObjGoldenEgg::stateSlam(int phase)
{
	Vec3f pos;

	if (phase == EGG_UPDATE) {
		if (mRoll.isSlamDone() != 0) {
			setState(GOLDEN_EGG_IDLE);
		} else if (checkThrown() != 0) {
			setState(GOLDEN_EGG_THROWN);
		} else if (mRoll.hasLanded() != 0) {
			pos.x      = mPos.x;
			f32 radius = mBody.mRadius;
			pos.y      = mPos.y - radius;
			pos.z      = mPos.z;
			u16 holder = mHolderId;
			u16 obj    = mObjId;
			eggSpawnShockWave(radius, obj, holder, &pos, mLevel.getPower(), mThrowTag);
			sePlay3D(0x81, &pos);
			pos.y += 5.0f;
			particleOnceById(gTL_PARTICLE, &pos, 0, 9);
			camShakeNearSoft(200.0f, &mPos);
			setState(GOLDEN_EGG_IDLE);
		}
	} else if (phase == EGG_DRAW) {
		mRoll.pushOff(mShape, mShape->mContacts, 0);
		mRoll.moveSlam();
		updateMatrix();
	}
}

// 0x800C05E4
extern "C" void goldenEggHatchWalk(void* node, GoldenEggWalk* walk)
{
	if (walk->mCount == 1) {
		njGetMatrix(walk->mMtx);
	}
	walk->mCount = walk->mCount + 1;
}

// 0x800C062C
void TObjGoldenEgg::stateHatch(int phase)
{
	Vec3f pos;
	Vec3f drawPos;
	GoldenEggWalk walk;
	EggRot rot;

	if (phase == EGG_ENTER) {
		mEggFlags.mBits |= 8;
		mTimer      = 60;
		mHatchModel = new MotionPlayer;
		mHatchModel->init(sGoldenEggHatchMotions, sEggMotions);
		mHatchModel->play(0);
		mGlow.stop();
		sePlay3D(0x96, &mPos);
		hatchNoticePost(mBody.mRadius, 0, 0x13, &mPos, 0x1E);
	} else if (phase == EGG_EXIT) {
		::operator delete(mHatchModel);
	} else if (phase == EGG_UPDATE) {
		if (goldenEggTick(&mTimer) != 0) {
			pos = mBody.mPos;
			pos.y -= mBody.mRadius;
			mFlags |= 0x10;
			shadowDisable(&mShadow);
			setKeyBit(500);
			chickenBossHatch(&pos);
			setLinkDone((SetLink*)&mLink);
			mFlags |= 1;
		}
	} else if (phase == EGG_DRAW) {
		mHatchModel->update();
		walk.mCount = 0;
		drawPos     = mBody.mPos;
		drawPos.y -= mBody.mRadius;
		njPushMatrixEx();
		njUnitMatrix(0);
		njTranslateEx(&drawPos);
		MotionPlayer* model = mHatchModel;
		int motion          = model->mCurrent;
		f32 frame           = model->mFrame;
		NJS_MOTION* mot     = model->getMotion(motion);
		walkMotion(frame, sEggModels[4], mot, (NodeCallback)goldenEggHatchWalk, (void*)&walk);
		njPopMatrixEx();
		mtxCopy(mRotMatrix, walk.mMtx);
		mtxTranslate((float*)mRotMatrix, (const float*)mRotMatrix, 0.0f, mBody.mRadius - 5.0f, 0.0f);
		rot       = mBody.mRot;
		f32 scale = mBody.mRadius / 5.0f;
		mtxConcatQuat((f32*)mRotMatrix, (const f32*)mRotMatrix, (const f32*)&rot);
		mtxScale(scale, scale, scale, mMatrix, mRotMatrix);
	}
}

// 0x800C087C
void TObjGoldenEgg::stateIdle(int phase)
{
	if (phase == EGG_ENTER) {
		mTimer = 60;
		mFx    = 0;
	} else if (phase == EGG_EXIT) {
		seStop3D(&mSound);
		fxStop(mFx);
	} else if (phase == EGG_UPDATE) {
		if (checkHatchTouch() != 0) {
			setState(GOLDEN_EGG_HATCH);
		}
	} else if (phase == EGG_DRAW) {
		if (mTimer != 0 && --mTimer == 0) {
			sePlay3DHeld(200.0f, &mSound, 0x86, 0, (const Vec3f*)&mPos);
			mFx = fxSpawnFollow(1, 7, 0x1A, mRotMatrix);
		}
		mSound.mPos = mPos;
		mRoll.pushOff(mShape, mShape->mContacts, 0);
		mRoll.moveGround();
		updateMatrix();
	}
}

// 0x800C0990
void TObjGoldenEgg::stateThrown(int phase)
{
	if (phase == EGG_UPDATE) {
		if (mRoll.isPathDone() != 0) {
			mRoll.leavePath();
			setState(GOLDEN_EGG_IDLE);
		}
	} else if (phase == EGG_DRAW) {
		mRoll.movePath();
		updateMatrix();
	}
}

// 0x800C0A00
void TObjGoldenEgg::stateAsleep(int phase)
{
	if (phase == EGG_ENTER) {
		mEggFlags.mBits |= 8;
		sePlay3DHeld(200.0f, &mSound, 0x86, 0, (const Vec3f*)&mPos);
	} else if (phase == EGG_EXIT) {
		seStop3D(&mSound);
		mEggFlags.mBits &= ~8;
	}
}

// 0x800C0A74
void TObjGoldenEgg::stateBounce(int phase)
{
	if (phase == EGG_ENTER) {
		mAuraRoll.start();
	} else if (phase == EGG_EXIT) {
		mAuraRoll.stop();
	} else if (phase == EGG_UPDATE) {
		if (mRoll.isRollDone() != 0) {
			setState(GOLDEN_EGG_IDLE);
		} else if (mRoll.isPastHolder() != 0) {
			setState(GOLDEN_EGG_IDLE);
		}
	} else if (phase == EGG_DRAW) {
		mRoll.pushOff(mShape, mShape->mContacts, 0);
		mRoll.moveBounce();
		updateMatrix();
		mAuraRoll.update();
	}
}

// 0x800C0B40
void TObjGoldenEgg::stateSlow(int phase)
{
	if (phase == EGG_ENTER) {
		mTimer = *(u16*)((u8*)playerGetParams(0) + 0x1A);
		mAuraSlow.start();
		sePlay3D(0x80, &mPos);
	} else if (phase == EGG_EXIT) {
		mAuraSlow.stop();
	} else if (phase == EGG_UPDATE) {
		if (mRoll.isRollDone() != 0) {
			setState(GOLDEN_EGG_IDLE);
		} else if (mRoll.hitWall() != 0) {
			mRoll.reboundHard();
			setState(GOLDEN_EGG_BOUNCE);
		} else if (goldenEggTick(&mTimer) != 0) {
			mRoll.reboundHard();
			setState(GOLDEN_EGG_BOUNCE);
		}
	} else if (phase == EGG_DRAW) {
		mRoll.pushOff(mShape, mShape->mContacts, 0);
		mRoll.moveSlow();
		updateMatrix();
		mAuraSlow.update();
	}
}

// 0x800C0C58
int TObjGoldenEgg::msgPush(ObjMsg* msg)
{
	njAddVector(&mBody.mAccel, &((ObjVecMsg*)msg)->mVec);
	return 1;
}

// 0x800C0C84
int TObjGoldenEgg::msgFeed(ObjMsg* msg)
{
	int level = mLevel.mSize;
	mLevel.feed(((EggFeedMsg*)msg)->mFood, ((EggFeedMsg*)msg)->mAmount);
	int now = mLevel.mSize;
	if (now != level) {
		onSizeChange(now);
	}
	return 1;
}

// 0x800C0CE4
int TObjGoldenEgg::msgRoll(ObjMsg* msg)
{
	switch (mState) {
	case GOLDEN_EGG_IDLE:
		mRoll.kick(((EggKickMsg*)msg)->mAngle);
		return 1;
	case GOLDEN_EGG_SLAM:
		mRoll.kickHeld();
		return 1;
	case GOLDEN_EGG_ROLLED:
	case GOLDEN_EGG_BOUNCE:
	case GOLDEN_EGG_SLOW:
		mRoll.reboundHard();
		setState(GOLDEN_EGG_BOUNCE);
		return 1;
	default:
		return 0;
	}
}

// 0x800C0D78
int TObjGoldenEgg::msgGetLevel(ObjMsg* msg)
{
	mLevel.getProgress(msg);
	((EggLevelMsg*)msg)->mHits = 0;
	return 1;
}

// 0x800C0DB4
int TObjGoldenEgg::msgAbilities(ObjMsg* msg)
{
	return 0x3B;
}

// 0x800C0DBC
int TObjGoldenEgg::msgGetFlags(ObjMsg* msg)
{
	int flags = isHandleBusy();
	switch (mState) {
	case GOLDEN_EGG_ROLLED:
	case GOLDEN_EGG_BOUNCE:
	case GOLDEN_EGG_SLOW:
		flags |= 0x200;
		break;
	}
	return flags;
}

// 0x800C0E10
int TObjGoldenEgg::msgGetVel(ObjMsg* msg)
{
	((ObjVecMsg*)msg)->mVec = mBody.mVel;
	return 1;
}

// 0x800C0E30
int TObjGoldenEgg::msgWake(ObjMsg* msg)
{
	if (mState == GOLDEN_EGG_ASLEEP) {
		mLink.get()->setData((void*)1);
		setState(GOLDEN_EGG_IDLE);
		return 1;
	}
	return 0;
}

// 0x800C0E88
int TObjGoldenEgg::msgRollBy(ObjMsg* msg)
{
	if (isHolder(msg) == 0) {
		return 0;
	}
	if (mState == GOLDEN_EGG_HELD) {
		clearHolder();
		mRoll.setVel(&((EggThrowMsg*)msg)->mVel);
		mThrowTag = (void*)scoreNewChain(0);
		setState(GOLDEN_EGG_ROLLED);
		return 1;
	}
	return 0;
}

// 0x800C0F0C
int TObjGoldenEgg::msgTake(ObjMsg* msg)
{
	switch (mState) {
	case GOLDEN_EGG_IDLE:
		if (takeBy(msg) != 0) {
			setState(GOLDEN_EGG_HELD);
			return 1;
		}
		return 0;
	case GOLDEN_EGG_ROLLED:
	case GOLDEN_EGG_BOUNCE:
	case GOLDEN_EGG_SLOW:
		if (takeBy(msg) != 0) {
			mBody.mVel = gVecZero;
			setState(GOLDEN_EGG_HELD);
			return 1;
		}
		return 0;
	}
	return 0;
}

// 0x800C0FE0
int TObjGoldenEgg::msgRelease(ObjMsg* msg)
{
	if (isHolder(msg) == 0) {
		return 0;
	}
	if (mState == GOLDEN_EGG_HELD) {
		clearHolder();
		setState(GOLDEN_EGG_IDLE);
		return 1;
	}
	return 0;
}

// 0x800C1048
int TObjGoldenEgg::msgThrow(ObjMsg* msg)
{
	if (isHolder(msg) == 0) {
		return 0;
	}
	if (mState == GOLDEN_EGG_HELD) {
		EggPathMsg* path = (EggPathMsg*)msg;
		clearHolder();
		mRoll.attachPathAt(path->mPath, path->mPoint, path->mSpeed);
		setState(GOLDEN_EGG_THROWN);
		return 1;
	}
	return 0;
}

// 0x800C10C8
int TObjGoldenEgg::msgSlam(ObjMsg* msg)
{
	if (isHolder(msg) == 0) {
		return 0;
	}
	if (mState == GOLDEN_EGG_HELD) {
		clearHolder();
		mRoll.startSlam(&((EggThrowMsg*)msg)->mVel);
		mThrowTag = (void*)scoreNewChain(1);
		setState(GOLDEN_EGG_SLAM);
		return 1;
	}
	return 0;
}

// 0x800C114C
// the draw object's texture slot always texture 0
extern "C" void goldenEggTexNum(int, int map, int slot, int flags)
{
	gjLoadTextureNum(0, map, slot, flags);
}

// 0x800C1170
// the extra stage that adds the sheen
extern "C" void goldenEggTev(int stage)
{
	u8 s = stage;
	GXSetTevColorIn(s, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
	GXSetTevColorOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(s, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(s, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
}

// 0x800C1214
extern "C" void goldenEggTexGen(u8 stage)
{
	GXSetTexCoordGen2(stage, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX5, GX_TRUE, GX_PTTEXMTX19);
}

struct GjModel;
struct NJS_QUATERNION;

typedef char EggShard_size_check[(sizeof(EggShard) == 0x68) ? 1 : -1];

/**
 * @size 0x28
 */
class TGoldenEggBreakDemo : public TObject {
public:
	TGoldenEggBreakDemo(TObject* parent);
	virtual ~TGoldenEggBreakDemo();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	u16 mTime;        // _20
	TCamera* mCamera; // _24
};

typedef char TGoldenEggBreak_size_check[(sizeof(TGoldenEggBreak) == 0x38) ? 1 : -1];
typedef char TGoldenEggBreakDemo_size_check[(sizeof(TGoldenEggBreakDemo) == 0x28) ? 1 : -1];

const char* gTGoldenEggBreakName = "TGoldenEggBreak";

// 0x800C124C
struct EggQuatIm {
	f32 v[3];
};

TGoldenEggBreak::TGoldenEggBreak(TObject* parent, const Vec3f* pos, const f32* rot, f32 radius)
    : TObject(parent)
{
	_00     = (u32)gTGoldenEggBreakName;
	mSize   = 0x38;
	mRadius = 7.5f;
	mFloor  = pos->y - mRadius;
	mLife   = 0xB4;
	mTime   = 0;
	mTask   = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);

	int count = 0;
	NJS_OBJECT* node;
	NJS_OBJECT* parts = sGoldenEggModels[1];
	if (parts != 0) {
		for (node = parts->child; node != 0; node = node->sibling) {
			count++;
		}
	}
	mCount  = count;
	mShards = new EggShard[count];

	if (count != 0) {
		f32 scale = 0.2f * mRadius;
		f32 matrix[12];
		mtxSetTransV(matrix, pos);
		mtxConcatQuat(matrix, matrix, rot);
		mtxScale(scale, scale, scale, matrix, matrix);

		EggShard* shard = mShards;
		node            = sGoldenEggModels[1]->child;
		for (int i = 0; i < count; i++) {
			shard->mModel = node->model;
			mtxMultVec(matrix, (Vec3f*)node->pos, &shard->mPos);
			Vec3f dir;
			mtxMultVecSR(matrix, (Vec3f*)node->pos, &dir);
			vecNormalize(&dir, &dir);
			vecScale(250.0f, &shard->mVel, &dir);
			shard->mSpin.x               = 0.0f;
			shard->mSpin.y               = 0.0f;
			shard->mSpin.z               = 0.0f;
			shard->mRot[0]               = rot[0];
			*(EggQuatIm*)&shard->mRot[1] = *(const EggQuatIm*)&rot[1];
			shard++;
			node = node->sibling;
		}
	}

	f32 at[12];
	mtxSetTransV(at, pos);
	fxSpawnAt(0, 7, 0x3D, at);
}

// 0x800C1464
EggShard::EggShard()
{
}

// 0x800C1468
TGoldenEggBreak::~TGoldenEggBreak()
{
	delete[] mShards;
	removeDrawTask(mTask);
}

static inline EggRot goldenRotMul(EggRot rot, const EggRot* by)
{
	njMultiQuaternion((const NJS_QUATERNION*)&rot, (const NJS_QUATERNION*)by, (NJS_QUATERNION*)&rot);
	return rot;
}

static inline EggRot goldenRotScale(EggRot rot, f32 scale)
{
	quatScale((Quat*)&rot, scale);
	return rot;
}

// 0x800C14E4
void TGoldenEggBreak::draw()
{
	EggShard* shard;
	int i;
	f32 scale;
	Vec3f gravity;
	Vec3f step;

	shard     = mShards;
	scale     = 0.2f * mRadius;
	gravity.x = 0.0f;
	gravity.y = -400.0f;
	gravity.z = gravity.x;
	for (i = 0; i < mCount; i++, shard++) {
		if (mTime > 0x3C) {
			vecScaleAdd(1.0f / 120.0f, &shard->mVel, &gravity, &shard->mVel);
			vecScaleAdd(1.0f / 120.0f, &shard->mPos, &shard->mVel, &shard->mPos);
			vecScale(1.0f / 120.0f, &step, &shard->mSpin);
			EggRot made;
			made.mW         = 0.0f;
			made.mAxis.v[0] = step.x;
			made.mAxis.v[1] = step.y;
			made.mAxis.v[2] = step.z;
			EggRot half     = goldenRotScale(goldenRotMul(made, (const EggRot*)shard->mRot), 0.5f);
			quatAdd((Quat*)shard->mRot, (const Quat*)&half);
			quatNormalize((Quat*)shard->mRot);
			vecScale(0.97f, &shard->mVel, &shard->mVel);
		}
		if (shard->mPos.y < mFloor) {
			shard->mPos.y = mFloor;
			shard->mVel.x *= 0.7f;
			shard->mVel.y *= -0.7f;
			shard->mVel.z *= 0.7f;
			step.x = 0.0f;
			step.y = -1.0f;
			step.z = 0.0f;
			njOuterProduct(&shard->mVel, &step, &step);
			vecScale(1.0f / mRadius, &shard->mSpin, &step);
		}
		mtxSetTransV(shard->mMatrix, &shard->mPos);
		mtxConcatQuat(shard->mMatrix, shard->mMatrix, shard->mRot);
		mtxScale(scale, scale, scale, shard->mMatrix, shard->mMatrix);
	}

	if (--mLife == 0) {
		mFlags |= 1;
	}
	mTime++;
}

// 0x800C173C
void TGoldenEggBreak::render()
{
	njSetTexture(sGoldenEggTexLists[1]);
	gjLoadTexture(sGoldenEggTexLists[2], 0, 0, 0, 1);

	f32 matrix[12];
	PSMTXCopy(_gj_matrix_, matrix);
	matrix[3]  = 0.0f;
	matrix[7]  = 0.0f;
	matrix[11] = 0.0f;
	GXLoadTexMtxImm(matrix, GX_TEXMTX5, GX_MTX3x4);
	gjSetNumPostTexGens(1);
	gjSetNumPostTevStages(1);
	gjSetPostTevStageCallback((void*)goldenEggTev);
	gjSetPostTexGenCallback((void*)goldenEggTexGen);
	_nj_control_3d_flag_ |= 0x80000;
	shadowAlphaBegin();

	EggShard* shard;
	shard = mShards;
	int i;
	i = 0;
	while (i < mCount) {
		njPushMatrixEx();
		njMultiMatrix(0, shard->mMatrix);
		gjDrawModel((GjModel*)shard->mModel);
		njPopMatrixEx();
		i++;
		shard++;
	}

	shadowAlphaEnd();
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	_nj_control_3d_flag_ &= 0xFFF7FFFF;
}

// 0x800C184C
extern "C" void* goldenEggBreakDemoNew()
{
	return new TGoldenEggBreakDemo(gTL_07);
}

const char* gTGoldenEggBreakDemoName = "TGoldenEggBreakDemo";

// 0x800C188C
TGoldenEggBreakDemo::TGoldenEggBreakDemo(TObject* parent)
    : TObject(parent)
{
	_00   = (u32)gTGoldenEggBreakDemoName;
	mSize = 0x28;
	mTime = 0xB4;

	Vec3f pos;
	pos.x = 0.0f;
	pos.y = 100000.0f;
	pos.z = 0.0f;
	f32 rot[4];
	njCreateQuaternion(0, 0.0f, 0.0f, 1.0f, (NJS_QUATERNION*)rot);

	new TGoldenEggBreak(this, &pos, rot, 7.5f);

	Vec3f at;
	Vec3f back;
	back.x = 0.0f;
	back.y = 0.0f;
	back.z = -1.0f;
	vecScaleAdd(50.0f, &at, &back, &pos);
	TCamera* camera = createTCamFix(0, 3, &pos, &at);
	camera->setBlendStep(1.0f);
	mCamera = camera;
	frameBlurStart(0x78, 0.6f);
	sePlayFor(0x2B3, 0xB4);
}

// 0x800C1994
TGoldenEggBreakDemo::~TGoldenEggBreakDemo()
{
	mCamera->mFlags |= 1;
}

// 0x800C1A00
void TGoldenEggBreakDemo::draw()
{
	if (mTime != 0) {
		mTime--;
	}
}

// 0x800C1A18
int TObjGoldenEgg::getKind()
{
	return 1;
}

// 0x800C1A20
Vec3f* TObjGoldenEgg::getPos()
{
	return &mPos;
}

// 0x800C1A28
SetLinkMember* TObjGoldenEgg::getLink()
{
	return &mLink;
}

// 0x800C1A30
f32 TObjGoldenEgg::getRadiusV()
{
	return mBody.mRadius;
}

// 0x800C1A38
int goldenEggTick(s16* timer)
{
	if (*timer > 0) {
		if (--*timer == 0) {
			return 1;
		}
	}
	return 0;
}
