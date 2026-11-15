#include "egg/EggCreature.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "camera/TCamera.h"
#include "egg/TMorphEggAnim2.h"
#include "io/ResBank.h"
#include "io/NinjaFile.h"
#include "camera/TCameraCtrl.h"
#include "animal/TAniManager.h"
#include "animal/AniMount.h"
#include "stage/TEventCtrl.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "core/Mem.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
#include "egg/TEggCreatureTypeE.h"
#include "engine/Colli.h"
#include "camera/TCamPath.h"
#include "engine/DrawState.h"
#include "object/ObjShadow.h"
#include "engine/FxSystem.h"
#include "sound/Sound3DPlay.h"
#include "stage/TGameState.h"
#include "stage/TStageScore.h"
#include "engine/GroundQuery.h"
#include "engine/TParticle.h"
#include "sound/SoundCtrl.h"
#include "egg/TEggContentsIntro.h"
#include "animal/AniModel.h"
#include "core/TMainTask.h"
#include "animal/RacerMover.h"
struct ObjMsg;
struct DrawStateSlot;
struct DrawState;
struct TObject;
struct ColliShape;
struct GjActor;
struct TEggContentsIntro;
struct NJS_TEXLIST;

struct MtxBlock {
	f32 m[3][4];
};

class TArc;
class TCamera;

extern "C" {
extern const int sAnimalLeaveSounds[] = {
	452, 450, 448, 454, 458, 456, 460, 478, 474, 472, 476, -1, -1, -1, -1, -1, -1, -1, -1, 452, 454,
}; // the sound when each kind goes
extern const int sAnimalHatchEffects[] = {
	35, 34, 37, 30, 38, 36, 39, 37, 38, 36, 34, 35, 30, 34, 37, 30, 38, 37, 34, 35, 30,
}; // the hatch effect of each kind
extern const int sAnimalHatchCries[] = {
	451, 449, 447, 453, 457, 455, 459, 477, 473, 471, 475, -1, -1, -1, -1, -1, -1, -1, -1, 451, 453,
}; // the cry of each kind as it hatches
extern const int sAnimalIntroText[] = {
	21, 20, 23, 22, 25, 24, 26, 32, 31, 33, 30, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
}; // the line of text each kind gets
}

// What the common model arc holds the egg's model for each kind
struct AniCommonModel {
	int mCount;       // _00
	void** mObjects;  // _04
	void** mTexLists; // _08
	f32** mScales;    // _0C
	void** mTextures; // _10 bound to mGvms
	u8** mGvms;       // _14
};

// A message between objects with the sender's id
struct AniMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mId; // _04
};

// Told to the game when an animal first joins a player
struct AniJoinMsg {
	u16 mType;   // _00 0xA1
	u8 _02[2];   // _02
	u16 mHolder; // _04
	u16 mId;     // _06
	int mBig;    // _08 a kind past 10
	int _0C;     // _0C
};

#define gAniCommon ((AniCommonModel*)gAnimalCommonModel)

#define random01() (3.0517578e-5f * rand())

// a - b returned by value
static inline Vec3f subVec(const Vec3f* a, const Vec3f* b)
{
	Vec3f r;
	gjSubVector(&r, a, b);
	return r;
}

// v scaled by s returned by value
static inline Vec3f scaledVec(f32 s, const Vec3f* v)
{
	Vec3f r;
	gjScaleVector(s, &r, v);
	return r;
}

// A random value from base up to base + range
static inline f32 randomRange(f32 range, f32 base)
{
	return range * random01() + base;
}

// The way from b to a flat
// Drops the height and makes it a unit direction or zero when too short
static inline void flatten(Vec3f* v)
{
	v->y = 0.0f;
	if (njUnitVector(v) < gUnitVectorMin) {
		*v = gVecZero;
	}
}

static inline Vec3f addVec(const Vec3f* a, const Vec3f* b)
{
	Vec3f r;
	gjAddVector(&r, a, b);
	return r;
}

static inline Vec3f addVecBy(const Vec3f* a, Vec3f b)
{
	Vec3f r;
	gjAddVector(&r, a, &b);
	return r;
}

// A place 20 out from where the holder stands on the camera's side and turned a little
// A macro its locals sit in the caller's own scope
#define BESIDE_HOLDER(at)               \
	Vec3f cam;                          \
	camGetDrawPos(&cam);                \
	Vec3f side = subVec(&cam, &(at));   \
	flatten(&side);                     \
	vecRotateY(&side, &side, 0x3000);   \
	gjScaleVector(20.0f, &side, &side); \
	njAddVector(&(at), &side)

template <typename T>
static inline void killObj(T*& obj)
{
	if (obj != 0) {
		obj->mFlags |= 1;
		obj = 0;
	}
}

static inline void copyMatrix(f32* dst, const f32* src)
{
	for (int i = 0; i < 12; i++) {
		dst[i] = src[i];
	}
}

// 0x8009952C
extern "C" const char* aniModelArcName(int kind)
{
	if (kind < sAnimalModelArcCount) {
		return sAnimalModelArcs[kind];
	}
	return 0;
}

// 0x80099554
extern "C" const char* aniParamArcName(int kind)
{
	if (kind < sAnimalParamArcCount) {
		return sAnimalParamArcs[kind];
	}
	return 0;
}

// 0x8009957C
// The param file of every kind and the common model arc with its textures
extern "C" void aniLoadFiles()
{
	TArc** out;
	aniTexSwapReset();
	for (int i = 0; i < sAnimalParamArcCount; i++) {
		sAnimalParamArcRes[i] = 0;
		out                   = &sAnimalParamArcRes[i];
		const char* name      = aniParamArcName(i);
		if (name != 0) {
			loadArc(name, out);
			gAnimalParams[i] = *(AniParams**)((u8*)*out + 0x4C);
		}
	}
	loadArc("ani_model_common.arc", &sAnimalCommonArc);
	*(AniCommonModel**)&gAnimalCommonModel = (AniCommonModel*)sAnimalCommonArc->mData;
	for (int i = 0; i < gAniCommon->mCount; i++) {
		bindGvmTextures(gAniCommon->mGvms[i], (NJS_TEXLIST*)gAniCommon->mTextures[i]);
	}
}

// 0x80099658
extern "C" void aniFreeFiles()
{
	if (sAnimalCommonRes != 0) {
		releaseResource(sAnimalCommonRes);
		sAnimalCommonRes = 0;
	}
	for (int i = 0; i < gAniCommon->mCount; i++) {
		releaseTextures((NJS_TEXLIST*)gAniCommon->mTextures[i]);
	}
	releaseResource(sAnimalCommonArc);
	sAnimalCommonArc = 0;
	for (int i = 0; i < sAnimalParamArcCount; i++) {
		if (sAnimalParamArcRes[i] != 0) {
			releaseResource(sAnimalParamArcRes[i]);
			sAnimalParamArcRes[i] = 0;
		}
	}
}

// The animals as far as the factory needs them
#define DECLARE_ANIMAL(name, size)                                     \
	class name : public TEggCreatureTypeE {                            \
	public:                                                            \
		name(TObject* parent, int player, int kind, const Vec3f* pos); \
		u8 _384[size - 0x384];                                         \
	};
DECLARE_ANIMAL(TAniClionePenguin, 0x3BC)
DECLARE_ANIMAL(TAniKijiSai, 0x3D4)
DECLARE_ANIMAL(TAniHariAzarashi, 0x3A0)
DECLARE_ANIMAL(TAniSaruKoumori, 0x3C0)
DECLARE_ANIMAL(TAniKumaKame, 0x38C)
DECLARE_ANIMAL(TAniPelicanDog, 0x3A0)
DECLARE_ANIMAL(TAniRabitOwl, 0x3B8)
DECLARE_ANIMAL(TAniOrixPanther, 0x3F4)
DECLARE_ANIMAL(TAniBullBoar, 0x3F4)
DECLARE_ANIMAL(TAniDevilMomonga, 0x408)
DECLARE_ANIMAL(TAniTigerHawk, 0x3CC)

