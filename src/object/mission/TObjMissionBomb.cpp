#include "engine/TMobileObject.h"
struct DrawTask;
#include "io/Resource.h"
#include "item/TItemManage.h"
#include "stage/SetManager.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "object/ObjModel.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCamera.h"
#include "ui/AeArc.h"
#include "io/NinjaFile.h"
#include "object/TObjExplode.h"
#include "core/MathUtil.h"
#include "object/ObjShadow.h"
#include "object/TObjLauncher.h"
#include "engine/Color.h"
#include "engine/DrawState.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjAction.h"
#include "stage/TGameState.h"
#include "sound/Sound3DPlay.h"
#include "engine/FxSystem.h"
#include "camera/TCameraCtrl.h"
#include "engine/Colli.h"
#include "engine/GroundQuery.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "stage/SetSlot.h"
#include "object/TObjKey00.h"
#include "stage/StageTime.h"
#include "engine/TParticle.h"
#include "core/VecMath.h"
#include "egg/EggLevel.h"
#include "object/mission/TObjMissionBomb.h"
#include "core/TMainTask.h"
#include "core/SdkCalls.h"
struct NJS_TEXLIST;
struct ObjMsg;
struct SetLink;
struct CharaColli;
struct DrawState;
struct DrawStateSlot;
struct NJS_OBJECT;
struct Fx;

#define GX_FIFO_F32 (*(volatile f32*)0xCC008000)
#define GX_FIFO_U32 (*(volatile u32*)0xCC008000)

enum BombState {
	BOMB_GROUND  = 0,
	BOMB_HELD    = 1,
	BOMB_SLAM    = 2,
	BOMB_ROLLING = 3,
	BOMB_EXPLODE = 4,
};

enum BombPhase {
	BOMB_ENTER  = 0,
	BOMB_EXIT   = 1,
	BOMB_UPDATE = 2,
	BOMB_CHECK  = 3,
};

enum BombFlags {
	BOMB_FLAG_BURST   = 2, // the roll hit something it should blow up on
	BOMB_FLAG_EXPLODE = 8,
};

class TObjMissionBomb;

struct BombNotice {
	u16 mType; // _00
	u8 _02[2];
	u32 mLimit; // _04
	u32 mLeft;  // _08
};

// What the bomb's draw state asks for when it sets up the second texture
struct BombTexSetup {
	u8 mTexMap;          // _00
	u8 mTexCoord;        // _01
	void (*mTexGen)(u8); // _04
	void (*mTev)(u8);    // _08
};

typedef char TObjMissionBomb_size_check[(sizeof(TObjMissionBomb) == 0x374) ? 1 : -1];

// 0x80109A58
// EggRoll calls this for each thing the roll touches
static void onBombRollHit(void*, void*, void*, void*, void*, u32 flags, TObjMissionBomb* bomb);

// 0x80109A70
static void bombTev(u8 stage);

// 0x80109B14
static void bombTexGen(u8 coord);

// 0x80107F90
static void bindBombTextures(TArc* arc)
{
	void** texlists = (void**)findArcExport(arc, "bomb_texlists");
	void** textures = (void**)findArcExport(arc, "bomb_textures");
	for (int i = 0; i < 2; i++) {
		bindGvmTextures((u8*)textures[0], (NJS_TEXLIST*)texlists[i]);
	}
}

// 0x80108000
static void releaseBombTextures(TArc* arc)
{
	void** texlists = (void**)findArcExport(arc, "bomb_texlists");
	if (texlists != 0) {
		for (int i = 0; i < 2; i++) {
			releaseTextures((NJS_TEXLIST*)texlists[i]);
		}
	}
}

// 0x80108058
extern "C" void bindBombArc(TArc* arc)
{
	bindBombTextures(arc);
}

// 0x80108078
extern "C" void releaseBombArc(TArc* arc)
{
	releaseBombTextures(arc);
}

// 0x80108098
extern "C" void initMissionBomb()
{
	sMissionBombArc = 0;
}

