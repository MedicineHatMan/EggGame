#include "egg/ObjEgg.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "item/TItemManage.h"
#include "object/ObjModel.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/CharaColli.h"
#include "ui/AeArc.h"
#include "egg/EggLevel.h"
#include "object/TObjEggYolk.h"
#include "object/ObjAction.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "io/NinjaFile.h"
#include "object/TObjPlayer.h"
#include "animal/AniSpawn.h"
#include "object/ObjShadow.h"
#include "engine/TMobileObject.h"
#include "stage/TStageScore.h"
#include "camera/TCameraCtrl.h"
#include "camera/TCamera.h"
#include "engine/Color.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "sound/Sound3DPlay.h"
#include "engine/TParticle.h"
#include "object/TObjEggRing.h"
#include "engine/Colli.h"
#include "engine/GroundQuery.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "stage/TGameState.h"
#include "egg/EggCreature.h"
#include "engine/FxSystem.h"
#include "stage/SetSlot.h"
#include "egg/TEggTextureManager.h"
#include "core/VecMath.h"
#include "egg/EggBody.h"
#include "object/TObjKey00.h"
#include "egg/TEggEffBreak.h"
#include "stage/TEventCtrl.h"
#include "ui/HatchNotice.h"
#include "engine/ActorMotion.h"
#include "core/TMainTask.h"
#include "animal/RacerMover.h"
struct NJS_TEXLIST;
struct CharaColli;
struct NJS_OBJECT;
struct DrawState;
struct DrawStateSlot;
struct Fx;
struct ColliShape;
struct SetLink;
struct TEggTextureManager;
struct TEggCreature;

// TObjEgg class ID 0x19 in the placement files
// See docs/notes.md The helpers are in EggRoll.cpp and EggLevel.cpp
extern "C" {
int EggLevel::pickRandomType(f32);

extern const MotionEntry sEggHatchMotions[1] = {
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 15.0f, 0.21f, 0.0f, 0 },
};
extern const int sEggItemIdsHigh[24];
}

int gEggAssetsLoaded;
NJS_OBJECT** sEggHatchModels;
NJS_MOTION** sEggHatchMotionData;
NJS_TEXLIST** sEggTexLists;
TArc* gEggArc;

// Each form has an effect to start and an effect to keep in step
struct EggFormFuncs {
	void (TObjEgg::*mEnter)();
	void (TObjEgg::*mUpdate)();
};

static EggFormFuncs sEggForms[9] = {
	{ 0, 0 },
	{ &TObjEgg::enterFormA, &TObjEgg::formFollow },
	{ &TObjEgg::enterFormB, &TObjEgg::formFollow },
	{ &TObjEgg::enterFormC, &TObjEgg::formFollowTop },
	{ &TObjEgg::enterFormD, &TObjEgg::formFollow },
	{ &TObjEgg::enterFormE, &TObjEgg::formFollow },
	{ &TObjEgg::enterFormF, 0 },
	{ &TObjEgg::enterFormG, &TObjEgg::formFollow },
	{ &TObjEgg::enterFormH, 0 },
};

inline void* TObjEgg::operator new(unsigned long size, void* pool)
{
	return objHeapAlloc(pool, size);
}

inline void TObjEgg::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

inline EggHandle::~EggHandle()
{
}

// 0x80142E4C
extern "C" void loadEggAssets()
{
	TArc* arc;
	loadArc("ge_egg.arc", &arc);
	if (arc != 0) {
		sEggHatchModels     = (NJS_OBJECT**)findArcExport(arc, "models");
		sEggHatchMotionData = (NJS_MOTION**)findArcExport(arc, "motions");
		sEggTexLists        = (NJS_TEXLIST**)findArcExport(arc, "texlists");
		void** textures     = (void**)findArcExport(arc, "textures");
		for (int i = 0; i < 7; i++) {
			bindGvmTextures((u8*)*textures, sEggTexLists[i]);
		}
	}
	gEggArc = arc;
	loadEggLevels();
	loadEggYolk();
	gEggAssetsLoaded = 1;
}

extern "C" {
extern const int sEggItemIdsHigh[] = {
	1, 0, 3, 2, 5, 4, 6, 21, 22, 23, 10, 8, 7, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
};
}

extern "C" void unloadEggAssets()
{
	for (int i = 0; i < 7; i++) {
		releaseTextures(sEggTexLists[i]);
	}
	freeEggYolk();
	unloadEggLevels();
	releaseResource(gEggArc);
}

const char* gTObjEggName = "TObjEgg";