// 0x80099710
// Some animals stand for several kinds
extern "C" TEggCreature* aniCreate(int player, int kind, Vec3f* pos)
{
	TEggCreature* animal;
	switch (kind) {
	case 0:
		animal = new TAniClionePenguin(gTL_05, player, kind, pos);
		break;
	case 1:
		animal = new TAniKijiSai(gTL_05, player, kind, pos);
		break;
	case 2:
		animal = new TAniHariAzarashi(gTL_05, player, kind, pos);
		break;
	case 3:
		animal = new TAniSaruKoumori(gTL_05, player, kind, pos);
		break;
	case 4:
		animal = new TAniKumaKame(gTL_05, player, kind, pos);
		break;
	case 5:
		animal = new TAniPelicanDog(gTL_05, player, kind, pos);
		break;
	case 6:
		animal = new TAniRabitOwl(gTL_05, player, kind, pos);
		break;
	case 7:
		animal = new TAniOrixPanther(gTL_05, player, kind, pos);
		break;
	case 8:
		animal = new TAniBullBoar(gTL_05, player, kind, pos);
		break;
	case 9:
		animal = new TAniDevilMomonga(gTL_05, player, kind, pos);
		break;
	case 10:
		animal = new TAniTigerHawk(gTL_05, player, kind, pos);
		break;
	case 11:
		animal = new TAniBullBoar(gTL_05, player, kind, pos);
		break;
	case 12:
		animal = new TAniDevilMomonga(gTL_05, player, kind, pos);
		break;
	case 13:
		animal = new TAniOrixPanther(gTL_05, player, kind, pos);
		break;
	case 14:
		animal = new TAniClionePenguin(gTL_05, player, kind, pos);
		break;
	case 15:
		animal = new TAniOrixPanther(gTL_05, player, kind, pos);
		break;
	case 16:
		animal = new TAniBullBoar(gTL_05, player, kind, pos);
		break;
	case 17:
		animal = new TAniDevilMomonga(gTL_05, player, kind, pos);
		break;
	case 18:
		animal = new TAniOrixPanther(gTL_05, player, kind, pos);
		break;
	case 19:
		animal = new TAniClionePenguin(gTL_05, player, kind, pos);
		break;
	case 20:
		animal = new TAniSaruKoumori(gTL_05, player, kind, pos);
		break;
	default:
		animal = 0;
		break;
	}
	return animal;
}

const char* gTEggCreatureTypeEName = "TEggCreatureTypeE";

// 0x80099BA8
TEggCreatureTypeE::TEggCreatureTypeE(TObject* parent, int player, int kind, const Vec3f* pos)
    : TEggCreature(parent, 1)
    , mKind(kind)
    , mSpawnPos(*pos)
{
	_00                 = (u32)gTEggCreatureTypeEName;
	mSize               = 900;
	TObjAction::mPlayer = player;
	mLightTask          = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x2A, 0);
	init();
}

// 0x80099C9C
TEggCreatureTypeE::~TEggCreatureTypeE()
{
	killObj(mEggModel);
	killObj(mHatchCamera);
	killObj(*(TObject**)&mMorph);
	killObj(mIntro);
	if (mActor != 0) {
		memFree(mActor);
		mActor = 0;
	}
	killFx();
	mParams = 0;
	if (mAsset.mLink.mList != 0) {
		aniAssetDetach(&mAsset);
	}
	eggTypeStopAllFx(this);
	removeDrawTask(mLightTask);
}

// 0x80099E08
void TEggCreatureTypeE::init()
{
	mParams           = gAnimalParams[mKind];
	mCount            = *(int*)mParams->mValues;
	mMode             = -1;
	mSubMode          = -1;
	mStep             = -1;
	mMotionId         = -1;
	mFree             = 0;
	AniParams* params = mParams;
	f32 accel         = params->mAccel;
	f32 top           = params->mTopSpeed;
	mMover.mKind      = params->mReturnStyle - 3;
	mMover.mAccel     = accel;
	mMover.mTopSpeed  = top;
	mMover.mF0C       = 0.05f;
	mMover.mTurn      = 0x400;
	mMover.mPos       = mSpawnPos;
	mMover.mGravity   = -0.08166667f;
	mStartPos         = mSpawnPos;
	mPos              = mSpawnPos;
	mVel.x            = 0.0f;
	mVel.y            = 0.0f;
	mVel.z            = 0.0f;
	mRot[0]           = 0;
	mRot[1]           = 0;
	mRot[2]           = 0;
	f32 scale         = mParams->mBaseScale;
	mScale.x          = scale;
	mScale.y          = scale;
	mScale.z          = scale;
	mHolderRef.setId(0xFFFF);
	mTarget.setId(0xFFFF);
	mGoal.x  = 0.0f;
	mGoal.y  = -10000.0f;
	mGoal.z  = 0.0f;
	mDir.x   = 0.0f;
	mDir.y   = 0.0f;
	mDir.z   = 0.0f;
	_188     = 0.0f;
	_184     = 0.0f;
	mMotion  = 0;
	mActor   = 0;
	_1AC     = 0;
	mTexList = 0;
	mNodes   = 0;
	mPlayer.init(0, 0);
	mColli    = 0;
	mShape    = 0;
	mHitValue = 0;
	mFx       = 0;
	mtxIdentity(mFxMatrix);
	m270 = 0;
	for (s16 i = 0; i < 10; i++) {
		mFollowers[i] = 0;
	}
	mFollowerCount = 0;
	mEvents        = 0;
	mEventCount    = 0;
	mTexAnims      = *mParams->mTexAnims;
	mTexAnimCount  = mParams->mTexAnimCount;
	mFirstJoin     = 1;
	mEvent         = 0;
	mEggModel      = 0;
	mHatchCamera   = 0;
	mMorph         = 0;
	mIntro         = 0;
	mTimer         = 0;
	mFaded         = 0;
	mBounceBase    = 1.0f;
	mBounceAmp     = 0.0f;
	mBounceAngle   = 0;
	mBounceSpeed   = 0;
	mAniFlags      = 0;
	mLightTask->mFlags |= 1;
	setMode(MODE_LOAD);
}

// 0x8009A068
// Falls back to its holder or home when it falls out of the world or strays too far
void TEggCreatureTypeE::draw()
{
	if (debugEditActive() != 0) {
		updateMatrix();
		return;
	}
	if (mColli != 0) {
		colliPlaceShapes(mColli, &mPos, mShape);
	}
	TObjAction* holder = mHolderRef.get();
	if (holder != 0) {
		Vec3f at = *holder->getPos();
		if (mPos.y < -3000.0f || njDistanceP2P(&mPos, &at) > 3500.0f) {
			Vec3f to;
			camGetPos((u16)mHolderRef.getId(), &to);
			to.y   = at.y;
			mPos.x = to.x;
			mPos.y = to.y;
			mPos.z = to.z;
		}
		mFree = 0;
	} else {
		if (mPos.y < -5000.0f) {
			mPos = mSpawnPos;
		}
		mFree = 1;
	}
	mPush.x = 0.0f;
	mPush.y = 0.0f;
	mPush.z = 0.0f;
	runMode(mMode, 2);
	updateMatrix();
	if ((mAniFlags & 2) == 0) {
		updateMotion();
	}
	updateShadow();
}

// 0x8009A1D4
// Hides it off camera
void TEggCreatureTypeE::vfn_10()
{
	if (mLight.isVisible(mLightTask, 1000.0f, &mPos, 2.0f * mShape->mSizeX, 0) != 0) {
		mLightTask->mFlags &= ~1;
	} else {
		mLightTask->mFlags |= 1;
	}
}