// 0x801080A4
extern "C" void termMissionBomb()
{
	if (sMissionBombArc != 0) {
		releaseBombTextures(sMissionBombArc);
		releaseResource(sMissionBombArc);
	}
	sMissionBombArc = 0;
}

// 0x801080E0
static void loadMissionBomb()
{
	loadArc("obj_ms_bomb.arc", &sMissionBombArc);
	if (sMissionBombArc != 0) {
		bindBombTextures(sMissionBombArc);
	}
}

// 0x8010811C
extern "C" int loadTObjMissionBomb()
{
	loadMissionBomb();
	return 1;
}

// 0x80108140
extern "C" TObject* createTObjMissionBomb(TObject* parent, SetDesc* desc)
{
	if (!desc->isValid() && keyBitIsSet(desc->getInt(1)) != 0) {
		return 0;
	}
	TObjMissionBomb* bomb = new TObjMissionBomb(parent, desc->mObjId);
	if (bomb != 0) {
		bomb->mLink.get()->attach(desc);
		bomb->initFromDesc(desc);
		bomb->start();
	}
	return bomb;
}

// 0x80108208
// A bomb something else drops with thirty seconds left
extern "C" TObjMissionBomb* spawnTObjMissionBomb(u16 owner, const Vec3f* pos)
{
	TObjMissionBomb* bomb = new TObjMissionBomb(gTL_04, 0xFFFF);
	if (bomb != 0) {
		bomb->mLimit    = 30000;
		bomb->mHolderId = owner;
		bomb->setPos(pos);
		bomb->start();
	}
	return bomb;
}

const char* gTObjMissionBombName = "TObjMissionBomb";

// 0x80108288
TObjMissionBomb::TObjMissionBomb(TObject* parent, u16 id)
    : TObjAction(parent)
    , mOwner(0xFFFF)
    , mState(-1)
    , mSwitch(-1)
    , mTimeLeft(0)
    , mLimit(0)
    , mRopeSeg(6)
    , mRopeFrac(1.0f)
    , mModels(0)
    , mTexLists(0)
{
	_00   = (u32)gTObjMissionBombName;
	mSize = 0x374;
	setObjId(this, id);

	mColli = colliCreate(1, 1);
	colliSetOwner(mColli, (u16)mObjId);
	colliSetMasks((CharaColli*)mColli, 8, 0xF3, 0xF0, 0);
	colliShapeSetSphere(colliGetShape((CharaColli*)mColli, 0));
	colliGetShape((CharaColli*)mColli, 0)->mFlags = 0x42C;
	colliGetShape((CharaColli*)mColli, 0)->mSizeX = 7.5f;
	mDrawTask                                     = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);

	mBody.mBounce  = 0.0f;
	mBody.mRadius  = 7.5f;
	EggRoll* r     = roll();
	r->mOwner      = (TObjEgg*)this;
	r->mHitCb      = (void*)onBombRollHit;
	r->mHitArg     = this;
	r->mBody       = &mBody;
	mHandle.mOwner = this;
	mShadow._30    = 15.0f;
	mShadow._34    = 15.0f;

	if (sMissionBombArc != 0) {
		loadModels(sMissionBombArc);
	} else if (itemHolderAttach(2, &mItem) != 0) {
		int has = mItem.hasItem();
		if ((has ? mItem.mEntry->mData.mArc : 0) != 0) {
			loadModels(has ? mItem.mEntry->mData.mArc : 0);
		}
	}
}

// 0x80108518
TObjMissionBomb::~TObjMissionBomb()
{
	if (mItem.mNode.mList != 0) {
		itemHolderRelease(&mItem);
	}
	removeDrawTask(mDrawTask);
	colliFree((CharaColli*)mColli);
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	unregisterId();
}

// 0x80108614
void TObjMissionBomb::loadModels(TArc* arc)
{
	mModels   = (void**)findArcExport(arc, "bomb_models");
	mTexLists = (void**)findArcExport(arc, "bomb_texlists");
}

