#include "object/ObjAction.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "egg/TEggCreature.h"
#include "item/TItemManage.h"
#include "object/ObjModel.h"
#include "player/PlyState.h"
#include "player/TPlyEyelid.h"
#include "player/TPlyComb.h"
#include "player/TPlyCrow.h"
#include "ui/TSpeakCursor.h"
#include "engine/LightZone.h"
#include "enemy/SpawnGuard.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/CharaColli.h"
#include "io/NinjaFile.h"
#include "player/PlyHealth.h"
#include "player/PlyPool.h"
#include "ui/TBlindCtrl.h"
#include "core/MathUtil.h"
#include "egg/ObjEgg.h"
#include "engine/TFootPrinter.h"
#include "object/ObjShadow.h"
#include "object/TObjItem.h"
#include "camera/TCameraCtrl.h"
#include "sound/SoundCtrl.h"
#include "camera/TCamera.h"
#include "player/Player.h"
extern HitVec sPlayerStartPos[]; // TObjPlayer.cpp
#include "object/TObjLauncher.h"
#include "core/Mem.h"
#include "engine/DrawState.h"
#include "stage/TGameState.h"
#include "core/Pad.h"
#include "engine/TParticle.h"
#include "engine/FxSystem.h"
#include "ui/AeArc.h"
#include "object/TObjEggRing.h"
#include "sound/Sound3DPlay.h"
#include "engine/Colli.h"
#include "stl/math.h"
#include "egg/EggCreature.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "Ninja/ninja.h"
#include "engine/Color.h"
#include "engine/GroundQuery.h"
#include "stage/TGame.h"
#include "engine/ActorMotion.h"
#include "object/TObjPlayer.h"
#include "player/TPlyEffStInvincible.h"
#include "player/TPlyEffStBlind.h"
#include "player/TPlyEffStDrain.h"
#include "player/TPlyEffStStrength.h"
#include "player/TPlyEffStSuper.h"
#include "object/TObjClosedEgg.h"
#include "core/VecMath.h"
#include "player/PlyTint.h"
#include "stage/TEventCtrl.h"
#include "core/TSceneManager.h"
#include "engine/WorldMesh.h"
#include "core/TMainTask.h"
#include "enemy/boss/TEneLastExBossCore.h"
#include "item/TItem.h"
struct TObject;
struct FlagWord;
struct ObjMsg;
struct ObjShadow;
struct Vec3f;
struct CharaColli;
struct ColliShape;
struct DrawState;
struct DrawStateSlot;
struct NJS_QUATERNION;
struct TPlyEffStSleep;
struct TPlyEffStSuper;
struct TPlyEffStStrength;
struct TPlyEffStDrain;
struct TPlyEffStBlind;
struct TPlyEffStInvincible;
struct NJS_OBJECT;
struct GjActor;
struct ColliContact;
struct Fx;
struct SceneRecord;

#define PLY_SLOT(obj, off, type) ((type)(*(void***)((u8*)(obj) + 0x18))[(off) / 4])
// Virtual calls on the player a mount or anything else that shares the player's slots
class PlySlots {
public:
	u8 _00[0x18];
	// _18 = vtable
	virtual void s08();                // _08
	virtual void s0C();                // _0C
	virtual void s10();                // _10
	virtual void s14();                // _14
	virtual void s18();                // _18
	virtual void s1C();                // _1C
	virtual void s20();                // _20
	virtual void s24();                // _24
	virtual void s28();                // _28
	virtual void s2C();                // _2C
	virtual void s30();                // _30
	virtual void s34();                // _34
	virtual void s38();                // _38
	virtual void s3C();                // _3C
	virtual void s40();                // _40
	virtual void s44();                // _44
	virtual void s48();                // _48
	virtual void s4C();                // _4C
	virtual void s50();                // _50
	virtual void s54();                // _54
	virtual void s58();                // _58
	virtual void s5C();                // _5C
	virtual void s60();                // _60
	virtual void s64();                // _64
	virtual void s68();                // _68
	virtual int s6C();                 // _6C
	virtual void s70(int);             // _70
	virtual void s74(int);             // _74
	virtual void s78();                // _78
	virtual void s7C();                // _7C
	virtual void s80(u8*);             // _80
	virtual void s84();                // _84
	virtual void s88();                // _88
	virtual void s8C();                // _8C
	virtual void s90(const ItemSpec*); // _90
	virtual void s94(f32*);            // _94
};

// The same for items and effects
class PlyItemSlots {
public:
	u8 _00[0x18];
	// _18 = vtable
	virtual void s08();       // _08
	virtual void s0C();       // _0C
	virtual void s10();       // _10
	virtual void s14();       // _14
	virtual void s18();       // _18
	virtual void s1C();       // _1C
	virtual void s20();       // _20
	virtual void s24();       // _24
	virtual void s28();       // _28
	virtual void s2C();       // _2C
	virtual void s30();       // _30
	virtual void s34();       // _34
	virtual void s38();       // _38
	virtual void s3C();       // _3C
	virtual void s40();       // _40
	virtual void s44();       // _44
	virtual void s48();       // _48
	virtual void s4C();       // _4C
	virtual void s50();       // _50
	virtual void s54();       // _54
	virtual void s58();       // _58
	virtual void s5C();       // _5C
	virtual void s60();       // _60
	virtual void s64();       // _64
	virtual void s68();       // _68
	virtual void s6C();       // _6C
	virtual void s70();       // _70
	virtual void s74();       // _74
	virtual void s78(Vec3f*); // _78
	virtual void s7C();       // _7C
	virtual void s80();       // _80
	virtual void s84();       // _84
	virtual void s88();       // _88
	virtual void s8C();       // _8C
	virtual void s90();       // _90
	virtual void s94();       // _94
};

#define PLY_CHANGE(player, create)                                                                  \
	do {                                                                                            \
		void* pool = (player)->mStates;                                                             \
		releasePlyState(pool, changePlyState(&(player)->mState, (player), create(pool, (player)))); \
	} while (0)
#define PLY_ENDSTATE(player)                                \
	do {                                                    \
		if ((player)->mState != 0) {                        \
			((TPlyState*)(player)->mState)->vfn_20(player); \
		}                                                   \
	} while (0)

struct PlyMsgVec : public PlyMsg {
	inline PlyMsgVec(u16 type)
	    : PlyMsg(type)
	{
		mTo = 0xFFFF;
	}

	Vec3f mVec; // _08
};

// A direction with no target the constructor leaves mTo alone
struct PlyMsgDir {
	inline PlyMsgDir(u16 type)
	{
		mFrom = 0xFFFF;
		mType = type;
	}

	u16 mType; // _00
	u8 _02[2];
	u16 mFrom;  // _04
	u16 mTo;    // _06
	Vec3f mVec; // _08
};

struct PlyMsgRaw {
	u16 mType; // _00
	u8 _02[2];
	u16 mFrom; // _04
};

// Messages 0x6A and 0x6D ask an egg to be picked up
struct PlyMsgPick : public PlyMsg {
	inline PlyMsgPick(u16 type = 0x6A)
	    : PlyMsg(type)
	{
		mHandle = 0;
	}

	EggHandleBase* mHandle; // _08
	Vec3f mPos;             // _0C
	EggRot mRot;            // _18
	f32 mRadius;            // _28
};

// A local vector The DOL copies these a member at a time which the implicit copy does not
struct PlyVec : public Vec3f {
	inline PlyVec() { }
	inline PlyVec(const Vec3f& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
	}
};

// What plyReportToGame sends to the game and asks of the egg
struct PlyReportMsg {
	u16 mType; // _00
	u16 _02;
	u16 mPlayer; // _04
	u16 _06;
	int mValue; // _08
};
struct PlyReportFloat {
	u16 mType; // _00
	u16 _02;
	u16 mPlayer; // _04
	u16 _06;
	f32 mValue; // _08
};
struct PlyAskEgg {
	u16 mType;  // _00
	f32 mValue; // _04
	u8 mKind;   // _08
};
struct PlyReportEgg {
	u16 mType; // _00
	u16 _02;
	u16 mPlayer; // _04
	u16 _06;
	f32 mValue; // _08
	u8 mKind;   // _0C
};

// What the ground ray callbacks keep
struct PlyRayHit {
	void* mMesh; // _00
	u32 mTri;    // _04
	Vec3f mPos;  // _08
	u32 mFlags;  // _14
	f32 mDist;   // _18
};

// The context of the searches through a collider's contacts
struct PlyGrabSearch {
	u16 mId;            // _00
	Vec3f mPos;         // _04
	int mAngle;         // _10
	int mCone;          // _14
	f32 mReach;         // _18
	TObjAction* mFound; // _1C
};

struct PlyGroundHit {
	MeshColli* mMesh; // _00 the nearest walkable triangle
	u16 mTri;         // _04
	Vec3f mAnyPos;    // _08 the nearest of anything
	f32 mAnyDist;     // _14
	u32 mAnyFlags;    // _18 every flag hit
	Vec3f mPos;       // _1C
	f32 mDist;        // _28
	u32 mFlags;       // _2C
};

struct PlyArray;
struct PlyDamageSearch;
struct PlyEdgeSearch;
struct PlyEggSearch;
struct PlyGrabSearch;
struct PlyGroundHit;
struct PlyJoint;
struct PlyMoveHit;
struct PlyMsg;
struct PlyMsgRaw;
struct PlyPose;
struct PlyRayHit;
struct PlyRollSearch;
struct PlySlamSearch;
struct PlyStatusEntry;
struct PlyTalkSearch;

extern "C" {

// Motion drawing

// Debug names unused in the game the first slot is empty
char* sPlayerDebugNames[6] = { 0, "CAMOUFLAUGE", "ALL EGG MAX", "EGG BREAK", "STEAL", "HEAL(S)" };

extern const ColliShape sPlayerEmitter0Shape[1] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 2.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
extern const ColliShape sPlayerEmitter1Shapes[3] = {
	{ { 0.0f, 0.0f, 5.0f }, 0x0, 0, 0x0, 10.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 15.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 10.0f }, 0x0, 0, 0x0, 10.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};

// Defined below and used before that or by other units
}

// 0x80034098
extern "C" int plyGlobalsInit()
{
	lbl_802C4D68 = 0;
	lbl_802C4D6C = -1;
	lbl_802C4D70 = 0;
	lbl_802C4D74 = 1;
	plyGlobalsClear();
	return 1;
}

// 0x800340D8
extern "C" void plyGlobalsEmpty()
{
}

// 0x800340DC
extern "C" int* plyGlobalsClear()
{
	lbl_802C4D78    = 0;
	lbl_802C4D7C    = 0;
	lbl_802C4D80    = 0;
	lbl_8027FAC0[1] = 0;
	lbl_8027FAC0[0] = 0;
	lbl_8027FAC0[3] = 0;
	lbl_8027FAC0[2] = 0;
	return lbl_8027FAC0;
}

// 0x80034108
TPlyState::TPlyState()
{
	mName = gTPlyStateName;
	mSize = 0x14;
}

// 0x80034160
void TPlyState::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	plyDefaultMessage(player, msg);
}

// 0x80034188
extern "C" void plyLoadShared()
{
	gPlyParams      = (PlyParams*)playerGetParams(0);
	sPlayerMotions  = (NJS_MOTION**)findArcExport(gPlayerArc, "motions");
	sPlayerTexLists = (NJS_TEXLIST**)findArcExport(gPlayerArc, "texlists");
}

// 0x800341D4
extern "C" void plyUnloadShared()
{
	sPlayerMotions  = 0;
	sPlayerTexLists = 0;
}

// 0x800341E4
TChrPlayer::TChrPlayer(TObject* parent, int playerNo, u8 charId, int active)
    : TObjPlayer(parent, playerNo, charId)
{
	plyClearWord((u32*)_3C);
	_D5 = 0;
	plyHealthInit(&mHealth);
	((ObjShadow*)_148)->__ct();
	plyTintStop((PlyTint*)_1A8);
	((SpawnGuard*)_1EC)->__ct();
	initPlyStateSlot(&mState);
	((ObjRef*)_2BC)->__ct();
	((FlagByte*)_31C)->__ct();
	_320 = 0;
	_324 = 0;
	_328 = 0;
	_32C = 0;
	_330 = 0;
	_334 = 0;
	_338 = -1;
	((FlagWord*)&mPlayFlags)->__ct();
	poseArrayInit((PlyArray*)&mPose);
	jointArrayInit((PlyArray*)&mJoints);
	((FlagWord*)&mBitsD8)->__ct();
	((FlagWord*)&mBitsDC)->__ct();
	((FlagWord*)&mBitsE0)->__ct();
	((FlagWord*)&mBitsE4)->__ct();
	((ItemHolder*)_4F0)->__ct();
	((ItemHolder*)_508)->__ct();
	_520 = 0;
	__construct_array(mRiders, riderArrayCtor, riderArrayDtor, 0x30, 8);

	_D4         = 0;
	mTimerD6    = 0;
	mMoveAmount = 0.0f;
	mMoveAngle  = 0;
	mAngle      = 0;
	_184        = 0;
	_340        = gVecZero;
	_194        = 0;
	mTimerF8    = 0;
	mTargetId   = -1;
	_2A0        = -1;
	_2CC        = -1;
	mFade       = 0;
	_318        = 1;

	// Each character has its own archive
	TArc* arc = gPlayerArcs[mCharId];
	if (arc == 0) {
		mModels   = 0;
		mTexLists = 0;
	} else {
		mModels       = (void**)findArcExport(arc, "models");
		mTexLists     = (void**)findArcExport(arc, "texlists");
		void* motions = findArcExport(arc, "motions");
		if (motions != 0) {
			mBody.init(gPlayerBodyMotions, motions);
			mBodyCopy.init(gPlayerBodyMotions, motions);
		}
		void* faceMotions = findArcExport(arc, "fa_mtn");
		void* faceShapes  = findArcExport(arc, "fa_sm");
		if (faceMotions != 0 && faceShapes != 0) {
			mFace.init(sPlayerFaceMotions, faceMotions, faceShapes);
		}
		void* leftHand = findArcExport(arc, "lh_mtn");
		if (leftHand != 0) {
			mLeftHand.init(sPlayerLeftHandMotions, leftHand);
		}
		void* rightHand = findArcExport(arc, "rh_mtn");
		if (rightHand != 0) {
			mRightHand.init(sPlayerRightHandMotions, rightHand);
		}
	}

	poseArrayResize((PlyArray*)&mPose, 0x4B);
	jointArrayResize((PlyArray*)&mJoints, 0x4B);

	mEmitter0 = colliCreate(1, 1);
	colliSetOwner(mEmitter0, (u16)mObjId);
	colliCopyShapes((CharaColli*)mEmitter0, (const ColliShape*)sPlayerEmitter0Shape);
	colliSetMasks((CharaColli*)mEmitter0, 1, 0, 0, 0);

	mEmitter1 = colliCreate(3, 1);
	colliSetOwner(mEmitter1, (u16)mObjId);
	colliCopyShapes((CharaColli*)mEmitter1, (const ColliShape*)sPlayerEmitter1Shapes);
	colliSetMasks((CharaColli*)mEmitter1, 1, 0xFF, 0xFF, 0x10);
	colliSetList((CharaColli*)mEmitter1, 1);
	*(int*)(colliGetShape((CharaColli*)mEmitter1, 0) + 0x24) = 0x10;
	*(int*)(colliGetShape((CharaColli*)mEmitter1, 1) + 0x24) = 0x10;
	*(int*)(colliGetShape((CharaColli*)mEmitter1, 2) + 0x24) = 0x10;

	mDrawTask = addDrawTask(this, (void (*)(void*))plyDrawTask, 0x2C, 0);
	if (flagTest(&gPlayerFlags, 0) != 0) {
		*(f32*)(_148 + 0x30) = 15.0f;
		*(f32*)(_148 + 0x34) = 15.0f;
		mShadowTask          = 0;
	} else {
		mShadowTask = addDrawTask(this, (void (*)(void*))plyDrawTaskShadow, 0x1F, 0);
	}

	_34C = new TPlyEyelid(this);
	_4D0 = new TObject(this);
	_198 = 0;
	reset();

	mPos = sPlayerStartPos[playerNo];
	plySetAngle(this, sPlayerStartAngle[playerNo]);
	updateMatrix();

	mMobile         = new TMobileObject(gTL_04);
	mMobile->mOwner = this;
	_2C8            = new TSpeakCursor(this);

	if (mModels == 0) {
		mRightHandBuffer = 0;
		mLeftHandBuffer  = 0;
		mFaceBuffer      = 0;
		mBodyBuffer      = 0;
	} else {
		mBodyBuffer      = (void**)createActor((NJS_OBJECT*)mModels[0]);
		mFaceBuffer      = createActor((NJS_OBJECT*)mModels[2]);
		mLeftHandBuffer  = createActor((NJS_OBJECT*)mModels[3]);
		mRightHandBuffer = createActor((NJS_OBJECT*)mModels[4]);
		poseModels(1);
	}

	mModel = new TPlyComb(this);
	mModel->build(mModels[1]);
	mModel->mTexture = mTexLists[0];
	mModel->setRoot((f32*)(mJoints + 0xAE0));
	if (flagTest(&gPlayerFlags, 1) != 0) {
		mModel->readPose(*sPlayerMotions, 0.0f);
		*((u8*)mModel + 0x20) = 0;
	}

	void* mem = operator new(0xC);
	mStates   = mem != 0 ? initPlyPool(mem, 0x80) : 0;

	if (active == 0) {
		mFlags |= 4;
		mFlags |= 0x10;
		mModel->mFlags |= 0x10;
	} else {
		playMotion(0);
		PLY_CHANGE(this, createPlyStateStand);
	}
}

// 0x800348AC
// The riders' array destructor Riders follow the ground they stand on
extern "C" void* riderArrayDtor(void* rider, s16 del)
{
	if (rider != 0) {
		riderDtor((AniRider*)rider, -1);
		if (del > 0) {
			operator delete(rider);
		}
	}
	return rider;
}

// 0x800348F8
extern "C" void* riderArrayCtor(void* rider)
{
	riderInit((AniRider*)rider);
	return rider;
}

// 0x80034928
// Back to the state a character is in at the start of a stage full health stood still
void TChrPlayer::reset()
{
	clearFlagWord((u32*)&mPlayFlags);
	mPlayFlags |= 0x200;
	mPlayFlags |= 0x1000;
	mPlayFlags |= 0x2000;
	mHealth.mMin = 0;
	mHealth.mMax = 100;
	mHealth.mNow = 100;
	mTimerD6     = 0;
	mMoveAmount  = 0.0f;
	mMoveAngle   = 0;
	_184         = 0;
	_340         = gVecZero;
	mTimerF8     = 0;
	mTargetId    = -1;
	colliEnable(mEmitter0);
	_2F8.x = 0.0f;
	_2F8.y = 0.0f;
	_2F8.z = 0.0f;
	mVel.x = 0.0f;
	mVel.y = 0.0f;
	mVel.z = 0.0f;
	_23C.x = 0.0f;
	_23C.y = 0.0f;
	_23C.z = 0.0f;
	mAge   = 0;
	effectListClear(effects());
	_D0 = 0;
}

// 0x80034A18
TChrPlayer::~TChrPlayer()
{
	freePlyPool(mStates, 1);
	if (((ItemHolder*)_508)->mNode.mList != 0) {
		itemHolderRelease((ItemHolder*)_508);
	}
	if (((ItemHolder*)_4F0)->mNode.mList != 0) {
		itemHolderRelease((ItemHolder*)_4F0);
	}
	colliFree(mEmitter0);
	colliFree(mEmitter1);
	removeDrawTask(mDrawTask);
	if (mShadowTask != 0) {
		removeDrawTask(mShadowTask);
	}
	memFree(mBodyBuffer);
	memFree(mFaceBuffer);
	memFree(mLeftHandBuffer);
	memFree(mRightHandBuffer);
	__destroy_arr((void*)mRiders, (void**)riderArrayDtor, 0x30, 8);
	jointArrayDtor((PlyArray*)&mJoints, -1);
	poseArrayDtor((PlyArray*)&mPose, -1);
	((ObjRef*)_2BC)->~ObjRef();
	if (&mState != 0) {
		plyStateSlotDtor(&mState, 0);
	}
	((SpawnGuard*)_1EC)->~SpawnGuard();
	((ObjShadow*)_148)->~ObjShadow();
}

// 0x80034BA4
// Slot _70
void TChrPlayer::setHealth(int amount)
{
	if ((mPlayFlags & 0x1000000) == 0) {
		plyHealthSet(&mHealth, amount);
	}
}

// 0x80034BD4
// Slot _74
void TChrPlayer::addHealth(int amount)
{
	if ((mPlayFlags & 0x1000000) == 0) {
		plyHealthSet(&mHealth, mHealth.mNow + amount);
	}
}

// 0x80034C0C
// Slot _80 Takes an item by its spec bit 0x10 of the flags holds it for later use bit 0x40 only runs its slot _6C and
// anything else is worn which is announced with message 0xB5
extern "C" void plyTakeItem(TChrPlayer* player, const ItemSpec* spec)
{
	if (spec->mFlags & 0x10) {
		if (((ItemHolder*)player->_508)->mNode.mList != 0) {
			itemHolderRelease((ItemHolder*)player->_508);
		}
		itemHolderAttach(spec->mKind, (ItemHolder*)player->_508);
	} else if (spec->mFlags & 0x40) {
		TObject* item = giveItem(player->mObjId, spec->mKind);
		if (item != 0) {
			((PlyItemSlots*)item)->s6C();
		}
	} else {
		if (((ItemHolder*)player->_4F0)->mNode.mList != 0) {
			itemHolderRelease((ItemHolder*)player->_4F0);
		}
		itemHolderAttach(spec->mKind, (ItemHolder*)player->_4F0);
		for (int i = 0; i < 0x11; i++) {
			int form = sPlayerFormPairs[i * 2];
			if (form == spec->mKind) {
				struct {
					u16 mType;
					u16 _02;
					u16 mPlayer;
					u16 _06;
					int mId;
				} msg;
				msg.mType   = 0xB5;
				msg.mPlayer = player->mObjId;
				msg.mId     = sPlayerFormPairs[i * 2 + 1];
				aeSendMessage(&msg);
				break;
			}
		}
	}
}

// 0x80034D14
extern "C" int plyTakePickup(TChrPlayer* player, u8* pickup)
{
	if ((pickup[5] & 0x10) != 0 && flagTest(&gPlayerFlags, 2) != 0) {
		*(int*)(pickup + 0xC) = 0;
	}
	((PlySlots*)player)->s80(pickup + 4);
	return 1;
}

// 0x80034D84
// Uses the held item ends the one in use then starts what the second holder has
extern "C" void plyUseHeldItem(TChrPlayer* player)
{
	if (player->_520 != 0) {
		((PlyItemSlots*)player->_520)->s74();
		delete (TObject*)player->_520;
		player->_520 = 0;
		player->_D0  = 0;
	}
	bool has = ((ItemHolder*)player->_508)->hasItem();
	if (has) {
		TObject* item = giveItem(player->mObjId, (has ? ((ItemHolder*)player->_508)->mEntry->mData.mKind : 0xFFFF));
		if (item != 0) {
			((PlyItemSlots*)item)->s70();
		}
		player->_520 = item;
		((PlySlots*)player)->s90(findItemSpec(((ItemHolder*)player->_508)->firstId()));
		itemHolderRelease((ItemHolder*)player->_508);
	}
}

// 0x80034E88
// Slot _A4 Swaps the outer model or goes back to the character's own with a null model
void TChrPlayer::vfn_A4(void* model, void* texList)
{
	if (model == 0) {
		mModel->build(mModels[1]);
		mModel->mTexture = mTexLists[0];
	} else {
		mModel->build(model);
		mModel->mTexture = texList;
	}
	if (flagTest(&gPlayerFlags, 1) != 0) {
		mModel->readPose(*sPlayerMotions, 0.0f);
	}
}

// 0x80034F18
// Holds an egg remembering it as the last one held too
extern "C" void plyHoldEgg(TChrPlayer* player, TObjAction* egg)
{
	player->mEggRef.setId(egg->mObjId);
	if (player->mLastEggRef.mList != player->mEggRef.mList) {
		player->mLastEggRef.assign(player->mEggRef);
	}
	egg->mHolderId = player->mObjId;
}

// 0x80034F74
// Tells a TObjEgg what form this player's ability bits give it
extern "C" void plyApplyEggForm(TChrPlayer* player, TObjAction* egg)
{
	if (egg->_00 == (u32)gTObjEggName) {
		int form = 0;
		for (u32 i = 0; i < 8; i++) {
			if (flagTest(&player->mBitsD8, sPlayerFormFlags[i][0]) != 0) {
				form = sPlayerFormFlags[i][1];
				break;
			}
		}
		((TObjEgg*)egg)->setForm(form);
		if (flagTest(&player->mBitsDC, 0xC) != 0) {
			((TObjEgg*)egg)->setDamageMul(2);
		}
	}
}