// 0x8009A250
void TEggCreatureTypeE::render()
{
	if (mAsset.mLink.mList != 0) {
		setDrawAmbient(&mLight, &mDrawState);
		DrawStateSlot* slot = getDrawStateSlot();
		njPushMatrixEx();
		njMultiMatrix(0, mNodeMatrix);
		beginDrawState(slot, (const DrawState*)&mDrawState);
		njSetTexture(mTexList);
		beginTexAnim();
		shadowAlphaBegin();
		if (mActor != 0) {
			gjDrawActor(mActor);
		}
		shadowAlphaEnd();
		endTexAnim();
		endDrawState((DrawStateSlot*)slot);
		njPopMatrixEx();
	}
}

// 0x8009A2F0
// Attacks while it is with its holder or goes once it has none left
void TEggCreatureTypeE::vfn_70()
{
	if ((mMode == MODE_FOLLOW || mMode == MODE_RETURN || mMode == MODE_JUMP) && (int)mHolderRef.getId() != 0xFFFF) {
		if (mCount == 0) {
			setMode(MODE_VANISH);
		} else {
			setMode(MODE_DASH);
			mCount--;
			mCount = mCount < 0 ? 0 : mCount;
		}
	}
}

// 0x8009A390
int TEggCreatureTypeE::vfn_74()
{
	return 0;
}

// 0x8009A398
void TEggCreatureTypeE::vfn_78()
{
	s16 mode = mMode;
	if (mode != MODE_VANISH) {
		if (mCount == 0) {
			setMode(MODE_VANISH);
		} else if (mode == MODE_JUMP || mode == MODE_ATTACK) {
			mSubMode = MODE_RETURN;
		} else {
			setMode(MODE_RETURN);
		}
		mHolderRef.setId(0xFFFF);
	}
}

// 0x8009A418
// Goes in a puff once
void TEggCreatureTypeE::vanish()
{
	if ((mAniFlags & 0x4000) == 0) {
		Vec3f pos;
		pos = *getPos();
		f32 m[12];
		mtxSetTransV(m, &pos);
		fxSpawnAt(1, 9, 6, m);
		sePlay3D(sAnimalLeaveSounds[mKind], &mPos);
		mFlags |= 1;
		mAniFlags |= 0x4000;
	}
}

// 0x8009A4C8
void TEggCreatureTypeE::vfn_80(const void* body)
{
	mVel = *(const Vec3f*)body;
	setMode(MODE_JUMP);
}

// 0x8009A504
// Tells its new holder it is here
void TEggCreatureTypeE::attachToHolder()
{
	mHolderRef.setId(mHolderId);
	TObjAction* holder = mHolderRef.get();
	if (holder != 0) {
		AniMsg msg;
		msg.mType = 0x33;
		msg.mId   = mObjId;
		holder->sendToHandle((ObjMsg*)&msg);
	}
}

// 0x8009A56C
// A player touching it while it is free takes it
void TEggCreatureTypeE::checkContacts()
{
	s16 pushes = 0;
	for (ColliContact* contact = mColli->mContacts; contact != 0; contact = contact->mNext) {
		if (colliContactNoBit10(contact) == 0) {
			continue;
		}
		u16 owner = colliContactOwnerU16(contact);
		njAddVector(&mPush, &contact->mPush);
		if (mMode == MODE_FREE && (u16)owner < 4 && (mAniFlags & 0x300) == 0) {
			setHolder(owner);
			mBounceBase = mScale.y;
			mBounceAmp  = 0.5f;
			mAniFlags |= 0x100;
			mAniFlags |= 2;
			continue;
		}
		if ((mAniFlags & 4) != 0 && (mAniFlags & 0x200) == 0 && colliContactInGroup8(contact) != 0 && colliContactHasBit8(contact) != 0) {
			TObjAction* other = findObjById(colliContactOwner(contact));
			if (other != 0) {
				HitInfo info;
				if (other->onHit(this, colliContactTag(contact), (int*)&info) != 0 && njScalor(&info.mPos) > 1.0f) {
					startBounce();
					continue;
				}
			}
		}
		pushes++;
	}
	if (pushes > 1 && (mAniFlags & 0x200) == 0 && (mAniFlags & 0x100) == 0) {
		mAniFlags |= 0x400;
	}
}

// 0x8009A70C
// Whether what it dashes at is touching it
int TEggCreatureTypeE::touchedTarget()
{
	ColliContact* contact;
	for (contact = mColli->mContacts; contact != 0; contact = contact->mNext) {
		if (colliContactNoBit10(contact) != 0) {
			u32 owner = colliContactOwner(contact);
			return (u16)mTarget.getId() == (u16)owner;
		}
	}
	return 0;
}

// 0x8009A78C
void TEggCreatureTypeE::pushOut()
{
	for (ColliContact* contact = mColli->mContacts; contact != 0; contact = contact->mNext) {
		if (colliContactNoBit10(contact) != 0) {
			njAddVector(&mPush, &contact->mPush);
		}
	}
}

// 0x8009A7E8
void TEggCreatureTypeE::setHolder(u16 id)
{
	mHolderId = id;
	mHolderRef.setId(id);
	mFree = 0;
}

// 0x8009A824
// Back to what it did or gone once its attacks are spent
void TEggCreatureTypeE::endAttack()
{
	if (mCount == 0) {
		setMode(MODE_VANISH);
	} else {
		setMode(mSubMode);
	}
}

// 0x8009A860
void TEggCreatureTypeE::setMode(int mode)
{
	if (mMode != -1) {
		runMode(mMode, 1);
	}
	mSubMode = mMode;
	mMode    = -2;
	if (mode != -1) {
		runMode(mode, 0);
	}
	mMode = mode;
}

// 0x8009A8CC
void TEggCreatureTypeE::runMode(int mode, int phase)
{
	switch (mode) {
	case MODE_LOAD:
		modeLoad(phase);
		break;
	case MODE_HATCH:
		modeHatch(phase);
		break;
	case MODE_FREE:
		modeFree(phase);
		break;
	case MODE_GREET:
		modeGreet(phase);
		break;
	case MODE_FOLLOW:
		modeFollow(phase);
		break;
	case MODE_RETURN:
		modeReturn(phase);
		break;
	case MODE_JUMP:
		modeJump(phase);
		break;
	case MODE_DASH:
		modeDash(phase);
		break;
	case MODE_ATTACK:
		onPhase(phase);
		break;
	case MODE_VANISH:
		modeVanish(phase);
		break;
	}
}

// 0x8009A988
// Waits for the model file and its textures then makes the actor
void TEggCreatureTypeE::modeLoad(int phase)
{
	if (phase == 0) {
		return;
	}
	if (phase == 2) {
		if (aniAssetAttach(mKind, &mAsset) != 0) {
			TArc* arc = mAsset.mFile->mArc;
			if (arc != 0) {
				mMotion   = (AniModelFile*)findArcExport(arc, "AniModel");
				mPolyAnim = findArcExport(arc, "polyanim");
				mFxBank   = mAsset.mFile->mFxBank;
				if (mMotion == 0 || mFxBank == -1) {
					return;
				}
				AniModelFile::AniTexList* list = *mMotion->mTexList;
				for (u32 i = 0; i < (u32)list->mCount; i++) {
					if (list->mEntries[i].mTexture == 0) {
						return;
					}
				}
				setMode(MODE_HATCH);
			} else {
				mFlags |= 1;
			}
		} else {
			mFlags |= 1;
		}
	} else if (phase == 1) {
		if (mMotion != 0) {
			NJS_OBJECT* obj = (NJS_OBJECT*)*mMotion->mObject;
			mActor          = (GjActor*)(memAlloc(gjActorSize(obj)));
			gjCreateActor(obj, (u8*)mActor);
			_1AC     = 0;
			mTexList = (NJS_TEXLIST*)*mMotion->mTexList;
			mPlayer.init((const MotionEntry*)mNodes, mMotion->mMotions);
		}
	}
}