// 0x80108668
void TObjMissionBomb::start()
{
	updateMatrix();
	mFuse     = particleGenById(this, &mPos, 0, 0xF7);
	mTimeLeft = mLimit;
	for (int i = 0; i < 8; i++) {
		mRope.mNodes[i].mPos.x = mPos.x;
		mRope.mNodes[i].mPos.y = 3.0f * i + mPos.y + 15.0f;
		mRope.mNodes[i].mPos.z = mPos.z;
		mRope.mNodes[i].mPrev  = mRope.mNodes[i].mPos;
		mRope.mNodes[i].mVel.x = 0.0f;
		mRope.mNodes[i].mVel.y = 0.0f;
		mRope.mNodes[i].mVel.z = 0.0f;
	}
	mRope.mNodes[7].mPos.x += 10.0f;
	setState(BOMB_GROUND);
}

// 0x8010875C
void TObjMissionBomb::setPos(const Vec3f* pos)
{
	mPos       = *pos;
	mBody.mPos = *pos;
	colliShapeSetPos(colliGetShape(mColli, 0), pos);
}

// 0x801087BC
// The rope is verlet Its root sits on the bomb and everything below it swings
void TObjMissionBomb::updateRope(f32 groundY)
{
	Vec3f dir;
	Vec3f bend;
	Vec3f next;
	Vec3f up;
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;
	mtxMultVecSR(mMatrix, &up, &up);

	vecScaleAdd(8.5f, &mRope.mNodes[0].mPos, &up, &mPos);
	mRope.mNodes[0].mPrev = mRope.mNodes[0].mPos;

	int i;
	for (i = 1; i < 8; i++) {
		BombRopeNode* node = &mRope.mNodes[i];
		vecScaleAdd(0.85f, &node->mPos, &node->mVel, &node->mPos);
		mRope.mNodes[i].mPos.y += -1.0f / 6.0f;
	}

	for (i = 0; i < 8; i++) {
		if (i != 0) {
			vecSub(&dir, &mRope.mNodes[i].mPos, &mRope.mNodes[i - 1].mPos);
			f32 stretch = 0.45f * (vecNormalize(&dir, &dir) - 3.0f);
			vecScaleAdd(-stretch, &mRope.mNodes[i].mPos, &dir, &mRope.mNodes[i].mPos);
			vecScaleAdd(stretch, &mRope.mNodes[i - 1].mPos, &dir, &mRope.mNodes[i - 1].mPos);
		} else {
			dir = up;
		}
		if (i != 7) {
			vecSub(&next, &mRope.mNodes[i + 1].mPos, &mRope.mNodes[i].mPos);
			vecNormalize(&next, &next);
			vecSub(&bend, &dir, &next);
			if (njInnerProduct(&dir, &next) < 0.6f) {
				vecScaleAdd(0.5f, &mRope.mNodes[i + 1].mPos, &bend, &mRope.mNodes[i + 1].mPos);
			}
		}
		f32 below = mRope.mNodes[i].mPos.y - groundY;
		if (below < 0.0f) {
			mRope.mNodes[i].mPos.y -= 0.1f * below;
		}
	}

	for (i = 1; i < 8; i++) {
		Vec3f* pos  = &mRope.mNodes[i].mPos;
		Vec3f* prev = &mRope.mNodes[i].mPrev;
		vecSub(&mRope.mNodes[i].mVel, pos, prev);
		*prev = *pos;
	}

	if (mFuse != 0) {
		Vec3f spark;
		vecLerp(mRopeFrac, &mRope.mNodes[mRopeSeg].mPos, &mRope.mNodes[mRopeSeg + 1].mPos, &spark);
		mFuse->mPos = spark;
	}
}

// 0x80108A8C
int TObjMissionBomb::handleMessage(ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x01:
		return onPush(msg);
	case 0x40:
		return onGetType();
	case 0x41:
		return onGetFlags();
	case 0x43:
		return onGetVel(msg);
	case 0x64:
		return onThrow(msg);
	case 0x69:
		return onDrop(msg);
	case 0x6A:
		return onPickUp(msg);
	case 0x71:
		return onSlam(msg);
	default:
		return 0;
	}
}

// 0x80108B50
int TObjMissionBomb::sendToHandle(ObjMsg* msg)
{
	return mHandle.send(msg);
}