// 0x80035028
extern "C" void plyForgetEgg(TChrPlayer* player)
{
	player->mEggRef.setId(0xFFFF);
}

// 0x80035054
extern "C" TObjAction* plyGetEgg(TChrPlayer* player)
{
	return player->mEggRef.get();
}

// 0x80035078
extern "C" void plySendMountOff(TChrPlayer* player)
{
	TObjAction* obj = player->mMountRef.get();
	if (obj != 0) {
		ObjMsg msg;
		msg.mType = 0x44;
		obj->sendToHandle(&msg);
	}
}

// 0x800350C0
extern "C" TObjAction* plyGetHangObject(TChrPlayer* player)
{
	return player->mHangRef.get();
}

// 0x800350E4
extern "C" TObjAction* plyGetPlaceObject(const TChrPlayer* player)
{
	return findObjById(player->_2A0);
}

// 0x80035108
// Sends the held egg off in front of the player with message 0x64
extern "C" int plyPushEgg(TChrPlayer* player, TObjAction* egg)
{
	Vec3f dir;
	f32 mtx[12];

	vecFromAngleXZ(gPlyParams->_B4, &dir, player->mAngle);
	PlyMsgVec msg(0x64);
	msg.mFrom = player->mObjId;
	msg.mVec  = dir;
	egg->sendToHandle((ObjMsg*)&msg);
	plyPlayVoice(player, 0xA1, 0);
	plyRumble(player, 0xB);
	plyPlaySe(player, 0x7F);
	mtxSetTrans((float*)mtx, player->mPos.x, 5.0f + player->mPos.y, player->mPos.z);
	mtxRotateY(mtx, mtx, player->mAngle);
	mtxScale(2.0f, 2.0f, 2.0f, mtx, mtx);
	fxSpawnAt(1, 0, 7, mtx);
	plyForgetEgg(player);
	player->mMobile->setHandle(0);
	return 1;
}

// 0x80035230
// The same with message 0x6C which the egg can refuse
extern "C" int plyPushEggChecked(TChrPlayer* player, TObjAction* egg)
{
	Vec3f dir;
	f32 mtx[12];

	vecFromAngleXZ(gPlyParams->_B4, &dir, player->mAngle);
	PlyMsgDir msg(0x6C);
	msg.mFrom = player->mObjId;
	msg.mVec  = dir;
	if (egg->sendToHandle((ObjMsg*)&msg) == 0) {
		return 0;
	}
	plyPlayVoice(player, 0xA1, 0);
	plyRumble(player, 0xB);
	plyPlaySe(player, 0x7F);
	mtxSetTrans((float*)mtx, player->mPos.x, 5.0f + player->mPos.y, player->mPos.z);
	mtxRotateY(mtx, mtx, player->mAngle);
	mtxScale(2.0f, 2.0f, 2.0f, mtx, mtx);
	fxSpawnAt(1, 0, 7, mtx);
	plyForgetEgg(player);
	player->mThrownEggRef.setId(egg->mObjId);
	player->mMobile->setHandle(0);
	return 1;
}

// 0x80035370
int TChrPlayer::getBodyRow()
{
	return mBody.mCurrent;
}

// 0x80035378
// The body plays on two players and the row's flags pick a pose for the hands
void TChrPlayer::playMotion(int row)
{
	plyPlayMotionSynced(&mBody, row);
	plyPlayMotionSynced(&mBodyCopy, row);
	u32 flags = mBody.getUser(row);
	if (flags != 0) {
		if (flags & 2) {
			mLeftHand.playBlend(8, 0.08f);
			mRightHand.playBlend(8, 0.08f);
		} else if (flags & 4) {
			mLeftHand.playBlend(9, 0.08f);
			mRightHand.playBlend(9, 0.08f);
		} else if (flags & 8) {
			mLeftHand.play(0xB);
			mRightHand.play(0xB);
		}
	}
}

// 0x80035444
void TChrPlayer::blendMotion(int row)
{
	mBody.playBlend(row, 0.08f);
	mBodyCopy.playBlend(row, 0.08f);
	u32 flags = mBody.getUser(row);
	if (flags != 0) {
		if (flags & 2) {
			mLeftHand.playBlend(8, 0.08f);
			mRightHand.playBlend(8, 0.08f);
		} else if (flags & 4) {
			mLeftHand.playBlend(9, 0.08f);
			mRightHand.playBlend(9, 0.08f);
		} else if (flags & 8) {
			mLeftHand.play(0xB);
			mRightHand.play(0xB);
		}
	}
}

// 0x8003551C
void TChrPlayer::setMotionSpeed(f32 speed)
{
	mBody.mSpeed     = speed;
	mBodyCopy.mSpeed = speed;
}

// 0x80035528
extern "C" void plyPlayCopyMotion(TChrPlayer* player, MotionPlayer* motion, int row)
{
	plyPlayMotionSynced(motion, row);
}

// 0x80035550
// Applies the acceleration and caps the fall at 200 a second
extern "C" void plyApplyGravity(TChrPlayer* player)
{
	vecScaleAdd(1.0f / 60.0f, &player->mVel, &player->_2F8, &player->mVel);
	f32 lim = 200.0f;
	if (player->mVel.y < -lim) {
		player->mVel.y = -lim;
	}
}

// 0x800355A4
// How far the moving ground under the player went this frame
extern "C" void plyAddGroundMotion(TChrPlayer* player, Vec3f* pos)
{
	Vec3f before;
	Vec3f now;
	MeshColli* mesh = (MeshColli*)plyGetProbe(player, 5, 0, 0, 0);
	if (mesh != 0 && (mesh->mFlags & 2) != 0) {
		mtxMultVec((const f32*)(player->_1EC + 0x20), &player->_494, &before);
		mtxMultVec(mesh->mMatrix, &player->_494, &now);
		vecSub(&now, &now, &before);
		njAddVector(pos, &now);
	}
}

// 0x80035634
extern "C" void plyMoveAndCollide(TChrPlayer* player)
{
	plyCollectPushes(player);
	plyPushFromWalls(player);
	plyMoveByVel(player);
	plyFindGroundFalling(player);
	player->updateMatrix();
}

// 0x80035680
// Hangs the player from the ledge they climb with the motion's own offset removed
extern "C" void plyHangOnLedge(TChrPlayer* player)
{
	MeshColli* mesh = player->mRiders[7].mMesh;
	if (mesh != 0) {
		f32 mtx[12];
		Vec3f world;
		Vec3f hand;
		Vec3f hip;
		Vec3f offset;
		Vec3f dir;
		Vec3f from;
		Vec3f to;

		player->updateMotions();
		mtxIdentity(mtx);
		plyApplyScale(player, mtx);
		mtxGetPos(player->mPose[0].mMtx, &hip);
		mtxGetPos(player->mPose[36].mMtx, &hand);
		mtxMultVec(mtx, &hip, &hip);
		mtxMultVec(mtx, &hand, &hand);
		vecSub(&offset, &hand, &hip);
		vecRotateY(&offset, &offset, player->mAngle);
		mtxMultVec(mesh->mMatrix, &player->mRiders[7].mPos, &world);
		vecSub(&player->mPos, &world, &offset);
		vecRotateY(&hand, &offset, player->mAngle);
		vecSub(&dir, &world, &offset);
		mtxMultVecSR(mesh->mMatrix, &player->_4B8, &offset);
		plySetAngle(player, vecAngleXZ(&offset));
		mtxSetTransV(player->mMatrix, &dir);
		mtxConcatQuat((f32*)player->mMatrix, (const f32*)player->mMatrix, (const f32*)&player->mRot);
		plyApplyScale(player, player->mMatrix);
		player->updateJoints();
		from = player->mPos;
		from.y += 8.0f;
		to = player->mPos;
		plyCheckDeathRay(player, &from, &to);
	}
}

// 0x80035800
// The same hanging from a part of the object in the fourth reference
extern "C" void plyHangOnObject(TChrPlayer* player)
{
	TObjAction* obj = player->mHangRef.get();
	if (obj != 0) {
		f32 mtx[12];
		f32 node[12];
		Vec3f world;
		Vec3f hand;
		Vec3f hip;
		Vec3f offset;
		Vec3f dir;

		player->updateMotions();
		mtxIdentity(mtx);
		plyApplyScale(player, mtx);
		mtxGetPos(player->mPose[0].mMtx, &hip);
		mtxGetPos(player->mPose[36].mMtx, &hand);
		mtxMultVec(mtx, &hip, &hip);
		mtxMultVec(mtx, &hand, &hand);
		vecSub(&offset, &hand, &hip);
		vecRotateY(&offset, &offset, player->mAngle);
		obj->getNodeMatrix(player->_2A2, node);
		mtxGetPos(node, &world);
		vecSub(&player->mPos, &world, &offset);
		vecRotateY(&hand, &offset, player->mAngle);
		vecSub(&dir, &world, &offset);
		mtxMultVecSR(node, &gAxisZ, &offset);
		plySetAngle(player, vecAngleXZ(&offset));
		mtxSetTransV(player->mMatrix, &dir);
		mtxConcatQuat((f32*)player->mMatrix, (const f32*)player->mMatrix, (const f32*)&player->mRot);
		plyApplyScale(player, player->mMatrix);
		player->updateJoints();
	}
}

// 0x80035964
// Stands on top of the rolling body
extern "C" void plyStandOnMobile(TChrPlayer* player)
{
	PlyVec from(player->mPos);
	TMobileObject* mobile = player->mMobile;
	PlyVec to(mobile->mBody.mPos);
	to.y += mobile->mBody.mRadius - 1.5f;
	plyCheckDeathRay(player, &from, &to);
	player->mPos = to;
	player->updateMatrix();
}

static inline f32 plySqrt(f32 x)
{
	if (x > 0.0f) {
		f64 guess         = __frsqrte(x);
		guess             = 0.5 * guess * (3.0 - guess * guess * x);
		guess             = 0.5 * guess * (3.0 - guess * guess * x);
		guess             = 0.5 * guess * (3.0 - guess * guess * x);
		volatile f32 root = x * guess;
		x                 = root;
	}
	return x;
}

// 0x80035A00
// Works out how the player's matrix sits against the body they climb onto
extern "C" void plyMountMobile(TChrPlayer* player)
{
	Vec3f centre;
	Vec3f joint;
	Vec3f dir;
	EggRot rot;
	f32 sin;
	f32 mtx[12];
	f32 bodyMtx[12];
	f32 inverse[12];
	int yaw;
	int pitch;
	f32 cos;

	rot = player->mMobile->mBody.mRot;
	mtxFromQuat(bodyMtx, (f32*)&rot);
	centre = player->mMobile->mBody.mPos;
	mtxGetPos(plyGetJoint(player, 0x1B), &joint);
	vecSub(&dir, &centre, &joint);
	vecNormalize(&dir, &dir);
	f32 radius;
	radius = player->mMobile->mBody.mRadius;
	yaw    = (int)(10430.38043493439 * (f32)atan2(dir.x, dir.z));
	njSinCos(yaw, &sin, &cos);
	if (radius >= 15.0f) {
		f32 side;
		side    = radius - 5.0f;
		f32 dot = dir.x * sin + dir.z * cos;
		dot *= plySqrt(radius * radius - side * side);
		pitch = (int)(10430.38043493439 * (f32)atan2(dot, side));
	} else {
		pitch = 0;
	}
	mtxIdentity(mtx);
	mtxRotateY(mtx, mtx, yaw);
	mtxRotateX(mtx, mtx, -pitch);
	mtxInverse(inverse, bodyMtx);
	mtxConcat(player->_270, inverse, mtx);
	player->mMobile->setFacing(bodyMtx);
}

// 0x80035BF8
// Keeps the nearest hit of a ray
extern "C" int plyDeathRayHit(f32 dist, MeshColli* mesh, u16 tri, const Vec3f* pos, PlyRayHit* hit)
{
	MeshTri* face = &mesh->mTris[tri];
	if (hit->mDist > dist) {
		hit->mDist  = dist;
		hit->mPos   = *pos;
		hit->mFlags = face->mMask;
	}
	return 1;
}

// 0x80035C40
// Looks along a move for ground that kills
extern "C" void plyCheckDeathRay(TChrPlayer* player, Vec3f* from, Vec3f* to)
{
	PlyRayHit hit;
	hit.mMesh  = 0;
	hit.mDist  = 100000.0f;
	hit.mFlags = 0;
	if (groundQueryLine((const void*)from, (const void*)to, (void*)0x810C, (void*)plyDeathRayHit, (void*)&hit) != 0
	    && (hit.mFlags & 0x810C) != 0) {
		player->mStartPos = hit.mPos;
		player->mPlayFlags |= 0x80000000;
		if (hit.mFlags & 4) {
			player->mPlayFlags |= 0x4000000;
		}
		if (hit.mFlags & 0x100) {
			player->mPlayFlags |= 0x8000000;
		}
		if (hit.mFlags & 8) {
			player->mPlayFlags |= 0x20000000;
		}
		if (hit.mFlags & 0x8000) {
			player->mPlayFlags |= 0x40000000;
		}
	}
}

// 0x80035D34
// Rides on the body using the offset plyMountMobile worked out
extern "C" void plyFollowMobile(TChrPlayer* player)
{
	Vec3f pos;
	Vec3f from;
	EggRot rot;
	f32 mtx[12];

	from.x = player->mPos.x;
	from.y = player->mPos.y;
	from.z = player->mPos.z;
	pos.x  = player->mMobile->mBody.mPos.x;
	pos.y  = player->mMobile->mBody.mPos.y;
	pos.z  = player->mMobile->mBody.mPos.z;
	rot    = player->mMobile->mBody.mRot;
	mtxSetTransV(mtx, &pos);
	mtxConcatQuat((f32*)mtx, (const f32*)mtx, (const f32*)&rot);
	mtxConcat(mtx, mtx, player->_270);
	mtxTranslate((float*)player->mMatrix, (const float*)mtx, 0.0f, 0.0f, -(2.0f + player->mMobile->mBody.mRadius));
	plyApplyScale(player, player->mMatrix);
	mtxGetPos(player->mMatrix, &player->mPos);
	mtxToQuat((const f32*)player->mMatrix, (f32*)&player->mRot);
	plyCheckDeathRay(player, &from, &player->mPos);
}

// 0x80035E38
extern "C" void plyPosFromMatrix(TChrPlayer* player)
{
	mtxGetPos(player->mMatrix, &player->mPos);
	mtxToQuat((const f32*)player->mMatrix, (f32*)&player->mRot);
}

// 0x80035E78
// A footprint under one foot at frame 12 and the other at frame 27
extern "C" void plyFootstepFx(TChrPlayer* player)
{
	PlyGroundHit hit;
	Vec3f foot;
	Vec3f normal;

	if (player->mBodyCopy.passedFrame(12.0f) != 0) {
		mtxGetPos((const f32*)(player->mJoints + 0x3F0), &foot);
	} else if (player->mBodyCopy.passedFrame(27.0f) != 0) {
		mtxGetPos((const f32*)(player->mJoints + 0x240), &foot);
	} else {
		return;
	}
	hit.mMesh    = 0;
	hit.mAnyDist = 100000.0f;
	foot.y += 1.5f;
	hit.mAnyFlags = 0;
	hit.mDist     = 100000.0f;
	if (groundQueryDown((const void*)&foot, (void*)0x81, (void*)plyAnyRayHit, (void*)&hit) != 0) {
		meshTriNormal((MeshColli*)hit.mMesh, hit.mTri, &normal);
		if (!(normal.y < 0.5f)) {
			footPrintAdd(&hit.mAnyPos, &normal, 0, (f32)player->mAngle);
			if (hit.mAnyFlags & 0x80) {
				plyPlaySe(player, 0x90);
			} else {
				plyPlaySe(player, 0x75);
			}
		}
	}
}

// 0x80035FB4
void TChrPlayer::updateMotions()
{
	mBody.update();
	mBodyCopy.update();
	mFace.update();
	mLeftHand.update();
	mRightHand.update();
	poseModels(1);
}

// 0x80036010
extern "C" void drawPlayerMotion(void* buffer, MotionPlayer* player)
{
	if (player->isBlending() != 0) {
		gjMotionLinkActor((GjActor*)buffer, (void*)player->getBlend(), player->mBlendTime);
	} else {
		int cur   = player->mCurrent;
		f32 frame = player->mFrame;
		gjMotionActor(frame, (GjActor*)buffer, player->getMotion(cur));
	}
}

// 0x80036098
// Writes each part's current motion into its buffer and those into the joints of the pose
void TChrPlayer::poseModels(int force)
{
	void* motion;
	void* shape;

	plyReadMotion((NJS_OBJECT*)mBodyBuffer[0], &mBody, mPose, plyNodeFrom26);
	plyReadMotion((NJS_OBJECT*)mBodyBuffer[0], &mBodyCopy, mPose, plyNodeTo25);
	plySetEyelidNodes(_34C->mOpen, &mPose[64], &mPose[62]);
	njPushMatrixEx();
	njUnitMatrix(0);
	plyBuildPose(mBodyBuffer, mPose);
	njPopMatrixEx();

	njSetMatrix((f32*)0, mPose[56].mMtx);
	mFace.getCurrent(&motion, &shape);
	if (motion != 0 || force != 0) {
		gjShapeActor((GjActor*)mFaceBuffer, motion, (GjShape*)shape, mFace.mFrame);
	}

	njSetMatrix((f32*)0, mPose[45].mMtx);
	if (force != 0 || mLeftHand.getMotion(mLeftHand.mCurrent) != 0) {
		drawPlayerMotion(mLeftHandBuffer, &mLeftHand);
	}

	njSetMatrix((f32*)0, mPose[35].mMtx);
	if (force != 0 || mRightHand.getMotion(mRightHand.mCurrent) != 0) {
		drawPlayerMotion(mRightHandBuffer, &mRightHand);
	}
}

// 0x800361E4
void TChrPlayer::updateJoints()
{
	int poseOffset;
	int jointOffset;
	int i       = 0;
	poseOffset  = 0;
	jointOffset = 0;
	while (mJointCount > i) {
		mtxConcat((f32*)(jointOffset + mJoints), (const f32*)mMatrix, (const f32*)((u8*)mPose + poseOffset + 0x38));
		i          = i + 1;
		poseOffset = 0x68 + poseOffset;
		jointOffset += 0x30;
	}
}

// 0x80036250
void TChrPlayer::updateMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxConcatQuat((f32*)mMatrix, (const f32*)mMatrix, (const f32*)&mRot);
	plyApplyScale(this, mMatrix);
}

// 0x800362A0
// Poses the character and moves its effects
void TChrPlayer::updatePose()
{
	Vec3f pos;

	updateMotions();
	updateJoints();
	mtxGetPos((const f32*)(mJoints + 0x510), &pos);
	colliPlaceShapesRotY(mEmitter0, &pos, mAngle, sPlayerEmitter0Shape);
	colliPlaceShapesRotY(mEmitter1, &pos, mAngle, sPlayerEmitter1Shapes);
}

// 0x80036314
// Slot _28
int TChrPlayer::getRotY()
{
	return mAngle;
}

// 0x8003631C
// Slot _20
Vec3f* TChrPlayer::getPos()
{
	return &mPos;
}

// 0x80036324
// Slot _DC
void TChrPlayer::setPos(const Vec3f* pos)
{
	f32 x  = pos->x;
	f32 y  = pos->y;
	mPos.x = x;
	f32 z  = pos->z;
	mPos.y = y;
	mPos.z = z;
}

// 0x80036340
// Slot _44
int TChrPlayer::canAct()
{
	if ((mPlayFlags & 0x1000000) != 0) {
		return 0;
	}
	return flagTest(&mBitsDC, 0) == 0 ? 1 : 0;
}

// 0x80036384
// Slot _24 Node 0 is midway between joints 0x24 and 0x2E
void TChrPlayer::getNodeMatrix(int node, f32* mtx)
{
	if (node == 0) {
		Vec3f b;
		Vec3f a;
		Vec3f mid;
		mtxGetPos(plyGetJoint(this, 0x24), &a);
		mtxGetPos(plyGetJoint(this, 0x2E), &b);
		vecAdd(&mid, &b, &a);
		vecScale(0.5f, &mid, &mid);
		mtxSetTransV(mtx, &mid);
	}
}

// 0x80036408
// Slot _58 Nothing hits itself
int TChrPlayer::onHit(const TObjAction* other, int arg, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	if (other->mObjId == mObjId) {
		return 0;
	}
	if (arg == 0) {
		hit->mFlags.setBit(0xB);
		hit->mDamage = 0;
		return 1;
	}
	if (arg == 1) {
		hit->mDamage = 0xA;
		return 1;
	}
	return 0;
}

// 0x80036488
// Runs every frame despite its name
// This update reuses each loaded flag word so it reads past the field's volatile
void TChrPlayer::draw()
{
	f32 matrix[12];

	if (debugEditActive() != 0) {
		updatePose();
		return;
	}

	if (((*(u32*)&mPlayFlags) & 0x1000000) == 0 && ((*(u32*)&mPlayFlags) & 2) == 0) {
		int alive = ((PlySlots*)this)->s6C();
		if (alive == 0) {
			plyDie(this);
			plyDropItems(this);
			playMotion(0x14);
			mVel.x = 0.0f;
			mVel.y = 0.0f;
			mVel.z = 0.0f;
			PLY_CHANGE(this, createPlyStateDie);
		} else if (((*(u32*)&mPlayFlags) & 0x80000000) != 0) {
			// Deadly ground was found on the way here and the flags say which kind
			plyLoseAllAndDie(this);
			mPrevPos = mStartPos;
			mPos     = mStartPos;
			plySetAngle(this, mAngle);
			if ((*(u32*)&mPlayFlags) & 0x8000000) {
				PLY_CHANGE(this, createPlyStateDieBurn);
			} else if ((*(u32*)&mPlayFlags) & 0x20000000) {
				PLY_CHANGE(this, createPlyStateDieSandy);
			} else if ((*(u32*)&mPlayFlags) & 0x4000000) {
				PLY_CHANGE(this, createPlyStateDieDrown);
			} else if ((*(u32*)&mPlayFlags) & 0x40000000) {
				PLY_CHANGE(this, createPlyStateDie);
			}
		} else if (mPos.y < -400.0f) {
			plyLoseAllAndDie(this);
			plySetAngle(this, mAngle);
			PLY_CHANGE(this, createPlyStateDie);
		}
	}

	// Input
	if (flagTest(&mBitsDC, 6) != 0 || ((*(u32*)&mPlayFlags) & 0x2000) == 0 || ((*(u32*)&mPlayFlags) & 0x1000000) != 0) {
		mInput.clear();
	} else if (flagTest(&mBitsDC, 5) != 0 || flagTest(&mBitsDC, 0xE) != 0) {
		mInput.readConfirm(mObjId);
	} else {
		mtxSetRotY(matrix, camGetYaw((s8)mPlayer));
		mInput.read(mObjId, matrix);
	}
	if (((*(u32*)&mPlayFlags) & 0x1000000) == 0 && eventIsBusy() == 0 && (mInput.mPressed & PLY_BTN_Y) != 0) {
		plyUseWornItem(this);
	}

	(*(u32*)&mPlayFlags) &= ~0x2000000;
	_248.x    = 0.0f;
	_248.y    = 0.0f;
	_248.z    = 0.0f;
	mTargetId = -1;

	// The current state runs here first the input then its checks if the input started nothing
	if (flagTest(&mBitsDC, 6) != 0) {
		if (plySearchDamage(this) != 0) {
			plyEndMove(this);
			blendMotion(0xC);
			PLY_CHANGE(this, createPlyStateDamage);
		}
	} else {
		TPlyState* state = (TPlyState*)mState;
		int handled      = state != 0 ? state->handleInput(this, &mInput) : 0;
		if (handled == 0 && mState != 0) {
			((TPlyState*)mState)->check(this);
		}
		runPlyState(&mState, this);
	}

	if (((*(u32*)&mPlayFlags) & 0x2000000) == 0) {
		((ObjRef*)_2BC)->setId(0xFFFF);
		_2C8->show();
	}
	if (mTimerF8 != 0) {
		mTimerF8--;
	}
	plyTintUpdate((PlyTint*)_1A8);
	if (mTimerD6 != 0) {
		mTimerD6--;
	}
	mMoveAmount = 0.0f;
	mMoveAngle  = 0;
	plyCountDown(&_198);
	mAge++;
	((SpawnGuard*)_1EC)->set(getPos(), 3.0f);
	_2F8.x = 0.0f;
	_2F8.y = gPlyParams->_7C;
	_2F8.z = 0.0f;
	_23C.x = 0.0f;
	_23C.y = 0.0f;
	_23C.z = 0.0f;
	plyReportToGame(this);
	if (mShadowTask == 0 && ((*(u32*)&mPlayFlags) & 4) == 0) {
		if (mRiders[6].mMesh != 0 && ((*(u32*)&mPlayFlags) & 0x1000) != 0) {
			shadowSetGround((ObjShadow*)_148, &mRiders[6].mPos, (const Vec3f*)0);
			shadowEnable((ObjShadow*)_148);
		} else {
			shadowDisable((ObjShadow*)_148);
		}
	}
	if (mFade != 0) {
		mFade--;
	}
	plyUpdateStatus(this);
}