// 0x8009AB00
// Out of the egg it opens the animal falls out and the game shows its name
void TEggCreatureTypeE::modeHatch(int phase)
{
	if (phase == 0) {
		setMotionSpeed(1.0f);
		playMotion(0, 1, 1.0f);
		eggTypeStopAllFx(this);
		mTimer    = 150;
		mAniFlags = 2;
		mStep     = 0;
		mJump     = mParams->mHatchJump;
		mEggModel = 0;
		mMorph    = new TMorphEggAnim2(this, 2);
		mtxSetTransV(mFxMatrix, &mPos);
		mFx    = fxSpawnAt(1, 7, sAnimalHatchEffects[mKind], mFxMatrix);
		mEvent = 0;
	} else if (phase == 2) {
		switch (mStep) {
		case 0:
			hatchOpen();
			break;
		case 1:
			hatchFall();
			break;
		case 2:
			hatchIntro();
			break;
		}
		int timer = mTimer;
		if (timer == 0) {
			sePlay3D(sAnimalHatchCries[mKind], &mPos);
			mTimer = -1;
		} else if (timer > 0) {
			mTimer = timer - 1;
		}
	} else if (phase == 1) {
		eggTypeStopAllFx(this);
		clearExtraFx();
		mFaded = 0;
	}
}

// 0x8009AC88
// Wanders about home until a player picks it up
void TEggCreatureTypeE::modeFree(int phase)
{
	if (phase == 0) {
		setMotionSpeed(mParams->mWalkRate);
		playMotion(2, 1, 0.075f);
		mStartPos = mPos;
		mTimer    = 600;
		mAniFlags = 0;
		mStep     = 8;
		pickWanderGoal();
	} else if (phase == 2) {
		checkContacts();
		if ((mAniFlags & 0x200) != 0) {
			runBounce();
		} else if ((mAniFlags & 0x100) != 0) {
			if (mFirstJoin != 0) {
				mFirstJoin = 0;
				AniJoinMsg msg;
				msg.mBig    = 0;
				msg._0C     = 0;
				msg.mType   = 0xA1;
				msg.mHolder = mHolderId;
				msg.mId     = mObjId;
				if (mKind >= 11) {
					msg.mBig = 1;
				}
				sendGameMsg((ObjMsg*)&msg);
			}
			if (aniManagerGetMode() == 0) {
				TObjAction* holder = mHolderRef.get();
				if (holder != 0) {
					AniMsg msg;
					msg.mType = 0x17;
					msg.mId   = mObjId;
					if (holder->sendToHandle((ObjMsg*)&msg) != 0) {
						Vec3f at = *holder->getPos();
						int rot  = holder->getRotY();
						f32 m[12];
						mtxSetTransV(m, &at);
						mtxRotateY(m, m, rot);
						mtxTranslate((float*)m, (const float*)m, 16.0f, 0.0f, 0.0f);
						mtxGetPos(m, &at);
						mPos    = at;
						mRot[1] = rot;
						eventHideObj((u8*)this);
						Vec3f top = at;
						top.y += 10.0f;
						createTAniGreetMark(this, &top);
						setMode(MODE_GREET);
						return;
					}
				}
			} else {
				Vec3f top = mPos;
				top.y += 10.0f;
				createTAniGreetMark(this, &top);
				sePlay3D(0x6C, &mPos);
				setMode(MODE_GREET);
			}
		} else {
			runWander();
		}
		njAddVector(&mPos, &mPush);
	} else if (phase == 1) {
		mScale.y = mParams->mBaseScale;
		mAniFlags &= ~2;
	}
}

// 0x8009AF50
void TEggCreatureTypeE::modeGreet(int phase)
{
	if (phase == 0) {
		attachToHolder();
		setMotionSpeed(1.0f);
		playMotion(5, 1, 0.075f);
		mTimer    = 0;
		mAniFlags = 0x8000;
		mStep     = -1;
	} else if (phase == 2 && mPlayer.isDone() != 0) {
		setMode(MODE_FOLLOW);
	}
}

// 0x8009AFE8
// Wanders by its holder and comes back when it is more than 70 away
void TEggCreatureTypeE::modeFollow(int phase)
{
	if (phase == 0) {
		setMotionSpeed(1.0f);
		playMotion(1, 1, 0.075f);
		TObjAction* holder = mHolderRef.get();
		if (holder != 0) {
			mStartPos = *holder->getPos();
		} else {
			mStartPos = mPos;
		}
		mTimer    = 600;
		mAniFlags = 0;
		mStep     = 7;
	} else if (phase == 2) {
		if ((int)mHolderRef.getId() == 0xFFFF) {
			setMode(MODE_FREE);
		} else {
			TObjAction* holder = mHolderRef.get();
			mStartPos          = *holder->getPos();
			checkContacts();
			if ((mAniFlags & 0x200) != 0) {
				runBounce();
			} else {
				runWander();
				njAddVector(&mPos, &mPush);
				if (holder != 0 && distXZ(&mPos, &mStartPos) > 70.0f) {
					setMode(MODE_RETURN);
				}
			}
		}
	} else if (phase == 1) {
		mScale.y = mParams->mBaseScale;
		mAniFlags &= ~2;
	}
}

// 0x8009B18C
// Back to its holder in the animal's own way
void TEggCreatureTypeE::modeReturn(int phase)
{
	if (phase == 0) {
		mStartPos = mPos;
		mTimer    = 30;
		mAniFlags = 0;
		mStep     = mParams->mReturnStyle;
		if (mStep == 3) {
			playMotion(2, 1, 0.075f);
		} else if (mStep == 4) {
			playMotion(9, 1, 0.075f);
		}
		setMotionSpeed(mParams->mWalkRate);
	} else if (phase == 2) {
		Vec3f from = mPos;
		checkContacts();
		switch (mStep) {
		case 3:
			if ((mAniFlags & 0x200) != 0) {
				runBounce();
				return;
			}
			returnWalk();
			break;
		case 4:
			returnHop();
			break;
		case 5:
			returnRun();
			break;
		case 6:
			returnJump();
			break;
		}
		njAddVector(&mPos, &mPush);
		Vec3f to = mPos;
		if ((mAniFlags & 0x40) != 0 && mRot[2] == 0) {
			if (njDistanceP2P(&from, &to) < 0.5f) {
				if (mTimer-- == 0) {
					setMode(MODE_FOLLOW);
				}
			} else {
				mTimer = 30;
			}
		}
		if ((mAniFlags & 0x20) == 0) {
			f32 speed      = njScalor(&mVel);
			mPlayer.mSpeed = speed * ((1.0f - mMotionSpeed) / mParams->mTopSpeed) + mMotionSpeed;
		}
	} else if (phase == 1) {
		mScale.y = mParams->mBaseScale;
		mAniFlags &= ~2;
	}
}

// 0x8009B3A8
void TEggCreatureTypeE::modeJump(int phase)
{
	if (phase == 0) {
		setMotionSpeed(1.0f);
		playMotion((u16)mJump->mMotion, 1, 0.075f);
		mStartPos = mPos;
	} else if (phase == 2) {
		pushOut();
		runJump();
		njAddVector(&mPos, &mPush);
	} else if (phase == 1) {
		mAniFlags |= 4;
		mAniFlags &= ~1;
		mAniFlags &= ~2;
	}
}