// 0x80108B74
void TObjMissionBomb::initFromDesc(SetDesc* desc)
{
	int minutes = desc->getInt(0);
	if (minutes <= 0) {
		minutes = 1;
	}
	mLimit = (u8)minutes * 60000;
	Vec3f pos;
	desc->getPos(&pos);
	pos.y += mBody.mRadius;
	setPos(&pos);
	mSwitch = desc->getInt(1);
}

// 0x80108C28
void TObjMissionBomb::debugPrint(void*)
{
	njPrint(0xA0013, ":LIMIT TIME(MINUTE)");
	njPrint(0xA0014, ":KEY ID");
}

// 0x80108C70
void TObjMissionBomb::draw()
{
	if (debugEditActive() != 0) {
		updateMatrix();
		return;
	}
	stageTimerTick(&mTimeLeft);
	if ((mFlagBits.mBits & BOMB_FLAG_EXPLODE) == 0) {
		if (mTimeLeft == 0) {
			setState(BOMB_EXPLODE);
		} else if ((mFlagBits.mBits & BOMB_FLAG_BURST) != 0) {
			setState(BOMB_EXPLODE);
		} else if (mPos.y < 0.0f) {
			setState(BOMB_EXPLODE);
		}
	}
	runState(mState, BOMB_CHECK);
	runState(mState, BOMB_UPDATE);
	mPos = mBody.mPos;
	colliShapeSetPos(colliGetShape(mColli, 0), &mPos);

	EggHits hits;
	f32 ground = 0.0f;
	if ((u32)meshColliProbe((EggHit*)&hits, (const Vec3f*)&mPos, -1) != 0 && (hits.mMask & 1) != 0) {
		shadowSetGround(&mShadow, &hits.mHits[0].mPos, &hits.mHits[0].mNormal);
		shadowEnable(&mShadow);
		ground = 3.0f + hits.mHits[0].mPos.y;
	} else {
		shadowDisable(&mShadow);
	}
	updateRope(ground);

	BombNotice notice;
	notice.mType  = 0x89;
	notice.mLimit = mLimit;
	notice.mLeft  = mTimeLeft;
	sendGameMsg((ObjMsg*)&notice);

	if (mTimeLeft < 10000) {
		f32 t     = mTimeLeft / 10000.0f;
		mRopeSeg  = (int)(7.0f * t);
		mRopeFrac = (21.0f * t - 3.0f * mRopeSeg) / 3.0f;
	}
}

// 0x80108E90
void TObjMissionBomb::vfn_10()
{
	if (cameraIsVisible(1000.0f, mBody.mRadius, &mPos, 0) == 0) {
		mDrawTask->mFlags |= 1;
	} else {
		mDrawTask->mFlags &= ~1;
	}
}