// 0x800369BC
// Queues the character's draw task picking the variant its status calls for and hides it when off screen
void TChrPlayer::vfn_10()
{
	f32 matrix[12];
	Vec3f centre;
	Vec3f screen;

	vfn_A8(0x1B, matrix);
	mtxGetPos(matrix, &centre);
	u8 visible = cameraIsVisible(4000.0f, 3.0f, &centre, &screen) != 0 && (mPlayFlags & 4) == 0;

	if (visible) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}

	if (flagTest(&mBitsDC, 2) != 0) {
		mDrawTask->mFunc = plyDrawTaskGlass;
		moveDrawTask(mDrawTask, 0x2C);
	} else if (flagTest(&mBitsDC, 0) != 0) {
		mDrawTask->mFunc = plyDrawTaskFlat;
		moveDrawTask(mDrawTask, 0x2A);
	} else {
		mDrawTask->mFunc = plyDrawTask;
		moveDrawTask(mDrawTask, 0x2C);
	}

	u16 id = mObjId;
	if ((u16)playerGetDrawView() == id) {
		sPlayerScreenPos   = screen;
		gPlayerGoalVisible = visible;
	}
}

// 0x80036B20
void TChrPlayer::setMove(f32 amount, int kind)
{
	mMoveAmount = amount;
	mMoveAngle  = kind;
}

// 0x80036B2C
// Looks for an egg in reach in a cone about the facing
extern "C" TObjAction* plyFindEggInReach(TChrPlayer* player, f32 reach, int cone)
{
	PlyGrabSearch search;
	search.mId    = player->mObjId;
	search.mPos   = player->mPos;
	search.mAngle = player->getRotY();
	search.mReach = reach;
	search.mCone  = cone;
	search.mFound = 0;
	forEachContact((CharaColli*)player->mEmitter1, (ContactCallback)plyGrabSearchHit, &search);
	return search.mFound;
}

// 0x80036BC8
// Asks an egg to be picked up
extern "C" int plyAttachEgg(TChrPlayer* player, TObjAction* egg)
{
	PlyMsgPick msg;
	msg.mFrom = player->mObjId;
	if (egg->sendToHandle((ObjMsg*)&msg) != 0) {
		player->mMobile->setHandle(msg.mHandle);
		player->mMobile->reset(msg.mRadius, &msg.mPos, &msg.mRot);
		return 1;
	}
	return 0;
}

// 0x80036C64
extern "C" void plyStopAim(TChrPlayer* player)
{
	if (*(volatile u32*)&player->mPlayFlags & 8) {
		player->mPlayFlags &= ~8;
	}
}

// 0x80036C80
// Sends the crow out of joint 0x38
extern "C" void plySpawnCrow(TChrPlayer* player)
{
	f32 mtx[12];
	Vec3f pos;
	EggRot rot;

	mtxGetPos(plyGetJoint(player, 0x38), &pos);
	mtxToQuat((const f32*)player->mMatrix, (f32*)&rot);
	mtxSetTransV(mtx, &pos);
	mtxConcatQuat((f32*)mtx, (const f32*)mtx, (const f32*)&rot);
	mtxTranslate((float*)mtx, (const float*)mtx, 0.0f, 0.0f, 5.0f);
	player->_194 = (int)new TPlyCrow(player, player->mObjId, mtx);
}

// 0x80036D24
// Stops the last voice line and plays another from the head
extern "C" void plyPlayVoice(TChrPlayer* player, int id, int extra)
{
	Vec3f pos;

	seStop(player->_2CC);
	int group = gBattleGroups[player->mCharId];
	mtxGetPos((const f32*)(player->mJoints + 0xA80), &pos);
	if ((u16)extra == 0) {
		player->_2CC = sePlay3DGroup(id, &pos, group);
	} else {
		player->_2CC = ((int (*)(int, const Vec3f*, int, int))sePlay3DExtra)(id, &pos, extra, group);
	}
	if (flagTest(&player->mBitsDC, 1) != 0) {
		seSetPitch(player->_2CC, 0x1800);
	}
}

// 0x80036DD0
extern "C" void plyPlaySe(TChrPlayer* player, int id)
{
	sePlay3D(id, &player->mPos);
}

// 0x80036DFC
// Reads the stick Returns whether it is pushed at all
extern "C" int plyGetMove(TChrPlayer* player, f32* amount, int* angle)
{
	if (amount != 0) {
		*amount = player->mMoveAmount;
	}
	if (angle != 0) {
		*angle = player->mMoveAngle;
	}
	return player->mMoveAmount != 0.0f;
}

// 0x80036E3C
// Moves value toward target at this rate per second
extern "C" f32 plyApproach(f32 rate, f32 value, f32 target)
{
	f32 t = rate * (1.0f / 60.0f);
	if (t > 1.0f) {
		t = 1.0f;
	}
	return value * (1.0f - t) + target * t;
}

// 0x80036E68
// The same for an angle
extern "C" f32 plyApproachAngle(f32 rate, int angle, int target)
{
	f32 t = rate * (1.0f / 60.0f);
	if (t > 1.0f) {
		t = 1.0f;
	}
	return t * (f32)angleDiff(angle, target) + (f32)angle;
}

// 0x80036EF4
// Air control thrust from the stick drag and the facing eases level
extern "C" void plyAirControl(TChrPlayer* player)
{
	f32 amount;
	int angle;
	Vec3f force;
	f32 rot[4];

	if (plyGetMove(player, &amount, &angle) != 0) {
		f32 strength = amount * gPlyParams->_74;
		if (player->mVel.y < 0.0f) {
			strength *= 2.0f;
		}
		vecFromAngleXZ(strength, &force, angle);
	} else {
		force.x = 0.0f;
		force.y = 0.0f;
		force.z = 0.0f;
	}
	force.x -= 60.0f * (player->mVel.x * gPlyParams->mDrag);
	force.y -= 60.0f * (player->mVel.y * gPlyParams->_4C);
	force.z -= 60.0f * (player->mVel.z * gPlyParams->mDrag);
	vecAdd(&player->_2F8, &force, &player->_2F8);
	njCreateQuaternion(player->mAngle, 0.0f, 1.0f, 0.0f, (NJS_QUATERNION*)rot);
	njInterpolateQuaternion((const NJS_QUATERNION*)(&player->mRot), (const NJS_QUATERNION*)rot, (NJS_QUATERNION*)&player->mRot, 0.5f);
}

// 0x80037000
// Pushes the body while riding it faster under status 7
extern "C" void plyRideMove(TChrPlayer* player)
{
	f32 amount;
	int angle;
	Vec3f vel;
	PlyParams* params = gPlyParams;

	if (plyGetMove(player, &amount, &angle) == 0) {
		player->_340.z = 0.0f;
	} else {
		f32 top = 120.0f;
		if (flagTest(&player->mBitsDC, 7) != 0) {
			top *= 1.5f;
		}
		player->_340.z = plyApproach(0.5f * params->mAccel, player->_340.z, top);
		plySetAngle(player, (int)plyApproachAngle(60.0f * params->mTurnSpeed, player->mAngle, angle));
		player->mMobile->steer(player->_340.z, player->getRotY());
	}
	vel = player->mMobile->mBody.mVel;
	player->setMotionSpeed(vecLength(&vel) / 100.0f);
}

// 0x8003711C
// Slows the body by how hard the stick is against the way the player faces it
extern "C" void plyBrakeMove(TChrPlayer* player)
{
	f32 amount;
	PlyVec centre(player->mMobile->mBody.mPos);
	Vec3f to;
	int angle;
	int facing = angleToXZ(&player->mPos, &centre);

	if (plyGetMove(player, &amount, &angle) != 0) {
		int diff = angleDist(angle, facing);
		if (diff >= 0x6000) {
			player->mMobile->applyDrag(1.0f);
		} else if (diff >= 0x4000) {
			player->mMobile->applyDrag(0.5f);
		} else {
			player->mMobile->applyDrag(0.0f);
		}
	} else {
		player->mMobile->applyDrag(0.0f);
	}
	vecFromAngleXZ(1.0f + player->mMobile->mBody.mRadius, &to, facing + 0x8000);
	vecAdd(&to, &to, &centre);
	vecSub(&to, &to, &player->mPos);
	f32 len = vecLenXZ(&to);
	plySetGroundVel(player, 60.0f * len, vecAngleXZ(&to));
	plySetAngle(player, facing);
}

// 0x80037268
// Sets the velocity along the ground under the player
extern "C" void plySetGroundVel(TChrPlayer* player, f32 speed, int angle)
{
	Vec3f dir;
	Vec3f normal;

	if (plyGetProbe(player, 5, 0, 0, &normal) != 0) {
		vecFromAngleXZ(speed, &dir, angle);
		plyProjectOnPlane(&player->mVel, &normal, &dir);
	}
}

// 0x800372DC
// Holds gravity off for the first frames of a jump
extern "C" void plyJumpHold(TChrPlayer* player)
{
	int t = player->_198;
	if (t != 0 && t <= gPlyParams->_0E - gPlyParams->_0C) {
		player->_2F8.y += gPlyParams->_78;
	}
}

// 0x80037314
// Blinks while hurt unless the colour effect is running
extern "C" int plyIsDrawn(TChrPlayer* player)
{
	if (player->mTimerD6 != 0 && (player->mTimerD6 & 1) == 0 && plyTintIsDone((PlyTint*)player->_1A8) != 0) {
		return 0;
	}
	return 1;
}

// 0x80037360
extern "C" void plyDrawBegin(TChrPlayer* player)
{
	void* lights = *(void**)((u8*)getSceneRecord() + 0xC);
	if (lights != 0) {
		((LightGroup*)lights)->addAll(player->mStatus);
		lightGroupLoad((LightGroup*)lights);
	}
	shadowAlphaBegin();
	lightZoneBegin(player->mObjId);
}

// 0x800373B8
extern "C" void plyDrawEnd(TChrPlayer* player)
{
	lightZoneEnd(player->mObjId);
	shadowAlphaEnd();
	void* lights = *(void**)((u8*)getSceneRecord() + 0xC);
	if (lights != 0) {
		((LightGroup*)lights)->removeAll(player->mStatus);
		lightGroupLoad((LightGroup*)lights);
	}
}

// 0x80037410
// The normal draw task
extern "C" void plyDrawTask(void* obj)
{
	TChrPlayer* player = (TChrPlayer*)obj;
	ObjDrawState state;
	if (plyIsDrawn(player) != 0) {
		if (plyTintIsDone((PlyTint*)player->_1A8) == 0) {
			state.mFlags |= DRAWSTATE_TEV2;
			state.mTev2 = *(u32*)(player->_1A8 + 4);
		}
		DrawStateSlot* slot = getDrawStateSlot();
		plyDrawBegin(player);
		njSetTexture(player->mTexLists[0]);
		njPushMatrixEx();
		njMultiMatrix(0, player->mMatrix);
		beginDrawState(slot, (const DrawState*)&state);
		plyDrawParts(player);
		endDrawState(slot);
		njPopMatrixEx();
		player->mModel->drawModel(&state);
		plyDrawEnd(player);
	}
}