// 0x80142F64
TObjEgg::TObjEgg(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
    , mAuraA(this)
    , mAuraB(this)
{
	_00   = (u32)gTObjEggName;
	mSize = sizeof(TObjEgg);
	setObjId(this, desc->mObjId);
	setup();
	mLink.get()->attach((SetDesc*)desc);
	initFromDesc((SetDesc*)desc);
	if (desc->isDataEmpty() != 0) {
		mLink.get()->setData((void*)0);
	}
	if ((u32)mLink.get()->getData() & 1) {
		setState(EGG_STATE_APPEAR);
	} else {
		setState(EGG_STATE_IDLE);
	}
	mHome = mDrawPos;
}

TObjEgg::~TObjEgg()
{
	runState(mState, EGG_EXIT);
	if (mTexture != 0) {
		int id                      = mLevel.getType();
		TEggTextureManager* manager = getTEggTextureManager();
		manager->release(id);
	}
	if (mFormFx != 0) {
		fxStop((Fx*)mFormFx);
	}
	removeDrawTask(mDrawObj);
	if (mAuraDrawObj != 0) {
		removeDrawTask((DrawTask*)mAuraDrawObj);
	}
	colliFree((CharaColli*)mColl);
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	unregisterId();
}

void TObjEgg::initFromDesc(SetDesc* desc)
{
	Vec3f pos;
	int level;
	int previous = mLevel.getType();
	int id       = desc->getInt(0);
	if (id == 99) {
		if (desc->isValid() != 0) {
			mLevel.setType(0);
		} else {
			mLevel.setType(EggLevel::pickRandomType(3.0517578E-5f * (f32)rand()));
		}
	} else {
		mLevel.setType(id);
	}
	applySize();
	level = mLevel.getType();
	if (previous != level || mTexture == 0) {
		TEggTextureManager* manager = getTEggTextureManager();
		if (mTexture != 0) {
			manager->release(previous);
			mTexture = 0;
		}
		manager->acquire(level);
		mTexture = manager->getImage(level);
	}
	if (level >= 0x38) {
		mEggFlags.mBits |= 2;
	} else {
		mEggFlags.mBits &= ~2;
	}
	desc->getPos(&pos);
	pos.y += mBody.mRadius;
	setPos(&pos);
	updateMatrix();
}

void TObjEgg::debugPrint(void*)
{
	njPrint(0xA0013, ":ID %d", mLevel.getType());
}

// 0x80143474
void TObjEgg::setup()
{
	f32 one = 1.0f;

	mHits      = 0;
	mContentId = -1;
	_78        = one;
	mOwnerId   = 0xFFFF;
	mHitTimer  = 0;
	mFeedTimer = 0;
	mPulse     = 0;
	mTick      = 0;
	_C0 = _C2      = (u16)0xFFFF;
	mEggParam      = playerGetParamsTail();
	mPlayerParam   = playerGetParams(0);
	one            = 1.0f;
	mState         = EGG_STATE_NONE;
	mWobble        = one;
	mWobbleVal     = one;
	mWobbleVel     = 0.0f;
	mTexture       = 0;
	mForm          = 0;
	mFormFx        = 0;
	mDamageMul     = 1;
	mDamageAdd     = 0;
	mAuraDrawObj   = 0;
	mHandle.mOwner = this;
	mColl          = colliCreate(1, 1);
	colliSetOwner(mColl, (u16)mObjId);
	colliSetMasks(mColl, 8, 0xF3, 0xF0, 0);
	colliShapeSetSphere(colliGetShape(mColl, 0));
	*(int*)((u8*)colliGetShape(mColl, 0) + 0x24) = 0x42C;
	mBody.mBounce                                = 0.0f;
	mRoll.mOwner                                 = this;
	mRoll.mPath                                  = &mCollide;
	mRoll.mBody                                  = &mBody;
	mDrawObj                                     = addDrawTask(this, (void (*)(void*))eggDrawNormal, 0x21, 0);
	mAuraA.mBody                                 = &mBody;
	mAuraB.mBody                                 = &mBody;
}

extern "C" TObject* createEgg(TObject* parent, SetDesc* desc)
{
	return new (gObjHeap) TObjEgg(parent, desc);
}

int TObjEgg::sendToHandle(ObjMsg* msg)
{
	return mHandle.send(msg);
}

int TObjEgg::getRotY()
{
	return mPushAngle;
}

// 0x80143648
int TObjEgg::onMessage(ObjMsg* msg)
{
	switch (msg->mType) {
	case 1:
		return msgPush(msg);
	case 0xE:
		return msgHit(msg);
	case 0x18:
		return msgFeed(msg);
	case 0x2A:
		return msgRoll(msg);
	case 0x76:
		return msgGetLevel(msg);
	case 0x40:
		return msgAbilities(msg);
	case 0x41:
		return msgGetFlags(msg);
	case 0x43:
		return msgGetVel(msg);
	case 0x75:
		return msgFeedMax(msg);
	case 100:
		return msgRollBy(msg);
	case 0x65:
		return msgStop(msg);
	case 0x69:
		return msgRelease(msg);
	case 0x6A:
		return msgTake(msg);
	case 0x6B:
		return msgThrow(msg);
	case 0x6C:
		return msgRollByPlayer(msg);
	case 0x6D:
		return msgTakeIdle(msg);
	case 0x71:
		return msgSlam(msg);
	}
	return 0;
}

f32 TObjEgg::getRadius()
{
	return mBody.mRadius;
}

Vec3f* TObjEgg::getPos()
{
	return &mDrawPos;
}

void TObjEgg::setPos(const Vec3f* pos)
{
	mDrawPos   = *pos;
	mBody.mPos = *pos;
	colliShapeSetPos(colliGetShape(mColl, 0), pos);
}

int TObjEgg::checkHit(EggHits* hits, int index)
{
	if (hits->mMask & (1 << index)) {
		f32 radius = getRadius();
		return getPos()->y - radius - 1.0f <= *(f32*)((u8*)hits + index * 0x4C + 0x28);
	}
	return 0;
}

void TObjEgg::setDamageMul(s8 mul)
{
	if (mul < 1) {
		mul = 1;
	}
	mDamageMul = mul;
	if (mHandle.mHolder != 0) {
		mHandle.mDamage = mul * mLevel.getPower();
	}
}

void TObjEgg::setDamageAdd(s8 add)
{
	mDamageAdd = add;
	if (mHandle.mHolder != 0) {
		mHandle.mDamage = add + mLevel.getPower();
	}
}

extern "C" void shatterEgg(TObjEgg* egg)
{
	egg->shatter();
}

void TObjEgg::shatterSilently()
{
	ObjMsg msg;
	msg.mType = 0x15;
	sendMsgById(mOwnerId, &msg);
	setState(EGG_STATE_EXPIRE);
}

// 0x801439E0
void TObjEgg::draw()
{
	Vec3f hitDir;
	Vec3f delta;
	ObjMsg msg;

	if (debugEditActive() != 0) {
		updateMatrix();
		return;
	}

	if (mHitTimer != 0) {
		mHitTimer--;
	}

	if ((mEggFlags.mBits & 8) == 0) {
		if (mHitTimer == 0 && eggFindHit((EggColl*)mColl, this, &hitDir) != 0) {
			if (mHits == 2) {
				shatter();
				return;
			}
			if (3.0517578E-5f * (f32)rand() < 0.3f) {
				shatter();
				return;
			}
			mHitTimer = 90;
			mHits++;
			mBody.mVel = hitDir;
			msg.mType  = 0x15;
			sendMsgById(mOwnerId, &msg);
			setState(EGG_STATE_BOUNCE_UP);
		}
		if (mRoll.getMask() & 0x100) {
			shatter();
			return;
		}
		if (mDrawPos.y < -50.0f) {
			setState(EGG_STATE_FALL);
			return;
		}
	}

	if (mFeedTimer != 0 && --mFeedTimer == 0) {
		int level = mLevel.mSize;
		mLevel.setPoints(mSavedPoints);
		int now = mLevel.mSize;
		if (level != now) {
			onSizeChange(now);
		}
	}

	mGrow.tick();
	if (mGrow.isActive() != 0) {
		setRadius(mGrow.getTarget());
	}

	runState(mState, EGG_UPDATE);
	runState(mState, EGG_DRAW);

	EggHits hits;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)getPos(), -1) != 0 && (hits.mMask & 1) != 0) {
		delta = mBody.mVel;
		if ((mTick & 3) == 0 && njScalor(&delta) > 1.0f) {
			checkHit(&hits, 1);
		}
		shadowSetFromContact(&mShadow, (u8*)&hits);
		shadowEnable(&mShadow);
	} else {
		shadowDisable(&mShadow);
	}

	mTick++;
	mPulse += 0x800;
	mGlow.update();
	mTilt.update();
	mDrawPos = mBody.mPos;
	colliShapeSetPos(colliGetShape(mColl, 0), &mDrawPos);
	if (sEggForms[mForm].mUpdate != 0) {
		(this->*sEggForms[mForm].mUpdate)();
	}
	if ((mEggFlags.mBits & 0x10) == 0 && vecDistanceSq(&mHome, &mDrawPos) > 400.0f) {
		mEggFlags.mBits |= 0x10;
	}
}