// 0x80108EF8
void TObjMissionBomb::render()
{
	f32 mtx[12];
	mtxConcat(mtx, gjGetMatrix(), mMatrix);
	if (mModels == 0) {
		mtxScale(7.5f, 7.5f, 7.5f, mtx, mtx);
		debugWireSphere(mtx, &gColorBlack);
		return;
	}

	DrawStateSlot* slot = getDrawStateSlot();
	ObjDrawState state;
	njPushMatrix(mtx);
	njSetTexture(mTexLists[1]);
	gjLoadTextureNum(0, 0, 0, 1);

	BombTexSetup setup;
	state.mFlags |= 0x2000;
	setup.mTexMap   = 1;
	setup.mTexCoord = 1;
	setup.mTexGen   = (void (*)(u8))bombTexGen;
	setup.mTev      = bombTev;
	state.mExtra    = &setup;

	f32 tex[12];
	PSMTXCopy(_gj_matrix_, tex);
	tex[3]  = 0.0f;
	tex[7]  = 0.0f;
	tex[11] = 0.0f;
	GXLoadTexMtxImm(tex, GX_TEXMTX5, GX_MTX3x4);

	shadowAlphaBegin();
	beginDrawState(slot, (const DrawState*)&state);
	njSetTexture(mTexLists[0]);
	gjDrawObject((NJS_OBJECT*)mModels[0]);
	endDrawState(slot);
	shadowAlphaEnd();
	njPopMatrixEx();

	// The part of the fuse still unburnt up to the spark
	njPushMatrixEx();
	njSetTexture(mTexLists[1]);
	njSetTextureNum(1);
	gjStartVertex3D();
	gjStartVertexTexOpaque();
	gjSetTexture();
	GXSetLineWidth(0x14, GX_TO_ONE);
	GXEnableTexOffsets(GX_TEXCOORD0, GX_ENABLE, GX_ENABLE);
	GXBegin(GX_LINESTRIP, GX_VTXFMT4, mRopeSeg + 2);

	f32 uv             = 0.0f;
	BombRopeNode* node = mRope.mNodes;
	for (int i = 0; i <= mRopeSeg; i++) {
		f32 x, y, z;
		z           = node->mPos.z;
		y           = node->mPos.y;
		x           = node->mPos.x;
		GX_FIFO_F32 = x;
		GX_FIFO_F32 = y;
		GX_FIFO_F32 = z;
		GX_FIFO_U32 = 0xFFFFFFFF;
		GX_FIFO_F32 = uv;
		GX_FIFO_F32 = uv;
		node++;
	}
	Vec3f spark;
	vecLerp(mRopeFrac, &node[-1].mPos, &node[0].mPos, &spark);
	GX_FIFO_F32 = spark.x;
	GX_FIFO_F32 = spark.y;
	GX_FIFO_F32 = spark.z;
	GX_FIFO_U32 = 0xFFFFFFFF;
	GX_FIFO_F32 = uv;
	GX_FIFO_F32 = uv;

	GXEnableTexOffsets(GX_TEXCOORD0, GX_DISABLE, GX_DISABLE);
	gjEndVertex();
	njPopMatrixEx();
}

// 0x80109140
int TObjMissionBomb::onHit(const TObjAction* other, int, int* result)
{
	if (other->mObjId == mHolderId) {
		return 0;
	}
	switch (mState) {
	case BOMB_ROLLING:
		return roll()->hitWhileRolling(result, 10);
	case BOMB_HELD:
		return ((TMobileObject*)mHandle.mHolder)->takeHit((HitInfo*)result, 10);
	case BOMB_SLAM:
		return roll()->hitWhileHeld(result, 10);
	}
	return 0;
}

// 0x801091DC
void TObjMissionBomb::runState(int state, int phase)
{
	switch (state) {
	case BOMB_GROUND:
		stateGround(phase);
		break;
	case BOMB_HELD:
		stateHeld(phase);
		break;
	case BOMB_SLAM:
		stateSlam(phase);
		break;
	case BOMB_ROLLING:
		stateRolling(phase);
		break;
	case BOMB_EXPLODE:
		stateExplode(phase);
		break;
	}
}

// 0x8010925C
void TObjMissionBomb::setState(int state)
{
	if (mState != -1) {
		runState(mState, BOMB_EXIT);
	}
	if (state != -1) {
		runState(state, BOMB_ENTER);
	}
	mState = state;
}

// 0x801092BC
int TObjMissionBomb::isHolder(BombThrowMsg* msg)
{
	return msg->mHolder == mHolderId;
}

// 0x801092D4
int TObjMissionBomb::getMsgFlags()
{
	return 0;
}

// 0x801092DC
int TObjMissionBomb::attachTo(BombHoldMsg* msg)
{
	if (mHandle.mHolder == 0) {
		msg->mHandle = &mHandle;
		msg->mPos    = mBody.mPos;
		msg->mRot    = mBody.mRot;
		msg->mRadius = mBody.mRadius;
		mOwner       = msg->mHolder;
		mHolderId    = msg->mHolder;
		return 1;
	}
	return 0;
}

// 0x80109350
void TObjMissionBomb::release()
{
	mOwner = 0xFFFF;
}

// 0x80109360
void TObjMissionBomb::updateMatrix()
{
	mBody.makeMatrix(mMatrix);
}