// 0x800374D8
extern "C" void plyDrawGlassTev(u8 stage)
{
	GXSetTevOrder(stage, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
	GXSetTevColorIn(stage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_HALF, GX_CC_ZERO);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

// 0x8003757C
extern "C" void plyDrawGlassCoord(u8 coord)
{
	GXSetTexCoordGen2(coord, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX5, GX_TRUE, GX_PTTEXMTX19);
}

// 0x800375B4
// The draw task under status 2 with the extra texture pass
extern "C" void plyDrawTaskGlass(void* obj)
{
	TChrPlayer* player = (TChrPlayer*)obj;
	struct {
		u8 mStages;
		u8 mCoords;
		void* mCoordFn;
		void* mStageFn;
	} extra;
	ObjDrawState state;
	f32 mtx[12];

	if (plyIsDrawn(player) != 0) {
		njPushMatrixEx();
		njMultiMatrix(0, player->mMatrix);
		PSMTXCopy(_gj_matrix_, mtx);
		mtx[3]  = 0.0f;
		mtx[7]  = 0.0f;
		mtx[11] = 0.0f;
		GXLoadTexMtxImm(mtx, GX_TEXMTX5, GX_MTX3x4);
		state.mFlags |= DRAWSTATE_EXTRA;
		extra.mStages  = 1;
		extra.mCoords  = 1;
		extra.mCoordFn = plyDrawGlassCoord;
		extra.mStageFn = plyDrawGlassTev;
		state.mExtra   = &extra;
		njSetTexture(sPlayerTexLists[0]);
		gjLoadTextureNum(0, 1, 1, 1);
		DrawStateSlot* slot = getDrawStateSlot();
		plyDrawBegin(player);
		njSetTexture(player->mTexLists[0]);
		beginDrawState(slot, (const DrawState*)&state);
		plyDrawParts(player);
		endDrawState((DrawStateSlot*)slot);
		njPopMatrixEx();
		player->mModel->drawModel(&state);
		plyDrawEnd(player);
	}
}

// 0x800376DC
// The draw task under status 0 pass 0x400
extern "C" void plyDrawTaskFlat(void* obj)
{
	TChrPlayer* player = (TChrPlayer*)obj;
	ObjDrawState state;
	state.mFlags        = 0x400;
	DrawStateSlot* slot = getDrawStateSlot();
	njPushMatrixEx();
	njMultiMatrix(0, player->mMatrix);
	beginDrawState(slot, (const DrawState*)&state);
	plyDrawParts(player);
	endDrawState((DrawStateSlot*)slot);
	njPopMatrixEx();
	player->mModel->drawModel(&state);
}

// 0x8003775C
// The shadow task pass 0x800
extern "C" void plyDrawTaskShadow(void* obj)
{
	TChrPlayer* player = (TChrPlayer*)obj;
	ObjDrawState state;
	int alpha;
	if ((s8)player->_318 != 0) {
		alpha = (int)(128.0f * (1.0f - (f32)player->mFade / 10.0f));
	} else {
		alpha = (int)(128.0f * ((f32)player->mFade / 10.0f));
	}
	setColorRed(&state.mMaterial, alpha);
	state.mFlags        = 0x800;
	DrawStateSlot* slot = getDrawStateSlot();
	beginDrawState(slot, (const DrawState*)&state);
	njPushMatrixEx();
	njTranslateEx(&player->mPos);
	njQuaternionEx((const NJS_QUATERNION*)&player->mRot);
	plyDrawParts(player);
	njPopMatrixEx();
	endDrawState((DrawStateSlot*)slot);
	player->mModel->drawModel(&state);
}

// 0x80037870
// Two rows that both have flag 1 change without leaving the current frame which keeps feet in step
extern "C" void plyPlayMotionSynced(MotionPlayer* player, int row)
{
	int current  = *(volatile s16*)&player->mCurrent;
	u32 oldFlags = player->getUser(current);
	u32 newFlags = player->getUser(row);
	if ((newFlags & 1) != 0 && (oldFlags & 1) != 0) {
		if (current != row) {
			player->playSynced(row, 0.5f);
		}
	} else if (current != row) {
		player->play(row);
	}
}

// 0x800378F8
extern "C" void plyDrawParts(TChrPlayer* player)
{
	beginActorDraw();
	drawActor((GjActor*)player->mBodyBuffer);
	drawActor((GjActor*)player->mFaceBuffer);
	drawActor((GjActor*)player->mLeftHandBuffer);
	drawActor((GjActor*)player->mRightHandBuffer);
	endActorDraw();
}

// 0x80037948
// Turns the two eyelid nodes to how far open they are
extern "C" void plySetEyelidNodes(f32 open, PlyPose* upper, PlyPose* lower)
{
	int angle      = (int)(182.04445f * (40.0f * open));
	upper->mAng[0] = angle;
	if (upper->mRotMode != 2) {
		goto skipUpper;
	}
	njXYZAngleToQuaternion(upper->mAng, (NJS_QUATERNION*)upper->mQuat);
skipUpper:
	lower->mAng[0] = -angle;
	if (lower->mRotMode == 2) {
		njXYZAngleToQuaternion(lower->mAng, (NJS_QUATERNION*)lower->mQuat);
	}
}

// 0x800379CC
extern "C" int plyNodeAny()
{
	return 1;
}

// 0x800379D4
// The body's lower joints take the first motion and the upper the copy
extern "C" u8 plyNodeFrom26(int node)
{
	return node >= 0x1A;
}

// 0x800379F0
extern "C" u8 plyNodeTo25(int node)
{
	return node <= 0x19;
}

// 0x80037A0C
extern "C" void plyReadMotion(NJS_OBJECT* node, MotionPlayer* player, void* pose, void* test)
{
	if (player->isBlending() != 0) {
		plyReadMotionLink(node, player->getBlend(), pose, test, player->mBlendTime);
	} else {
		int cur            = player->mCurrent;
		f32 frame          = player->mFrame;
		NJS_MOTION* motion = player->getMotion(cur);
		plyReadMotionPlain(node, motion, pose, test, frame);
	}
}

// 0x80037AAC
extern "C" int plyReadMotionPlain(NJS_OBJECT* node, void* motion, void* pose, void* test, f32 frame)
{
	njStartMotionObj(motion, frame);
	return plyReadNodes(node, pose, 0, test);
}

// 0x80037AF8
extern "C" void plyReadMotionLink(NJS_OBJECT* node, void* blend, void* pose, void* test, f32 frame)
{
	njStartMotionLink((const NJS_MOTION_LINK*)blend, frame);
	plyReadLinkNodes(node, pose, 0, test);
}

// 0x80037B44
// Walks the node tree reading this frame's data into the pose of each node the test passes
extern "C" int plyReadNodes(NJS_OBJECT* tree, void* pose, int index, void* test)
{
	do {
		if (((int (*)(int))test)(index) != 0) {
			PlyPose* out  = (PlyPose*)pose + index;
			out->mRotMode = njGetMotionNodeData(tree, (f32*)&out->mPos, out->mAng, (f32*)&out->mScale, (NJS_QUATERNION*)out->mQuat);
		} else {
			njSetNextMotionNodeEx();
		}
		index++;
		if (tree->child != 0) {
			index = plyReadNodes(tree->child, pose, index, test);
		}
		tree = tree->sibling;
	} while (tree != 0);
	return index;
}

// 0x80037BF0
extern "C" int plyReadLinkNodes(NJS_OBJECT* tree, void* pose, int index, void* test)
{
	do {
		if (((int (*)(int))test)(index) != 0) {
			PlyPose* out  = (PlyPose*)pose + index;
			out->mRotMode = njGetMotionLinkNodeData(tree, (f32*)&out->mPos, out->mAng, (f32*)&out->mScale, (NJS_QUATERNION*)out->mQuat);
		} else {
			njSetNextMotionLinkNode();
		}
		index++;
		if (tree->child != 0) {
			index = plyReadLinkNodes(tree->child, pose, index, test);
		}
		tree = tree->sibling;
	} while (tree != 0);
	return index;
}

// 0x80037C9C
extern "C" void plyBuildPose(void** buffer, void* pose)
{
	gjBeginCalcActor((GjActor*)buffer);
	plyBuildNodes((NJS_OBJECT*)buffer[0], 0, pose);
	gjEndCalcActor((GjActor*)buffer);
}

// 0x80037CE4
// Builds each node's matrix from its pose
extern "C" int plyBuildNodes(NJS_OBJECT* tree, int index, void* pose)
{
	do {
		PlyPose* out = (PlyPose*)pose + index;
		njPushMatrixEx();
		njTranslateEx(&out->mPos);
		switch (out->mRotMode) {
		case 0:
			njRotateEx(out->mAng, (s32)0);
			break;
		case 1:
			njRotateEx(out->mAng, (s32)1);
			break;
		default:
			njQuaternionEx((const NJS_QUATERNION*)out->mQuat);
			break;
		}
		njScaleEx(&out->mScale);
		njGetMatrix(out->mMtx);
		if (tree->model != 0 && (tree->evalflags & 8) == 0) {
			gjCalcActor(tree->model);
		}
		index++;
		if (tree->child != 0) {
			index = plyBuildNodes(tree->child, index, pose);
		}
		njPopMatrixEx();
		tree = tree->sibling;
	} while (tree != 0);
	return index;
}

// 0x80037DCC
// The same request as plyAttachEgg
extern "C" int plyAttachEgg2(TChrPlayer* player, TObjAction* egg)
{
	PlyMsgPick msg;
	msg.mFrom = player->mObjId;
	if (egg->sendToHandle((ObjMsg*)&msg) != 0) {
		player->mMobile->setHandle(msg.mHandle);
		player->mMobile->reset(msg.mRadius, &msg.mPos, &msg.mRot);
		return 1;
	}
	return 0;
}

// 0x80037E68
// Picks an egg up and tells it with message 0x66
extern "C" int plyPickUpEgg(TChrPlayer* player, TObjAction* egg)
{
	if (plyAttachEgg2(player, egg) != 0) {
		plyHoldEgg(player, egg);
		PlyMsg msg(0x66);
		msg.mFrom = player->mObjId;
		egg->sendToHandle((ObjMsg*)&msg);
		return 1;
	}
	return 0;
}

// 0x80037EEC
// Lets go of what a reference holds with message 0x69
extern "C" void plyReleaseRef(TChrPlayer* player, ObjRef* ref)
{
	if ((int)ref->getId() != 0xFFFF) {
		PlyMsg msg(0x69);
		msg.mFrom = player->mObjId;
		ref->get()->sendToHandle((ObjMsg*)&msg);
		ref->setId(0xFFFF);
	}
}

// 0x80037F78
extern "C" void plyReleaseEgg(TChrPlayer* player, TObjAction*)
{
	plyReleaseRef(player, &player->mEggRef);
	player->mMobile->setHandle(0);
}

// 0x80037FB4
// Back to this player's start point
extern "C" void plyToStartPoint(TChrPlayer* player)
{
	plyLetGoAll(player);
	player->setPos(&sPlayerStartPos[player->mObjId]);
	plySetAngle(player, sPlayerStartAngle[(u16)player->mObjId]);
	player->updateMatrix();
}

// 0x8003802C
// Slot _4C
int TChrPlayer::sendToHandle(ObjMsg* msg)
{
	TPlyState* state = (TPlyState*)mState;
	if (state != 0) {
		state->onMessage(this, msg);
	} else {
		return plyDefaultMessage(this, msg);
	}
}

// 0x8003807C
// Drops what is worn and held as pickups where the player stands
extern "C" void plyDropItems(TChrPlayer* player)
{
	bool has = ((ItemHolder*)player->_4F0)->hasItem();
	if (has) {
		int kind = has ? ((ItemHolder*)player->_4F0)->mEntry->mData.mKind : 0xFFFF;
		int from = player->mObjId;
		spawnItemThrown(from, kind, player->getPos());
		itemHolderRelease((ItemHolder*)player->_4F0);
		struct {
			u16 mType;
			u16 _02;
			u16 mPlayer;
			u16 _06;
			int mId;
		} msg;
		msg.mType   = 0xB5;
		msg.mPlayer = player->mObjId;
		msg.mId     = -1;
		aeSendMessage(&msg);
	}
	if (player->_520 != 0) {
		int kind = *(int*)((u8*)player->_520 + 0x34);
		int from = player->mObjId;
		spawnItemThrown(from, kind, player->getPos());
		((PlyItemSlots*)player->_520)->s74();
		delete (TObject*)player->_520;
		player->_520 = 0;
		player->_D0  = 0;
	}
}

// 0x80038198
extern "C" int plySetFlagBounce(TChrPlayer* player)
{
	player->mPlayFlags |= 0x10000000;
	return 1;
}

// 0x800381AC
// Slot _D4 Character 0 changes to the models in archive 0
int TChrPlayer::vfn_D4()
{
	if (mCharId != 0) {
		return 0;
	}
	TArc* arc = gPlayerArcs[0];
	if (arc == 0) {
		return 0;
	}
	mModels   = (void**)findArcExport(arc, "models");
	mTexLists = (void**)findArcExport(arc, "texlists");
	memFree(mBodyBuffer);
	memFree(mFaceBuffer);
	memFree(mLeftHandBuffer);
	memFree(mRightHandBuffer);
	mBodyBuffer      = (void**)createActor((NJS_OBJECT*)mModels[0]);
	mFaceBuffer      = createActor((NJS_OBJECT*)mModels[2]);
	mLeftHandBuffer  = createActor((NJS_OBJECT*)mModels[3]);
	mRightHandBuffer = createActor((NJS_OBJECT*)mModels[4]);
	poseModels(1);
	mModel->build(mModels[1]);
	mModel->mTexture = mTexLists[0];
	mPlayFlags |= 0x200;
	return 1;
}

// 0x800382C4
// Slot _D8 Character 0 changes to the models in archive 4
int TChrPlayer::vfn_D8()
{
	if (mCharId != 0) {
		return 0;
	}
	TArc* arc = gPlayerArcs[4];
	if (arc == 0) {
		return 0;
	}
	mModels   = (void**)findArcExport(arc, "models");
	mTexLists = (void**)findArcExport(arc, "texlists");
	memFree(mBodyBuffer);
	memFree(mFaceBuffer);
	memFree(mLeftHandBuffer);
	memFree(mRightHandBuffer);
	mBodyBuffer      = (void**)createActor((NJS_OBJECT*)mModels[0]);
	mFaceBuffer      = createActor((NJS_OBJECT*)mModels[2]);
	mLeftHandBuffer  = createActor((NJS_OBJECT*)mModels[3]);
	mRightHandBuffer = createActor((NJS_OBJECT*)mModels[4]);
	poseModels(1);
	mModel->build(mModels[1]);
	mModel->mTexture = mTexLists[0];
	mPlayFlags &= ~0x200;
	return 1;
}

// 0x800383DC
extern "C" int plyIsFalling(TChrPlayer* player)
{
	return player->mVel.y < 0.0f;
}

// 0x800383F4
// Ends the current state's move
extern "C" void plyEndMove(TChrPlayer* player)
{
	PLY_ENDSTATE(player);
}

// 0x80038430
extern "C" void plyAirNudge(TChrPlayer* player)
{
	f32 amount;
	int angle;
	Vec3f force;

	if (plyGetMove(player, &amount, &angle) != 0) {
		vecFromAngleXZ(0.01f * amount, &force, angle);
		njAddVector(&player->_2F8, &force);
	}
}

// 0x80038490
// Slides along facing the stick
extern "C" void plySlideMove(TChrPlayer* player)
{
	int angle = player->mAngle;
	int want;
	Vec3f force;

	if (plyGetMove(player, 0, &want) != 0) {
		angle = angleApproach(angle, want, 0x100);
		plySetAngle(player, angle);
	}
	force.x = 0.0f;
	force.y = 0.0f;
	force.z = gPlyParams->_90;
	vecRotateY(&force, &force, angle);
	force.x -= 60.0f * (player->mVel.x * gPlyParams->mDrag);
	force.z -= 60.0f * (player->mVel.z * gPlyParams->mDrag);
	vecScaleAdd(1.0f / 60.0f, &player->mVel, &force, &player->mVel);
	player->mVel.y = -0.5f;
}

// 0x8003856C
// Adds up the pushes from what the body collider touches other than the egg it holds
extern "C" void plyCollectPushes(TChrPlayer* player)
{
	CharaColli* colli = player->mEmitter0;
	if (colli->mContactCount != 0) {
		for (ColliContact* contact = colli->mContacts; contact != 0; contact = contact->mNext) {
			if (contact->mShape == 0 && (colliContactShape(contact)->mFlags & 0x10) == 0) {
				int egg   = (u16)player->mEggRef.getId();
				int owner = (u16)colliContactOwner(contact);
				if (owner != egg) {
					vecScaleAdd(0.5f, &player->_23C, &contact->mPush, &player->_23C);
					if (vecLength(&contact->mPush) > 0.0f && vecLenXZ(&contact->mPush) < 0.0001f) {
						player->_23C.z += 1.0f;
					}
				}
			}
		}
	}
}

// 0x80038644
extern "C" void plySetAngle(TChrPlayer* player, int angle)
{
	player->mAngle = angle;
	njCreateQuaternion(angle, 0.0f, 1.0f, 0.0f, (NJS_QUATERNION*)&player->mRot);
}

// 0x80038680
// Shows or hides the shadow task
extern "C" void plyShowShadowTask(TChrPlayer* player, int on)
{
	if (player->mShadowTask != 0) {
		if (on != 0) {
			player->mShadowTask->mFlags &= ~1;
		} else {
			player->mShadowTask->mFlags |= 1;
		}
	}
}

// 0x800386B4
// Bit 1 on the ground bit 0x80 frozen
extern "C" int plyGetStatusBits(TChrPlayer* player)
{
	int status = 0;
	if (plyOnGround(player) != 0) {
		status |= 1;
	}
	if ((player->mPlayFlags & 0x1000000) != 0) {
		status |= 0x80;
	}
	return status;
}

// 0x80038704
// Message 1 a push
extern "C" int plyMsgPush(TChrPlayer* player, u8* msg)
{
	vecAdd(&player->_23C, &((ObjVecMsg*)msg)->mVec, &player->_23C);
	return 1;
}

// 0x80038734
// Message 0xE die
extern "C" int plyMsgDie(TChrPlayer* player)
{
	plyDie(player);
	player->playMotion(0x14);
	PLY_CHANGE(player, createPlyStateDie);
	return 1;
}

// 0x8003879C
// Message 0xF sink in sand
extern "C" int plyMsgSink(TChrPlayer* player)
{
	if ((player->mPlayFlags & 0x1000000) != 0) {
		return 0;
	}
	player->mPrevPos = player->mPos;
	plyLoseAllAndDie(player);
	PLY_CHANGE(player, createPlyStateDieSandy);
	return 1;
}

// 0x80038824
// Message 0x10 leave play
extern "C" int plyMsgDisable(TChrPlayer* player)
{
	if ((player->mPlayFlags & 2) != 0) {
		return 0;
	}
	player->mPlayFlags |= 2;
	player->mPlayFlags |= 4;
	if ((int)player->mEggRef.getId() != 0xFFFF) {
		plyReleaseEgg(player, player->mEggRef.get());
	}
	PLY_ENDSTATE(player);
	player->mModel->mFlags |= 0x10;
	colliDisable(player->mEmitter0);
	plyShowShadowTask(player, 0);
	PLY_CHANGE(player, createPlyStateDisable);
	return 0;
}

// 0x80038918
// Message 0x13 back in play
extern "C" int plyMsgEnable(TChrPlayer* player)
{
	player->mPlayFlags &= ~2;
	player->mPlayFlags &= ~4;
	player->mModel->mFlags &= ~0x10;
	colliEnable(player->mEmitter0);
	plyShowShadowTask(player, 1);
	PLY_CHANGE(player, createPlyStateStand);
	return 0;
}

// 0x800389AC
// Message 0x33 swaps what the fifth reference holds telling the old one with message 0x46
extern "C" int plyMsgSetMount(TChrPlayer* player, u8* msg)
{
	TObjAction* obj = player->mMountRef.get();
	if (obj != 0) {
		ObjMsg out;
		out.mType = 0x46;
		obj->sendToHandle(&out);
	}
	player->mMountRef.setId(((ObjIdMsg*)msg)->mId);
	return 1;
}

// 0x80038A14
// Message 0x3C
extern "C" int plyMsgGetStatus(TChrPlayer* player)
{
	return plyGetStatusBits(player);
}

// 0x80038A34
// Message 0x3E the egg's id
extern "C" int plyMsgGetEggId(TChrPlayer* player)
{
	return player->mEggRef.getId();
}

// 0x80038A5C
// Message 0x3F where to look at the player
extern "C" int plyMsgGetLookPos(TChrPlayer* player, u8* msg)
{
	Vec3f* out = &((ObjVecMsg*)msg)->mVec;
	if (plyGetMobile(player) != 0) {
		*out = player->mMobile->mBody.mPos;
	} else {
		out->x = player->mPos.x;
		out->y = 10.0f + player->mPos.y;
		out->z = player->mPos.z;
	}
	return 1;
}

// 0x80038AD8
// Message 0x3D
extern "C" int plyMsgGetId(TChrPlayer* player)
{
	return player->mObjId;
}

// 0x80038AE0
// Message 0x77 an event takes over
extern "C" int plyMsgEvent(TChrPlayer* player)
{
	PLY_CHANGE(player, createPlyStateEvent);
	return 0;
}

// 0x80038B38
// Message 0x78 an event with this place and facing
extern "C" int plyMsgEventAt(TChrPlayer* player, u8* msg)
{
	plyPlayVoice(player, 0xDA, 0);
	player->playMotion(0x2F);
	player->mFace.play(6);
	player->mLeftHand.play(6);
	player->mRightHand.play(6);
	player->setPos(&((ObjVecMsg*)msg)->mVec);
	plySetAngle(player, *(int*)(msg + 0x10));
	player->updateMatrix();
	PLY_CHANGE(player, createPlyStateEvent);
	return 1;
}

// 0x80038BFC
// Message 0x7A apologise
extern "C" int plyMsgSorry(TChrPlayer* player)
{
	plyToStartPoint(player);
	player->mPlayFlags |= 0x1000000;
	colliDisable(player->mEmitter0);
	PLY_CHANGE(player, createPlyStateSorry);
	return 1;
}

// 0x80038C6C
// Message 0x7C place the player on an object's node
extern "C" int plyMsgPlace(TChrPlayer* player, u8* msg)
{
	if (player->_2A0 != 0xFFFF) {
		return 0;
	}
	TObjAction* egg = plyGetEgg(player);
	if (egg != 0) {
		plyReleaseEgg(player, egg);
	}
	PLY_ENDSTATE(player);
	player->mVel.x = 0.0f;
	player->mVel.y = 0.0f;
	player->mVel.z = 0.0f;
	player->_2A0   = ((PlyPlaceMsg*)msg)->_04;
	player->_2A2   = ((PlyPlaceMsg*)msg)->_06;
	player->playMotion(((PlyPlaceMsg*)msg)->mMotion);
	PLY_CHANGE(player, createPlyStatePlace);
	return 1;
}

// 0x80038D3C
// Message 0x7D
extern "C" int plyMsgStageClear(TChrPlayer* player, u8* msg)
{
	plyStartEvent(player);
	player->playMotion(0x2F);
	player->mFace.play(6);
	player->mLeftHand.play(6);
	player->mRightHand.play(6);
	plyPlayVoice(player, 0xDA, 0);
	player->setPos(&((ObjVecMsg*)msg)->mVec);
	plySetAngle(player, *(int*)(msg + 0x10));
	player->updateMatrix();
	return 1;
}

// 0x80038DDC
// Message 0x82 restart at the start point
extern "C" int plyMsgRestart(TChrPlayer* player)
{
	plyToStartPoint(player);
	camResetCamera((s8)player->mPlayer);
	player->playMotion(0);
	PLY_CHANGE(player, createPlyStateStand);
	player->updatePose();
	return 0;
}

// 0x80038E58
// Message 0x84 victory
extern "C" int plyMsgVictory(TChrPlayer* player)
{
	PLY_CHANGE(player, createPlyStateVictory);
	return 0;
}

// 0x80038EB0
// Message 0x85
extern "C" int plyMsgAltModels(TChrPlayer* player)
{
	return player->vfn_D8();
}

// 0x80038EDC
// What messages do when the state does not take them
extern "C" int plyDefaultMessage(TChrPlayer* player, ObjMsg* msg)
{
	if ((player->mPlayFlags & 2) != 0) {
		switch (msg->mType) {
		case 0x13:
			return plyMsgEnable(player);
		}
		return 0;
	}
	switch (msg->mType) {
	case 0x1:
		return plyMsgPush(player, (u8*)msg);
	case 0xE:
		return plyMsgDie(player);
	case 0xF:
		return plyMsgSink(player);
	case 0x10:
		return plyMsgDisable(player);
	case 0x33:
		return plyMsgSetMount(player, (u8*)msg);
	case 0x3C:
		return plyMsgGetStatus(player);
	case 0x3E:
		return plyMsgGetEggId(player);
	case 0x3F:
		return plyMsgGetLookPos(player, (u8*)msg);
	case 0x3D:
		return plyMsgGetId(player);
	case 0x77:
		return plyMsgEvent(player);
	case 0x78:
		return plyMsgEventAt(player, (u8*)msg);
	case 0x7A:
		return plyMsgSorry(player);
	case 0x7C:
		return plyMsgPlace(player, (u8*)msg);
	case 0x7D:
		return plyMsgStageClear(player, (u8*)msg);
	case 0x82:
		return plyMsgRestart(player);
	case 0x84:
		return plyMsgVictory(player);
	case 0x85:
		return plyMsgAltModels(player);
	}
	return 0;
}

// 0x80039070
extern "C" TMobileObject* plyGetMobile(TChrPlayer* player)
{
	if (player->mMobile->mHandle != 0) {
		return player->mMobile;
	}
	return 0;
}

// 0x80039088
extern "C" void plyStartEvent(TChrPlayer* player)
{
	plyLetGoAll(player);
	PLY_CHANGE(player, createPlyStateEvent);
}

// 0x800390E0
// Lets go of everything and stops
extern "C" void plyLetGoAll(TChrPlayer* player)
{
	plyReleaseEgg(player, plyGetEgg(player));
	plyReleaseRef(player, &player->mThrownEggRef);
	PLY_ENDSTATE(player);
	player->_2F8.x = 0.0f;
	player->_2F8.y = 0.0f;
	player->_2F8.z = 0.0f;
	player->mVel.x = 0.0f;
	player->mVel.y = 0.0f;
	player->mVel.z = 0.0f;
	player->_23C.x = 0.0f;
	player->_23C.y = 0.0f;
	player->_23C.z = 0.0f;
}

// 0x8003916C
extern "C" void plyGetMatrix(TChrPlayer* player, void* out)
{
	mtxCopy((f32*)player->mMatrix, (const f32*)out);
}

// 0x80039190
// Removes from a vector what points along the normal
extern "C" void plyProjectOnPlane(Vec3f* out, Vec3f* normal, Vec3f* vec)
{
	vecScaleAdd(-njInnerProduct(vec, normal), out, normal, vec);
}

// 0x800391DC
extern "C" int plyTrySpinBounce(TChrPlayer* player)
{
	TMobileObject* mobile = plyGetMobile(player);
	if (mobile != 0) {
		mobile->stopBrake();
		return 1;
	}
	return 0;
}

// 0x80039214
extern "C" void plyHoldJump(TChrPlayer* player)
{
	TMobileObject* mobile = plyGetMobile(player);
	if (mobile != 0) {
		mobile->addLift();
	}
}

// 0x80039240
// Slams the egg forward within 50 degrees of the facing
extern "C" void plySlamEgg(TChrPlayer* player)
{
	int angle;
	TObjAction* egg = plyGetEgg(player);
	if (egg != 0) {
		if (plyGetMove(player, 0, &angle) != 0) {
			if (angleDist(angle, player->mAngle) > 0x238E) {
				if (angleDiff(angle, player->mAngle) > 0) {
					angle = player->mAngle - 0x238E;
				} else {
					angle = player->mAngle + 0x238E;
				}
			}
		} else {
			angle = player->mAngle;
		}
		PlyMsgDir msg(0x71);
		msg.mFrom = player->mObjId;
		vecFromAngleXZ(100.0f, &msg.mVec, angle);
		egg->sendToHandle((ObjMsg*)&msg);
		plyForgetEgg(player);
		player->mMobile->setHandle(0);
	}
}

// 0x80039334
extern "C" void plyKeepSteering(TChrPlayer* player)
{
	f32 amount;
	int angle;
	if (plyGetMove(player, &amount, &angle) != 0) {
		plyGetMobile(player)->steer(amount, angle);
	}
}

// 0x80039384
extern "C" int plyHasNormalModels(TChrPlayer* player)
{
	return player->mPlayFlags & 0x200;
}

// 0x80039390
extern "C" int plySetMoveIfAny(TChrPlayer* player, f32 amount, int angle)
{
	if (amount != 0.0f) {
		player->setMove(amount, angle);
		return 1;
	}
	return 0;
}

// 0x800393C8
// Aims the egg if message 0x40 says it can be
extern "C" int plyStartAim(TChrPlayer* player)
{
	TObjAction* egg = plyGetEgg(player);
	if (egg == 0) {
		return 0;
	}
	PlyMsgT msg(0x40);
	msg.mFrom = player->mObjId;
	if (egg->sendToHandle((ObjMsg*)&msg) & 1) {
		if ((player->mPlayFlags & 8) == 0) {
			player->mPlayFlags |= 8;
			plyPlayVoice(player, 0xA2, 0);
		}
		return 1;
	}
	return 0;
}

// 0x80039470
extern "C" int plyIsAiming(TChrPlayer* player)
{
	return player->mPlayFlags & 8;
}

// 0x8003947C
// Whether to brake rolling at speed with the stick let go or the stick 40 degrees or more off the way the body rolls
extern "C" int plyCheckBrake(TChrPlayer* player, f32 amount, int angle)
{
	PlyVec vel(player->mMobile->mBody.mVel);
	int brake;
	f32 speed = vecLenXZ(&vel);
	if (speed >= sPlySpeedLimit && amount == 0.0f) {
		brake = 1;
	} else if (angleDist(vecAngleXZ(&vel), angle) >= 0x71C7) {
		brake = 1;
	} else {
		brake = 0;
	}
	if (brake != 0) {
		plyPlaySe(player, 0x7E);
	}
	return brake;
}

// 0x80039540
extern "C" int plyHasMobile(TChrPlayer* player)
{
	return plyGetMobile(player) ? 1 : 0;
}

struct PlyEggSearch {
	u16 mId;            // _00
	Vec3f mPos;         // _04
	int mAngle;         // _10
	int mNotHit;        // _14 set while the player has not been hurt
	int mSuper;         // _18 status 0xD is on which also lets class gTObjLastBossShotName be picked up
	TObjAction* mFound; // _1C
	Vec3f mFoundPos;    // _20
};

// 0x80039568
// Looks through the contacts of the reach collider for an egg that can be picked up from the front
extern "C" void plyEggSearchHit(ColliContact* contact, PlyEggSearch* search, int* stop)
{
	if (contact->mShape == 1 && (colliContactShape(contact)->mFlags & 0x20) != 0) {
		TObjAction* obj = findObjById(colliContactOwner(contact));
		if (obj != 0) {
			Vec3f dir;
			if (obj->_00 == (u32)gTObjLastBossShotName && search->mSuper == 0) {
				return;
			}
			if (search->mNotHit != 0 && search->mId != obj->mHolderId) {
				return;
			}
			vecSub(&dir, &search->mPos, &contact->mPos);
			PlyMsgT ask(0x40);
			ObjMsg ask2;
			ask2.mType = 0x41;
			ask.mFrom  = search->mId;
			int can    = obj->sendToHandle((ObjMsg*)&ask);
			int kind   = obj->sendToHandle(&ask2);
			if ((can & 2) == 0 || (kind & 0x200) == 0) {
				return;
			}
			struct {
				u16 mType;
				Vec3f mFront;
			} front;
			front.mType = 0x43;
			if (obj->sendToHandle((ObjMsg*)&front) != 0) {
				int a = vecAngleXZ(&dir);
				int b = vecAngleXZ(&front.mFront);
				if (angleDist(a, b) < 0x4000) {
					search->mFound    = obj;
					search->mFoundPos = contact->mPos;
					*stop             = 1;
				}
			}
		}
	}
}

// 0x800396FC
// Picks up a nearby egg turning to face it
extern "C" bool plyTryPickUpBehind(TChrPlayer* player)
{
	PlyEggSearch search;
	search.mId     = player->mObjId;
	search.mPos    = player->mPos;
	search.mAngle  = player->mAngle;
	search.mFound  = 0;
	search.mNotHit = player->mTimerF8 == 0;
	search.mSuper  = flagTest(&player->mBitsDC, 0xD);
	forEachContact((CharaColli*)player->mEmitter1, (ContactCallback)plyEggSearchHit, &search);
	if (search.mFound != 0) {
		plySetAngle(player, angleToXZ(&player->mPos, &search.mFoundPos));
		PlyMsg msg(0x65);
		msg.mFrom = player->mObjId;
		search.mFound->sendToHandle((ObjMsg*)&msg);
		plyRumble(player, 0x14);
		Vec3f pos = player->mPos;
		pos.y += 5.0f;
		particleOnceById(gTL_PARTICLE, &pos, 0, 0x27);
		return plyPickUpEgg(player, search.mFound) != 0;
	}
	return 0;
}

// 0x80039844
// Where a ray crosses a segment on the ground plane
extern "C" int plyRayCrossSegment(Vec3f* from, Vec3f* dir, Vec3f* a, Vec3f* b, Vec3f* onRay, Vec3f* onEdge)
{
	f32 ax    = a->x;
	f32 az    = a->z;
	f32 bx    = b->x;
	f32 bz    = b->z;
	f32 fx    = from->x;
	f32 fz    = from->z;
	f32 dx    = dir->x;
	f32 dz    = dir->z;
	f32 edgeX = bx - ax;
	f32 edgeZ = bz - az;
	f32 denom = edgeX * dz - edgeZ * dx;
	if (denom > 0.0f) {
		f32 t = (dx * (az - fz) - dz * (ax - fx)) / denom;
		if (t >= 0.0f && t <= 1.0f) {
			f32 u = (edgeX * (fz - az) - edgeZ * (fx - ax)) / (dx * edgeZ - dz * edgeX);
			if (u >= 0.0f && u <= 1.0f) {
				vecScaleAdd(u, onRay, dir, from);
				vecSub(onEdge, b, a);
				vecScaleAdd(t, onEdge, onEdge, a);
				return 1;
			}
		}
	}
	return 0;
}

// 0x80039960
// Grabs the edge of the ground in the stick's direction if there is a drop beyond it
extern "C" int plyTryGrabEdge(TChrPlayer* player, f32 amount, int angle)
{
	Vec3f normal;
	Vec3f from;
	Vec3f to;
	Vec3f dir;
	Vec3f a;
	Vec3f b;
	Vec3f onRay;
	Vec3f onEdge;
	u16 tri;

	if (amount < 0.1f || amount > 0.7f) {
		return 0;
	}
	if (player->_340.z > gPlyParams->mRunSpeed) {
		return 0;
	}
	MeshColli* mesh = (MeshColli*)plyGetProbe(player, 5, &tri, 0, &normal);
	if (mesh == 0) {
		return 0;
	}
	if (normal.y < 0.9f) {
		return 0;
	}
	vecFromAngleXZ(0.5f, &dir, angle);
	plyProjectOnPlane(&dir, &normal, &dir);
	if (mesh->mFlags & 2) {
		mtxMultVec(mesh->mMatrix, &player->_494, &from);
		from.y += 0.0001f;
	} else {
		from.x = player->mPos.x;
		from.y = 0.0001f + player->mPos.y;
		from.z = player->mPos.z;
	}
	vecAdd(&to, &from, &dir);
	EggHits hits;
	if (meshColliProbeLine(hits.mHits, &from, &to, 1) != 0) {
		return 0;
	}
	if (meshColliProbe(hits.mHits, &to, 1) != 0 && *(f32*)hits.mHits[0]._3C < 5.0f) {
		return 0;
	}
	int edge;
	for (edge = 0; edge < 3; edge++) {
		meshTriEdge(mesh, tri, (u16)edge, &a, &b);
		if (plyRayCrossSegment(&from, &dir, &a, &b, &onRay, &onEdge) != 0) {
			break;
		}
	}
	if (edge == 3) {
		return 0;
	}
	vecSub(&dir, &a, &b);
	njOuterProduct(&dir, &normal, &dir);
	plySetAngle(player, vecAngleXZ(&dir));
	riderSetMesh((Rider*)&player->mRiders[7], (RiderList*)mesh);
	mtxMultVec(mesh->mInverse, &onRay, &player->mRiders[7].mPos);
	mtxMultVecSR(mesh->mInverse, &dir, &player->_4B8);
	plyClearProbe(player, 5);
	return 1;
}

// 0x80039BC0
extern "C" int plyStopIfGrounded(TChrPlayer* player)
{
	if (plyOnGround(player) != 0) {
		player->mVel.x = 0.0f;
		player->mVel.y = 0.0f;
		player->mVel.z = 0.0f;
		return 1;
	}
	return 0;
}

struct PlyDamageSearch {
	TChrPlayer* mPlayer; // _00
	TObjAction* mFrom;   // _04
	int mDamage;         // _08
	int mHurt;           // _0C
	FlagWord mFlags;     // _10
	Vec3f mPos;          // _14
};

// 0x80039C10
// Asks what touches the body collider what it does
extern "C" void plyDamageSearchHit(ColliContact* contact, PlyDamageSearch* search, int* stop)
{
	if (contact->mShape == 0 && (colliContactShape(contact)->mFlags & 8) != 0) {
		TObjAction* obj = findObjById(colliContactOwner(contact));
		if (obj != 0) {
			HitInfo hit;
			if (obj->onHit(search->mPlayer, colliContactTag(contact), (int*)&hit) != 0) {
				search->mPos = contact->mPos;
				if (hit.mFlags.testBit(0xE) != 0) {
					search->mFrom = obj;
					flagWordSetBit(&search->mFlags, 0xE);
					*stop = 1;
				} else if (search->mHurt == 0) {
					search->mFrom   = obj;
					search->mDamage = hit.mDamage;
					search->mHurt   = 1;
				}
			}
		}
	}
}

// 0x80039D14
// Takes damage from what touches the player
extern "C" int plySearchDamage(TChrPlayer* player)
{
	PlyDamageSearch search;
	search.mPlayer = player;
	search.mFrom   = 0;
	search.mHurt   = player->mTimerD6 != 0;
	forEachContact((CharaColli*)player->mEmitter0, (ContactCallback)plyDamageSearchHit, &search);
	if (search.mFrom == 0) {
		return 0;
	}

	PlyMsgRaw told;
	told.mType = 0x2A;
	told.mFrom = player->mObjId;
	search.mFrom->sendToHandle((ObjMsg*)&told);

	if (flagTest(&player->mBitsDC, 5) != 0 || flagTest(&player->mBitsDC, 0xE) != 0) {
		PlyEffect* effect = plyFindEffect(player, 2, 5);
		if (effect != 0) {
			effectListRemove(player->effects(), effect);
		}
		effect = plyFindEffect(player, 2, 6);
		if (effect != 0) {
			effectListRemove(player->effects(), effect);
		}
	}
	if (flagTest(&player->mBitsDC, 3) != 0) {
		return 0;
	}
	u16 holder = search.mFrom->mHolderId;
	if ((u16)(s16)holder != 0xFFFF) {
		player->mTargetId = holder;
	}
	if (flagTest(&search.mFlags, 0xE) != 0) {
		((PlySlots*)player)->s70(0);
	} else if (flagTest(&player->mBitsDC, 2) != 0) {
		((PlySlots*)player)->s74(-search.mDamage / 2);
	} else {
		((PlySlots*)player)->s74(-search.mDamage);
	}

	Vec3f vel;
	Vec3f away;
	vecSub(&away, &player->mPos, &search.mPos);
	vecFromAngleXZ(60.0f, &vel, vecAngleXZ(&away));
	player->mVel.x = vel.x;
	player->mVel.y = 100.0f;
	player->mVel.z = vel.z;
	plySetAngle(player, vecAngleXZ(&away) + 0x8000);
	if ((int)player->mEggRef.getId() != 0xFFFF) {
		plyReleaseEgg(player, player->mEggRef.get());
	}
	plyTintPulse((PlyTint*)player->_1A8);
	player->mTimerD6 = gPlyParams->_04;
	plyPlayVoice(player, 0xA9, 0);
	plyRumble(player, 0xE);
	struct {
		u16 mType;
		u16 _02;
		u16 mPlayer;
		u16 _06;
		int mHealth;
	} msg;
	msg.mType   = 0xB2;
	msg.mPlayer = player->mObjId;
	msg.mHealth = ((PlySlots*)player)->s6C();
	aeSendMessage(&msg);
	return 1;
}

// 0x80039FC8
// Stops and lets go of the egg if there is no good ground to roll it on
extern "C" int plyCheckEggLost(TChrPlayer* player)
{
	Vec3f normal;
	TObjAction* egg = plyGetEgg(player);
	if (egg == 0) {
		player->mVel.x = 0.0f;
		player->mVel.y = 0.0f;
		player->mVel.z = 0.0f;
		return 1;
	}
	if (plyGetProbe(player, 5, 0, 0, &normal) == 0) {
		player->mVel.x = 0.0f;
		player->mVel.y = 0.0f;
		player->mVel.z = 0.0f;
		plyReleaseEgg(player, egg);
		return 1;
	}
	if (normal.y < 0.5f) {
		player->mVel.x = 0.0f;
		player->mVel.y = 0.0f;
		player->mVel.z = 0.0f;
		plyReleaseEgg(player, egg);
		return 1;
	}
	if (plyGetProbe(player, 0, 0, 0, 0) != 0) {
		player->mVel.x = 0.0f;
		player->mVel.y = 0.0f;
		player->mVel.z = 0.0f;
		plyReleaseEgg(player, egg);
		return 1;
	}
	return 0;
}

// 0x8003A0D4
extern "C" int plyNoLedge(TChrPlayer* player)
{
	return player->mRiders[5].mMesh == 0 ? 1 : 0;
}

// 0x8003A0E4
extern "C" int plyBounceFlag(TChrPlayer* player)
{
	return player->mPlayFlags & 0x10000000;
}

// 0x8003A0F0
// Jumps back off the egg
extern "C" int plyJumpOffEgg(TChrPlayer* player)
{
	Vec3f vel;
	if ((int)player->mEggRef.getId() != 0xFFFF) {
		plyPlayVoice(player, 0xA0, 0);
		plyRumble(player, 9);
		player->_198 = gPlyParams->_0E;
		vel.x        = 0.0f;
		vel.y        = 150.0f;
		vel.z        = -100.0f;
		vecRotateY(&vel, &player->mVel, player->mAngle);
		plyReleaseEgg(player, plyGetEgg(player));
		return 1;
	}
	return 0;
}

struct PlyEdgeSearch {
	MeshColli* mMesh; // _00
	u16 mTri;         // _04
	Vec3f mFrom;      // _08
	Vec3f mDir;       // _14
	Vec3f mHit;       // _20
	Vec3f mNormal;    // _2C
	Vec3f mA;         // _38
	Vec3f mB;         // _44
	f32 mDist;        // _50
	f32 mLow;         // _54
	f32 mHigh;        // _58
};

// 0x8003A1A0
// For each climbable triangle keeps the nearest edge the player's ray crosses in the height range
extern "C" void plyEdgeSearchHit(MeshColli* mesh, u16 tri, PlyEdgeSearch* search)
{
	Vec3f a;
	Vec3f b;
	Vec3f normal;
	Vec3f onRay;
	Vec3f onEdge;

	if ((*(u32*)((u8*)mesh->mTris + tri * 0x28 + 8) & 1) == 0) {
		return;
	}
	meshTriNormalU16(mesh, tri, &normal);
	if (normal.y < 0.9f) {
		return;
	}
	int edge;
	for (edge = 0; edge < 3; edge++) {
		meshTriEdge(mesh, tri, (u16)edge, &a, &b);
		if (plyRayCrossSegment(&search->mFrom, &search->mDir, &b, &a, &onRay, &onEdge) != 0) {
			break;
		}
	}
	if (edge == 3) {
		return;
	}
	if (onEdge.y < search->mLow || onEdge.y > search->mHigh) {
		return;
	}
	f32 dist = distXZ(&onEdge, &search->mFrom);
	if (dist >= search->mDist) {
		return;
	}
	search->mMesh   = mesh;
	search->mTri    = tri;
	search->mA      = a;
	search->mB      = b;
	search->mDist   = dist;
	search->mNormal = normal;
	search->mHit    = onEdge;
}

// 0x8003A300
// Catches a ledge while falling
extern "C" int plyTryCatchLedge(TChrPlayer* player)
{
	u8 list[0xC];
	struct {
		Vec3f mPos;
		f32 mRadius;
	} sphere;
	Vec3f dir;
	Vec3f pos;
	Vec3f from;
	Vec3f to;
	PlyEdgeSearch search;

	if (player->mVel.y > 0.0f) {
		return 0;
	}
	search.mMesh   = 0;
	search.mFrom.x = player->mPos.x;
	search.mFrom.y = 10.0f + player->mPos.y;
	search.mFrom.z = player->mPos.z;
	vecFromAngleXZ(5.0f, &search.mDir, player->mAngle);
	search.mHigh   = 10.0f + search.mFrom.y;
	search.mLow    = search.mFrom.y;
	search.mDist   = 100000.0f;
	sphere.mPos    = search.mFrom;
	sphere.mRadius = 5.0f;
	queryBegin(list);
	queryGatherBall((MeshList*)list, (const void*)&sphere);
	meshListWalk((MeshList*)list, (void*)plyEdgeSearchHit, (void*)&search);
	if (search.mMesh == 0) {
		return 0;
	}
	pos.x = search.mHit.x;
	pos.y = 0.0001f + search.mHit.y;
	pos.z = search.mHit.z;
	EggHits hits;
	plyProjectOnPlane(&dir, &search.mNormal, &search.mDir);
	vecNormalize(&dir, &dir);
	vecScaleAdd(-5.0f, &from, &dir, &pos);
	vecScaleAdd(5.0f, &to, &dir, &pos);
	if (meshListProbeLine((EggHit*)hits.mHits, (MeshList*)list, (const Vec3f*)&from, (const Vec3f*)&to, 1) != 0) {
		return 0;
	}
	vecSub(&dir, &search.mA, &search.mB);
	vecNormalize(&dir, &dir);
	vecScaleAdd(5.0f, &from, &dir, &pos);
	if (meshListProbeLine((EggHit*)hits.mHits, (MeshList*)list, (const Vec3f*)&pos, (const Vec3f*)&from, 1) != 0) {
		return 0;
	}
	vecScaleAdd(-5.0f, &to, &dir, &pos);
	if (meshListProbeLine((EggHit*)hits.mHits, (MeshList*)list, (const Vec3f*)&pos, (const Vec3f*)&to, 1) != 0) {
		return 0;
	}
	vecSub(&dir, &search.mA, &search.mB);
	njOuterProduct(&dir, &search.mNormal, &dir);
	plySetAngle(player, vecAngleXZ(&dir));
	vecScaleAdd(0.0001f, &pos, &dir, &search.mHit);
	riderSetMesh((Rider*)&player->mRiders[7], (RiderList*)search.mMesh);
	mtxMultVec(search.mMesh->mInverse, &pos, &player->mRiders[7].mPos);
	mtxMultVecSR(search.mMesh->mInverse, &dir, &player->_4B8);
	return 1;
}

// 0x8003A5C0
extern "C" int plySetHangObject(TChrPlayer* player, u16 id, u16 part)
{
	findObjById((u16)id);
	player->mHangRef.setId(id);
	player->_2A2 = part;
	return 1;
}

// 0x8003A60C
// Finds an egg within reach and within the cone in front of the player
extern "C" void plyGrabSearchHit(ColliContact* contact, PlyGrabSearch* search, int* stop)
{
	if (contact->mShape == 1 && (colliContactShape(contact)->mFlags & 4) != 0) {
		TObjAction* obj = findObjById(colliContactOwner(contact));
		if (obj != 0) {
			Vec3f dir;
			vecSub(&dir, &contact->mPos, &search->mPos);
			f32 reach = search->mReach + *(f32*)((u8*)colliContactShape(contact) + 0x18);
			if (!(vecLenXZ(&dir) > reach)) {
				PlyMsgT msg(0x40);
				msg.mFrom = search->mId;
				if (obj->sendToHandle((ObjMsg*)&msg) & 2) {
					if (angleDist(vecAngleXZ(&dir), search->mAngle) <= search->mCone) {
						search->mFound = obj;
						*stop          = 1;
					}
				}
			}
		}
	}
}

// 0x8003A718
// Picks up an egg in the way the stick points
extern "C" int plyTryPickUpInFront(TChrPlayer* player, f32 amount, int angle)
{
	if (amount == 0.0f) {
		return 0;
	}
	PlyGrabSearch search;
	search.mId    = player->mObjId;
	search.mPos   = player->mPos;
	search.mAngle = angle;
	search.mReach = 3.0f;
	search.mCone  = 0x2000;
	search.mFound = 0;
	forEachContact((CharaColli*)player->mEmitter1, (ContactCallback)plyGrabSearchHit, &search);
	if (search.mFound != 0) {
		return plyPickUpEgg(player, search.mFound);
	}
	return 0;
}

// 0x8003A7C4
// Jumps
extern "C" int plyTryJump(TChrPlayer* player, f32 amount, int angle)
{
	if (plyOnGround(player) == 0) {
		return 0;
	}
	plyLeaveGround(player);
	particleOnceById(player, &player->mPos, 0, 0);
	plyPlaySe(player, 0x78);
	plyPlayVoice(player, 0xA0, 0);
	plyRumble(player, 9);
	player->_198 = gPlyParams->_0E;
	if (amount > 0.0f) {
		plySetAngle(player, angle);
	}
	vecFromAngleXZ(amount * gPlyParams->_6C, &player->mVel, angle);
	if (flagTest(&player->mBitsD8, 1) != 0) {
		player->mVel.y = 1.5f * gPlyParams->_70;
	} else {
		player->mVel.y = gPlyParams->_70;
	}
	return 1;
}

// 0x8003A8D4
// Jumps with no checks from a ledge or a wall
extern "C" int plyForceJump(TChrPlayer* player, f32 amount, int angle)
{
	player->_198 = gPlyParams->_0E;
	if (amount > 0.0f) {
		plySetAngle(player, angle);
	}
	vecFromAngleXZ(amount * gPlyParams->_6C, &player->mVel, angle);
	player->mVel.y = gPlyParams->_70;
	plyPlayVoice(player, 0xA3, 0);
	return 1;
}

// 0x8003A968
// Lets go when walking away from the egg
extern "C" int plyTryLetGoEgg(TChrPlayer* player, f32 amount, int angle)
{
	TObjAction* egg = plyGetEgg(player);
	if (egg != 0) {
		PlyVec dir(*egg->getPos());
		vecSub(&dir, &dir, &player->mPos);
		if (amount > 0.0f && angleDist(vecAngleXZ(&dir), angle) > 0x4000) {
			plyReleaseEgg(player, egg);
			return 1;
		}
	}
	return 0;
}

// 0x8003AA28
extern "C" int plyIsMobileSlow(TChrPlayer* player)
{
	TMobileObject* mobile = plyGetMobile(player);
	if (mobile == 0) {
		return 0;
	}
	if (*(u32*)((u8*)mobile + 0x234) == 0) {
		return 0;
	}
	PlyVec vel(mobile->mBody.mVel);
	return vecLenXZ(&vel) < 1.0f;
}

// 0x8003AA9C
extern "C" int plyOnGround(TChrPlayer* player)
{
	return plyGetProbe(player, 5, 0, 0, 0) != 0;
}

// 0x8003AAD8
extern "C" int plyStopIfMounted(TChrPlayer* player)
{
	if ((int)player->mMountRef.getId() != 0xFFFF) {
		player->mVel.x = 0.0f;
		player->mVel.y = 0.0f;
		player->mVel.z = 0.0f;
		return 1;
	}
	return 0;
}

struct PlyRollSearch {
	u16 mId;            // _00
	Vec3f mPos;         // _04
	int mAngle;         // _10
	TObjAction* mFound; // _14
};

// 0x8003AB34
extern "C" void plyRollSearchHit(ColliContact* contact, PlyRollSearch* search, int* stop)
{
	if (colliContactShape(contact)->mFlags & 0x400) {
		TObjAction* obj = findObjById(colliContactOwner(contact));
		if (obj != 0) {
			Vec3f dir;
			dir.x = contact->mPos.x;
			dir.y = contact->mPos.y + obj->getRadiusV();
			dir.z = contact->mPos.z;
			vecSub(&dir, &dir, &search->mPos);
			if (vecLenXZ(&dir) < 5.0f) {
				search->mFound = obj;
				*stop          = 1;
			}
		}
	}
}

// 0x8003ABE8
// With ability 3 lands onto an egg and starts rolling it
extern "C" u16 plyTryLandOnEgg(TChrPlayer* player)
{
	if (flagTest(&player->mBitsD8, 3) == 0) {
		return 0xFFFF;
	}
	if (player->mVel.y >= 0.0f) {
		return 0xFFFF;
	}
	PlyRollSearch search;
	search.mId    = player->mObjId;
	search.mPos   = player->mPos;
	search.mAngle = player->getRotY();
	search.mFound = 0;
	forEachContact((CharaColli*)player->mEmitter0, (ContactCallback)plyRollSearchHit, &search);
	if (search.mFound == 0) {
		return 0xFFFF;
	}
	PlyMsgPick msg(0x6D);
	msg.mFrom = player->mObjId;
	if (search.mFound->sendToHandle((ObjMsg*)&msg) == 0) {
		return 0xFFFF;
	}
	player->mMobile->setHandle(msg.mHandle);
	player->mMobile->reset(msg.mRadius, &msg.mPos, &msg.mRot);
	player->mMobile->startRoll();
	plyHoldEgg(player, search.mFound);
	return search.mFound->mObjId;
}

// 0x8003AD44
extern "C" int plyTryEggJump(TChrPlayer* player, f32 amount, int angle)
{
	if (plyGetMobile(player) == 0) {
		return 0;
	}
	PlyMsgT msg(0x40);
	msg.mFrom = player->mObjId;
	if ((plyGetEgg(player)->sendToHandle((ObjMsg*)&msg) & 8) == 0) {
		return 0;
	}
	if (amount != 0.0f) {
		plyGetMobile(player)->startAir();
		player->setMove(amount, angle);
		return 1;
	}
	return 0;
}

// 0x8003AE14
extern "C" int plyTryBounce(TChrPlayer* player)
{
	TMobileObject* mobile = plyGetMobile(player);
	if (mobile != 0) {
		plyPlaySe(player, 0x85);
		if (mobile->mBody.mVel.y < 100.0f) {
			mobile->stopBrake();
			return 1;
		}
	}
	return 0;
}

// 0x8003AE78
extern "C" int plyTryRollDash(TChrPlayer* player)
{
	u16 tri;
	TMobileObject* mobile = plyGetMobile(player);
	if (mobile == 0) {
		return 0;
	}
	MeshColli* mesh = (MeshColli*)plyGetProbe(player, 5, &tri, 0, 0);
	if (mesh == 0) {
		return 0;
	}
	if (*(u32*)((u8*)mesh->mTris + tri * 0x28 + 8) & 0x400) {
		return 0;
	}
	plyStopAim(player);
	if (mobile->tryDash() == 0) {
		return 0;
	}
	plyPlaySe(player, 0x7D);
	return 1;
}

// 0x8003AF30
extern "C" int plyTryBounceJump(TChrPlayer* player)
{
	TMobileObject* mobile = plyGetMobile(player);
	if (mobile != 0) {
		return mobile->tryUp();
	}
	return 0;
}

// 0x8003AF64
// Launches the egg or an egg within reach wider on a strong push
extern "C" int plyTryLaunchEgg(TChrPlayer* player, f32 amount, int angle)
{
	TMobileObject* mobile = plyGetMobile(player);
	if (mobile != 0) {
		plyStopAim(player);
		return mobile->launch(amount, angle);
	}
	TObjAction* egg;
	if (amount > 0.5f) {
		egg = plyFindEggInReach(player, 8.0f, 0x3800);
	} else {
		egg = plyFindEggInReach(player, 4.0f, 0x2000);
	}
	if (egg != 0 && plyAttachEgg(player, egg) != 0) {
		if (player->mMobile->launch(amount, angle) != 0) {
			plyHoldEgg(player, egg);
			return 1;
		}
		return 0;
	}
	return 0;
}

// 0x8003B058
extern "C" int plyTryThrow(TChrPlayer* player)
{
	TMobileObject* mobile = plyGetMobile(player);
	if (mobile != 0) {
		mobile->stopBrake();
		return 1;
	}
	return 0;
}

// 0x8003B090
extern "C" int plyTrySpinStand(TChrPlayer* player, f32 amount)
{
	TMobileObject* mobile = plyGetMobile(player);
	if (mobile != 0) {
		if (amount > 0.0f) {
			PlyVec vel(mobile->mBody.mVel);
			if (vecLenXZ(&vel) < 1.0f) {
				return mobile->launch(0.0f, 0);
			}
		} else {
			return mobile->launch(0.0f, 0);
		}
	}
	return 0;
}

// 0x8003B130
// Bit 0x400 of the triangle underfoot
extern "C" int plyOnSlipperyGround(TChrPlayer* player)
{
	u16 tri;
	MeshColli* mesh = (MeshColli*)plyGetProbe(player, 5, &tri, 0, 0);
	if (mesh == 0) {
		return 0;
	}
	return (*(u32*)((u8*)mesh->mTris + tri * 0x28 + 8) >> 10) & 1;
}

// 0x8003B188
// Sliding down a slope that is too steep
extern "C" int plyOnSteepGround(TChrPlayer* player)
{
	Vec3f normal;
	if (plyGetProbe(player, 5, 0, 0, &normal) == 0) {
		return 0;
	}
	if (normal.y < gPlyParams->mSlipNormal && player->mVel.y < 0.0f) {
		return 1;
	}
	return 0;
}

// 0x8003B204
// Starts talking to what the speak cursor is on
extern "C" int plyTryTalk(TChrPlayer* player)
{
	TObjAction* obj = ((ObjRef*)player->_2BC)->get();
	if (obj == 0) {
		return 0;
	}
	PlyMsg msg(0x70);
	msg.mFrom = player->mObjId;
	if (obj->sendToHandle((ObjMsg*)&msg) == 0) {
		return 0;
	}
	Vec3f pos = *obj->getPos();
	plySetAngle(player, angleToXZ(&player->mPos, &pos));
	player->updateMatrix();
	return 1;
}

// 0x8003B2D4
extern "C" int plyTrySpinDash(TChrPlayer* player, f32 amount, int angle)
{
	Vec3f dir;
	TMobileObject* mobile = plyGetMobile(player);
	if (mobile != 0) {
		if (amount == 0.0f) {
			vecFromAngleXZ(1.0f, &dir, player->getRotY());
		} else {
			vecFromAngleXZ(1.0f, &dir, angle);
		}
		return mobile->startThrown(&dir);
	}
	return 0;
}

// 0x8003B378
extern "C" int plyTryHop(TChrPlayer* player)
{
	int landed            = 0;
	TMobileObject* mobile = plyGetMobile(player);
	if (mobile != 0) {
		landed = mobile->tryLand();
		if (landed != 0) {
			plyPlayVoice(player, 0xA7, 0);
			plyPlaySe(player, 0x78);
		}
	}
	return landed;
}

// 0x8003B3DC
extern "C" int plyMobileUnderTen(TChrPlayer* player)
{
	TMobileObject* mobile = plyGetMobile(player);
	if (mobile == 0) {
		return 0;
	}
	if (plyGetMove(player, 0, 0) == 0) {
		PlyVec vel(mobile->mBody.mVel);
		return vecLenXZ(&vel) < 10.0f;
	}
	return 0;
}

// 0x8003B468
extern "C" int plyMobileStopped(TChrPlayer* player)
{
	TMobileObject* mobile = plyGetMobile(player);
	if (mobile == 0) {
		return 0;
	}
	if (plyGetMove(player, 0, 0) == 0) {
		PlyVec vel(mobile->mBody.mVel);
		if (vecLenXZ(&vel) < 10.0f) {
			return 1;
		}
	}
	return 0;
}

// 0x8003B4F4
// The Y button uses what is worn unless its spec says bit 0x20
extern "C" int plyUseWornItem(TChrPlayer* player)
{
	bool has = ((ItemHolder*)player->_4F0)->hasItem();
	if (!has) {
		return 0;
	}
	if (findItemSpec((has ? ((ItemHolder*)player->_4F0)->mEntry->mData.mKind : 0xFFFF))->mFlags & 0x20) {
		return 0;
	}
	TObject* item = giveItem(player->mObjId, ((ItemHolder*)player->_4F0)->firstId());
	if (item != 0) {
		((PlyItemSlots*)item)->s6C();
	}
	itemHolderRelease((ItemHolder*)player->_4F0);
	struct {
		u16 mType;
		u16 _02;
		u16 mPlayer;
		u16 _06;
	} msg;
	msg.mType   = 0xB6;
	msg.mPlayer = player->mObjId;
	aeSendMessage(&msg);
	return 1;
}

// 0x8003B5CC
extern "C" int plySetMoveIfPushed(TChrPlayer* player, f32 amount, int angle)
{
	if (amount > gPlyParams->_3C) {
		player->setMove(amount, angle);
		return 1;
	}
	return 0;
}

// 0x8003B608
extern "C" int plySetMoveIfPushed2(TChrPlayer* player, f32 amount, int angle)
{
	if (amount > gPlyParams->_3C) {
		player->setMove(amount, angle);
		return 1;
	}
	return 0;
}

// 0x8003B644
// The stick is pushed and not against the facing
extern "C" int plyTryEndBrake(TChrPlayer* player, f32 amount, int angle)
{
	if (amount > gPlyParams->_3C && angleDist(angle, player->mAngle) < 0x4000) {
		player->setMove(amount, angle);
		return 1;
	}
	return 0;
}

// 0x8003B6C4
// What the stick asks of a walking player 1 stop 2 turn sharply 4 turn back at speed
extern "C" int plyGetTurnKind(TChrPlayer* player)
{
	f32 amount;
	int angle;
	PlyParams* params = gPlyParams;
	if (plyGetMove(player, &amount, &angle) == 0) {
		if (player->_340.z <= 0.0001f) {
			return 1;
		}
	} else if (angleDist(angle, player->mAngle) >= params->_28) {
		if (amount >= params->mSteepSlope) {
			return 4;
		}
		return 2;
	}
	return 0;
}

struct PlyTalkSearch {
	u16 mId;            // _00
	TObjAction* mFound; // _04
	Vec3f mPos;         // _08
};

// 0x8003B760
// Finds what can be talked to by message 0x40 bit 0x40
extern "C" void plyTalkSearchHit(ColliContact* contact, PlyTalkSearch* search, int* stop)
{
	if (contact->mShape == 0 && (colliContactShape(contact)->mFlags & 0x200) != 0) {
		TObjAction* obj = findObjById(colliContactOwner(contact));
		if (obj != 0) {
			PlyMsgT msg(0x40);
			msg.mFrom = search->mId;
			if (obj->sendToHandle((ObjMsg*)&msg) & 0x40) {
				search->mFound = obj;
				search->mPos   = contact->mPos;
				*stop          = 1;
			}
		}
	}
}

// 0x8003B820
// Keeps the speak cursor on what can be talked to at the height message 0x8A gives
extern "C" void plyUpdateSpeakCursor(TChrPlayer* player)
{
	PlyTalkSearch search;
	search.mId    = player->mObjId;
	search.mFound = 0;
	forEachContact((CharaColli*)player->mEmitter1, (ContactCallback)plyTalkSearchHit, &search);
	if (search.mFound != 0) {
		struct {
			u16 mType;
			f32 mHeight;
		} msg;
		msg.mType = 0x8A;
		((ObjRef*)player->_2BC)->setId(search.mFound->mObjId);
		player->_2C8->hide();
		f32 height;
		if (search.mFound->sendToHandle((ObjMsg*)&msg) != 0) {
			height = msg.mHeight;
		} else {
			height = 10.0f;
		}
		player->_2C8->place(&search.mPos, height);
		player->mPlayFlags |= 0x2000000;
	}
}

struct PlySlamSearch {
	PlyMsgRaw mAsk; // _00
	u8 _06[2];
	u16 mFront; // _08
	u8 _0A[2];
	Vec3f mDir; // _0C
	u16 mFound; // _18
	u8 _1A[2];
	int mAngle; // _1C
};

// 0x8003B8D8
// Finds an egg to pick up from behind
extern "C" void plySlamSearchHit(ColliContact* contact, PlySlamSearch* search, int* stop)
{
	if ((colliContactShape(contact)->mFlags & 0x20) != 0 && contact->mShape == 2) {
		int owner = colliContactOwner(contact);
		if (sendMsgById(owner, (ObjMsg*)&search->mAsk) & 2) {
			if (sendMsgById(owner, (ObjMsg*)&search->mFront) != 0 && angleDist(vecAngleXZ(&search->mDir), search->mAngle) >= 0x6000) {
				search->mFound = owner;
				*stop          = 1;
			}
		}
	}
}

// 0x8003B978
// Returns the id of an egg behind the player or 0xFFFF
extern "C" u16 plyFindEggBehind(TChrPlayer* player)
{
	PlySlamSearch search;
	search.mAsk.mType = 0x40;
	search.mAsk.mFrom = 0xFFFF;
	search.mFront     = 0x43;
	search.mFound     = 0xFFFF;
	search.mAngle     = player->mAngle;
	forEachContact((CharaColli*)player->mEmitter1, (ContactCallback)plySlamSearchHit, &search);
	if (search.mFound == 0xFFFF) {
		return 0xFFFF;
	}
	TObjAction* obj = findObjById(search.mFound);
	if (obj == 0) {
		return 0xFFFF;
	}
	if (obj->_00 == (u32)gTObjLastBossShotName && flagTest(&player->mBitsDC, 0xD) == 0) {
		return 0xFFFF;
	}
	PlyMsg msg(0x65);
	msg.mFrom = player->mObjId;
	if (obj->sendToHandle((ObjMsg*)&msg) == 0) {
		return 0xFFFF;
	}
	return search.mFound;
}

// 0x8003BA88
extern "C" void plyHandSearchHit(ColliContact* contact, PlyMsg* msg, int* stop)
{
	if (contact->mShape == 1 && (colliContactShape(contact)->mFlags & 4) != 0) {
		if (sendMsgById(colliContactOwner(contact), (ObjMsg*)msg) != 0) {
			msg->mTo = colliContactOwner(contact);
			*stop    = 1;
		}
	}
}

// 0x8003BB00
// Gives message 0x67 to what is in reach and forgets the egg
extern "C" u16 plyGiveEgg(TChrPlayer* player)
{
	PlyMsg msg(0x67);
	msg.mFrom = player->mObjId;
	msg.mTo   = 0xFFFF;
	forEachContact((CharaColli*)player->mEmitter1, (ContactCallback)plyHandSearchHit, &msg);
	player->mEggRef.setId(msg.mTo);
	return msg.mTo;
}

// 0x8003BB6C
extern "C" void plyUpdateMatrixPose(TChrPlayer* player)
{
	player->updateMatrix();
	player->updatePose();
}

// 0x8003BBA0
// Turns a vector about a normal as far as the angle approach lets it go
extern "C" void plyTurnAboutNormal(Vec3f* out, Vec3f* vec, Vec3f* normal, int target, int rate)
{
	f32 rot[4];
	f32 mtx[12];

	int angle = vecAngleXZ(vec);
	njCreateQuaternionV((const Vec3f*)normal, angleDiff(angle, angleApproach(angle, target, rate)), (NJS_QUATERNION*)rot);
	mtxFromQuat(mtx, rot);
	mtxMultVecSR(mtx, vec, out);
}

// 0x8003BC28
// Sliding on an egg gravity follows the ground the stick steers and friction depends on the ground
extern "C" void plySlideOnEgg(TChrPlayer* player)
{
	Vec3f normal;
	Vec3f gravity;
	Vec3f along;
	Vec3f side;
	struct {
		int x;
		int y;
		int z; // leans into the turn
	} angles;
	f32 amount;
	int stick;
	u16 tri;

	MeshColli* mesh = (MeshColli*)plyGetProbe(player, 5, &tri, 0, &normal);
	if (mesh == 0) {
		return;
	}
	MeshTri* face = &mesh->mTris[tri];
	gravity.y     = gPlyParams->_7C;
	gravity.x     = 0.0f;
	gravity.z     = 0.0f;
	plyProjectOnPlane(&player->_2F8, &normal, &gravity);
	int heading    = vecAngleXZ(&player->mVel);
	player->mAngle = (int)plyApproachAngle(2.0f, player->mAngle, heading);
	angles.x       = 0;
	angles.y       = player->mAngle;
	angles.z       = 0;
	plyProjectOnPlane(&along, &normal, &player->mVel);
	f32 friction;
	if (plyGetMove(player, &amount, &stick) != 0) {
		int diff = angleDist(stick, heading);
		if (diff >= 0xE38 && diff <= 0x71C7) {
			int to;
			if (angleDiff(heading, stick) < 0) {
				to       = player->mAngle - 0x1000;
				angles.z = (int)(4096.0f * amount);
			} else {
				to       = player->mAngle + 0x1000;
				angles.z = (int)(-4096.0f * amount);
			}
			vecFromAngleXZ(1.0f, &side, to);
			plyProjectOnPlane(&side, &normal, &side);
			f32 scale;
			if (side.y > 0.0001f) {
				scale = 0.1f;
			} else if (side.y < -0.1f) {
				scale = 1.0f;
			} else {
				scale = 0.7f;
			}
			to = angleApproach(heading, to, (int)(512.0f * (amount * scale)));
			plyTurnAboutNormal(&player->mVel, &player->mVel, &normal, to, 0x200);
		}
		if (face->mMask & 0x400) {
			if (diff <= 0x2000) {
				friction = gPlyParams->_A0;
			} else if (diff >= 0x6AAA) {
				friction = gPlyParams->_9C;
			} else {
				friction = gPlyParams->_98;
			}
		} else {
			friction = gPlyParams->_A4;
		}
	} else if (face->mMask & 0x400) {
		friction = gPlyParams->_98;
	} else {
		friction = gPlyParams->_A4;
	}
	vecScaleAdd(-friction / (1.0f / 60.0f), &player->_2F8, &along, &player->_2F8);
	njZXYAngleToQuaternion((const s32*)&angles, (NJS_QUATERNION*)&player->mRot);
}

// 0x8003BEB0
// Sliding down a steep slope
extern "C" void plySlideDownSlope(TChrPlayer* player)
{
	Vec3f normal;
	Vec3f gravity;
	Vec3f along;

	if (plyGetProbe(player, 5, 0, 0, &normal) != 0) {
		gravity.set(0.0f, 2.0f * gPlyParams->_7C, 0.0f);
		plyProjectOnPlane(&player->_2F8, &normal, &gravity);
		plySetAngle(player, (int)plyApproachAngle(2.0f, player->mAngle, vecAngleXZ(&player->mVel)));
		plyProjectOnPlane(&along, &normal, &player->mVel);
		f32 friction;
		if (normal.y >= 0.85f) {
			friction = 2.0f * gPlyParams->_B0;
		} else if (normal.y >= gPlyParams->mSlipNormal) {
			friction = gPlyParams->_B0;
		} else {
			friction = gPlyParams->_AC;
		}
		vecScaleAdd(-friction / (1.0f / 60.0f), &player->_2F8, &along, &player->_2F8);
	}
}

// 0x8003BFC4
// Pushing the egg along the ground from behind it
extern "C" void plyRollMove(TChrPlayer* player)
{
	u16 tri;
	f32 amount;
	int angle;
	Vec3f to;
	Vec3f normal;
	PlyParams* params = gPlyParams;

	MeshColli* mesh = (MeshColli*)plyGetProbe(player, 5, &tri, 0, &normal);
	if (plyGetMove(player, &amount, &angle) == 0) {
		player->_340.z = 0.0f;
	} else if (mesh != 0 && (*(u32*)((u8*)mesh->mTris + tri * 0x28 + 8) & 0x400) != 0) {
		player->_340.z = plyApproach(0.5f * params->mAccel, player->_340.z, 0.0f);
	} else {
		f32 top = 120.0f;
		if (flagTest(&player->mBitsDC, 7) != 0) {
			top *= 1.5f;
		}
		player->_340.z = plyApproach(0.5f * params->mAccel, player->_340.z, top);
	}
	if (plyGetMove(player, &amount, &angle) != 0) {
		player->mMobile->moveAlongGround(player->_340.z, angle);
	}
	PlyVec centre(player->mMobile->mBody.mPos);
	plySetAngle(player, angleToXZ(&player->mPos, &centre));
	int back = angleToXZ(&player->mPos, &centre);
	vecFromAngleXZ(4.0f + player->mMobile->mBody.mRadius, &to, back + 0x8000);
	vecAdd(&to, &to, &centre);
	vecSub(&to, &to, &player->mPos);
	f32 len = vecLenXZ(&to);
	vecFromAngleXZ(len / (1.0f / 60.0f), &to, vecAngleXZ(&to));
	if (mesh != 0) {
		plyProjectOnPlane(&player->mVel, &normal, &to);
	} else {
		vecAdd(&player->mVel, &to, &player->mVel);
	}
	plyMoveAndStep(player);
}

// 0x8003C1C0
// The same at the dash's top speed
extern "C" void plyDashMove(TChrPlayer* player)
{
	f32 amount;
	int angle;
	Vec3f to;
	Vec3f normal;
	PlyParams* params = gPlyParams;

	MeshColli* mesh = (MeshColli*)plyGetProbe(player, 5, 0, 0, &normal);
	if (mesh != 0) {
		if (plyGetMove(player, &amount, &angle) != 0) {
			f32 top = params->_C4;
			if (flagTest(&player->mBitsDC, 7) != 0) {
				top *= 1.5f;
			}
			player->_340.z = plyApproach(params->mAccel, player->_340.z, top);
		} else {
			player->_340.z = 0.0f;
		}
		if (plyGetMove(player, &amount, &angle) != 0) {
			player->mMobile->moveAlongGround(player->_340.z, angle);
		}
		PlyVec centre(player->mMobile->mBody.mPos);
		plySetAngle(player, angleToXZ(&player->mPos, &centre));
		int back = angleToXZ(&player->mPos, &centre);
		vecFromAngleXZ(4.0f + player->mMobile->mBody.mRadius, &to, back + 0x8000);
		vecAdd(&to, &to, &centre);
		vecSub(&to, &to, &player->mPos);
		f32 len = vecLenXZ(&to);
		vecFromAngleXZ(len / (1.0f / 60.0f), &to, vecAngleXZ(&to));
		if (mesh != 0) {
			plyProjectOnPlane(&player->mVel, &normal, &to);
		} else {
			vecAdd(&player->mVel, &to, &player->mVel);
		}
		plyMoveAndStep(player);
	}
}

// 0x8003C374
extern "C" void plyJumpMove(TChrPlayer* player)
{
	plyFollowMobile(player);
}

// 0x8003C394
// Lays a velocity on the ground plane keeping its speed
extern "C" void plyLayOnGround(Vec3f* out, Vec3f* normal, Vec3f* vel)
{
	Vec3f dir;
	f32 speed = vecNormalize(&dir, vel);
	plyProjectOnPlane(&dir, normal, &dir);
	vecNormalize(&dir, &dir);
	vecScale(speed, out, &dir);
}

// 0x8003C404
// Walking
extern "C" void plyWalkMove(TChrPlayer* player)
{
	f32 amount;
	int angle;
	Vec3f vel;
	Vec3f normal;
	Vec3f gravity;
	PlyParams* params = gPlyParams;

	MeshColli* mesh = (MeshColli*)plyGetProbe(player, 5, 0, 0, &normal);
	if (mesh == 0) {
		return;
	}
	if (plyGetMove(player, &amount, &angle) != 0) {
		f32 top;
		if (player->_340.z < params->mRunSpeed) {
			if (amount < params->mSteepSlope) {
				top = params->mRunSpeed;
			} else {
				top = params->_5C;
			}
		} else {
			top = params->_5C;
		}
		if (flagTest(&player->mBitsDC, 7) != 0) {
			top *= 1.5f;
		}
		if (normal.y < 0.5f) {
			player->_340.z = 0.0f;
		} else if (normal.y < params->mSlipNormal) {
			player->_340.z = vecLength(&player->mVel);
		} else {
			player->_340.z = plyApproach(params->mAccel, player->_340.z, top);
		}
		plyAddGroundMotion(player, &player->_23C);
		plySetAngle(player, (int)plyApproachAngle(60.0f * params->mTurnSpeed, player->mAngle, angle));
	} else {
		player->_340.z = 0.0f;
		plyAddGroundMotion(player, &player->_23C);
		if (mesh->mFlags & 2) {
			mtxMultVecSR(mesh->mMatrix, &player->_4A0, &vel);
			plySetAngle(player, vecAngleXZ(&vel));
		}
	}
	vecFromAngleXZ(player->_340.z, &vel, player->mAngle);
	plyLayOnGround(&player->mVel, &normal, &vel);
	if (normal.y < params->mSlipNormal) {
		gravity.set(0.0f, gPlyParams->_7C, 0.0f);
		plyProjectOnPlane(&player->_2F8, &normal, &gravity);
		vecScaleAdd(1.0f / 60.0f, &player->mVel, &player->_2F8, &player->mVel);
	}
}

// 0x8003C5FC
extern "C" void plyMoveAndStep(TChrPlayer* player)
{
	plyCollectPushes(player);
	plyPushFromWalls(player);
	plyMoveByVel(player);
	plyFindGroundWalking(player);
	player->updateMatrix();
}

struct PlyMoveHit {
	void* mMesh; // _00
	u16 mTri;    // _04
	Vec3f mPos;  // _08
	u32 mFlags;  // _14
	f32 mDist;   // _18
	Vec3f mPos2; // _1C
	u32 mFlags2; // _28
	f32 mDist2;  // _2C
};

// 0x8003C648
// Keeps the nearest wall and apart from that the nearest of anything
extern "C" int plyMoveRayHit(f32 dist, MeshColli* mesh, u16 tri, const Vec3f* pos, PlyMoveHit* hit)
{
	MeshTri* face = &mesh->mTris[tri];
	if (((face->mMask & 1) != 0) && (hit->mDist > dist)) {
		hit->mMesh  = mesh;
		hit->mTri   = tri;
		hit->mPos   = *pos;
		hit->mFlags = face->mMask;
		hit->mDist  = dist;
	}
	if (hit->mDist2 > dist) {
		hit->mDist2  = dist;
		hit->mPos2   = *pos;
		hit->mFlags2 = face->mMask;
	}
	return 1;
}

// 0x8003C6D4
// Moves the player by a frame of velocity stopping short of walls and noting deadly ground
extern "C" void plyMoveByVel(TChrPlayer* player)
{
	Vec3f from;
	Vec3f to;
	Vec3f vel;
	Vec3f dir;
	PlyMoveHit hit;

	from.x = player->mPos.x;
	from.y = 8.0f + player->mPos.y;
	from.z = player->mPos.z;
	vel    = player->mVel;
	vecScaleAdd(1.0f / 60.0f, &to, &vel, &from);
	vecAdd(&to, &player->_23C, &to);
	vecAdd(&to, &player->_248, &to);
	hit.mMesh   = 0;
	hit.mDist2  = 100000.0f;
	hit.mDist   = 100000.0f;
	hit.mFlags2 = 0;
	if (groundQueryLine((const void*)&from, (const void*)&to, (void*)0x810D, (void*)plyMoveRayHit, (void*)&hit) != 0) {
		if (hit.mMesh != 0) {
			vecSub(&dir, &to, &from);
			vecNormalize(&dir, &dir);
			vecScaleAdd(hit.mDist - 0.01f, &to, &dir, &from);
		}
		if (hit.mFlags2 & 0x810C) {
			player->mStartPos = hit.mPos2;
			player->mPlayFlags |= 0x80000000;
			if (hit.mFlags2 & 4) {
				player->mPlayFlags |= 0x4000000;
			}
			if (hit.mFlags2 & 0x100) {
				player->mPlayFlags |= 0x8000000;
			}
			if (hit.mFlags2 & 8) {
				player->mPlayFlags |= 0x20000000;
			}
			if (hit.mFlags2 & 0x8000) {
				player->mPlayFlags |= 0x40000000;
			}
		}
	}
	player->mPos.x = to.x;
	player->mPos.y = to.y - 8.0f;
	player->mPos.z = to.z;
}

// 0x8003C89C
// One wall probe Pushes out along the direction if it is too near
extern "C" int plyWallProbe(TChrPlayer* player, f32 radius, void* list, Vec3f* from, Vec3f* dir, Vec3f* push, PlyRider* rider)
{
	Vec3f to;
	EggHits hits;

	riderDetach((Rider*)rider);
	vecScaleAdd(0.01f + radius, &to, dir, from);
	if (meshListProbeLine((EggHit*)hits.mHits, (MeshList*)list, (const Vec3f*)from, (const Vec3f*)&to, 1) != 0) {
		if (*(f32*)hits.mHits[0]._3C < radius) {
			vecScaleAdd(-(radius - *(f32*)hits.mHits[0]._3C), push, dir, push);
		}
		riderSetMesh((Rider*)rider, (RiderList*)(*(void**)(hits.mHits[0]._3C + 8)));
		rider->mTri    = *(u16*)(hits.mHits[0]._3C + 0xC);
		rider->mPos    = hits.mHits[0].mPos;
		rider->mNormal = hits.mHits[0].mNormal;
		return 1;
	}
	return 0;
}

// 0x8003C9BC
// Pushes away from walls on four sides
extern "C" void plyPushFromWalls(TChrPlayer* player)
{
	EggHits hits; // built and never used
	f32 mtx[12];
	u8 list[0xC];
	struct {
		Vec3f mPos;
		f32 mRadius;
	} sphere;
	Vec3f from;
	Vec3f dir;
	Vec3f push;

	push.x = 0.0f;
	push.y = 0.0f;
	push.z = 0.0f;
	from.x = player->mPos.x;
	from.y = 8.0f + player->mPos.y;
	from.z = player->mPos.z;
	player->mPlayFlags &= ~0x400;
	clearFlagByte(player->_31C);
	queryBegin(list);
	sphere.mPos    = from;
	sphere.mRadius = 4.01f;
	queryGatherBall((MeshList*)list, (const void*)&sphere);
	mtxFromQuat((f32*)mtx, (const f32*)&player->mRot);

	dir.x = 0.0f;
	dir.y = 0.0f;
	dir.z = 1.0f;
	mtxMultVecSR(mtx, &dir, &dir);
	if (plyWallProbe(player, 4.0f, list, &from, &dir, &push, &player->mRiders[0]) != 0) {
		player->mPlayFlags |= 0x400;
		flagByteSet(player->_31C, 0);
	}

	dir.x = 0.0f;
	dir.y = 0.0f;
	dir.z = -1.0f;
	mtxMultVecSR(mtx, &dir, &dir);
	if (plyWallProbe(player, 4.0f, list, &from, &dir, &push, &player->mRiders[1]) != 0) {
		flagByteSet(player->_31C, 1);
	}

	dir.x = 1.0f;
	dir.y = 0.0f;
	dir.z = 0.0f;
	mtxMultVecSR(mtx, &dir, &dir);
	if (plyWallProbe(player, 4.0f, list, &from, &dir, &push, &player->mRiders[3]) != 0) {
		flagByteSet(player->_31C, 3);
	}

	dir.x = -1.0f;
	dir.y = 0.0f;
	dir.z = 0.0f;
	mtxMultVecSR(mtx, &dir, &dir);
	if (plyWallProbe(player, 4.0f, list, &from, &dir, &push, &player->mRiders[2]) != 0) {
		flagByteSet(player->_31C, 2);
	}

	player->_248 = push;
}

// Stores the ground the player now stands on tracking a moving mesh
static inline void plyStandOn(TChrPlayer* player, PlyGroundHit& hit)
{
	riderSetMesh((Rider*)&player->mRiders[5], (RiderList*)hit.mMesh);
	player->mRiders[5].mTri = hit.mTri;
	player->mRiders[5].mPos = hit.mPos;
	player->mPos.y          = hit.mPos.y;
	if (hit.mMesh->mFlags & 2) {
		mtxMultVec(hit.mMesh->mInverse, &hit.mPos, &player->_494);
		mtxCopy((f32*)(player->_1EC + 0x20), hit.mMesh->mMatrix);
		vecFromAngleXZ(1.0f, &player->_4A0, player->mAngle);
		mtxMultVecSR(hit.mMesh->mInverse, &player->_4A0, &player->_4A0);
	}
}

static inline void plyShadowOn(TChrPlayer* player, PlyGroundHit& hit)
{
	riderSetMesh((Rider*)&player->mRiders[6], (RiderList*)hit.mMesh);
	player->mRiders[6].mTri = hit.mTri;
	player->mRiders[6].mPos = hit.mAnyPos;
}

// 0x8003CC08
// Finds the ground under a falling player
extern "C" void plyFindGroundFalling(TChrPlayer* player)
{
	Vec3f from;
	Vec3f normal;
	PlyGroundHit hit;

	from.x = player->mPos.x;
	from.y = 0.0001f + (8.0f + player->mPos.y);
	from.z = player->mPos.z;
	plyClearProbe(player, 5);
	plyClearProbe(player, 6);
	hit.mMesh     = 0;
	hit.mAnyDist  = 100000.0f;
	hit.mAnyFlags = 0;
	hit.mDist     = 100000.0f;
	if (groundQueryDown((const void*)&from, (void*)0x18D, (void*)plyGroundRayHit, (void*)&hit) != 0 && hit.mMesh != 0) {
		meshTriNormal((MeshColli*)hit.mMesh, hit.mTri, &normal);
		if (player->mPos.y <= 0.0001f + hit.mPos.y && normal.y >= 0.5f) {
			plyStandOn(player, hit);
		}
		plyShadowOn(player, hit);
	}
}

// 0x8003CDA0
// Finds the ground under a walking player who steps down slopes that are not too steep
extern "C" void plyFindGroundWalking(TChrPlayer* player)
{
	Vec3f from;
	Vec3f normal;
	PlyGroundHit hit;

	from.x = player->mPos.x;
	from.y = 8.0f + player->mPos.y;
	from.z = player->mPos.z;
	plyClearProbe(player, 5);
	plyClearProbe(player, 6);
	hit.mMesh     = 0;
	hit.mAnyDist  = 100000.0f;
	hit.mAnyFlags = 0;
	hit.mDist     = 100000.0f;
	if (groundQueryDown((const void*)&from, (void*)0x18D, (void*)plyGroundRayHit, (void*)&hit) != 0 && hit.mMesh != 0) {
		meshTriNormal((MeshColli*)hit.mMesh, hit.mTri, &normal);
		f32 step;
		if (normal.y > 0.5f) {
			step = 8.0f;
		} else {
			step = 0.0f;
		}
		if (player->mPos.y - step <= 0.0001f + hit.mPos.y) {
			plyStandOn(player, hit);
		}
		plyShadowOn(player, hit);
	}
}

// 0x8003CF3C
// Looks for ground from up to 20 above for a player landing on it
extern "C" int plyFindLanding(TChrPlayer* player)
{
	Vec3f from;
	PlyGroundHit hit;

	f32 up  = 10.0f;
	f32 eps = 0.0001f;
	from.x  = player->mPos.x;
	from.y  = eps + (player->mPos.y + up);
	from.z  = player->mPos.z;
	plyClearProbe(player, 5);
	plyClearProbe(player, 6);
	hit.mMesh     = 0;
	hit.mAnyDist  = 100000.0f;
	hit.mAnyFlags = 0;
	hit.mDist     = 100000.0f;
	if (groundQueryDown((const void*)&from, (void*)0x18D, (void*)plyGroundRayHit, (void*)&hit) != 0) {
		if (hit.mMesh != 0 && player->mPos.y <= 20.0f + hit.mPos.y) {
			plyStandOn(player, hit);
		}
		if (hit.mMesh != 0) {
			plyShadowOn(player, hit);
		}
		return 1;
	}
	return 0;
}

// 0x8003D0C4
// Rumble for this player's pad
extern "C" void plyRumble(TChrPlayer* player, int strength)
{
	padVibStart((u8)player->mObjId, 0, 0x64, 0, strength);
}

// 0x8003D0FC
extern "C" f32* plyGetJoint(TChrPlayer* player, int joint)
{
	return (f32*)(player->mJoints + joint * 0x30);
}

// 0x8003D10C
// Slot _A8
void TChrPlayer::vfn_A8(int joint, f32* mtx)
{
	mtxCopy(mtx, (const f32*)(mJoints + joint * 0x30));
}

// 0x8003D13C
// Slot _AC the same for the outer model
void TChrPlayer::vfn_AC(int joint, f32* mtx) const
{
	f32* from;
	mModel->getJoint(joint, &from);
	mtxCopy(mtx, from);
}

// 0x8003D17C
extern "C" void plyLeaveGround(TChrPlayer* player)
{
	plyClearProbe(player, 5);
}

// 0x8003D1A0
// What probe N rests on with where and the normal
extern "C" void* plyGetProbe(const TChrPlayer* player, int probe, u16* tri, Vec3f* pos, Vec3f* normal)
{
	MeshColli* mesh;
	const PlyRider* rider = &player->mRiders[probe];
	mesh                  = rider->mMesh;
	if (mesh != 0) {
		if (tri != 0) {
			*tri = rider->mTri;
		}
		if (pos != 0) {
			if (mesh->mFlags & 2) {
				mtxMultVec(mesh->mMatrix, &rider->mPos, pos);
			} else {
				*pos = rider->mPos;
			}
		}
		if (normal != 0) {
			meshTriNormal(mesh, rider->mTri, normal);
		}
	}
	return mesh;
}

// 0x8003D24C
extern "C" void plyClearProbe(TChrPlayer* player, int probe)
{
	riderDetach((Rider*)&player->mRiders[probe]);
}

// 0x8003D27C
// Turns the shadow on or off
extern "C" void plySetShadow(TChrPlayer* player, int on)
{
	if (on != (s8)player->_318) {
		if (player->mShadowTask == 0) {
			if (on != 0) {
				player->mPlayFlags |= 0x1000;
			} else {
				player->mPlayFlags &= ~0x1000;
			}
		} else if (player->mFade == 0) {
			player->mFade = 10;
		}
		player->_318 = on;
	}
}

// 0x8003D2DC
// Status 1 shrinks the player and status 9 grows them
extern "C" void plyApplyScale(TChrPlayer* player, f32* mtx)
{
	if (flagTest(&player->mBitsDC, 1) != 0) {
		mtxScale(0.5f, 0.5f, 0.5f, mtx, mtx);
	} else if (flagTest(&player->mBitsDC, 9) != 0) {
		mtxScale(2.0f, 2.0f, 2.0f, mtx, mtx);
	}
}

// 0x8003D364
// Ability 2 ends a move in the air
extern "C" int plyStopInAir(TChrPlayer* player)
{
	player->_340.x = 0.0f;
	player->_340.y = 0.0f;
	player->_340.z = 0.0f;
	player->mVel.y = 0.0f;
	return 1;
}

// 0x8003D380
// Ability 0 the double jump
extern "C" int plyDoubleJump(TChrPlayer* player, PlyInput* input)
{
	player->_198 = gPlyParams->_0E;
	plyPlaySe(player, 0x78);
	plyPlayVoice(player, 0xA0, 0);
	plyRumble(player, 9);
	fxSpawnAt(1, 0, 0x66, player->mMatrix);
	if (input->mStick > 0.0f) {
		plySetAngle(player, input->mAngle);
		vecFromAngleXZ(gPlyParams->_6C * input->mStick, &player->mVel, input->mAngle);
		player->mVel.y = gPlyParams->_70;
	} else {
		player->mVel.y = gPlyParams->_70;
	}
	return 1;
}

// 0x8003D448
extern "C" void plySetMoveFromInput(TChrPlayer* player, PlyInput* input)
{
	player->mMoveAmount = input->mStick;
	player->mMoveAngle  = input->mAngle;
}

// 0x8003D45C
// Slot _90 Keeps the effect of the item in use which the status update applies each frame
extern "C" void plySetHeldEffect(TChrPlayer* player, const ItemSpec* spec)
{
	player->_D0    = 1;
	int kind       = spec->mFlags & 0xF;
	player->_D2[0] = kind;
	if (kind == 2) {
		player->_D2[1] = spec->mParam;
		return;
	}
	player->_D2[1] = spec->mParam;
}

// 0x8003D490
// Slot _94
extern "C" void plyClearHeldEffect(TChrPlayer* player)
{
	player->_D0 = 0;
}

// 0x8003D49C
// Slot _98 Applies an item's spec
extern "C" void plyApplySpec(TChrPlayer* player, const ItemSpec* spec)
{
	int kind = spec->mFlags & 0xF;
	if (kind == 0) {
		((PlySlots*)player)->s74(spec->mParam);
		plyPlaySe(player, 0x42);
		if ((int)spec->mParam <= 0xA) {
			fxSpawnAt(1, 0, 0x5F, player->mMatrix);
		} else {
			fxSpawnAt(1, 0, 0x5D, player->mMatrix);
		}
		plyTintFadeA((PlyTint*)player->_1A8);
	} else if (kind == 2 || kind == 1) {
		u8 id;
		if (kind == 2) {
			id = spec->mParam;
		} else {
			id = spec->mParam;
		}
		PlyEffect* effect;
		for (effect = player->effects()->mHead; effect != 0; effect = effect->mNext) {
			if (kind == effect->mKind && id == effect->mId) {
				break;
			}
		}
		if (effect == 0) {
			effect = effectListAdd(player->effects());
			if (effect != 0) {
				effect->mTime = (u16)(60.0f * (f32)spec->mTime);
				effect->mKind = kind;
				effect->mId   = id;
			}
		} else {
			effect->mTime = (u16)(60.0f * (f32)spec->mTime);
		}
	}
}

// 0x8003D620
// Slot _9C Adds a timed status from outside unless the player is dead
extern "C" void plyAddStatus(TChrPlayer* player, int id, u16 time)
{
	if ((player->mPlayFlags & 0x1000000) == 0) {
		PlyEffect* effect;
		for (effect = player->effects()->mHead; effect != 0; effect = effect->mNext) {
			if (effect->mKind == 2 && id == effect->mId) {
				break;
			}
		}
		if (effect == 0) {
			effect = effectListAdd(player->effects());
			if (effect != 0) {
				effect->mTime = time;
				effect->mKind = 2;
				effect->mId   = id;
			}
		} else {
			effect->mTime = time;
		}
	}
}

// 0x8003D6B4
// Slot _78 The ids of what is worn and held
extern "C" int plyGetItemIds(TChrPlayer* player, int* worn, int* held)
{
	*worn   = 0xFFFF;
	*held   = -1;
	int has = ((ItemHolder*)player->_4F0)->mNode.mList != 0;
	if (has == 0 && player->_520 == 0) {
		return 0;
	}
	if (has != 0) {
		*worn = has != 0 ? ((ItemHolder*)player->_4F0)->mEntry->mData.mKind : 0xFFFF;
	} else if (player->_520 != 0) {
		*worn = *(int*)((u8*)player->_520 + 0x34);
	}
	return 1;
}

// 0x8003D734
// Slot _7C Loses every item
extern "C" void plyLoseItems(TChrPlayer* player)
{
	if (((ItemHolder*)player->_508)->mNode.mList != 0) {
		itemHolderRelease((ItemHolder*)player->_508);
	}
	if (((ItemHolder*)player->_4F0)->mNode.mList != 0) {
		itemHolderRelease((ItemHolder*)player->_4F0);
	}
	struct {
		u16 mType;
		u16 _02;
		u16 mPlayer;
		u16 _06;
		int mId;
	} msg;
	msg.mType   = 0xB5;
	msg.mPlayer = player->mObjId;
	msg.mId     = -1;
	aeSendMessage(&msg);
	if (player->_520 != 0) {
		((PlyItemSlots*)player->_520)->s74();
		delete (TObject*)player->_520;
		player->_520 = 0;
		player->_D0  = 0;
	}
}

// One status bit and what runs when it changes
struct PlyStatusEntry {
	int mFlag;
	void (TChrPlayer::*mChange)(int on);
};

PlyStatusEntry gPlayerStatusEntries[10] = {
	{ 2, &TChrPlayer::statusFlag2 },   { 3, &TChrPlayer::statusInvincible }, { 4, &TChrPlayer::statusLowHealth },
	{ 5, &TChrPlayer::statusSleep },   { 6, &TChrPlayer::statusStop },       { 0xA, &TChrPlayer::statusBlind },
	{ 0xB, &TChrPlayer::statusDrain }, { 0xC, &TChrPlayer::statusStrength }, { 0xD, &TChrPlayer::statusSuper },
	{ 0xE, &TChrPlayer::statusDoze },
};

// 0x8003D7EC
// Rebuilds the ability and status bits from the timed effects and runs what changed
extern "C" void plyUpdateStatus(TChrPlayer* player)
{
	FlagWord before;
	PlyStatusEntry* entry;

	if (flagTest(&player->mBitsDC, 5) != 0 || flagTest(&player->mBitsDC, 0xE) != 0) {
		// Mashing A wakes the player sooner
		if (player->mInput.mPressed & 0x80) {
			PlyEffect* effect = (PlyEffect*)plyFindEffect(player, 2, 5);
			if (effect != 0 && effect->mTime > 2) {
				effect->mTime -= 2;
			}
			effect = (PlyEffect*)plyFindEffect(player, 2, 0xE);
			if (effect != 0 && effect->mTime > 2) {
				effect->mTime -= 2;
			}
		}
		player->_34C->close();
	}

	before = *(FlagWord*)&player->mBitsDC;
	clearFlagWord(&player->mBitsD8);
	clearFlagWord(&player->mBitsDC);
	if (player->effects()->mCount != 0) {
		PlyEffect* effect;
		PlyEffect* next;
		for (effect = player->effects()->mHead; effect != 0; effect = next) {
			next = effect->mNext;
			if (effect->mTime != 0) {
				if (--effect->mTime == 0) {
					effectListRemove(player->effects(), effect);
				}
			}
			switch (effect->mKind) {
			case 1:
				flagWordSetBit((FlagWord*)&player->mBitsD8, effect->mId);
				break;
			case 2:
				flagWordSetBit((FlagWord*)&player->mBitsDC, effect->mId);
				break;
			}
		}
	}
	if (player->_D0 != 0) {
		switch (player->_D2[0]) {
		case 1:
			flagWordSetBit((FlagWord*)&player->mBitsD8, player->_D2[1]);
			break;
		case 2:
			flagWordSetBit((FlagWord*)&player->mBitsDC, player->_D2[1]);
			break;
		}
	}

	flagWordXor(&before, &player->mBitsDC);
	if (before.mBits != 0) {
		for (u32 i = 0; i < 10; i++) {
			entry = &gPlayerStatusEntries[i];
			if (flagTest(&before, entry->mFlag) != 0) {
				(player->*entry->mChange)(flagTest(&player->mBitsDC, entry->mFlag));
			}
		}
	}
	flagWordOr(&player->mBitsD8, &player->mBitsE0);
	flagWordOr(&player->mBitsDC, &player->mBitsE4);

	if ((int)player->mBitsDC != 0) {
		if ((flagTest(&player->mBitsDC, 8) != 0 || flagTest(&player->mBitsDC, 0xE) != 0) && player->mAge % 300 == 0) {
			plyPlaySe(player, 0x49);
			((PlySlots*)player)->s74(5);
			plyTintFadeA((PlyTint*)player->_1A8);
		}
		if (flagTest(&player->mBitsDC, 0xB) != 0 && player->mAge % 300 == 0) {
			plyPlaySe(player, 0x43);
			((PlySlots*)player)->s74(-0xA);
			plyTintFadeB((PlyTint*)player->_1A8);
		}
	}
	if ((int)player->mEggRef.getId() != 0xFFFF) {
		plyApplyEggForm(player, player->mEggRef.get());
	}
	player->mMobile->setSpecial(flagTest(&player->mBitsD8, 0xC));
}

// 0x8003DB30
// Status 2 has nothing to start or end
void TChrPlayer::statusFlag2(int on)
{
}

// 0x8003DB34
void TChrPlayer::statusInvincible(int on)
{
	if (on != 0) {
		plyStartStatusEffect(this, (TObject*)createTPlyEffStInvincible(_4D0));
	} else {
		plyEndStatusEffect(this, 3);
	}
}

// 0x8003DB84
// Health is 10 until the status ends then what it was
void TChrPlayer::statusLowHealth(int on)
{
	if (on != 0) {
		_4D4 = ((PlySlots*)this)->s6C();
		((PlySlots*)this)->s70(0xA);
	} else {
		((PlySlots*)this)->s70(_4D4);
	}
}

// 0x8003DBF8
void TChrPlayer::statusSleep(int on)
{
	if (on != 0) {
		plyStartStatusEffect(this, (TObject*)createPlyEffStSleep(_4D0));
		_34C->close();
		plyReleaseRef(this, &mThrownEggRef);
	} else {
		plyEndStatusEffect(this, 5);
		_34C->reset();
	}
}

// 0x8003DC64
void TChrPlayer::statusStop(int on)
{
	if (on != 0) {
		plyLetGoAll(this);
		if ((int)mMountRef.getId() != 0xFFFF) {
			TEggCreature* mount = TEggCreature::cast(mMountRef.get());
			if (mount != 0 && mount->mCreatureKind == 0) {
				ObjMsg msg;
				msg.mType = 0x46;
				mount->sendToHandle(&msg);
				mMountRef.setId(0xFFFF);
			}
		}
	}
}

// 0x8003DCFC
void TChrPlayer::statusBlind(int on)
{
	if (on != 0) {
		plyStartStatusEffect(this, (TObject*)createTPlyEffStBlind(_4D0));
		blindOn((s8)mPlayer);
	} else {
		plyEndStatusEffect(this, 0xA);
		blindOff((s8)mPlayer);
	}
}

// 0x8003DD64
void TChrPlayer::statusDrain(int on)
{
	if (on != 0) {
		plyStartStatusEffect(this, (TObject*)createTPlyEffStDrain(_4D0));
	} else {
		plyEndStatusEffect(this, 0xB);
	}
}

// 0x8003DDB4
void TChrPlayer::statusStrength(int on)
{
	if (on != 0) {
		plyStartStatusEffect(this, (TObject*)createTPlyEffStStrength(_4D0));
	} else {
		plyEndStatusEffect(this, 0xC);
	}
}

// 0x8003DE04
void TChrPlayer::statusSuper(int on)
{
	if (on != 0) {
		plyStartStatusEffect(this, (TObject*)createTPlyEffStSuper(_4D0));
	} else {
		plyEndStatusEffect(this, 0xD);
	}
}

// 0x8003DE54
// Status 0xE looks like sleep but ends it by id 5
void TChrPlayer::statusDoze(int on)
{
	if (on != 0) {
		plyStartStatusEffect(this, (TObject*)createPlyEffStSleep(_4D0));
		_34C->close();
	} else {
		plyEndStatusEffect(this, 5);
		_34C->reset();
	}
}

// 0x8003DEB4
// Starts a status effect object on this player
extern "C" void plyStartStatusEffect(TChrPlayer* player, TObject* effect)
{
	if (effect != 0) {
		*(TChrPlayer**)((u8*)effect + 0x24) = player;
		((PlyItemSlots*)effect)->s20();
	}
}

// 0x8003DEF0
// Ends the status effect objects with this id
extern "C" void plyEndStatusEffect(TChrPlayer* player, int id)
{
	for (TObject* effect = player->_4D0->mChild; effect != 0; effect = effect->mNext) {
		if (id == *(int*)((u8*)effect + 0x20)) {
			effect->mFlags |= 1;
		}
	}
}

// 0x8003DF24
// Dead reports it to the game with who did it
extern "C" void plyDie(TChrPlayer* player)
{
	player->mPlayFlags |= 0x1000000;
	colliDisable(player->mEmitter0);
	((PlySlots*)player)->s70(0);
	if ((int)player->mMountRef.getId() != 0xFFFF) {
		ObjMsg msg;
		msg.mType = 0x46;
		player->mMountRef.get()->sendToHandle(&msg);
		player->mMountRef.setId(0xFFFF);
	}
	struct {
		u16 mType;
		u8 _02[2];
		u8 mPlayer;
		u8 mKiller;
	} report;
	report.mType   = 0xA8;
	report.mPlayer = player->mObjId;
	report.mKiller = player->mTargetId;
	sendGameMsg((ObjMsg*)&report);
}

// 0x8003DFE4
// Loses every item and dies
extern "C" void plyLoseAllAndDie(TChrPlayer* player)
{
	((PlySlots*)player)->s7C();
	plyDie(player);
}

// 0x8003E024
// Slot _B4 Whether the pad is read
void TChrPlayer::vfn_B4(int on)
{
	if (on != 0) {
		mPlayFlags |= 0x2000;
		return;
	}
	mPlayFlags &= ~0x2000;
}

// 0x8003E04C
// Slot _B8 Shows the player again and starts the Appear state
void TChrPlayer::resetCamera()
{
	mFlags &= ~4;
	mFlags &= ~0x10;
	mModel->mFlags &= ~0x10;
	PLY_CHANGE(this, createPlyStateAppear);
}

// 0x8003E0C8
// Slot _BC Plays the transformation with these motions from this place
void TChrPlayer::vfn_BC(const int* motions, f32* mtx)
{
	plyLetGoAll(this);
	_320 = motions[0];
	_324 = motions[1];
	_328 = motions[2];
	_32C = motions[3];
	_330 = motions[4];
	_334 = motions[5];
	mtxGetPos(mtx, &mPos);
	mtxToQuat(mtx, (f32*)&mRot);
	mtxCopy(mMatrix, mtx);
	PLY_CHANGE(this, createPlyStateTransform);
}

// 0x8003E17C
// Slot _C0 Goes into the mirror at this place
void TChrPlayer::vfn_C0(const Vec3f* pos, const EggRot* rot)
{
	plyLetGoAll(this);
	mPos = *pos;
	mRot = *rot;
	updateMatrix();
	playMotion(0xC);
	updatePose();
	PLY_CHANGE(this, createPlyStateInTheMirror);
}

// 0x8003E230
// Slot _C4 An event with its own motions
void TChrPlayer::vfn_C4(const int* motions, f32* mtx)
{
	plyLetGoAll(this);
	_320 = motions[0];
	_324 = motions[1];
	_328 = motions[2];
	_32C = motions[3];
	_330 = motions[4];
	_334 = motions[5];
	mtxGetPos(mtx, &mPos);
	mtxToQuat(mtx, (f32*)&mRot);
	mtxCopy(mMatrix, mtx);
	PLY_CHANGE(this, createPlyStateEventEx);
}

// 0x8003E2E4
// Slot _C8 the same
void TChrPlayer::vfn_C8(const int* motions, f32* mtx)
{
	plyLetGoAll(this);
	_320 = motions[0];
	_324 = motions[1];
	_328 = motions[2];
	_32C = motions[3];
	_330 = motions[4];
	_334 = motions[5];
	mtxGetPos(mtx, &mPos);
	mtxToQuat(mtx, (f32*)&mRot);
	mtxCopy(mMatrix, mtx);
	PLY_CHANGE(this, createPlyStateEventEx);
}

// 0x8003E398
// Poses every part directly at this frame of the event's motions
extern "C" void plyPoseAtFrame(TChrPlayer* player, f32 frame)
{
	u32* motion = (u32*)player->_320;
	f32 last    = motion[1] - 2;
	if (frame > last) {
		frame = last;
	}
	plyReadMotionPlain((NJS_OBJECT*)player->mBodyBuffer[0], motion, player->mPose, plyNodeAny, frame);
	player->updateJoints();
	plySetEyelidNodes(player->_34C->mOpen, &player->mPose[64], &player->mPose[62]);
	njPushMatrixEx();
	njUnitMatrix(0);
	plyBuildPose(player->mBodyBuffer, player->mPose);
	njPopMatrixEx();
	njSetMatrix((f32*)0, player->mPose[56].mMtx);
	if (player->_324 != 0) {
		gjShapeActor((GjActor*)player->mFaceBuffer, (void*)player->_324, (GjShape*)player->_328, frame);
	}
	njSetMatrix((f32*)0, player->mPose[45].mMtx);
	if (player->_330 != 0) {
		gjMotionActor(frame, (GjActor*)player->mLeftHandBuffer, (void*)player->_330);
	}
	njSetMatrix((f32*)0, player->mPose[35].mMtx);
	if (player->_32C != 0) {
		gjMotionActor(frame, (GjActor*)player->mRightHandBuffer, (void*)player->_32C);
	}
}

// 0x8003E4D8
// Keeps the nearest walkable triangle and the nearest of anything with all the flags hit
extern "C" int plyGroundRayHit(f32 dist, MeshColli* mesh, u16 tri, const Vec3f* pos, PlyGroundHit* hit)
{
	MeshTri* face = &mesh->mTris[tri];
	if (((face->mMask & 1) != 0) && (hit->mDist > dist)) {
		hit->mDist  = dist;
		hit->mMesh  = mesh;
		hit->mTri   = tri;
		hit->mPos   = *pos;
		hit->mFlags = face->mMask;
	}
	if (hit->mAnyDist > dist) {
		hit->mAnyDist  = dist;
		f32 x          = pos->x;
		f32 y          = pos->y;
		hit->mAnyPos.x = x;
		f32 z          = pos->z;
		hit->mAnyPos.y = y;
		hit->mAnyPos.z = z;
		hit->mAnyFlags |= face->mMask;
	}
	return 1;
}

// 0x8003E56C
// Keeps the nearest of anything
extern "C" int plyAnyRayHit(f32 dist, MeshColli* mesh, u16 tri, const Vec3f* pos, PlyGroundHit* hit)
{
	MeshTri* face = &mesh->mTris[tri];
	if (hit->mAnyDist > dist) {
		hit->mMesh     = mesh;
		hit->mTri      = tri;
		hit->mAnyDist  = dist;
		hit->mAnyPos.x = pos->x;
		hit->mAnyPos.y = pos->y;
		hit->mAnyPos.z = pos->z;
		hit->mAnyFlags |= face->mMask;
	}
	return 1;
}

// 0x8003E5C4
// Tells the game each frame the player's health and the state of the egg
extern "C" void plyReportToGame(TChrPlayer* player)
{
	PlyReportMsg msg;
	msg.mType   = 0xAD;
	msg.mPlayer = player->mObjId;
	msg.mValue  = ((PlySlots*)player)->s6C();
	aeSendMessage(&msg);
	int id = player->mEggRef.getId();
	if (id != 0xFFFF) {
		PlyAskEgg ask;
		ask.mType = 0x76;
		if (player->mEggRef.get()->sendToHandle((ObjMsg*)&ask) != 0) {
			PlyReportEgg egg;
			egg.mType   = 0xAF;
			egg.mPlayer = player->mObjId;
			egg.mValue  = ask.mValue;
			egg.mKind   = ask.mKind;
			aeSendMessage(&egg);
		} else {
			PlyReportFloat none;
			none.mType   = 0xB0;
			none.mPlayer = player->mObjId;
			none.mValue  = 0.0f;
			aeSendMessage(&none);
		}
	} else {
		PlyReportFloat none;
		none.mType   = 0xB0;
		none.mPlayer = player->mObjId;
		none.mValue  = 0.0f;
		aeSendMessage(&none);
	}
}

// 0x8003E6CC
// The timed effect of this kind and id
extern "C" PlyEffect* plyFindEffect(TChrPlayer* player, u8 kind, u8 id)
{
	for (PlyEffect* effect = player->effects()->mHead; effect != 0; effect = effect->mNext) {
		if (kind == effect->mKind && id == effect->mId) {
			return effect;
		}
	}
	return 0;
}

// 0x8003E708
// The mount in the fifth reference
extern "C" void* plyGetMount(TChrPlayer* player)
{
	return TEggCreature::cast(player->mMountRef.get());
}

// 0x8003E730
// Takes the place the mount gives
extern "C" void plyPlaceOnMount(TChrPlayer* player)
{
	Vec3f dir;
	void* mount = plyGetMount(player);
	if (mount != 0) {
		((PlySlots*)mount)->s94(player->mMatrix);
		plyApplyScale(player, player->mMatrix);
		plyPosFromMatrix(player);
		mtxMultVecSR(player->mMatrix, &gAxisZ, &dir);
		player->mAngle = vecAngleXZ(&dir);
	}
}

// Message 0x48 asks whether this is a mount, and the first that answers is kept
struct PlyMountMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mFrom;          // _04
	TObjAction* mFound; // _08
};

// 0x8003E7AC
// Finds a mount that takes message 0x48
extern "C" void plyMountSearchHit(ColliContact* contact, u8* msg, int* stop)
{
	if (colliContactShape(contact)->mFlags & 0x1000) {
		TObjAction* obj = findObjById(colliContactOwner(contact));
		if (obj != 0 && obj->sendToHandle((ObjMsg*)msg) != 0) {
			((PlyMountMsg*)msg)->mFound = obj;
			*stop                       = 1;
		}
	}
}

// 0x8003E82C
// Changes to a mount that is touching
extern "C" int plyTryChangeMount(TChrPlayer* player)
{
	PlyMountMsg ask;
	ask.mType  = 0x48;
	ask.mFrom  = player->mObjId;
	ask.mFound = 0;
	forEachContact((CharaColli*)player->mEmitter0, (ContactCallback)plyMountSearchHit, &ask);
	if (ask.mFound == 0) {
		return 0;
	}
	ObjMsg off;
	off.mType = 0x46;
	sendMsgByIdU16(player->mMountRef.getId(), &off);
	player->mMountRef.setId(ask.mFound->mObjId);
	return 1;
}

// 0x8003E8C0
// Gets off if the mount agrees
extern "C" int plyTryLeaveMount(TChrPlayer* player)
{
	PlyMsgRaw msg;
	msg.mType = 0x45;
	msg.mFrom = player->mObjId;
	if (sendMsgByIdU16(player->mMountRef.getId(), (ObjMsg*)&msg) == 0) {
		return 0;
	}
	player->mMountRef.setId(0xFFFF);
	return 1;
}

// 0x8003E92C
// Thrown off with this velocity
extern "C" void plyThrownOff(TChrPlayer* player, const Vec3f* vel)
{
	plyClearProbe(player, 5);
	plyPlayVoice(player, 0xA0, 0);
	plyRumble(player, 9);
	player->mVel = *vel;
	player->_198 = gPlyParams->_0E;
}

// 0x8003E9A0
// Throws what is worn in front if its spec says bit 0x20
extern "C" int plyThrowWornItem(TChrPlayer* player)
{
	Vec3f pos;
	bool has = ((ItemHolder*)player->_4F0)->hasItem();
	if (!has) {
		return 0;
	}
	if ((findItemSpec((has ? ((ItemHolder*)player->_4F0)->mEntry->mData.mKind : 0xFFFF))->mFlags & 0x20) == 0) {
		return 0;
	}
	TObject* item = giveItem(player->mObjId, ((ItemHolder*)player->_4F0)->firstId());
	if (item != 0) {
		vecFromAngleXZ(10.0f, &pos, player->mAngle);
		pos.y += 10.0f;
		vecAdd(&pos, &pos, &player->mPos);
		((PlyItemSlots*)item)->s78(&pos);
	}
	itemHolderRelease((ItemHolder*)player->_4F0);
	struct {
		u16 mType;
		u16 _02;
		u16 mPlayer;
		u16 _06;
	} msg;
	msg.mType   = 0xB6;
	msg.mPlayer = player->mObjId;
	aeSendMessage(&msg);
	return 1;
}

// 0x8003EAB0
// Slot _CC
void TChrPlayer::vfn_CC()
{
	plyToStartPoint(this);
	playMotion(0x40);
	plyPlayVoice(this, 0xB3, 0);
	PLY_CHANGE(this, createPlyStateEvent);
}

// 0x8003EB24
// Slot _D0
void TChrPlayer::vfn_D0()
{
	plyToStartPoint(this);
	playMotion(0x2C);
	PLY_CHANGE(this, createPlyStateEvent);
}

// 0x8003EB88
extern "C" void plyReleaseKickedEgg(TChrPlayer* player)
{
	plyReleaseRef(player, &player->mThrownEggRef);
}

// 0x8003EBAC
// Hides the player and the item in use
extern "C" void plyHide(TChrPlayer* player)
{
	player->mFlags |= 0x10;
	if (player->_520 != 0) {
		((TObject*)player->_520)->mFlags |= 0x10;
		((TObject*)player->_520)->mFlags |= 4;
	}
}

// 0x8003EC48
void TItem::vfn_6C()
{
}

// 0x8003EC4C
void TItem::vfn_70()
{
}

// 0x8003EC50
void TItem::vfn_74()
{
}

// 0x8003EC54
void TItem::place(const Vec3f* pos)
{
}

// 0x8003EC58
void TPlyState::check(TChrPlayer* player)
{
}

// 0x8003EC5C
int TPlyState::handleInput(TChrPlayer* player, PlyInput* input)
{
	return 0;
}

// 0x8003EC64
// Slot _6C
int TChrPlayer::getHealth()
{
	return mHealth.mNow;
}

// 0x8003EC6C
void TPlyState::vfn_20(TChrPlayer* player)
{
}

// 0x8003EC70
// Slot _B0 Keeps the event's motions
void TChrPlayer::vfn_B0(const int* motions)
{
	_320 = motions[0];
	_324 = motions[1];
	_328 = motions[2];
	_32C = motions[3];
	_330 = motions[4];
	_334 = motions[5];
}

// 0x8003ECA4
// Slot _84
extern "C" u16 plyGetHurtTimer(TChrPlayer* player)
{
	return *(u16*)&player->mTimerD6;
}

// 0x8003ECAC
// Slot _88
extern "C" int plyStateVfn30(const TChrPlayer* player)
{
	if (player->mState != 0) {
		return ((TPlyState*)player->mState)->vfn_30((TChrPlayer*)player);
	}
	return 0;
}

// 0x8003ECF0
int TPlyState::vfn_30(TChrPlayer* player)
{
	return 0;
}

// 0x8003ECF8
// Slot _8C
extern "C" int plyStateVfn2C(const TChrPlayer* player)
{
	if (player->mState != 0) {
		return ((TPlyState*)player->mState)->vfn_2C((TChrPlayer*)player);
	}
	return 0;
}

// 0x8003ED3C
int TPlyState::vfn_2C(TChrPlayer* player)
{
	return 0;
}

// 0x8003ED44
// Slot _A0 The egg's id
u16 TChrPlayer::vfn_A0()
{
	return mEggRef.getId();
}

// 0x8003ED68
// Slot _40
f32 TChrPlayer::vfn_40()
{
	return 12.0f;
}

// 0x8003ED70
void TPlyState::update(TChrPlayer* player)
{
}

// 0x8003ED74
void TPlyState::run(TChrPlayer* player)
{
	update(player);
}

// 0x8003EDA0
int TEggCreature::sendToHandle(ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x44:
		return onMsg44(msg);
	case 0x46:
		return onMsg46(msg);
	case 0x47:
		return onMsg47(msg);
	case 0x23:
		return onMsg23(msg);
	}
	return 0;
}