// 0x8009B468
// Runs at the goal startDash picked then attacks
void TEggCreatureTypeE::modeDash(int phase)
{
	if (phase == 0) {
		setMotionSpeed(mParams->mWalkRate);
		playMotion(2, 1, 0.075f);
		mTimer    = 0;
		mAniFlags = 0;
		mStep     = -1;
		startDash();
	} else if (phase == 2) {
		mVel = subVec(&mGoal, &mPos);
		gjScaleVector(0.5f, &mVel, &mVel);
		njAddVector(&mPos, &mVel);
		if (njScalor(&mVel) < 0.5f) {
			mPos      = mGoal;
			mHitValue = scoreNewChain(3);
			setMode(MODE_ATTACK);
		}
	}
}

// 0x8009B580
void TEggCreatureTypeE::modeVanish(int phase)
{
	if (phase == 0) {
		eggTypeStopAllFx(this);
		vanish();
	}
}

// 0x8009B5BC
// Draws as the opening egg does until the egg's model is made and done
void TEggCreatureTypeE::hatchOpen()
{
	*(DrawState*)&mDrawState = mMorph->mDraw;
	if (mFaded == 0) {
		sePlayFor(0x2A0, 600);
		mFaded = 1;
	}
	if (mEvent == 0) {
		if (mMorph->mFinished != 0) {
			mEvent = eventStartById(mObjId, 0);
		}
	} else if (mEggModel == 0) {
		makeEggModel();
		mLightTask->mFlags &= ~1;
	} else if (mMorph->mHatched != 0 && mEggModel->isDone() != 0) {
		killObj(*(TObject**)&mMorph);
		killFx();
		killObj(mEggModel);
		makeAnimalModel();
		*(DrawState*)&mDrawState = *(const DrawState*)getDefaultDrawState();
		moveDrawTask(mLightTask, (u16)mParams->mDrawKey);
		mAniFlags &= ~2;
		mStep = 1;
	}
}

// 0x8009B798
// Jumps out and lands then shows the camera and the text or ends the event
void TEggCreatureTypeE::hatchFall()
{
	if (mEggModel == 0) {
		makeAnimalModel();
		return;
	}
	if (mPlayer.passedFrame(mJump->mTakeoff) != 0) {
		mVel.x = 0.0f;
		mVel.y = 2.5f;
		mVel.z = 0.0f;
		mAniFlags |= 1;
	} else if (mPlayer.passedFrame(mJump->mLand) != 0 && (mAniFlags & 4) == 0) {
		mAniFlags |= 2;
	}
	if ((mAniFlags & 1) != 0) {
		EggHits hits;
		Vec3f probe = mPos;
		probe.y += 10.0f;
		mVel.y += -0.08166667f;
		if (mVel.y < 0.0f && meshColliProbe((EggHit*)hits.mHits, (const Vec3f*)&probe, (unsigned long)0xD) != 0) {
			f32 ground = hits.mHits[0].mPos.y - mPos.y;
			if (!(ground < mVel.y)) {
				mVel.y = ground;
				mAniFlags &= ~1;
				mAniFlags &= ~2;
				mAniFlags |= 4;
			}
		}
		njAddVector(&mPos, &mVel);
		if (mEggModel != 0) {
			*(MtxBlock*)mEggModel->getMtx() = *(MtxBlock*)mNodeMatrix;
		}
	}
	if (mPos.y < -2000.0f) {
		vanish();
	}
	if (mPlayer.isDone() != 0 && mEggModel->isDone() != 0 && (mAniFlags & 4) != 0) {
		killObj(mEggModel);
		if (_44 != 0) {
			makeHatchCamera();
			makeIntro();
			setMotionSpeed(1.0f);
			playMotion(1, 1, 0.075f);
			mStep = 2;
		} else {
			eventEnd();
			setMode(MODE_FREE);
		}
	}
}

// 0x8009BA38
void TEggCreatureTypeE::hatchIntro()
{
	if (mIntro->mDone != 0) {
		killObj(mHatchCamera);
		killObj(mIntro);
		eventEnd();
		setMode(MODE_FREE);
	}
}

// 0x8009BAC0
// The egg's model from the common arc
void TEggCreatureTypeE::makeEggModel()
{
	f32* scale = *gAniCommon->mScales;
	mEggModel  = createCamPath((s8)TObjAction::mPlayer, 2, *gAniCommon->mObjects, *gAniCommon->mTexLists, 0, *scale, *scale);
	if (mEggModel != 0) {
		mEggModel->attachToMtx();
		*(MtxBlock*)mEggModel->getMtx() = *(MtxBlock*)mNodeMatrix;
	}
}

// 0x8009BB60
// The animal's egg model from its own arc
void TEggCreatureTypeE::makeAnimalModel()
{
	f32* scale = *mMotion->mEggScale;
	mEggModel  = createCamPath((s8)TObjAction::mPlayer, 2, *mMotion->mEggObject, *mMotion->mEggTexList, 0, *scale, *scale);
	if (mEggModel != 0) {
		mEggModel->attachToMtx();
		*(MtxBlock*)mEggModel->getMtx() = *(MtxBlock*)mNodeMatrix;
	}
}

// 0x8009BC00
void TEggCreatureTypeE::makeHatchCamera()
{
	Vec3f pos = mPos;
	f32* cam  = mParams->mCamera;
	pos.y += cam[0];
	Vec3f look = mPos;
	look.x += cam[1];
	look.y += cam[2];
	look.z += cam[3];
	mHatchCamera = (TObject*)createTCamFix((s8)TObjAction::mPlayer, 2, &pos, &look);
}

// 0x8009BCB0
void TEggCreatureTypeE::makeIntro()
{
	if (mIntro == 0) {
		mIntro = createTEggContentsIntro(this, sAnimalIntroText[mKind]);
	}
}

// 0x8009BD00
void TEggCreatureTypeE::runWander()
{
	switch (mStep) {
	case 7:
		wanderRest();
		break;
	case 9:
		wanderLand();
		break;
	case 8:
		wanderWalk();
		break;
	}
	mRot[2] = angleApproach(mRot[2], 0, 0x80);
}

// 0x8009BD78
// Stands falling to the ground until it is time to walk
void TEggCreatureTypeE::wanderRest()
{
	if (--mTimer == 0) {
		mStep = 9;
		playMotion(7, 1, 0.075f);
		setMotionSpeed(1.0f);
		return;
	}
	EggHits hits;
	Vec3f probe = mPos;
	probe.y += 5.0f;
	mVel.y += -0.08166667f;
	mAniFlags &= ~4;
	if (meshColliProbe((EggHit*)hits.mHits, (const Vec3f*)&probe, (unsigned long)0xD) != 0 && (hits.mMask & 1) != 0) {
		f32 ground = hits.mHits[0].mPos.y - mPos.y;
		if (ground > mVel.y) {
			mVel.y = ground;
			mAniFlags |= 4;
		}
	}
	if ((mAniFlags & 4) == 0) {
		njAddVector(&mPos, &mVel);
	} else {
		mVel.x = 0.0f;
		mVel.y = 0.0f;
		mVel.z = 0.0f;
	}
}

// 0x8009BEB0
void TEggCreatureTypeE::wanderLand()
{
	const s8 type = motionEntry()->mType;
	if (mPlayer.isDone() != 0 || type != MOTION_ONCE) {
		mStep  = 8;
		mTimer = 300;
		pickWanderGoal();
		playMotion(2, 1, 0.075f);
		setMotionSpeed(mParams->mWalkRate);
	}
}

// 0x8009BF40
// Walks to the goal then rests a while
void TEggCreatureTypeE::wanderWalk()
{
	walkToGoal();
	if (distXZ(&mGoal, &mPos) < 0.25f || mTimer-- == 0) {
		if ((mAniFlags & 4) == 0) {
			mTimer = 300;
			pickWanderGoal();
		} else {
			mStep  = 7;
			mVel.x = 0.0f;
			mVel.y = 0.0f;
			mVel.z = 0.0f;
			mTimer = (int)randomRange(0.0f, 600.0f);
			playMotion(1, 1, 0.075f);
			setMotionSpeed(1.0f);
		}
	}
}