// 0x8010938C
int TObjMissionBomb::checkDamage()
{
	if (eggFirstDamage((void*)mColli, (EggContact*)mColli->mContacts, 0, (void*)this) != 0) {
		setState(BOMB_EXPLODE);
		return 1;
	}
	return 0;
}

// 0x801093E8
struct HeldVec {
	f32 v[3];
};

void TObjMissionBomb::stateHeld(int phase)
{
	if (phase == BOMB_CHECK) {
		if (checkDamage() != 0) {
			return;
		}
	} else if (phase == BOMB_UPDATE) {
		eggPushHeld((void*)mColli, (EggContact*)mColli->mContacts, 0, mHandle.mHolder);
		TMobileObject* holder = (TMobileObject*)mHandle.mHolder;
		f32 x                 = holder->mBody.mPos.x;
		f32 y                 = holder->mBody.mPos.y;
		f32 z                 = holder->mBody.mPos.z;
		HeldQuat* from        = (HeldQuat*)&holder->mBody.mRot;
		f32 w                 = from->w;
		NJS_QUATERNION* to    = (NJS_QUATERNION*)&mBody.mRot;
		NJS_QUATERNION q;
		*(HeldVec*)q.im   = *(HeldVec*)&from->v;
		mPos.x            = x;
		mPos.y            = y;
		mPos.z            = z;
		mBody.mPos.x      = x;
		mBody.mPos.y      = y;
		mBody.mPos.z      = z;
		to->re            = w;
		*(HeldVec*)to->im = *(HeldVec*)q.im;
		updateMatrix();
		roll()->checkBumps();
	}
}

// 0x801094A8
void TObjMissionBomb::stateRolling(int phase)
{
	if (phase == BOMB_CHECK) {
		if (checkDamage() == 0 && roll()->isRollDone() != 0) {
			setState(BOMB_GROUND);
		}
	} else if (phase == BOMB_UPDATE) {
		roll()->pushOff(mColli, mColli->mContacts, 0);
		roll()->moveRolling();
		updateMatrix();
	}
}

// 0x80109530
void TObjMissionBomb::stateSlam(int phase)
{
	if (phase == BOMB_CHECK) {
		if (checkDamage() != 0) {
			return;
		}
		if (roll()->isSlamDone() != 0) {
			setState(BOMB_GROUND);
		} else if (roll()->hasLanded() != 0) {
			camShakeNearSoft(200.0f, &mPos);
			setState(BOMB_EXPLODE);
		}
	} else if (phase == BOMB_UPDATE) {
		roll()->pushOff(mColli, mColli->mContacts, 0);
		roll()->moveSlam();
		updateMatrix();
	}
}

// 0x801095E4
void TObjMissionBomb::stateGround(int phase)
{
	if (phase == BOMB_CHECK) {
		if (checkDamage() != 0) {
			return;
		}
	} else if (phase == BOMB_UPDATE) {
		roll()->pushOff(mColli, mColli->mContacts, 0);
		roll()->moveGround();
		updateMatrix();
	}
}

// 0x80109650
void TObjMissionBomb::stateExplode(int phase)
{
	if (phase == BOMB_ENTER) {
		mFlagBits.mBits |= BOMB_FLAG_EXPLODE;
		shadowDisable(&mShadow);
		mTimer = 0;
		if (mFuse != 0) {
			mFuse->mFlags |= 1;
			mFuse = 0;
		}
		colliDisable(mColli);
		sePlay3D(0x2E, &mPos);
	} else if (phase == BOMB_CHECK) {
		if (mTimer == 240.0f) {
			u16 msg[2];
			msg[0] = 0x99;
			if (sendGameMsg((ObjMsg*)msg) != 0 && mSwitch >= 0) {
				setKeyBit(mSwitch);
			}
			setLinkDone((SetLink*)&mLink);
			mFlags |= 1;
		}
	} else if (phase == BOMB_UPDATE) {
		if (mTimer == 0) {
			f32 mtx[12];
			mtxSetTransV(mtx, &mPos);
			fxSpawnAt(1, 7, 2, mtx);
		} else if (mTimer == 20) {
			spawnExplode(&mPos, mHolderId, 10);
		} else if (mTimer == 30) {
			mFlags |= 0x10;
		}
		mTimer++;
	}
}