// 0x8003EE18
void TEggCreature::vfn_94(f32* out)
{
}

// 0x8003EE1C
extern "C" void plyEmpty7()
{
}

// 0x8003EE88
// The timed effects a list of ten nodes kept with a free list
PlyEffectList::PlyEffectList()
{
	mHead  = 0;
	mTail  = 0;
	mCount = 0;
	for (int i = 0; i < 9; i++) {
		mNodes[i].mNext = &mNodes[i + 1];
	}
	mNodes[9].mNext = 0;
	mFree           = mNodes;
}

// 0x8003EEDC
PlyEffectList::~PlyEffectList()
{
}

// 0x8003EF18
extern "C" PlyEffect* effectListAdd(PlyEffectList* list)
{
	PlyEffect* effect = effectListAlloc(list);
	if (effect == 0) {
		return 0;
	}
	if (list->mHead == 0) {
		list->mTail = effect;
		list->mHead = effect;
	} else {
		effect->mPrev      = list->mTail;
		list->mTail->mNext = effect;
		list->mTail        = effect;
	}
	list->mCount++;
	return effect;
}

// 0x8003EF8C
extern "C" void effectListClear(PlyEffectList* list)
{
	for (int i = 0; i < 9; i++) {
		list->mNodes[i].mNext = &list->mNodes[i + 1];
	}
	list->mNodes[9].mNext = 0;
	list->mFree           = list->mNodes;
	list->mTail           = 0;
	list->mHead           = 0;
	list->mCount          = 0;
}