// 0x8009C034
// Walks to beside its holder or home
void TEggCreatureTypeE::returnWalk()
{
	Vec3f at;
	Vec3f side;
	Vec3f cam;
	Vec3f goal;
	TObjAction* holder = mHolderRef.get();
	if (holder != 0) {
		at = *holder->getPos();
	} else {
		at = mSpawnPos;
		mAniFlags |= 0x40;
	}
	camGetDrawPos(&cam);
	const Vec3f& toCam = subVec(&cam, &at);
	side               = toCam;
	flatten(&side);
	vecRotateY(&side, &side, 0x3000);
	gjScaleVector(20.0f, &side, &side);
	njAddVector(&at, &side);
	if (distXZ(&at, &mPos) <= 10.0f) {
		goal = mPos;
		mAniFlags |= 0x40;
	} else {
		Vec3f dir = subVec(&mPos, &at);
		flatten(&dir);
		gjScaleVector(10.0f, &dir, &dir);
		goal = addVec(&at, &dir);
		mAniFlags &= 0x40;
	}
	mMover.mTarget = goal;
	mMover.mPos    = mPos;
	mMover.mVel    = mVel;
	mMover.mRot    = mRot;
	mMover.update();
	if ((mMover.mFlags & 1) != 0) {
		mAniFlags |= 4;
	}
	if ((mMover.mFlags & 2) != 0) {
		mAniFlags |= 0x20;
	}
	mPos = mMover.mPos;
	mVel = mMover.mVel;
	mRot = mMover.mRot;
	if ((mMover.mFlags & 0x10) != 0) {
		Vec3f launch;
		launch = mMover.mLaunch;
		jump(&launch, mParams->mJump);
	} else {
		if (njScalor(&mVel) > 0.35f * mParams->mTopSpeed) {
			playMotion(6, 1, 0.1f);
			setMotionSpeed(1.0f);
		} else {
			playMotion(2, 1, 0.1f);
			setMotionSpeed(mParams->mWalkRate);
		}
		checkLedge();
	}
}

// 0x8009C3D0
// One hop up then falls into a run
void TEggCreatureTypeE::returnHop()
{
	if ((mAniFlags & 1) == 0) {
		if (mPlayer.passedFrame(10.0f) != 0) {
			Vec3f v;
			vecFromAngleXZ(0.5f, &v, mRot[1]);
			v.y  = 0.8f;
			mVel = v;
			mAniFlags |= 1;
		}
	} else {
		mVel.y -= 0.020416668f;
		if (mVel.y <= 0.0f) {
			mStep = 5;
		} else {
			njAddVector(&mPos, &mVel);
		}
	}
}

// 0x8009C494
// Runs to beside its holder and stops when it is near and slow
void TEggCreatureTypeE::returnRun()
{
	Vec3f at;
	if ((int)mHolderRef.getId() == 0xFFFF) {
		at = mSpawnPos;
	} else {
		TObjAction* holder = mHolderRef.get();
		if (holder == 0) {
			return;
		}
		at = *holder->getPos();
	}
	BESIDE_HOLDER(at);
	f32 speed = njScalor(&mVel);
	if (njDistanceP2P(&at, &mPos) <= 35.0f && speed <= 0.4f) {
		playMotion(8, 1, 0.075f);
		setMotionSpeed(mParams->mWalkRate);
		mStep = 6;
	} else {
		playMotion(9, 1, 0.075f);
		setMotionSpeed(mParams->mWalkRate);
		Vec3f dir = subVec(&mPos, &at);
		flatten(&dir);
		gjScaleVector(10.0f, &dir, &dir);
		Vec3f to = addVec(&at, &dir);
		mAniFlags &= ~0x40;
		f32 y = to.y;
		y += 15.0f;
		mMover.mTarget.x = to.x;
		mMover.mTarget.y = y;
		mMover.mTarget.z = to.z;
		mMover.mPos      = mPos;
		mMover.mVel      = mVel;
		mMover.mRot      = mRot;
		mMover.update();
		mPos = mMover.mPos;
		mVel = mMover.mVel;
		mRot = mMover.mRot;
		mAniFlags &= ~4;
	}
}

// 0x8009C7C0
// Leaps to beside its holder landing on what it finds under it
void TEggCreatureTypeE::returnJump()
{
	Vec3f at;
	if ((int)mHolderRef.getId() == 0xFFFF) {
		at = mSpawnPos;
	} else {
		TObjAction* holder = mHolderRef.get();
		if (holder == 0) {
			return;
		}
		at = *holder->getPos();
	}
	BESIDE_HOLDER(at);
	Vec3f probe = at;
	Vec3f from  = mPos;
	f32 speed   = njScalor(&mVel);
	EggHits hits;
	Vec3f ground;
	if (meshColliProbe((EggHit*)hits.mHits, (const Vec3f*)&probe, (unsigned long)1) != 0 && (hits.mMask & 1) != 0) {
		ground = hits.mHits[0].mPos;
	} else {
		ground = at;
	}
	Vec3f dir = subVec(&mPos, &ground);
	flatten(&dir);
	gjScaleVector(10.0f, &dir, &dir);
	njAddVector(&ground, &dir);
	Vec3f d  = subVec(&ground, &from);
	f32 dist = njScalor(&d);
	if (dist > 50.0f) {
		mStep = 5;
		return;
	}
	speed += 0.05f;
	if (speed > mParams->mAccel * dist) {
		speed = mParams->mAccel * dist;
	}
	if (speed > 0.25f * mParams->mTopSpeed) {
		speed = 0.25f * mParams->mTopSpeed;
	}
	mRot[2] = angleApproach(mRot[2], 0, 0x80);
	Vec3f vel;
	vel.x = 0.0f;
	vel.y = 0.0f;
	vel.z = 1.0f;
	vecRotateY(&vel, &vel, mRot[1]);
	vel.y = -1.0f;
	if (njUnitVector(&vel) < gUnitVectorMin) {
		vel = gVecZero;
	}
	Vec3f top = addVecBy(&from, scaledVec(10.0f, &vel));
	top.y += 10.0f;
	gjScaleVector(speed, &vel, &vel);
	Vec3f next = addVec(&top, &vel);
	if (meshColliProbeLine((EggHit*)hits.mHits, (const Vec3f*)&top, (const Vec3f*)&next, (unsigned long)0x2201) != 0) {
		vel.x = vel.z = 0.0f;
	}
	if (meshColliProbe((EggHit*)hits.mHits, (const Vec3f*)&top, (unsigned long)1) != 0) {
		f32 y = hits.mHits[0].mPos.y - from.y;
		if (y > vel.y) {
			vel.y = y;
			if (mRot[2] == 0) {
				mAniFlags |= 4;
			}
		}
	}
	mVel = vel;
	njAddVector(&mPos, &mVel);
	if ((mAniFlags & 4) != 0) {
		if (motionEntry()->mType == MOTION_ONCE) {
			if (mPlayer.isDone() != 0) {
				mAniFlags |= 0x40;
				setMode(MODE_FOLLOW);
			}
		} else {
			mAniFlags |= 0x40;
			setMode(MODE_FOLLOW);
		}
	}
}