// 0x80143D64
void TObjEgg::vfn_10()
{
	Vec3f mouth;
	Vec3f screen;

	f32 radius = getRadius();
	if (cameraIsVisible(1000.0f, radius, &mDrawPos, &mouth) == 0) {
		mDrawObj->mFlags |= 1;
		return;
	}

	int alt = 0;
	mDrawObj->mFlags &= ~1;
	if (playerGetGoal(&screen) != 0) {
		if (mouth.z > screen.z) {
			f32 scaleB = -1.0f / mouth.z;
			f32 scaleA = -1.0f / screen.z;
			f32 my     = mouth.y * scaleB;
			f32 mx     = mouth.x * scaleB;
			radius *= scaleB;
			f32 dx = screen.x * scaleA - mx;
			f32 dy = screen.y * scaleA - my;
			if (dx * dx + dy * dy < radius * radius) {
				alt = 1;
			}
		}
		if (alt == 0 && mOwnerId != 0xFFFF && mLevel.mSize >= 5) {
			alt = 1;
		}
	}

	if (alt != 0) {
		_78             = 0.85f;
		mDrawObj->mFunc = (void (*)(void*))eggDrawAlt;
		moveDrawTask(mDrawObj, 0x22);
		mDrawObj->mSortKey = drawTaskDepthKey(mouth.z);
	} else {
		mDrawObj->mFunc = (void (*)(void*))eggDrawNormal;
		moveDrawTask(mDrawObj, 0x21);
	}
	if (mAuraDrawObj != 0) {
		mAuraDrawObj->mSortKey = drawTaskDepthKey(mouth.z);
	}
}

void TObjEgg::prepareContent()
{
	int id = eggContentA(mLevel.getType());
	if (id == -1) {
		id = eggContentB(mLevel.getType());
		if (id != 0xFFFF) {
			mContentKind = 1;
			mContentId   = id;
			itemLoad(id);
			itemHolderAttach(id, &mContent);
		}
	} else {
		mContentKind = 0;
		mContentId   = id;
		aniPreload(id);
	}
}

void TObjEgg::setForm(int form)
{
	if (form == mForm) {
		return;
	}
	if (mFormFx != 0) {
		fxStop(mFormFx);
		mFormFx = 0;
	}
	if (mAuraDrawObj != 0) {
		removeDrawTask(mAuraDrawObj);
		mAuraDrawObj = 0;
	}
	mForm = form;
	if (sEggForms[form].mEnter != 0) {
		(this->*sEggForms[form].mEnter)();
	}
	if (sEggForms[form].mUpdate != 0) {
		(this->*sEggForms[form].mUpdate)();
	}
}

// 0x8014403C
void TObjEgg::spawnContent(int firstTime)
{
	Vec3f pos;

	if (mContentId == -1) {
		return;
	}
	switch (mContentKind) {
	case 0:
		TEggCreature* content = aniSpawn(mContentId, mHolderId, &mDrawPos);
		if (content != 0 && firstTime != 0) {
			content->_44 = 1;
		}
		break;
	case 1:
		pos.x = mDrawPos.x;
		pos.y = 5.0f + (mDrawPos.y - mBody.mRadius);
		pos.z = mDrawPos.z;
		spawnItem(mHolderId, mContentId, &pos);
		break;
	}
}

void TObjEgg::die()
{
	mLink.get()->setData((void*)1);
	setLinkTimed((SetLink*)&mLink, 300);
	mFlags |= 1;
}

void TObjEgg::shatter()
{
	Vec3f pos;
	EggRot rot;
	ObjMsg msg;

	mGlow.stop();
	msg.mType = 0x15;
	sendMsgById(mOwnerId, &msg);
	pos        = mBody.mPos;
	rot        = mBody.mRot;
	f32 radius = mBody.mRadius;
	((void (*)(f32, const Vec3f*, const EggRot*, int))createTEggEffBreak)(radius, &pos, &rot, mLevel.getType());
	playSe(0x97);
	spawnEggYolk(radius, &pos);
	die();
}

void TObjEgg::updateMatrix()
{
	Vec3f pos;
	EggRot rot;
	u8 base[0x30];
	u8 grow[0x30];

	pos       = mBody.mPos;
	rot       = mBody.mRot;
	f32 scale = mBody.mRadius / 10.0f;
	mtxSetTransV((f32*)base, &pos);
	if (mTilt.getScale(grow) != 0) {
		mtxConcat((f32*)mRotMatrix, (const f32*)base, (const f32*)grow);
		mtxConcatQuat((f32*)mRotMatrix, (const f32*)mRotMatrix, (const f32*)&rot);
	} else {
		mtxConcatQuat((f32*)mRotMatrix, (const f32*)base, (const f32*)&rot);
	}
	mtxScale(scale, scale, scale, (float*)mModelMatrix, (const float*)base);
	getGrowScale(grow);
	mtxConcat((f32*)mMatrix, (const f32*)mRotMatrix, (const f32*)grow);
}

// 0x80144314
int TObjEgg::checkHatchTouch()
{
	struct {
		u16 mType;
		u8 _02[2];
		u16 mId;
	} notify;
	ObjMsg take;
	u16 taken;

	int id = mLevel.mSize;
	if (id == mLevel.getMaxSize() && eggFindHatchTouch((EggColl*)mColl, &taken) != 0) {
		colliDisable(mColl);
		take.mType = 0x15;
		sendMsgById(mOwnerId, &take);
		mHolderId    = taken;
		notify.mType = 0xA5;
		notify.mId   = taken;
		sendGameMsg((ObjMsg*)&notify.mType);
		return 1;
	}
	return 0;
}

int TObjEgg::checkThrown()
{
	void* target;
	u8 point[0xC];

	if (mRoll.findPath(&target, point) != 0) {
		mRoll.attachPath(target, point);
		return 1;
	}
	return 0;
}

void TObjEgg::limitVel(f32 limit)
{
	Vec3f vel;
	vel.x     = mBody.mVel.x;
	vel.y     = mBody.mVel.y;
	vel.z     = mBody.mVel.z;
	f32 speed = njUnitVector(&vel);
	if (speed < gUnitVectorMin) {
		vel = gVecZero;
	}
	if (speed > limit) {
		vecScale(limit, &vel, &vel);
		mBody.mVel = vel;
	}
}

void TObjEgg::setRadius(f32 radius)
{
	mBody.mRadius = radius;
	if (mHandle.mHolder != 0) {
		mHandle.mHolder->mBody.mRadius = radius;
	}
	colliGetShape(mColl, 0)->mSizeX = radius;
	f32 value                       = 2.0f * getRadius();
	mShadow._30                     = value;
	mShadow._34                     = value;
}

void TObjEgg::playSe(int id)
{
	sePlay3D(id, &mDrawPos);
}

int TObjEgg::takeBy(void* message)
{
	EggHoldMsg* msg = (EggHoldMsg*)message;
	if (mHandle.mHolder == 0 && mRoll.isGrounded() != 0) {
		msg->mHandle    = &mHandle;
		msg->mPos       = mBody.mPos;
		msg->mRot       = mBody.mRot;
		msg->mRadius    = mBody.mRadius;
		mOwnerId        = msg->mHolder;
		mHandle.mDamage = mDamageMul * mLevel.getPower();
		mHolderId       = msg->mHolder;
		return 1;
	}
	return 0;
}

void TObjEgg::clearHolder()
{
	mOwnerId = 0xFFFF;
}