// 0x8003EFE0
extern "C" void effectListRemove(PlyEffectList* list, PlyEffect* effect)
{
	if (effect->mPrev != 0) {
		effect->mPrev->mNext = effect->mNext;
	} else {
		list->mHead = effect->mNext;
	}
	if (effect->mNext != 0) {
		effect->mNext->mPrev = effect->mPrev;
	} else {
		list->mTail = effect->mPrev;
	}
	effectListFree(list, effect);
	list->mCount--;
}

// 0x8003F05C
extern "C" PlyEffect* effectListAlloc(PlyEffectList* list)
{
	PlyEffect* effect = list->mFree;
	if (effect != 0) {
		list->mFree   = effect->mNext;
		effect->mNext = 0;
		effect->mPrev = 0;
	}
	return effect;
}

// 0x8003F084
extern "C" void effectListFree(PlyEffectList* list, PlyEffect* effect)
{
	if (effect != 0) {
		effect->mNext = list->mFree;
		list->mFree   = effect;
	}
}

// 0x8003F09C
extern "C" void initPlyStateSlot(void** slot)
{
	*slot = 0;
}

// 0x8003F0A8
extern "C" void* plyStateSlotDtor(void* slot, s16 del)
{
	if (slot != 0 && del > 0) {
		operator delete(slot);
	}
	return slot;
}