// 0x8009CCAC
// Leaves the ground at the jump's frame and falls back to it then goes back to what it did
void TEggCreatureTypeE::runJump()
{
	if ((mAniFlags & 4) != 0) {
		if (motionEntry()->mType == MOTION_ONCE) {
			if (mPlayer.isDone() != 0) {
				setMode(mSubMode);
			}
		} else {
			setMode(mSubMode);
		}
	}
	if (mPlayer.passedFrame(mJump->mTakeoff) != 0) {
		mAniFlags |= 1;
	} else if (mPlayer.passedFrame(mJump->mLand) != 0 && (mAniFlags & 4) == 0) {
		mAniFlags |= 2;
	}
	if ((mAniFlags & 1) == 0) {
		return;
	}
	EggHits hits;
	Vec3f probe = mPos;
	probe.y += 10.0f;
	mAniFlags &= ~4;
	mVel.y += -0.08166667f;
	if (mVel.y < 0.0f) {
		f32 ground;
		if (meshColliProbe((EggHit*)hits.mHits, (const Vec3f*)&probe, (unsigned long)0xD) != 0) {
			ground = hits.mHits[0].mPos.y;
		} else {
			ground = mStartPos.y;
		}
		if (!(ground - mPos.y < mVel.y)) {
			mVel.y = 0.0f;
			mPos.y = ground;
			njAddVector(&mPos, &mVel);
			mVel.x = 0.0f;
			mVel.y = 0.0f;
			mVel.z = 0.0f;
			mAniFlags |= 4;
			mAniFlags &= ~1;
			mAniFlags &= ~2;
			return;
		}
	}
	njAddVector(&mPos, &mVel);
	if (mSubMode == MODE_VANISH && mEventCount > 0 && mPlayer.passedFrame(mEvents[0].mFrame) != 0) {
		sePlay3D(0x6E, &mPos);
	}
}

// 0x8009CF18
// A random place up to 20 from where it started
void TEggCreatureTypeE::pickWanderGoal()
{
	mGoal.x = 0.0f;
	mGoal.y = 0.0f;
	mGoal.z = 0.0f;
	mGoal.x = 2.0f * (random01() - 0.5f);
	mGoal.z = 2.0f * (random01() - 0.5f);
	if (njUnitVector(&mGoal) < gUnitVectorMin) {
		mGoal = gVecZero;
	}
	gjScaleVector(randomRange(20.0f, 0.0f), &mGoal, &mGoal);
	njAddVector(&mGoal, &mStartPos);
}

// 0x8009D03C
// Turns to the goal and walks stopping at walls and ledges
void TEggCreatureTypeE::walkToGoal()
{
	EggHits hits;
	Vec3f probe = mPos;
	probe.y += 5.0f;
	int target = angleToXZ(&mPos, &mGoal);
	int angle  = angleApproach(mRot[1], (u16)target, 0x200);
	mRot[1]    = angle;
	vecFromAngleXZ(0.25f, &mVel, angle);
	mAniFlags &= ~4;
	mVel.y += -0.08166667f;
	if (mVel.y < 0.0f && meshColliProbe((EggHit*)hits.mHits, (const Vec3f*)&probe, (unsigned long)0xD) != 0) {
		f32 ground = hits.mHits[0].mPos.y - mPos.y;
		if (ground > mVel.y) {
			mVel.y = ground;
			mAniFlags |= 4;
		}
	}
	Vec3f ahead = scaledVec(10.0f, &mVel);
	njAddVector(&ahead, &probe);
	if (meshColliProbeLine((EggHit*)hits.mHits, (const Vec3f*)&probe, (const Vec3f*)&ahead, (unsigned long)((-1))) != 0) {
		mVel.z = 0.0f;
		mVel.x = 0.0f;
		mAniFlags |= 8;
	}
	if (meshColliProbe((EggHit*)hits.mHits, (const Vec3f*)&ahead, (unsigned long)1) != 0 && (hits.mMask & 1) != 0) {
		if (mPos.y - hits.mHits[0].mPos.y >= 20.0f) {
			mVel.z = 0.0f;
			mVel.x = 0.0f;
			mAniFlags |= 0x10;
		} else {
			mAniFlags &= ~0x10;
		}
	} else {
		mVel.z = 0.0f;
		mVel.x = 0.0f;
		mAniFlags |= 0x10;
	}
	checkLedge();
	njAddVector(&mPos, &mVel);
}

// 0x8009D248
void TEggCreatureTypeE::updateShadow()
{
	EggHits hits;
	if (meshColliProbe((EggHit*)hits.mHits, (const Vec3f*)&mPos, (unsigned long)0xD) != 0 && (hits.mMask & 1) != 0) {
		shadowSetFromContact(&mShadow, (u8*)&hits);
	}
}

// 0x8009D2BC
void TEggCreatureTypeE::setMotionSpeed(f32 speed)
{
	mPlayer.mSpeed = speed;
	mMotionSpeed   = speed;
}

// 0x8009D2C8
// Blends into a motion unless the model already shows it and takes its events from the params
void TEggCreatureTypeE::playMotion(int motion, int events, f32 blend)
{
	if (mMotion == 0) {
		return;
	}
	const s16 current = mPlayer.mCurrent;
	if (current != -1) {
		const MotionEntry* table = (const MotionEntry*)mNodes;
		if (table[current].mMotion == table[(u16)motion].mMotion) {
			return;
		}
	}
	if (mMotionId != (u16)motion) {
		mMotionId = motion;
		mPlayer.playBlend(mMotionId, blend);
		updateMotion();
	}
	if (events != 0 && (u16)motion < 5) {
		mEventCount = gAnimalParams[mKind]->mEventSets[(u16)motion]->mCount;
		mEvents     = *gAnimalParams[mKind]->mEventSets[(u16)motion]->mEvents;
	} else {
		mEventCount = 0;
		mEvents     = 0;
	}
}

// 0x8009D3D0
// Swaps the textures the motion calls for at this frame
void TEggCreatureTypeE::beginTexAnim()
{
	const MotionEntry* table = (const MotionEntry*)mNodes;
	int motion               = table[mPlayer.mCurrent].mMotion;
	if (mTexAnims != 0) {
		for (int i = 0; i < mTexAnimCount; i++) {
			AniTexAnim* anim = &mTexAnims[i];
			if (motion == anim->mMotion) {
				f32 frame = mPlayer.mFrame;
				if (frame > anim->mFrom && frame < anim->mTo) {
					aniTexSwapBegin((TexList*)mTexList, anim->mA, anim->mB);
				}
			}
		}
	}
}

// 0x8009D4AC
void TEggCreatureTypeE::endTexAnim()
{
	aniTexSwapEnd();
}

// 0x8009D4CC
void TEggCreatureTypeE::updateMatrix()
{
	mtxSetTransV(mNodeMatrix, &mPos);
	mtxRotateY(mNodeMatrix, mNodeMatrix, mRot[1]);
	mtxRotateZ(mNodeMatrix, mNodeMatrix, mRot[2]);
	mtxScaleV(mNodeMatrix, mNodeMatrix, &mScale);
}

// 0x8009D530
void TEggCreatureTypeE::updateMotion()
{
	if (mMotion != 0) {
		mPlayer.update();
		poseActor();
	}
}

// 0x8009D574
// Poses the actor with the animal's bone callback for the motion if it has one
void TEggCreatureTypeE::poseActor()
{
	if (mActor == 0) {
		return;
	}
	gjPushUnitMatrix();
	if (mPlayer.isBlending() != 0) {
		gjMotionLinkActor(mActor, (void*)mPlayer.getBlend(), mPlayer.mBlendTime);
	} else {
		const MotionEntry* table = (const MotionEntry*)mNodes;
		int motion               = table[mPlayer.mCurrent].mMotion;
		if ((void**)m270 != 0 && ((void**)m270)[motion] != 0) {
			resetBones();
			gjSetMotionCallback((void (*)(NJS_OBJECT*))((void**)m270)[motion]);
		}
		int cur   = mPlayer.mCurrent;
		f32 frame = mPlayer.mFrame;
		gjMotionActor(frame, mActor, mPlayer.getMotion(cur));
		if ((void**)m270 != 0 && ((void**)m270)[motion] != 0) {
			gjSetMotionCallback(0);
			vfn_A0();
		}
	}
	njPopMatrixEx();
}