void TObjEgg::onSizeChange(int size)
{
	f32 radius = mBody.mRadius;
	mGrow.start(radius, mLevel.getRadius());
	if (mHandle.mHolder != 0) {
		mHandle.mDamage = mDamageMul * mLevel.getPower();
	}
	if (size == mLevel.getMaxSize()) {
		playSe(0x2D);
		mGlow.startFull(mModelMatrix);
	} else {
		playSe(0x87);
		mGlow.startGrow(mModelMatrix);
	}
}

void TObjEgg::applySize()
{
	setRadius(mLevel.getRadius());
}

extern "C" void eggTexAlphaOn(u8 stage)
{
	GXSetTexCoordGen2(stage, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX1, GX_TRUE, GX_PTTEXMTX19);
}

extern "C" void eggTevAlphaOn(int stage)
{
	u8 s = stage;
	GXSetTevOrder(s, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
	GXSetTevColorIn(s, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
	GXSetTevColorOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(s, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

extern "C" void eggTevCrack(int stage)
{
	u8 s = stage;
	GXSetTevOrder(s, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);
	GXSetTevOp(s, GX_DECAL);
}

extern "C" void eggTexAlphaOff(u8 stage)
{
	GXSetTexCoordGen2(stage, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX1, GX_TRUE, GX_PTTEXMTX19);
}

extern "C" void eggTevAlphaOff(int stage)
{
	u8 s = stage;
	GXSetTevKColorSel(s, GX_TEV_KCSEL_1_2);
	GXSetTevColorIn(s, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
	GXSetTevColorOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(s, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(s, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
}

extern "C" void eggSetTexMtx(int map)
{
	f32 mtx[12];
	PSMTXCopy(_gj_matrix_, mtx);
	mtx[3]  = 0.0f;
	mtx[7]  = mtx[3];
	mtx[11] = mtx[3];
	GXLoadTexMtxImm(mtx, map, GX_MTX3x4);
}

// 0x80144990
void TObjEgg::render(int alt)
{
	struct {
		u8 mStageA;
		u8 mStageB;
		void (*mTexFunc)(u8);
		void (*mTevFunc)(int);
	} tev;
	NJS_TEXLIST texlist;
	NJS_TEXNAME texname;
	u32 idleA;
	u32 idleB;
	u32 hitA;
	u32 hitB;

	if (mEggFlags.mBits & 0x20) {
		return;
	}
	DrawStateSlot* draw = getDrawStateSlot();
	ObjDrawState param;
	if (alt != 0) {
		param.mFlags |= 0x101;
		param.mMaterial = 0xFFFFFFC8;
		param.mBlend    = 0;
	}
	if (mHitTimer != 0) {
		param.mFlags |= 0x20;
		hitB = 0x64000000;
		hitA = 0xFF000000;
		lerpColor(0.5f * njSin(mHitTimer << 11) + 0.5f, &param.mTev2, &hitA, &hitB);
	} else {
		int id = mLevel.mSize;
		if (id == mLevel.getMaxSize() || mGrow.isActive() != 0) {
			idleA = 0xFFFFFF00;
			idleB = 0;
			param.mFlags |= 0x20;
			lerpColor(0.5f * njSin(mPulse) + 0.5f, &param.mTev2, &idleA, &idleB);
		}
	}
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	if (mForm == 6) {
		eggSetTexMtx(0x21);
		param.mFlags |= 0x2000;
		tev.mStageA  = 1;
		tev.mStageB  = 1;
		tev.mTexFunc = eggTexAlphaOn;
		tev.mTevFunc = eggTevAlphaOn;
		param.mExtra = &tev;
		njSetTexture(sEggTexLists[6]);
		gjLoadTextureNum(3, 1, 1, 1);
	} else if (mHits != 0) {
		param.mFlags |= 0x2000;
		tev.mStageA  = 0;
		tev.mStageB  = 1;
		tev.mTevFunc = eggTevCrack;
		param.mExtra = &tev;
		njSetTexture(sEggTexLists[6]);
		gjLoadTextureNum(mHits - 1, 1, 1, 1);
	} else if (mEggFlags.mBits & 2) {
		eggSetTexMtx(0x21);
		param.mFlags |= 0x2000;
		tev.mStageA  = 1;
		tev.mStageB  = 1;
		tev.mTexFunc = eggTexAlphaOff;
		tev.mTevFunc = eggTevAlphaOff;
		param.mExtra = &tev;
		njSetTexture(sEggTexLists[6]);
		gjLoadTextureNum(2, 1, 1, 1);
	}
	texlist.textures  = &texname;
	texlist.nbTexture = 1;
	if (mTexture == 0) {
		texname.texaddr = (*(NJS_TEXLIST**)sEggTexLists)->textures->texaddr;
	} else {
		texname.texaddr = (GjTexMem*)mTexture;
	}
	njSetTexture(&texlist);
	shadowAlphaBegin();
	beginDrawState(draw, (const DrawState*)&param);
	gjDrawObject(sEggHatchModels[1]);
	endDrawState(draw);
	if (mForm == 8) {
		njSetTexture(sEggTexLists[4]);
		gjDrawObject(sEggHatchModels[5]);
	}
	njPopMatrixEx();
	shadowAlphaEnd();
}

extern "C" void eggDrawNormal(TObjEgg* egg)
{
	egg->render(0);
}

extern "C" void eggDrawAlt(TObjEgg* egg)
{
	egg->render(1);
}

int TObjEgg::isHolder(ObjMsg* msg)
{
	return ((EggThrowMsg*)msg)->mHolder == mHolderId;
}

// 0x80144CF0
int TObjEgg::onHit(const TObjAction* other, int arg, int* result)
{
	int ret;

	if (other->mObjId == mHolderId) {
		return 0;
	}
	int power = mLevel.getPower();
	if (mForm != 0) {
		power = 1.2f * power;
	}
	power *= mDamageMul;
	switch (mState) {
	case EGG_STATE_ROLLED:
	case EGG_STATE_BOUNCE:
	case EGG_STATE_SLOW:
	case EGG_STATE_ROLLED_P:
	case EGG_STATE_ROLLED_P2:
		((HitInfo*)result)->_14 = (u32)mThrowTag;
		ret                     = mRoll.hitWhileRolling(result, power);
		break;
	case EGG_STATE_HELD:
		ret = mHandle.mHolder->takeHit((HitInfo*)result, power);
		break;
	case EGG_STATE_SLAM:
		((HitInfo*)result)->_14 = (u32)mThrowTag;
		ret                     = mRoll.hitWhileHeld(result, power);
		break;
	default:
		return 0;
	}
	if (other->_00 != _00) {
		return ret;
	}
	if (mForm == 8 && ((TObjEgg*)other)->mForm != 8) {
		((HitInfo*)result)->mDamage = mLevel.getPower();
		((HitInfo*)result)->mFlags.setBit(0xD);
		return 1;
	}
	if (mLevel.mSize > ((TObjEgg*)other)->mLevel.mSize) {
		((HitInfo*)result)->mDamage = mLevel.getPower();
		((HitInfo*)result)->mFlags.setBit(0xD);
		return 1;
	}
	return 0;
}

void TObjEgg::runState(int state, int phase)
{
	switch (state) {
	case EGG_STATE_APPEAR:
		stateAppear(phase);
		break;
	case EGG_STATE_FALL:
		stateFall(phase);
		break;
	case EGG_STATE_BOUNCE_UP:
		stateBounceUp(phase);
		break;
	case EGG_STATE_IDLE:
		stateIdle(phase);
		break;
	case EGG_STATE_HELD:
		stateHeld(phase);
		break;
	case EGG_STATE_SLAM:
		stateSlam(phase);
		break;
	case EGG_STATE_HATCH:
		stateHatch(phase);
		break;
	case EGG_STATE_ROLLED:
		stateRolled(phase);
		break;
	case EGG_STATE_THROWN:
		stateThrown(phase);
		break;
	case EGG_STATE_BOUNCE:
		stateBounce(phase);
		break;
	case EGG_STATE_SLOW:
		stateSlow(phase);
		break;
	case EGG_STATE_EXPIRE:
		stateExpire(phase);
		break;
	case EGG_STATE_ROLLED_P:
		stateRolledP(phase);
		break;
	case EGG_STATE_ROLLED_P2:
		stateRolledP2(phase);
		break;
	}
}

void TObjEgg::setState(int state)
{
	if (mState != EGG_STATE_NONE) {
		runState(mState, EGG_EXIT);
	}
	if (state != EGG_STATE_NONE) {
		runState(state, EGG_ENTER);
	}
	mState = state;
}

int TObjEgg::isHandleBusy()
{
	ObjMsg msg;
	if (mHandle.mHolder != 0) {
		msg.mType = 0x41;
		return mHandle.send(&msg);
	}
	return 0;
}

void TObjEgg::getGrowScale(void* mtx)
{
	if (mGrow.isActive() != 0) {
		mGrow.getScale(mtx);
		f32 scale = 0.2f;
		mtxScale(scale, scale, scale, (float*)mtx, (const float*)mtx);
	} else {
		f32 scale = 0.2f * mBody.mRadius;
		mtxSetScale((float*)mtx, scale, scale, scale);
	}
}

void TObjEgg::updateWobble()
{
	f32 two = 2.0f;
	mWobbleVel += 0.18f * -mWobbleVel - 0.1f * (mWobbleVal - 1.0f);
	mWobbleVal += mWobbleVel;
	mWobble = two - mWobbleVal;
}

void TObjEgg::formFollow()
{
	mtxSetTrans((float*)mFxMatrix, mDrawPos.x, mDrawPos.y - mBody.mRadius, mDrawPos.z);
}

void TObjEgg::formFollowTop()
{
	mtxSetTrans((float*)mFxMatrix, mDrawPos.x, mDrawPos.y, mDrawPos.z);
}

void TObjEgg::startEffect(int id)
{
	mFormFx = (Fx*)fxSpawnFollow(1, 0, (u16)id, mFxMatrix);
}

void TObjEgg::enterFormA()
{
	startEffect(0x68);
}

void TObjEgg::enterFormB()
{
	startEffect(0x6B);
}

void TObjEgg::enterFormC()
{
	startEffect(0x6E);
}

void TObjEgg::enterFormD()
{
	startEffect(0x91);
	mAuraDrawObj = addDrawTask(this, (void (*)(void*))eggDrawAura, 0x2A, 0);
}

void TObjEgg::enterFormE()
{
	startEffect(0x73);
}

void TObjEgg::enterFormF()
{
}

void TObjEgg::enterFormG()
{
	startEffect(0x78);
}

void TObjEgg::enterFormH()
{
}

extern "C" void eggDrawAura(TObjEgg* egg)
{
	DrawStateSlot* draw = getDrawStateSlot();
	shadowAlphaBegin();
	njPushMatrixEx();
	njMultiMatrix(0, egg->mMatrix);
	njSetTexture(sEggTexLists[5]);
	beginDrawState(draw, getDefaultDrawState());
	gjDrawObject(sEggHatchModels[6]);
	endDrawState(draw);
	njPopMatrixEx();
	shadowAlphaEnd();
}

void TObjEgg::stateAppear(int phase)
{
	if (phase == EGG_ENTER) {
		mTimer = 30;
		mEggFlags.mBits |= 8;
		fxSpawnAt(1, 7, 0x1D, mRotMatrix);
		if (eventIsBusy() == 0) {
			playSe(0x9C);
		}
	} else if (phase == EGG_EXIT) {
		mEggFlags.mBits &= ~8;
	} else if (phase == EGG_UPDATE) {
		if (--mTimer == 0) {
			setState(EGG_STATE_IDLE);
		}
	} else if (phase == EGG_DRAW) {
		f32 scale = 0.2f * mBody.mRadius;
		mtxScale(scale, scale * (f32)(30 - mTimer) / 30.0f, scale, mMatrix, mRotMatrix);
	}
}

// 0x80145414
void TObjEgg::stateHeld(int phase)
{
	Vec3f pos;
	EggRot rot;

	if (phase == EGG_UPDATE) {
		if (checkHatchTouch() != 0) {
			setState(EGG_STATE_HATCH);
			return;
		}
		f32 one    = 1.0f;
		f32 zero   = 0.0f;
		mWobble    = one;
		mWobbleVal = one;
		mWobbleVel = zero;
	} else if (phase == EGG_DRAW) {
		updateWobble();
		eggPushHeld((void*)mColl, (EggContact*)mColl->mContacts, 0, mHandle.mHolder);
		TMobileObject* holder = (TMobileObject*)mHandle.mHolder;
		pos                   = holder->mBody.mPos;
		rot                   = holder->mBody.mRot;
		setPos(&pos);
		mBody.mRot = rot;
		updateMatrix();
		mRoll.checkBumps();
	}
}

void TObjEgg::stateRolled(int phase)
{
	if (phase == EGG_ENTER) {
		mTimer = *(u16*)((u8*)playerGetParams(0) + 0x18);
		mAuraA.start();
	} else if (phase == EGG_EXIT) {
		mAuraA.stop();
	} else if (phase == EGG_UPDATE) {
		if (mRoll.isRollDone() != 0) {
			setState(EGG_STATE_IDLE);
		} else if (checkThrown() != 0) {
			setState(EGG_STATE_THROWN);
		} else if (mRoll.hitWall() != 0) {
			mRoll.rebound();
			setState(EGG_STATE_BOUNCE);
		} else if (--mTimer == 0) {
			mRoll.stopRolling();
			setState(EGG_STATE_SLOW);
		}
	} else if (phase == EGG_DRAW) {
		mRoll.pushOff(mColl, mColl->mContacts, 0);
		limitVel(*(f32*)((u8*)mEggParam + 0x10));
		mRoll.moveRolling();
		updateMatrix();
		mAuraA.update();
	}
}

void TObjEgg::stateSlam(int phase)
{
	Vec3f pos;

	if (phase == EGG_UPDATE) {
		if (mRoll.isSlamDone() != 0) {
			setState(EGG_STATE_IDLE);
		} else if (checkThrown() != 0) {
			setState(EGG_STATE_THROWN);
		} else if (mRoll.hasLanded() != 0) {
			pos.x      = mDrawPos.x;
			f32 radius = mBody.mRadius;
			pos.y      = mDrawPos.y - radius;
			pos.z      = mDrawPos.z;
			u16 holder = mHolderId;
			u16 obj    = mObjId;
			eggSpawnShockWave(radius, obj, holder, &pos, mLevel.getPower(), mThrowTag);
			playSe(0x81);
			pos.y += 5.0f;
			particleOnceById(gTL_PARTICLE, &pos, 0, 9);
			camShakeNearSoft(200.0f, &mDrawPos);
			setState(EGG_STATE_IDLE);
		}
	} else if (phase == EGG_DRAW) {
		mRoll.pushOff(mColl, mColl->mContacts, 0);
		mRoll.moveSlam();
		updateMatrix();
	}
}

extern "C" void eggHatchCallback(void* model, EggHatchWalk* walk)
{
	if (walk->mCount == 1) {
		njGetMatrix(walk->mMtx);
	}
	walk->mCount++;
}

// 0x80145800
void TObjEgg::stateHatch(int phase)
{
	EggHatchWalk walk;
	Vec3f shatterPos;
	EggRot rot;
	Vec3f drawPos;
	EggRot rotB;

	if (phase == EGG_ENTER) {
		mEggFlags.mBits |= 8;
		mTimer      = 60;
		mHatchModel = new MotionPlayer;
		mHatchModel->init(sEggHatchMotions, sEggHatchMotionData);
		mHatchModel->play(0);
		mGlow.stop();
		playSe(0x96);
		hatchNoticePost(mBody.mRadius, 0, 0x13, &mDrawPos, 0x1E);
		setForm(0);
		prepareContent();
	} else if (phase == EGG_EXIT) {
		if (mContent.mNode.mList != 0) {
			itemHolderRelease(&mContent);
		}
		delete mHatchModel;
	} else if (phase == EGG_UPDATE) {
		if (tickTimer16(&mTimer) != 0) {
			int first;
			if (gEggAssetsLoaded == 0) {
				first = 0;
			} else {
				u16 flag = mLevel.getType() + 0x4C8;
				if (keyBitIsSet(flag) == 0) {
					setKeyBit(flag);
					first = 1;
				} else {
					first = 0;
				}
			}
			spawnContent(first);
			shatterPos = mBody.mPos;
			rot        = mBody.mRot;
			f32 radius = mBody.mRadius;
			createTEggEffBreak(radius, &shatterPos, &rot, mLevel.getType());
			playSe(0x97);
			int fresh = 0;
			if (mHits == 0 && mLevel.getType() != 0) {
				fresh = 1;
			}
			scoreEgg((u8)fresh);
			die();
		}
	} else if (phase == EGG_DRAW) {
		mHatchModel->update();
		walk.mCount = 0;
		drawPos.x   = mBody.mPos.x;
		drawPos.y   = mBody.mPos.y;
		drawPos.z   = mBody.mPos.z;
		drawPos.y   = mBody.mPos.y - mBody.mRadius;
		njPushMatrixEx();
		njUnitMatrix(0);
		njTranslateEx(&drawPos);
		MotionPlayer* model = mHatchModel;
		int cur             = model->mCurrent;
		f32 frame           = model->mFrame;
		NJS_MOTION* mot     = model->getMotion(cur);
		walkMotion(frame, sEggHatchModels[4], mot, (NodeCallback)eggHatchCallback, (void*)&walk);
		njPopMatrixEx();
		mtxCopy((f32*)mRotMatrix, (const f32*)walk.mMtx);
		mtxTranslate((float*)mRotMatrix, (const float*)mRotMatrix, 0.0f, mBody.mRadius - 5.0f, 0.0f);
		rotB      = mBody.mRot;
		f32 scale = mBody.mRadius / 5.0f;
		mtxConcatQuat((f32*)mRotMatrix, (const f32*)mRotMatrix, (const f32*)&rotB);
		mtxScale(scale, scale, scale, mMatrix, mRotMatrix);
	}
}

void TObjEgg::stateBounceUp(int phase)
{
	f32 mtx[12];

	if (phase == EGG_ENTER) {
		mtxSetTransV(mtx, &mDrawPos);
		fxSpawnAt(1, 7, 0x48, mtx);
		mTilt.wobble();
		setForm(0);
	} else if (phase == EGG_UPDATE) {
		if (mRoll.isGrounded() != 0) {
			setState(EGG_STATE_IDLE);
		}
	} else if (phase == EGG_DRAW) {
		mRoll.pushOff(mColl, mColl->mContacts, 0);
		mRoll.moveGround();
		updateMatrix();
	}
}

// 0x80145BC0
void TObjEgg::stateFall(int phase)
{
	f32 mtx[12];

	if (phase == EGG_ENTER) {
		mTimer = 120;
		mEggFlags.mBits |= 8;
		mGlow.stop();
		f32 scale = mBody.mRadius / 5.0f;
		mtxSetTransV(mtx, &mDrawPos);
		mtxScale(scale, scale, scale, mtx, mtx);
		fxSpawnAt(1, 0, 0x20, mtx);
		setForm(0);
	} else if (phase == EGG_UPDATE) {
		if (--mTimer == 0) {
			die();
		}
	} else if (phase == EGG_DRAW) {
		mBody.mPos.addY(-0.5f);
		updateMatrix();
	}
}

void TObjEgg::stateIdle(int phase)
{
	if (phase == EGG_ENTER) {
		mTimer = 1800;
		setForm(0);
		setDamageMul(1);
		setDamageAdd(0);
	} else if (phase == EGG_UPDATE) {
		if (isInRange((void*)this, (const Vec3f*)&mDrawPos, 1000.0f) == 0) {
			mFlags |= 1;
		} else if (checkHatchTouch() != 0) {
			setState(EGG_STATE_HATCH);
		} else if ((mEggFlags.mBits & 0x10) != 0 && --mTimer == 0) {
			setState(EGG_STATE_EXPIRE);
		}
	} else if (phase == EGG_DRAW) {
		mRoll.pushOff(mColl, mColl->mContacts, 0);
		limitVel(*(f32*)((u8*)mEggParam + 4));
		mRoll.moveGround();
		updateMatrix();
	}
}

void TObjEgg::stateThrown(int phase)
{
	if (phase == EGG_ENTER) {
		setForm(0);
	} else if (phase == EGG_UPDATE) {
		if (mRoll.isPathDone() != 0) {
			mRoll.leavePath();
			setState(EGG_STATE_IDLE);
		}
	} else if (phase == EGG_DRAW) {
		mRoll.movePath();
		updateMatrix();
	}
}

void TObjEgg::stateBounce(int phase)
{
	if (phase == EGG_ENTER) {
		mAuraA.start();
	} else if (phase == EGG_EXIT) {
		mAuraA.stop();
	} else if (phase == EGG_UPDATE) {
		if (mRoll.isRollDone() != 0) {
			setState(EGG_STATE_IDLE);
		} else if (mRoll.isPastHolder() != 0) {
			setState(EGG_STATE_IDLE);
		}
	} else if (phase == EGG_DRAW) {
		mRoll.pushOff(mColl, mColl->mContacts, 0);
		mRoll.moveBounce();
		updateMatrix();
		mAuraA.update();
	}
}

void TObjEgg::stateSlow(int phase)
{
	if (phase == EGG_ENTER) {
		mTimer = *(u16*)((u8*)playerGetParams(0) + 0x1A);
		mAuraB.start();
		playSe(0x80);
	} else if (phase == EGG_EXIT) {
		mAuraB.stop();
	} else if (phase == EGG_UPDATE) {
		if (mRoll.isRollDone() != 0) {
			setState(EGG_STATE_IDLE);
		} else if (mRoll.hitWall() != 0) {
			mRoll.reboundHard();
			setState(EGG_STATE_BOUNCE);
		} else if (--mTimer == 0) {
			mRoll.reboundHard();
			setState(EGG_STATE_BOUNCE);
		}
	} else if (phase == EGG_DRAW) {
		mRoll.pushOff(mColl, mColl->mContacts, 0);
		mRoll.moveSlow();
		updateMatrix();
		mAuraB.update();
	}
}

// 0x80146030
void TObjEgg::stateExpire(int phase)
{
	EggRot rot;
	u8 base[0x30];
	f32 mtx[12];

	if (phase == EGG_ENTER) {
		mTimer = 30;
		mEggFlags.mBits |= 8;
		mGlow.stop();
		playSe(0x9C);
		mtxSetTransV((f32*)base, &mDrawPos);
		fxSpawnAt(1, 7, 0x1D, (const f32*)base);
	} else if (phase == EGG_UPDATE) {
		if (--mTimer == 0) {
			die();
		}
	} else if (phase == EGG_DRAW) {
		f32 xz;
		f32 t     = (f32)mTimer / 30.0f;
		f32 scale = 0.2f * mBody.mRadius;
		rot       = mBody.mRot;
		f32 grow  = 0.5f * t + 1.0f;
		xz        = scale * t;
		scale *= grow;
		mtxSetTransV(mtx, &mDrawPos);
		mtxScale(xz, scale, xz, mtx, mtx);
		mtxConcatQuat((f32*)mMatrix, (const f32*)mtx, (const f32*)&rot);
	}
}

// 0x80146190
void TObjEgg::stateRolledP(int phase)
{
	ObjMsg msg;
	f32 mtx[12];

	if (phase == EGG_ENTER) {
		mTimer = *(u16*)((u8*)playerGetParams(0) + 0x18);
		mEggFlags.mBits &= ~0x40;
	} else if (phase == EGG_EXIT) {
		if ((mEggFlags.mBits & 0x40) == 0) {
			msg.mType = 0x31;
			sendMsgById(mHolderId, &msg);
			s8 player = (s8)mPlayer;
			camSetTarget(player, findObjById(mHolderId));
			camResetCamera((s8)mPlayer);
		}
	} else if (phase == EGG_UPDATE) {
		mtxSetRotY((float*)mtx, (int)camGetYaw((s8)mPlayer));
		mPush.read(mHolderId, (f32*)mtx);
		if (mPush.mStick > 0.0f) {
			mEggFlags.mBits |= 0x40;
			setState(EGG_STATE_ROLLED_P2);
		} else if (mRoll.isRollDone() != 0) {
			setState(EGG_STATE_IDLE);
		} else if (checkThrown() != 0) {
			setState(EGG_STATE_THROWN);
		} else if (mRoll.hitWall() != 0) {
			mRoll.rebound();
			setState(EGG_STATE_BOUNCE);
		} else if (--mTimer == 0) {
			mRoll.stopRolling();
			setState(EGG_STATE_SLOW);
		}
	} else if (phase == EGG_DRAW) {
		mRoll.pushOff(mColl, mColl->mContacts, 0);
		limitVel(*(f32*)((u8*)mEggParam + 0x10));
		mRoll.moveRolling();
		updateMatrix();
		Vec3f vel;
		vel.x      = mBody.mVel.x;
		vel.y      = mBody.mVel.y;
		vel.z      = mBody.mVel.z;
		mPushAngle = vecAngleXZ(&vel);
	}
}

// 0x80146384
void TObjEgg::stateRolledP2(int phase)
{
	f32 mtxC[12];
	f32 mtxB[12];
	f32 mtxA[12];
	Vec3f start;
	Vec3f push;
	Vec3f vel;
	Vec3f pos;
	ObjMsg msg;

	if (phase == EGG_ENTER) {
		start.x = mBody.mPos.x;
		start.y = mBody.mPos.y;
		start.z = mBody.mPos.z;
		start.y = mBody.mPos.y - mBody.mRadius;
		mtxSetTransV(mtxC, &start);
		mPlayerFx = fxSpawnAt(1, 0, 0x7D, mtxC);
	} else if (phase == EGG_EXIT) {
		fxStop(mPlayerFx);
		msg.mType = 0x31;
		sendMsgById(mHolderId, &msg);
		s8 player = (s8)mPlayer;
		camSetTarget(player, findObjById(mHolderId));
		camResetCamera((s8)mPlayer);
	} else if (phase == EGG_UPDATE) {
		if (isInRange((void*)this, (const Vec3f*)&mDrawPos, 400.0f) == 0) {
			setState(EGG_STATE_IDLE);
		} else {
			mtxSetRotY((float*)mtxB, (int)camGetYaw((s8)mPlayer));
			mPush.read(mHolderId, (f32*)mtxB);
			if (checkThrown() != 0) {
				setState(EGG_STATE_THROWN);
			}
		}
	} else if (phase == EGG_DRAW) {
		if (mPush.mStick > 0.0f) {
			vecFromAngleXZ(300.0f * mPush.mStick, &push, (int)mPush.mAngle);
			njAddVector(&mBody.mAccel, &push);
		}
		mRoll.pushOff(mColl, mColl->mContacts, 0);
		limitVel(*(f32*)((u8*)mEggParam + 4));
		mRoll.moveRolling();
		updateMatrix();
		vel.x      = mBody.mVel.x;
		vel.y      = mBody.mVel.y;
		vel.z      = mBody.mVel.z;
		mPushAngle = vecAngleXZ(&vel);
		if (mPlayerFx != 0) {
			pos.x = mBody.mPos.x;
			pos.y = mBody.mPos.y;
			pos.z = mBody.mPos.z;
			pos.y = mBody.mPos.y - mBody.mRadius;
			mtxSetTransV(mtxA, &pos);
			mtxCopy(mPlayerFx->mMatrix, (const f32*)mtxA);
		}
	}
}

int TObjEgg::msgPush(ObjMsg* msg)
{
	njAddVector(&mBody.mAccel, &((ObjVecMsg*)msg)->mVec);
	return 1;
}

int TObjEgg::msgHit(ObjMsg* msg)
{
	if (mEggFlags.mBits & 8) {
		return 0;
	}
	shatter();
	return 1;
}

// 0x80146604
int TObjEgg::msgFeed(ObjMsg* msg)
{
	int level = mLevel.mSize;
	mLevel.feed(((EggFeedMsg*)msg)->mFood, ((EggFeedMsg*)msg)->mAmount);
	int now = mLevel.mSize;
	if (now != level) {
		onSizeChange(now);
	}
	return 1;
}

int TObjEgg::msgRoll(ObjMsg* msg)
{
	switch (mState) {
	case EGG_STATE_IDLE:
		mRoll.kick(((EggKickMsg*)msg)->mAngle);
		return 1;
	case EGG_STATE_SLAM:
		mRoll.kickHeld();
		return 1;
	case EGG_STATE_ROLLED:
	case EGG_STATE_BOUNCE:
	case EGG_STATE_SLOW:
	case EGG_STATE_ROLLED_P:
		mRoll.reboundHard();
		setState(EGG_STATE_BOUNCE);
		return 1;
	default:
		return 0;
	}
}

// 0x801466F8
int TObjEgg::msgFeedMax(ObjMsg* msg)
{
	int level    = mLevel.mSize;
	mSavedPoints = mLevel.mPoints;
	mLevel.feed(0, 100);
	int now = mLevel.mSize;
	if (now != level) {
		mFeedTimer = 1800;
		onSizeChange(now);
	}
	return 1;
}

int TObjEgg::msgGetLevel(ObjMsg* msg)
{
	mLevel.getProgress(msg);
	((EggLevelMsg*)msg)->mHits = mHits;
	return 1;
}

int TObjEgg::msgAbilities(ObjMsg* msg)
{
	return 0x3F;
}

int TObjEgg::msgGetFlags(ObjMsg* msg)
{
	int flags = isHandleBusy();
	switch (mState) {
	case EGG_STATE_ROLLED:
	case EGG_STATE_BOUNCE:
	case EGG_STATE_SLOW:
		flags |= 0x200;
		break;
	}
	return flags;
}

int TObjEgg::msgGetVel(ObjMsg* msg)
{
	((ObjVecMsg*)msg)->mVec = mBody.mVel;
	return 1;
}

int TObjEgg::msgRollBy(ObjMsg* msg)
{
	if (isHolder(msg) == 0) {
		return 0;
	}
	if (mState == EGG_STATE_HELD) {
		clearHolder();
		mRoll.setVel(&((EggThrowMsg*)msg)->mVel);
		mThrowTag = (void*)scoreNewChain(0);
		setState(EGG_STATE_ROLLED);
		return 1;
	}
	return 0;
}

int TObjEgg::msgStop(ObjMsg* msg)
{
	if (mState != EGG_STATE_ROLLED) {
		return 0;
	}
	mBody.mVel = gVecZero;
	setState(EGG_STATE_IDLE);
	return 1;
}

int TObjEgg::msgTake(ObjMsg* msg)
{
	switch (mState) {
	case EGG_STATE_IDLE:
		if (takeBy(msg) != 0) {
			setState(EGG_STATE_HELD);
			return 1;
		}
		return 0;
	case EGG_STATE_ROLLED:
	case EGG_STATE_BOUNCE:
	case EGG_STATE_SLOW:
		if (takeBy(msg) != 0) {
			mBody.mVel = gVecZero;
			setState(EGG_STATE_HELD);
			return 1;
		}
		return 0;
	}
	return 0;
}

int TObjEgg::msgRelease(ObjMsg* msg)
{
	if (isHolder(msg) == 0) {
		return 0;
	}
	if (mState == EGG_STATE_HELD) {
		clearHolder();
		setState(EGG_STATE_IDLE);
		return 1;
	}
	if (mState == EGG_STATE_ROLLED_P || mState == EGG_STATE_ROLLED_P2) {
		setState(EGG_STATE_IDLE);
		return 1;
	}
	return 0;
}

int TObjEgg::msgThrow(ObjMsg* msg)
{
	if (isHolder(msg) == 0) {
		return 0;
	}
	if (mState == EGG_STATE_HELD) {
		clearHolder();
		mRoll.attachPathAt(((EggPathMsg*)msg)->mPath, ((EggPathMsg*)msg)->mPoint, ((EggPathMsg*)msg)->mSpeed);
		setState(EGG_STATE_THROWN);
		return 1;
	}
	return 0;
}

// 0x80146AE4
int TObjEgg::msgRollByPlayer(ObjMsg* msg)
{
	if (isHolder(msg) == 0) {
		return 0;
	}
	if (mState != EGG_STATE_HELD) {
		return 0;
	}
	clearHolder();
	EggThrowMsg* roll = (EggThrowMsg*)msg;
	mPlayer           = *((s8*)findObjById(((EggThrowMsg*)msg)->mHolder) + 0x24);
	camSetTarget((s8)mPlayer, this);
	mPushAngle = vecAngleXZ(&roll->mVel);
	mRoll.setVel(&roll->mVel);
	mThrowTag = (void*)scoreNewChain(0);
	setState(EGG_STATE_ROLLED_P);
	return 1;
}

int TObjEgg::msgTakeIdle(ObjMsg* msg)
{
	EggHoldMsg* data = (EggHoldMsg*)msg;
	if (mState != EGG_STATE_IDLE) {
		return 0;
	}
	if (mHandle.mHolder != 0) {
		return 0;
	}
	if (mRoll.isGrounded() == 0) {
		return 0;
	}
	data->mHandle = &mHandle;
	data->mPos    = mBody.mPos;
	data->mRot    = mBody.mRot;
	data->mRadius = mBody.mRadius;
	mOwnerId      = data->mHolder;
	mHolderId     = data->mHolder;
	setState(EGG_STATE_HELD);
	return 1;
}

int TObjEgg::msgSlam(ObjMsg* msg)
{
	if (isHolder(msg) == 0) {
		return 0;
	}
	if (mState == EGG_STATE_HELD) {
		clearHolder();
		mRoll.startSlam(&((EggThrowMsg*)msg)->mVel);
		mThrowTag = (void*)scoreNewChain(1);
		setState(EGG_STATE_SLAM);
		return 1;
	}
	return 0;
}

// 0x80146CEC
extern "C" int eggContentA(int id)
{
	if (id >= 0x14 && id <= 0x21) {
		return sEggItemIdsHigh[id - 0x14];
	}
	if (id >= 0x38 && id <= 0x41) {
		return sEggItemIdsHigh[id - 0x38 + 14];
	}
	return -1;
}

extern "C" {
int sEggItemIdsLow[41] = {
	22, 23, 24, 25, 26, 27, 28, 29, 30, 0,  1,  31, 2,  32, 33, 34, 35, 3,  4,  5,  6,
	7,  8,  9,  10, 11, 12, 13, 14, 15, 17, 18, 16, 19, 20, 21, 36, 39, 37, 38, 40,
};
}

// 0x80146D44
extern "C" int eggContentB(int id)
{
	if (id >= 1 && id <= 0x13) {
		return sEggItemIdsLow[id - 1];
	}
	if (id >= 0x22 && id <= 0x37) {
		return sEggItemIdsLow[id - 0x22 + 19];
	}
	return 0xFFFF;
}

f32 TObjEgg::getRadiusV()
{
	return getRadius();
}

int TObjEgg::getKind()
{
	return 1;
}

inline int EggHandle::onMessage(ObjMsg* msg)
{
	return ((TObjEgg*)mOwner)->onMessage(msg);
}