// 0x8003F0E4
extern "C" void runPlyState(void** slot, TChrPlayer* player)
{
	if (*slot != 0) {
		((TPlyState*)*slot)->run(player);
	}
}

// 0x8003F11C
// Leaves the current state and enters the new one
extern "C" void* changePlyState(void** slot, TChrPlayer* player, void* next)
{
	TPlyState* old   = (TPlyState*)*slot;
	TPlyState* state = (TPlyState*)next;
	if (old != 0) {
		old->exit(player);
	}
	if (state != 0) {
		state->enter(player);
	}
	*slot = state;
	return old;
}

// 0x8003F194
extern "C" void flagWordOr(void* flags, void* other)
{
	*(u32*)flags |= *(u32*)other;
}

// 0x8003F1A8
extern "C" void flagWordXor(void* flags, void* other)
{
	*(u32*)flags ^= *(u32*)other;
}

// 0x8003F1BC
extern "C" int flagTest(void* flags, int bit)
{
	return *(u32*)flags & (1 << bit);
}

// 0x8003F1D0
FlagByte::FlagByte()
{
	mBits = 0;
}

// 0x8003F1DC
extern "C" void clearFlagByte(void* flags)
{
	((FlagByte*)flags)->mBits = 0;
}

// 0x8003F1E8
extern "C" void flagByteSet(void* flags, int bit)
{
	*(u8*)flags |= 1 << bit;
}