// 0x801097BC
int TObjMissionBomb::onPush(ObjMsg* msg)
{
	njAddVector(&mBody.mAccel, &((ObjVecMsg*)msg)->mVec);
	return 1;
}

// 0x801097E8
int TObjMissionBomb::onGetType()
{
	return 0x1B;
}

// 0x801097F0
int TObjMissionBomb::onGetFlags()
{
	int flags = getMsgFlags();
	switch ((u8)mState) {
	case BOMB_ROLLING:
		flags |= 0x200;
		break;
	}
	return flags;
}

// 0x80109830
int TObjMissionBomb::onGetVel(ObjMsg* msg)
{
	((ObjVecMsg*)msg)->mVec = mBody.mVel;
	return 1;
}

// 0x80109850
int TObjMissionBomb::onThrow(ObjMsg* msg)
{
	if (isHolder((BombThrowMsg*)msg) == 0) {
		return 0;
	}
	if ((s8)mState == BOMB_HELD) {
		release();
		roll()->setVel(&((BombThrowMsg*)msg)->mVel);
		setState(BOMB_ROLLING);
		return 1;
	}
	return 0;
}

// 0x801098C8
int TObjMissionBomb::onPickUp(ObjMsg* msg)
{
	switch (mState) {
	case BOMB_GROUND:
		setState(BOMB_HELD);
		return attachTo((BombHoldMsg*)msg);
	case BOMB_ROLLING:
		if (attachTo((BombHoldMsg*)msg) != 0) {
			mBody.mVel = gVecZero;
			setState(BOMB_HELD);
			return 1;
		}
		return 0;
	}
	return 0;
}

// 0x80109978
int TObjMissionBomb::onDrop(ObjMsg* msg)
{
	if (isHolder((BombThrowMsg*)msg) == 0) {
		return 0;
	}
	if ((s8)mState == BOMB_HELD) {
		release();
		setState(BOMB_GROUND);
		return 1;
	}
	return 0;
}

// 0x801099E0
int TObjMissionBomb::onSlam(ObjMsg* msg)
{
	if (isHolder((BombThrowMsg*)msg) == 0) {
		return 0;
	}
	if ((s8)mState == BOMB_HELD) {
		release();
		roll()->startSlam(&((BombThrowMsg*)msg)->mVel);
		setState(BOMB_SLAM);
		return 1;
	}
	return 0;
}

// 0x80109A58
static void onBombRollHit(void*, void*, void*, void*, void*, u32 flags, TObjMissionBomb* bomb)
{
	if (flags & 2) {
		bomb->mFlagBits.mBits |= BOMB_FLAG_BURST;
	}
}

// 0x80109A70
static void bombTev(u8 stage)
{
	u8 st = (u8)stage;
	GXSetTevColorIn(st, GX_CC_ZERO, GX_CC_TEXC, GX_CC_HALF, GX_CC_CPREV);
	GXSetTevColorOp(st, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(st, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(st, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(st, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
}

// 0x80109B14
static void bombTexGen(u8 coord)
{
	GXSetTexCoordGen2(coord, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX5, GX_TRUE, GX_PTTEXMTX19);
}

// 0x80109B4C
int TObjMissionBomb::getKind()
{
	return 3;
}

// 0x80109B54
Vec3f* TObjMissionBomb::getPos()
{
	return &mPos;
}

// 0x80109B5C
SetLink* TObjMissionBomb::getLink()
{
	return mLink.get();
}

// 0x80109B64
f32 TObjMissionBomb::getRadiusV()
{
	return mBody.mRadius;
}

// 0x80109B6C
int BombHandle::onMessage(ObjMsg* msg)
{
	return ((TObjMissionBomb*)mOwner)->handleMessage(msg);
}

// 0x80109B90
BombRope::BombRope()
{
	_120 = 0;
}

// 0x80109BE0
extern "C" void bombUnused1()
{
}

// 0x80109BE4
extern "C" void bombUnused2()
{
}