// 0x8009D694
void TEggCreatureTypeE::vfn_A0()
{
	f32 m[12];
	f32 bone[12];
	Vec3f pos;
	mtxSetTransV(m, &mPos);
	mtxRotateY(m, m, mRot[1]);
	for (s16 i = 0; i < mFollowerCount; i++) {
		if (mFollowers[i] == 0) {
			break;
		}
		mtxConcat((f32*)bone, (const f32*)m, (const f32*)((u8*)gAnimalExtraMatrices + i * 0x30));
		mtxGetPos(bone, &pos);
		mFollowers[i]->mPos = pos;
	}
	if ((mAniFlags & 0x8000) == 0 && mEvents != 0) {
		for (s16 i = 0; i < mEventCount; i++) {
			if (mPlayer.passedFrame(mEvents[i].mFrame) != 0) {
				int at = mEvents[i].mBone;
				if (at != -1) {
					mtxConcat((f32*)bone, (const f32*)m, (const f32*)(gAnimalBoneMatrices + at * 0x30));
					mtxGetPos(bone, &pos);
				} else {
					mtxGetPos(m, &pos);
				}
				particleOnceById(this, &pos, 0, mEvents[i].mEffect);
			}
		}
	}
}

// 0x8009D83C
extern "C" void eggTypeStopAllFx(TEggCreatureTypeE* animal)
{
	for (s16 i = 0; i < 10; i++) {
		if (animal->mFollowers[i] != 0) {
			animal->mFollowers[i]->mFlags |= 1;
			animal->mFollowers[i] = 0;
		}
	}
	animal->mFollowerCount = 0;
}

// 0x8009D88C
void TEggCreatureTypeE::clearExtraFx()
{
}

// 0x8009D890
// The effects some kinds keep on their bodies
void TEggCreatureTypeE::startFollowers()
{
	switch (mKind) {
	case 1:
		addFollower(&mPos, 0x94, 0);
		addFollower(&mPos, 0x94, 1);
		break;
	case 3:
		addFollower(&mPos, 0x9C, 0);
		addFollower(&mPos, 0x9F, 1);
		addFollower(&mPos, 0xA0, 2);
		break;
	case 8:
		addFollower(&mPos, 0x6D, 0);
		break;
	}
}

// 0x8009D954
void TEggCreatureTypeE::addFollower(Vec3f* pos, int id, s16 slot)
{
	if (mFollowerCount < 10 && slot < 10) {
		mFollowers[slot] = particleGenById(this, pos, 0, id);
		mFollowerCount++;
	}
}

// 0x8009D9C4
// Squashes and springs back after a hit held still for 15 frames first
void TEggCreatureTypeE::runBounce()
{
	mAniFlags |= 2;
	int timer = mTimer;
	if (timer > 0) {
		mTimer   = timer - 1;
		mScale.y = 0.1f;
	} else if (timer == 0) {
		sePlay3D(0x1CE, &mPos);
		mTimer--;
	} else {
		mBounceAmp -= 0.01f;
		mBounceAngle += mBounceSpeed;
		mBounceSpeed += 0x100;
		mBounceSpeed &= 0xFFFF;
		mBounceAngle &= 0xFFFF;
		if (mBounceAmp > 0.0f) {
			mScale.y = mBounceAmp * njSin(mBounceAngle) + mBounceBase;
		} else {
			mScale.y = mBounceBase;
			mAniFlags &= ~0x200;
			mAniFlags &= ~2;
		}
	}
}

// 0x8009DAC8
void TEggCreatureTypeE::startBounce()
{
	mTimer       = 15;
	mBounceBase  = mScale.y;
	mBounceAmp   = 0.5f;
	mBounceAngle = 0;
	mBounceSpeed = 0;
	mAniFlags |= 0x200;
	sePlay3D(0x1CD, &mPos);
}

// 0x8009DB24
// Jumps onto the ground 50 ahead when pushed up against it
void TEggCreatureTypeE::checkLedge()
{
	if ((mAniFlags & 0x400) != 0) {
		Vec3f vel;
		Vec3f to;
		vecFromAngleXZ(50.0f, &to, mRot[1]);
		njAddVector(&to, &mPos);
		if (hasGround(&to) != 0) {
			aimJump(&vel, &mPos, &to);
			jump(&vel, mParams->mJump);
			mAniFlags &= ~0x400;
		}
	}
}

// 0x8009DBBC
// The velocity to land on a place in a whole number of frames 60 to 120
void TEggCreatureTypeE::aimJump(Vec3f* vel, const Vec3f* from, const Vec3f* to)
{
	*vel     = subVec(to, from);
	f64 h    = __fabs(vel->y);
	f32 high = h;
	f32 dist = vecNormalizeXZ(vel, vel);
	u32 frames;
	if (high > dist) {
		frames = (u32)(60.0f * (high / 65.0f));
	} else {
		frames = (u32)(60.0f * (dist / 65.0f));
	}
	if (frames < 60) {
		frames = 60;
	} else if (frames > 120) {
		frames = 120;
	}
	gjScaleVector(dist / (f32)frames, vel, vel);
	vel->y = 0.040833335f * (f32)frames + high / (f32)frames;
}

// 0x8009DD10
// A goal by its target or ahead of its holder at a height for its kind
void TEggCreatureTypeE::startDash()
{
	Vec3f goal;
	Vec3f dir;
	dir.x              = 0.0f;
	dir.y              = 0.0f;
	dir.z              = 1.0f;
	TObjAction* holder = mHolderRef.get();
	if (holder == 0) {
		setMode(MODE_RETURN);
		return;
	}
	Vec3f from = *holder->getPos();
	holder->getRotY();
	mTarget.setId(camLockGetId(camGetDrawIndex()));
	goal               = from;
	TObjAction* target = mTarget.get();
	if (target != 0) {
		mTargetPos = *target->getPos();
		Vec3f d    = subVec(&mTargetPos, &goal);
		dir        = d;
		if (njUnitVector(&dir) < gUnitVectorMin) {
			dir = gVecZero;
		}
	} else {
		f32 m[12];
		mtxIdentity(m);
		mtxRotateY(m, m, holder->getRotY());
		mtxMultVec(m, &dir, &dir);
	}
	switch (mKind) {
	case 0:
	case 1:
	case 12:
	case 14:
		goal.y += 20.0f;
		break;
	case 3:
	case 20:
		goal.y += 28.0f;
		break;
	case 5:
		goal.y += 10.0f;
		break;
	case 6:
	case 9:
		goal.y += 15.0f;
		break;
	case 10:
		goal.y += 40.0f;
		break;
	}
	Vec3f step = scaledVec(20.0f, &dir);
	njAddVector(&goal, &step);
	mGoal   = goal;
	mDir    = dir;
	mRot[1] = vecAngleXZ(&mDir);
	mRot[2] = 0;
}

// 0x8009DFB8
void TEggCreatureTypeE::jump(const Vec3f* vel, AniLeap* desc)
{
	mVel  = *vel;
	mJump = desc;
	setMode(MODE_JUMP);
}

// 0x8009DFF8
void TEggCreatureTypeE::notifyHolder()
{
	u16 id = mHolderId;
	if (id != 0xFFFF) {
		camShakeHard((u16)(s16)id);
	}
}

// 0x8009E028
int TEggCreatureTypeE::hasGround(const Vec3f* pos)
{
	Vec3f probe = *pos;
	probe.y += 10.0f;
	EggHits hits;
	if (meshColliProbe((EggHit*)hits.mHits, (const Vec3f*)&probe, (unsigned long)1) != 0) {
		return 1;
	}
	return 0;
}