// The joints and the poses are growing arrays
struct PlyArray {
	void* mData;
	int mSize;
	int mCapacity;
};

struct PlyJoint {
	u32 w[0xC];
};

// 0x8003F200
extern "C" void jointArrayInit(PlyArray* array)
{
	array->mData     = 0;
	array->mSize     = 0;
	array->mCapacity = 0;
}

// 0x8003F214
extern "C" PlyArray* jointArrayDtor(PlyArray* array, s16 del)
{
	if (array != 0) {
		if (array->mData != 0) {
			operator delete[](array->mData);
		}
		if (del > 0) {
			operator delete(array);
		}
	}
	return array;
}

// 0x8003F268
extern "C" void jointArrayResize(PlyArray* array, int size)
{
	if (size != array->mSize) {
		jointArrayReserve(array, size);
		array->mSize = size;
	}
}

// 0x8003F2A8
extern "C" void poseArrayInit(PlyArray* array)
{
	array->mData     = 0;
	array->mSize     = 0;
	array->mCapacity = 0;
}

// 0x8003F2BC
extern "C" PlyArray* poseArrayDtor(PlyArray* array, s16 del)
{
	if (array != 0) {
		if (array->mData != 0) {
			operator delete[](array->mData);
		}
		if (del > 0) {
			operator delete(array);
		}
	}
	return array;
}

// 0x8003F310
extern "C" void poseArrayResize(PlyArray* array, int size)
{
	if (size != array->mSize) {
		poseArrayReserve(array, size);
		array->mSize = size;
	}
}

// 0x8003F350
extern "C" void poseArrayReserve(PlyArray* array, int capacity)
{
	if (capacity > array->mCapacity) {
		void* old = array->mData;
		poseArrayAlloc(array, capacity);
		if (old != 0) {
			poseArrayCopy(array, (PlyPose*)array->mData, (PlyPose*)old, array->mSize);
			operator delete[](old);
		}
	}
}

// 0x8003F3B0
extern "C" void jointArrayReserve(PlyArray* array, int capacity)
{
	if (capacity > array->mCapacity) {
		void* old = array->mData;
		jointArrayAlloc(array, capacity);
		if (old != 0) {
			jointArrayCopy(array, (PlyJoint*)array->mData, (PlyJoint*)old, array->mSize);
			operator delete[](old);
		}
	}
}

// 0x8003F410
extern "C" void jointArrayAlloc(PlyArray* array, int capacity)
{
	array->mData     = operator new[](capacity * 0x30);
	array->mCapacity = capacity;
}

// 0x8003F44C
extern "C" void poseArrayAlloc(PlyArray* array, int capacity)
{
	array->mData     = operator new[](capacity * 0x68);
	array->mCapacity = capacity;
}

// 0x8003F488
extern "C" void poseArrayCopy(PlyArray* array, PlyPose* to, PlyPose* from, int count)
{
	for (int i = 0; i < count; i++) {
		to[i] = from[i];
	}
}

// 0x8003F554
extern "C" void jointArrayCopy(PlyArray* array, PlyJoint* to, PlyJoint* from, int count)
{
	for (int i = 0; i < count; i++) {
		to[i] = from[i];
	}
}

MotionEntry gPlayerBodyMotions[66] = {
	{ MOTION_LOOP, 0, -1, 0, 0.0f, 31.0f, 0.5f, 0.0f, 4 },    { MOTION_LOOP, 1, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 5 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 3 },    { MOTION_ONCE, 7, -1, 0, 0.0f, 18.99f, 1.0f, 0.0f, 2 },
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 19.0f, 1.0f, 0.0f, 2 },    { MOTION_ONCE, 9, -1, 0, 0.0f, 19.0f, 0.35f, 0.0f, 0 },
	{ MOTION_LOOP, 26, -1, 0, 0.0f, 30.0f, 0.5f, 0.0f, 2 },   { MOTION_LOOP, 27, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 3 },
	{ MOTION_LOOP, 28, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 3 },   { MOTION_CHAIN, 35, -1, 11, 0.0f, 4.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 35, -1, 0, 0.0f, 6.0f, 1.0f, 0.0f, 8 },    { MOTION_ONCE, 35, -1, 0, 6.0f, 39.0f, 1.0f, 0.0f, 8 },
	{ MOTION_ONCE, 11, -1, 0, 0.0f, 14.0f, 1.0f, 0.0f, 8 },   { MOTION_LOOP, 8, -1, 0, 0.0f, 20.0f, 1.0f, 0.0f, 2 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },     { MOTION_LOOP, 23, -1, 0, 0.0f, 30.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 24, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 0 },   { MOTION_LOOP, 13, -1, 0, 0.0f, 89.0f, 0.5f, 0.0f, 2 },
	{ MOTION_ONCE, 14, -1, 0, 0.0f, 82.0f, 0.5f, 0.0f, 2 },   { MOTION_CHAIN, 10, -1, 0, 0.0f, 9.0f, 0.5f, 0.0f, 4 },
	{ MOTION_ONCE, 12, -1, 0, 0.0f, 59.0f, 0.5f, 0.0f, 0 },   { MOTION_ONCE, 17, -1, 0, 0.0f, 9.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 17, -1, 0, 9.0f, 16.0f, 0.5f, 0.0f, 0 },   { MOTION_CHAIN, 17, -1, 0, 16.0f, 29.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 33, -1, 0, 0.0f, 59.0f, 1.0f, 0.0f, 0 },   { MOTION_ONCE, 25, -1, 0, 0.0f, 19.0f, 0.6f, 0.0f, 0 },
	{ MOTION_BLEND, 15, -1, 6, 3.0f, 8.99f, 0.5f, 0.2f, 0 },  { MOTION_LOOP, 34, -1, 0, 0.0f, 28.99f, 0.4f, 0.0f, 0 },
	{ MOTION_CHAIN, 19, -1, 29, 0.0f, 29.0f, 0.5f, 0.0f, 0 }, { MOTION_LOOP, 20, -1, 0, 0.0f, 28.99f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 21, -1, 0, 0.0f, 53.0f, 1.0f, 0.0f, 0 },   { MOTION_ONCE, 21, -1, 0, 0.0f, 53.0f, 2.0f, 0.0f, 0 },
	{ MOTION_ONCE, 21, -1, 0, 50.0f, 0.0f, -1.5f, 0.0f, 0 },  { MOTION_ONCE, 22, -1, 0, 0.0f, 9.0f, 0.8f, 0.0f, 0 },
	{ MOTION_LOOP, 30, -1, 0, 0.0f, 28.99f, 0.3f, 0.0f, 2 },  { MOTION_LOOP, 18, -1, 0, 0.0f, 28.99f, 0.7f, 0.0f, 0 },
	{ MOTION_ONCE, 16, -1, 0, 0.0f, 29.0f, 1.0f, 0.0f, 0 },   { MOTION_LOOP, 36, -1, 0, 0.0f, 19.0f, 1.0f, 0.0f, 0 },
	{ MOTION_CHAIN, 31, -1, 39, 0.0f, 5.0f, 0.5f, 0.0f, 0 },  { MOTION_ONCE, 30, -1, 0, 0.0f, 1.0f, 0.0f, 0.0f, 0 },
	{ MOTION_ONCE, 32, -1, 0, 0.0f, 34.0f, 1.0f, 0.0f, 0 },   { MOTION_ONCE, 29, -1, 0, 0.0f, 14.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 28.0f, 0.5f, 0.0f, 0 },    { MOTION_LOOP, 37, -1, 0, 0.0f, 53.0f, 1.0f, 0.0f, 8 },
	{ MOTION_ONCE, 38, -1, 0, 0.0f, 243.0f, 1.0f, 0.0f, 0 },  { MOTION_ONCE, 39, -1, 0, 0.0f, 419.0f, 1.0f, 0.0f, 0 },
	{ MOTION_LOOP, 41, -1, 0, 0.0f, 196.0f, 1.0f, 0.0f, 0 },  { MOTION_ONCE, 40, -1, 0, 0.0f, 113.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 42, -1, 0, 0.0f, 115.0f, 1.0f, 0.0f, 0 },  { MOTION_ONCE, 43, -1, 0, 0.0f, 17.0f, 1.0f, 0.0f, 0 },
	{ MOTION_LOOP, 44, -1, 0, 0.0f, 13.0f, 1.0f, 0.0f, 0 },   { MOTION_LOOP, 45, -1, 0, 0.0f, 16.0f, 0.8f, 0.0f, 0 },
	{ MOTION_LOOP, 46, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 0 },   { MOTION_LOOP, 47, -1, 0, 0.0f, 60.0f, 1.0f, 0.0f, 0 },
	{ MOTION_LOOP, 48, -1, 0, 0.0f, 18.0f, 0.8f, 0.0f, 0 },   { MOTION_LOOP, 49, -1, 0, 0.0f, 25.0f, 1.0f, 0.0f, 0 },
	{ MOTION_LOOP, 50, -1, 0, 0.0f, 30.0f, 0.5f, 0.0f, 0 },   { MOTION_LOOP, 51, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 0 },
	{ MOTION_LOOP, 52, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 0 },   { MOTION_ONCE, 53, -1, 0, 0.0f, 63.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 54, -1, 0, 0.0f, 47.0f, 1.0f, 0.0f, 0 },   { MOTION_ONCE, 55, -1, 0, 0.0f, 53.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 56, -1, 0, 0.0f, 59.0f, 1.0f, 0.0f, 0 },   { MOTION_CHAIN, 58, -1, 15, 0.0f, 37.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 57, -1, 0, 0.0f, 141.0f, 1.0f, 0.0f, 0 },  { MOTION_ONCE, 59, -1, 0, 0.0f, 237.0f, 1.0f, 0.0f, 0 },
};

MotionEntry sPlayerFaceMotions[8] = {
	{ MOTION_ONCE, 0, 0, 0, 0.0f, 419.0f, 1.0f, 0.0f, 0 }, { MOTION_ONCE, 1, 1, 0, 0.0f, 244.0f, 1.0f, 0.0f, 0 },
	{ MOTION_LOOP, 2, 2, 0, 0.0f, 196.0f, 1.0f, 0.0f, 0 }, { MOTION_ONCE, 3, 3, 0, 0.0f, 9.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 3, 3, 0, 9.0f, 16.0f, 0.5f, 0.0f, 0 },  { MOTION_ONCE, 3, 3, 0, 16.0f, 29.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 4, 4, 0, 0.0f, 113.0f, 1.0f, 0.0f, 0 }, { MOTION_ONCE, 5, 5, 0, 0.0f, 115.0f, 1.0f, 0.0f, 0 },
};

MotionEntry sPlayerLeftHandMotions[13] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 419.0f, 1.0f, 0.0f, 0 }, { MOTION_ONCE, 1, -1, 0, 0.0f, 244.0f, 1.0f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 196.0f, 1.0f, 0.0f, 0 }, { MOTION_ONCE, 3, -1, 0, 0.0f, 9.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 9.0f, 16.0f, 0.5f, 0.0f, 0 },  { MOTION_ONCE, 3, -1, 0, 16.0f, 29.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 113.0f, 1.0f, 0.0f, 0 }, { MOTION_ONCE, 5, -1, 0, 0.0f, 115.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },   { MOTION_ONCE, 7, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },   { MOTION_ONCE, 9, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
	{ MOTION_ONCE, 10, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
};

MotionEntry sPlayerRightHandMotions[13] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 419.0f, 1.0f, 0.0f, 0 }, { MOTION_ONCE, 1, -1, 0, 0.0f, 244.0f, 1.0f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 196.0f, 1.0f, 0.0f, 0 }, { MOTION_ONCE, 3, -1, 0, 0.0f, 9.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 9.0f, 16.0f, 0.5f, 0.0f, 0 },  { MOTION_ONCE, 3, -1, 0, 16.0f, 29.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 113.0f, 1.0f, 0.0f, 0 }, { MOTION_ONCE, 5, -1, 0, 0.0f, 115.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },   { MOTION_ONCE, 7, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },   { MOTION_ONCE, 9, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
	{ MOTION_ONCE, 10, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
};
