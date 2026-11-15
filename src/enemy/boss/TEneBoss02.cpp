#include "engine/MotionPlayer.h"
#include "enemy/EnemyAnim.h"
#include "enemy/boss/BossParts.h"
#include "core/MslMath.h"
#include "object/ObjAction.h"

#include "enemy/EnemyMotionKey.h"
#include "io/ResBank.h"
#include "enemy/boss/TEneBoss02_GateShot.h"
#include "engine/DrawTask.h"
#include "enemy/boss/TEneBoss02.h"
#include "sound/SeHolder.h"
#include "Ninja/ninja.h"
#include "enemy/EnemyCommon.h"
#include "engine/MotionPose.h"
#include "camera/TCamera.h"
#include "engine/CharaColli.h"
#include "enemy/EnemyHit.h"
#include "enemy/ObjEnemy.h"
#include "enemy/boss/TEneBoss02_Pool.h"
#include "enemy/boss/TEneBoss02_PoolPow.h"
#include "object/ObjShadow.h"
#include "enemy/EnemyTexFx.h"
#include "enemy/EnemyFx.h"
#include "camera/TCameraCtrl.h"
#include "stl/math.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "egg/EggCreature.h"
#include "engine/TParticle.h"
#include "animal/RacerMover.h"

MotionEntry sBoss02Motions[20] = {
	{ 0, 18, -1, 0, 0.0f, 30.0f, 0.4f, 0.0f, 0 },
	{ 1, 0, -1, 0, 0.0f, 29.0f, 0.2f, 0.0f, 0 },
	{ 1, 1, -1, 0, 0.0f, 14.0f, 0.3f, 0.0f, 0 },
	{ 0, 2, -1, 0, 0.0f, 30.0f, 0.5f, 0.0f, 0 },
	{ 0, 17, -1, 0, 0.0f, 19.0f, 0.5f, 0.0f, 0 },
	{ 1, 9, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ 0, 7, -1, 0, 0.0f, 20.0f, 0.5f, 0.0f, 0 },
	{ 1, 8, -1, 0, 0.0f, 4.0f, 0.2f, 0.0f, 0 },
	{ 1, 6, -1, 0, 0.0f, 14.0f, 0.3f, 0.0f, 0 },
	{ 1, 12, -1, 0, 0.0f, 14.0f, 0.3f, 0.0f, 0 },
	{ 0, 13, -1, 0, 0.0f, 20.0f, 1.0f, 0.0f, 0 },
	{ 1, 14, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ 1, 11, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ 0, 15, -1, 0, 0.0f, 30.0f, 0.4f, 0.0f, 0 },
	{ 1, 16, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ 1, 3, -1, 0, 0.0f, 19.0f, 0.35f, 0.0f, 0 },
	{ 0, 19, -1, 0, 0.0f, 30.0f, 0.2f, 0.0f, 0 },
	{ 1, 4, -1, 0, 0.0f, 19.0f, 0.35f, 0.0f, 0 },
	{ 1, 5, -1, 0, 0.0f, 239.0f, 1.0f, 0.0f, 0 },
	{ 1, 10, -1, 0, 0.0f, 539.0f, 1.0f, 0.0f, 0 },
};

extern "C" int motionKeyOnce(f32 time, MotionEntry* entry, f32* frame)
{
	*frame = time * entry->mRate + entry->mStart;
	if (*frame > entry->mEnd) {
		*frame = entry->mEnd;
		return 1;
	}
	return 0;
}

// 0x800593E4
extern "C" void motionKeyLoop(f32 time, const MotionEntry* entry, f32* frame)
{
	f32 length = entry->mEnd - entry->mStart;
	f32 at     = njFraction(time * entry->mRate / length);
	if (at < 0.0f) {
		at += 1.0f;
	}
	at *= length;
	*frame = at + entry->mStart;
}

// 0x80059460
// From 0 to 1
static inline f32 clampUnit(f32 b)
{
	f32 out;
	if (b < 0.0f) {
		out = 0.0f;
	} else if (b > 1.0f) {
		out = 1.0f;
	} else {
		out = b;
	}
	return out;
}

extern "C" void motionKeyBlend(f32 time, f32 rate, f32* blend)
{
	*blend = time * rate;
	*blend = clampUnit(*blend);
}

// 0x80059498
// Updates to play the entry through
extern "C" f32 motionEntryLength(const MotionEntry* entry)
{
	return (entry->mEnd - entry->mStart) / entry->mRate;
}

// 0x800594B0
// Updates to reach frame
extern "C" f32 motionEntryTimeAt(const MotionEntry* entry, f32 frame)
{
	return (frame - entry->mStart) / entry->mRate;
}

// 0x800594C4
// Updates a blend at rate takes
extern "C" f32 motionBlendLength(f32 rate)
{
	return 1.0f / rate;
}

// 0x800594D0
// Plays the key the clock is in
extern "C" void motionKeyPlay(const MotionEntry* table, MotionKey* keys, EnemyMotion* motion, f32 time, f32 rate)
{
	MotionKey* next = keys;
	for (;;) {
		keys = next++;
		if (next->mKind == MotionKey::KEY_END) {
			break;
		}
		if (time < next->mTime) {
			break;
		}
	}
	time -= keys->mTime;
	f32 frame;
	switch (keys->mKind) {
	case MotionKey::KEY_ONCE:
		motionKeyOnce(time, (MotionEntry*)&table[keys->mEntry], &frame);
		enemyMotionSet(motion, keys->mEntry, frame);
		break;
	case MotionKey::KEY_LOOP:
		motionKeyLoop(time, &table[keys->mEntry], &frame);
		enemyMotionSet(motion, keys->mEntry, frame);
		break;
	case MotionKey::KEY_BLEND: {
		motionKeyBlend(time, rate, &frame);
		MotionKey* prev = keys - 1;
		if (prev->mKind == MotionKey::KEY_LOOP) {
			f32 from;
			motionKeyLoop(keys->mTime - prev->mTime, &table[prev->mEntry], &from);
			enemyMotionBlend(motion, prev->mEntry, from, keys->mEntry, table[keys->mEntry].mStart, frame);
		} else {
			enemyMotionBlendEnds(motion, prev->mEntry, keys->mEntry, frame);
		}
		break;
	}
	}
}

// 0x80059614
// The speed to jump height under gravity
extern "C" f32 jumpSpeed(f32 gravity, f32 height)
{
	return __fres(__frsqrte(2.0f * gravity * height));
}

// 0x80059630
// Updates to the top of a jump
extern "C" f32 jumpRiseTime(f32 gravity, f32 speed)
{
	return speed / gravity;
}

// 0x80059638
// How high a jump is after time
extern "C" f32 jumpHeightAt(f32 time, f32 gravity, f32 speed)
{
	f32 height = speed * time;
	height -= 0.5f * (time * (gravity * time));
	return height;
}

// Not in the DOL the linker stripped it
extern "C" f32 jumpHeightAtFrame(int frame, f32 gravity, f32 speed)
{
	return jumpHeightAt((f32)frame, gravity, speed);
}

// 0x80059654
// When a jump is at height going up and coming down
extern "C" void jumpTimesAt(f32 gravity, f32 speed, f32 height, f32* up, f32* down)
{
	gravity *= 0.5f;
	speed *= -1.0f;
	quadRoots(gravity, speed, height, up, down);
}

struct Vec3f;
struct ColliShape;
struct CharaColli;
struct NJS_QUATERNION;
struct NJS_VECTOR;
struct GjActor;

extern "C" {

inline void TEneBoss02::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}
extern const int sBoss02Nodes[]               = { 0, 65, 72, 105, 124, 125, -1 };
const char* gTEneBoss02Name                   = "TEneBoss02";
extern const EnemyFrameKey sBoss02FrameKey0[] = { { 4.0f, 0 }, { 15.0f, 5 }, { -1.0f, 0 } };
extern const EnemyFrameKey sBoss02FrameKey1[] = { { 5.0f, 4 }, { 20.0f, 3 }, { -1.0f, 0 } };
extern const EnemyFrameKey sBoss02FrameKey2[] = { { 1.0f, 0 }, { 14.0f, 3 }, { -1.0f, 0 } };

// 0x80256380
// the first data of the file
const EnemyFrameKey* sBoss02FrameKeys[3]     = { sBoss02FrameKey0, sBoss02FrameKey1, sBoss02FrameKey2 };
extern const ColliShape sBoss02BodyShapes[1] = {
	{ { 0.0f, 35.0f, 0.0f }, 0x0, 0, 0x0, 40.0f, 35.0f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 },
};
extern const ColliShape sBoss02BiteShapes[1] = {
	{ { 0.0f, 2.5f, 0.0f }, 0x0, 0, 0x0, 66.0f, 2.5f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 },
};
extern const ColliShape sBoss02SlamShapes[1] = {
	{ { 0.0f, 2.5f, 30.0f }, 0x0, 0, 0x0, 40.0f, 2.5f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 },
};
}

// The node walk the motion callbacks share
static int sNode;            // 0x802C4EE8
static int sPick;            // 0x802C4EEC
static const int* sPickList; // 0x802C4EF0
static Vec3f* sPickOut;      // 0x802C4EF4
static f32* sPickMtx;        // 0x802C4EF8
static int sTailCount;       // 0x802C4EFC
static int* sTailNodes;      // 0x802C4F00

static inline int toAngle(f32 x, f32 z)
{
	return (int)(10430.38043493439 * (f32)atan2(x, z));
}

// 0x80059684
extern "C" void initTEneBoss02()
{
	resBankLoad(&sBoss02Bank, "ar_ene_blue_boss.arc");
	resBankAddTextures(&sBoss02Bank, 0, "ene_blue_boss.gvm");
	resBankAddTextures(&sBoss02Bank, 1, "ene_blue_boss_suimen.gvm");
	sBoss02Models   = resBankGetModels(&sBoss02Bank);
	sBoss02Textures = resBankGetTextures(&sBoss02Bank);
	sBoss02Motion   = resBankGetMotion(&sBoss02Bank, 0);
	gBoss02Params   = (Boss02Params*)enemyParamGet(2);
	fxBankLoadDefault(&sBlueBossFx, "pa_blueboss.pad");
	sNode    = 0;
	sPick    = 0;
	sPickOut = 0;
	sPickMtx = 0;
	initBoss02Tail();
	setWaterShotAssets(*sBoss02Models, *sBoss02Textures);
	initBossGates();
	bossRippleInit();
}

// 0x80059768
extern "C" void shutdownTEneBoss02()
{
	bossRippleExit();
	freeBossGates();
	clearWaterShotAssets();
	freeBoss02Tail();
	fxBankRelease(&sBlueBossFx);
	resBankRemoveTextures(&sBoss02Bank, 1);
	resBankRemoveTextures(&sBoss02Bank, 0);
	resBankRelease(&sBoss02Bank);
	sBoss02Models   = 0;
	sBoss02Textures = 0;
	sBoss02Motion   = 0;
}

// 0x800597D8
extern "C" void boss02CreatePoolWatcher()
{
	createPoolWatcher();
}

// 0x800597F8
extern "C" TObject* createTEneBoss02(TObject* parent, SetDesc* desc)
{
	return new TEneBoss02(parent, desc);
}

// 0x8005984C
extern "C" NJS_TEXLIST** shapeTEneBoss02()
{
	return sBoss02Textures;
}

// 0x80059854
extern "C" const void* paramsTEneBoss02()
{
	return sBoss02Nodes;
}

// 0x80059860
TEneBoss02::TEneBoss02(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	_00   = (u32)gTEneBoss02Name;
	mSize = 0x3F8;
	init(desc);
	initBossBar();
}

// 0x800598FC
void TEneBoss02::init(SetDesc* desc)
{
	motionCtrlSetPlayer(&mAnim, &mBossMotion);
	motionCtrlInit(&mAnim, sBoss02Motions, sBoss02Motion);
	motionCtrlPlay(&mAnim, 0);
	mBody.mRot.x = 0;
	makeColliders();
	mTail      = new BossTail(8);
	mTransform = 0;
	mNextNode  = 0;
	mBrain     = 0;
	readEnemyDesc(desc);
	mPose      = (GjActor*)allocPose(sBoss02Models[1]);
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mLayer     = 0x21;
	mBossFlags = 0;
	mKnock.x   = 0.0f;
	mKnock.y   = 0.0f;
	mKnock.z   = 0.0f;
	mFrameKey  = 0;
	mMaxHealth = 500;
	mHealth    = 500;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 80.0f;
	mShadow._34 = 80.0f;
	shadowEnable(&mShadow);
	mTarget.x = 0.0f;
	mTarget.y = 0.0f;
	mTarget.z = 0.0f;
	mAim.x    = 0.0f;
	mAim.y    = 0.0f;
	mAim.z    = 0.0f;
	mAimSlot  = 0;
	mBaseY    = 0.0f;
	mGlowFx   = 0;
	mTrail    = 0;
	glowOff();
	mSwimPhase  = 0.0f;
	mTurnStep   = 0;
	mTurnFrom   = 0;
	mMoveFrom.x = 0.0f;
	mMoveFrom.y = 0.0f;
	mMoveFrom.z = 0.0f;
	mTurnRate   = 1.0f;
	mMoveRate   = 1.0f;
	mTint.reset();
	mView      = 0;
	mViewShown = -1;
	mDemo      = 0;
	seCueClear(&mCue);
	_1BC       = 0;
	mStep      = 0;
	mCount     = 0;
	mTurnSpeed = 0;
	mTime      = 0.0f;
	mTime1     = 0.0f;
	mTime2     = 0.0f;
	mTime3     = 0.0f;
	mTime4     = 0.0f;
	measure();
	createBrain();
}

// 0x80059AF4
// The body's cylinder 40 across and 35 high
extern "C" void* createBoss02Body(int id)
{
	CharaColli* colli = colliCreate(1, 1);
	Vec3f center;
	center.x = 0.0f;
	center.y = 0.0f;
	center.z = 0.0f;
	colliShapeMakeCylinder(colliGetShapes(colli), &center, 1, 40.0f, 35.0f);
	((void (*)(CharaColli*, u16))colliSetOwner)(colli, id);
	colliSetMasks(colli, 2, 0xFF, 0, 0);
	colliShapeSetBit8(colli, 0);
	colliShapeSetTag(colli, 0, 0);
	return colli;
}

// 0x80059B98
void TEneBoss02::makeColliders()
{
	mCollider = (CharaColli*)createBoss02Body(mObjId);
	mAttack   = colliCreate(1, 1);
	colliCopyShapes(mAttack, (const ColliShape*)sBoss02BiteShapes);
	colliSetOwner(mAttack, (u16)mObjId);
	colliSetMasks(mAttack, 2, 0xFF, 0, 4);
	colliShapeSetBit8(mAttack, 0);
	colliShapeSetBit10(mAttack, 0);
	colliShapeSetTag(mAttack, 0, 1);
	colliDisable(mAttack);
}

// Starts a pick the points of the nodes in list go to out in order
static void beginPick(const int* list, Vec3f* out);
static int pickDone();
static int pickNode();

// 0x80059C40
// Walks the model keeping the matrix of the first picked node
int TEneBoss02::walkBody(NJS_OBJECT* obj)
{
	int more;
	do {
		njPushMatrixEx();
		njMotionTransformEx(obj);
		if (pickNode() != 0 && sPickMtx != 0) {
			njGetMatrix(sPickMtx);
			sPickMtx = 0;
		}
		more = pickDone() == 0;
		sNode++;
		if (more && obj->child != 0) {
			more = walkBody(obj->child);
		}
		njPopMatrixEx();
		obj = obj->sibling;
	} while (more && obj != 0);
	return more;
}

// 0x80059CEC
void TEneBoss02::measure()
{
	measureBody();
	measureTail();
}

// 0x80059D20
// Where the head sits from the body at frame 4 of motion 1 flat
void TEneBoss02::measureBody()
{
	int nodes[3] = { 0x68, 0x69, -1 };
	Vec3f pts[2];
	void* motion;

	motion = sBoss02Motion[1];
	beginPick(nodes, pts);
	gjPushUnitMatrix();
	bodyTilt();
	njStartMotionObj(motion, 4.0f);
	walkBody(sBoss02Models[1]);
	njPopMatrixEx();
	mNeck    = pts[1];
	mNeckDir = pts[1];
	njSubVector(&mNeckDir, &pts[0]);
	mNeckDir.y = 0.0f;
	if (njUnitVector(&mNeckDir) < gUnitVectorMin) {
		mNeckDir = gVecZero;
	}
}

// 0x80059E1C
// The tail's rest pose from motion 18 its points the length of each link and the tip
void TEneBoss02::measureTail()
{
	f32 mtx[12];
	int nodes[9];
	Vec3f d;
	Vec3f* pts;
	f32* lens;
	int i;
	f32* cs;
	f32* sn;
	void* motion;
	int j;

	for (i = 0; i < 8; i++) {
		nodes[i] = i + 0x41;
	}
	nodes[8] = -1;
	motion   = sBoss02Motion[0x12];
	pts      = mTail->mPoints;
	lens     = mTail->mLengths;
	beginPick(nodes, pts);
	sPickMtx = mtx;
	gjPushUnitMatrix();
	bodyTilt();
	njStartMotionObj(motion, 0.0f);
	walkBody(sBoss02Models[1]);
	njPopMatrixEx();
	gjSetMotionCallback(0);
	sn      = (f32*)&d + 1;
	cs      = (f32*)&d + 2;
	lens[0] = 0.0f;
	for (j = 1; j < 8; j++) {
		d = pts[j];
		njSubVector(&d, &pts[j - 1]);
		lens[j] = njScalor(&d);
	}
	d.x = 0.0f;
	njSinCos(0x2A00, sn, cs);
	njPushMatrix(mtx);
	njInvertMatrix(0);
	njCalcVectorEx(&d, &mTipLocal);
	njPopMatrixEx();
	mTailStep = pts[1];
	njSubVector(&mTailStep, &pts[0]);
	mTail->reset();
}

// 0x80059FA0
TEneBoss02::~TEneBoss02()
{
	if (mDemo != 0) {
		mDemo->mFlags |= 1;
		mDemo = 0;
	}
	if (mGlowFx != 0) {
		mGlowFx->mFlags |= 1;
		mGlowFx = 0;
	}
	if (mTrail != 0) {
		mTrail->mFlags |= 1;
		mTrail = 0;
	}
	unlock();
	removeDrawTask(mDrawTask);
	if (_1BC != 0) {
		_1BC->mFlags |= 1;
		_1BC = 0;
	}
	freePose((void**)&mPose);
	if (mBrain != 0) {
		delete mBrain;
		mBrain = 0;
	}
	if (mTail != 0) {
		delete mTail;
		mTail = 0;
	}
	colliRetire(mAttack);
	mAttack = 0;
	colliRetire(mCollider);
	mCollider = 0;
	resetBossView();
}

// 0x8005A12C
void TEneBoss02::setPos(const Vec3f* pos)
{
	f32 x        = pos->x;
	f32 y        = pos->y;
	mBody.mPos.x = x;
	f32 z        = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x8005A170
f32 TEneBoss02::vfn_40()
{
	return 90.0f;
}

// 0x8005A178
// The base of the body under the last part at the body's height
void bodyBase(const Vec3f* pos, const HitVec* parts, Vec3f* out)
{
	out->x = parts[5].x;
	out->y = parts[5].y;
	out->z = parts[5].z;
	out->y = pos->y;
}

// 0x8005A19C
void placeBody(void* colli, Vec3f* pos)
{
	colliPlaceShapes((CharaColli*)colli, pos, (const ColliShape*)sBoss02BodyShapes);
	colliRefreshBounds((CharaColli*)colli);
	colliBoundsOn((CharaColli*)colli);
}

// 0x8005A1E0
// The bite sits on the body's base and the slam along the facing
void TEneBoss02::syncCollider()
{
	Vec3f base;
	bodyBase(&mBody.mPos, mParts, &base);
	placeBody(mCollider, &base);
	if (mBossFlags & BOSS_BITE) {
		colliEnable(mAttack);
		colliPlaceShapes(mAttack, &base, sBoss02BiteShapes);
		colliRefreshBounds(mAttack);
		colliBoundsOn(mAttack);
	} else if (mBossFlags & BOSS_SLAM) {
		colliEnable(mAttack);
		colliPlaceShapesRotY(mAttack, &mBody.mPos, mBody.mRot.y, sBoss02SlamShapes);
		colliRefreshBounds(mAttack);
		colliBoundsOn(mAttack);
	} else {
		colliDisable(mAttack);
	}
}

// 0x8005A2A4
// The shadow on the water grows with the body's length and its depth
void fitShadow(f32 waterY, const Vec3f* pos, const HitVec* parts, ObjShadow* shadow)
{
	Vec3f ground;
	Vec3f d;
	f32 size;

	ground.y = parts[5].y;
	ground.x = parts[5].x;
	ground.z = parts[5].z;
	ground.y = waterY;
	d.x      = parts[4].x;
	d.y      = parts[4].y;
	d.z      = parts[4].z;
	njSubVector(&d, &parts[5]);
	size        = 1.7f * njScalor(&d);
	f64 h       = fabs(pos->y - waterY);
	size        = size * distanceFalloff(h);
	shadow->_30 = size;
	shadow->_34 = size;
	shadowSetGround(shadow, &ground, 0);
}

// 0x8005A368
void TEneBoss02::playDemo(int model, int motion)
{
	void* object       = resBankGetObjects(&sBoss02Bank)[model];
	NJS_MOTION** table = resBankGetMotion(&sBoss02Bank, 1);

	mDemo = createCamPath(0, 3, object, table[motion], 0, 1.0f, 1.0f);
	if (mDemo != 0) {
		((TCamera*)mDemo)->setCut();
	}
}

// 0x8005A3F8
void TEneBoss02::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
}

// 0x8005A418
// A kind-1 hit does nothing
int TEneBoss02::getDamage(EnemyHit* hit)
{
	if (hit->mKind == 1) {
		return 0;
	}
	return hit->mInfo.mDamage;
}

// 0x8005A434
int TEneBoss02::getHitKind(const void* touch, HitInfo* info)
{
	if (mBossFlags & BOSS_GUARDED) {
		return 1;
	}
	if (mBossFlags & BOSS_OPEN) {
		return 3;
	}
	return info->mFlags.testBit(4) != 0 ? 3 : 1;
}

// 0x8005A490
// A player takes 10 or 20 from the slam
int TEneBoss02::onHit(const TObjAction* other, int shape, int* result)
{
	HitInfo* info = (HitInfo*)result;
	int hit       = 0;
	if (isPlayerObj(other) != 0) {
		info->mFlags.clear();
		info->mPos.x = 0.0f;
		info->mPos.y = 0.0f;
		info->mPos.z = 0.0f;
		if (shape == 0) {
			info->mDamage = 10;
		} else if (shape == 1) {
			if (mBossFlags & BOSS_BITE) {
				info->mDamage = 10;
			} else {
				info->mDamage = 20;
			}
		}
		hit = 1;
	} else if (isEggObj(other) != 0 && shape == 1) {
		hit = 1;
		info->mFlags.clear();
		info->mPos.x  = 0.0f;
		info->mPos.y  = 0.0f;
		info->mPos.z  = 0.0f;
		info->mDamage = 1;
		info->mFlags.setBit(0xD);
	}
	return hit;
}

// 0x8005A580
void TEneBoss02::setTarget(const Vec3f* target)
{
	mTarget = *target;
}

// 0x8005A59C
void TEneBoss02::setAim(const Vec3f* aim)
{
	mAim = *aim;
}

// 0x8005A5B8
// The mouth and the facing
int TEneBoss02::getMouth(Vec3f* out)
{
	out->x = mParts[3].x;
	out->y = mParts[3].y;
	out->z = mParts[3].z;
	return mBody.mRot.y;
}

// 0x8005A5D8
int TEneBoss02::finishMotion()
{
	return waitMotion();
}

// 0x8005A5F8
void TEneBoss02::setHealth(int health)
{
	mMaxHealth = health;
	mHealth    = health;
}

// 0x8005A604
int TEneBoss02::isAppearDone()
{
	return isActionDone(ACT_APPEAR);
}

// 0x8005A628
int TEneBoss02::isRiseDone()
{
	return isActionDone(ACT_RISE);
}

// 0x8005A64C
int TEneBoss02::isFacing()
{
	return mBossFlags & BOSS_FACING;
}

// 0x8005A658
int TEneBoss02::isClose()
{
	return mBossFlags & BOSS_CLOSE;
}

// 0x8005A664
int TEneBoss02::isShootDone()
{
	return isActionDone(ACT_SHOOT);
}

// 0x8005A688
void TEneBoss02::setAimSlot(Vec3f* aim, int slot)
{
	mAim.x   = aim->x;
	mAim.y   = aim->y;
	mAim.z   = aim->z;
	mAimSlot = slot;
}

// 0x8005A6A8
int TEneBoss02::isShootGateDone()
{
	return isActionDone(ACT_SHOOT_GATE);
}

// 0x8005A6CC
int TEneBoss02::isRoarDone()
{
	return isActionDone(ACT_ROAR);
}

// 0x8005A6F0
int TEneBoss02::isSpitDone()
{
	return isActionDone(ACT_SPIT);
}

// 0x8005A714
int TEneBoss02::isDiveDone()
{
	return isActionDone(ACT_DIVE);
}

// 0x8005A738
int TEneBoss02::isDiveDeepDone()
{
	return isActionDone(ACT_DIVE_DEEP);
}

// 0x8005A75C
int TEneBoss02::isLeapDone()
{
	return isActionDone(ACT_LEAP);
}

// 0x8005A780
int TEneBoss02::isSurfaceDone()
{
	return isActionDone(ACT_SURFACE);
}

// 0x8005A7A4
int TEneBoss02::isGateRiseDone()
{
	return isActionDone(ACT_GATE_RISE);
}

// 0x8005A7C8
// Sparks off the surface where the hit landed
void TEneBoss02::onLightHit()
{
	Vec3f at;
	at.x      = mContact.mSurface.x;
	at.y      = mContact.mSurface.y;
	at.z      = mContact.mSurface.z;
	int angle = toAngle(at.x - mContact.mCenter.x, at.z - mContact.mCenter.z);
	particleOnceByIdRotDefault(&at, 1, 0x40, 0, angle, 30.0f);
	particleOnceByIdRotDefault(&at, 1, 0x41, 0, angle, 30.0f);
	seStartAtFar(0x10F, &mBody.mPos);
}

// 0x8005A878
void TEneBoss02::onHeavyHit()
{
	enemyFxBurst4F(&mContact.mSurface);
	mTint.start();
	creditKiller();
	hideBossBar();
	seStartAtFar(0x118, &mBody.mPos);
}

// 0x8005A8C8
void TEneBoss02::setTurnRate(f32 rate)
{
	mTurnRate = rate;
}

// 0x8005A8D0
void TEneBoss02::setMoveRate(f32 rate)
{
	mMoveRate = rate;
}

// 0x8005A8D8
inline Vec3f boss02SubVec(const Vec3f* a, const Vec3f* b)
{
	Vec3f d;
	gjSubVector(&d, a, b);
	return d;
}

void TEneBoss02::updateBody()
{
	int view = mView;
	if (view != mViewShown) {
		f32 a = view == 0 ? gBoss02Params->mView[0][0] : gBoss02Params->mView[1][0];
		f32 b;
		f32 c;
		if (view == 0) {
			b = gBoss02Params->mView[0][1];
			c = gBoss02Params->mView[0][2];
		} else {
			b = gBoss02Params->mView[1][1];
			c = gBoss02Params->mView[1][2];
		}
		setBossView(a, b, c);
		mViewShown = mView;
	}
	if (isBlocked() == 0 && takeHits(&mHit, &mContact) != 0) {
		mKnock = boss02SubVec(&mBody.mPos, &mContact.mPos);
		if (enemyHitTestFlag(&mHit, 0xB) == 0) {
			applyDamage(&mHit);
		}
	}
	mBrain->update(this);
	clearHitFlags();
	onActionPhase((s16)mAction, 1);
	poseTail();
	updatePose();
	placeParts();
	syncCollider();
	updateGlow();
	mTint.update();
	if (mBody.mPos.y >= 610.2f) {
		shadowEnable(&mShadow);
		fitShadow(610.2f, &mBody.mPos, mParts, &mShadow);
	} else {
		shadowDisable(&mShadow);
	}
	setBossBar(mMaxHealth, mHealth);
}

// 0x8005AAB0
void TEneBoss02::vfn_6C()
{
	if (returnZero() != 0) {
		poseTail();
		beginPartPick();
		gjPushUnitMatrix();
		bodyTilt();
		poseFirstFrame(sBoss02Motion, 0x14, mPose);
		njPopMatrixEx();
		endPartPick();
		placeParts();
		syncCollider();
	}
}

// 0x8005AB38
void TEneBoss02::createBrain()
{
	mBrain = Boss02FightBrain::create(this);
}

// 0x8005AB68
static void beginPick(const int* list, Vec3f* out)
{
	sPickList = list;
	sNode     = 0;
	sPick     = 0;
	sPickOut  = out;
	sPickMtx  = 0;
}

// 0x8005AB84
static int pickDone()
{
	return (u32)sPickList[sPick] >> 31;
}

// 0x8005AB9C
// Takes the point of the current node if it is the next one picked
static int pickNode()
{
	if (sNode == sPickList[sPick]) {
		njCalcPointEx(&gVecZero, sPickOut);
		sPick++;
		sPickOut++;
		return 1;
	}
	return 0;
}

// 0x8005AC08
// The motion callback picks and keeps the matrices of the tail's nodes
static void pickCallback(NJS_OBJECT*)
{
	pickNode();
	if (sNode >= 0x41 && sNode < 0x48) {
		njMultiMatrix(0, sPickMtx);
		sPickMtx += 12;
	}
	sNode++;
}

// 0x8005AC60
void TEneBoss02::beginPartPick()
{
	beginPick(sBoss02Nodes, mParts);
	sPickMtx = mTail->mMatrices;
	gjSetMotionCallback((void (*)(NJS_OBJECT*))pickCallback);
}

// 0x8005ACB0
void TEneBoss02::endPartPick()
{
	gjSetMotionCallback(0);
}

// 0x8005ACD4
void TEneBoss02::placeParts()
{
	int i;
	gjPushUnitMatrix();
	bodyMatrix();
	for (i = 0; i < 6; i++) {
		njCalcPointEx(&mParts[i], &mParts[i]);
	}
	njPopMatrixEx();
}

// 0x8005AD34
// In a demo pose the motion moves the body so the pose is drawn back to the origin
void TEneBoss02::updatePose()
{
	if (mBossFlags & BOSS_DEMO_POSE) {
		MotionNodePose pose;
		NJS_MOTION* motion = motionCtrlGetMotion(&mAnim);
		motionGetNodePose(&pose, *(NJS_OBJECT**)mPose, motion, motionCtrlFrame(&mAnim));
		motionPoseGetPos(&pose, &mBody.mPos);
		beginPartPick();
		gjPushUnitMatrix();
		njScale(0, 2.0f, 2.0f, 2.0f);
		gjTranslate(-mBody.mPos.x, -mBody.mPos.y, -mBody.mPos.z);
		applyPose(mPose, &mAnim);
		njPopMatrixEx();
		endPartPick();
	} else {
		beginPartPick();
		gjPushUnitMatrix();
		bodyTilt();
		applyPose(mPose, &mAnim);
		njPopMatrixEx();
		endPartPick();
	}
}

// 0x8005AE28
// Finds the nodes of the tail those up to node 0x42 without flag 0x10
static int countTail(NJS_OBJECT* obj, int* index, int* out, int max);

extern "C" void initBoss02Tail()
{
	int count;
	sPick      = 0x42;
	sTailCount = 0;
	sTailNodes = 0;
	sNode      = 0;
	count      = 0;
	countTail(sBoss02Models[1], &count, 0, -1);
	sTailCount             = count + 1;
	sTailNodes             = new int[sTailCount + 1];
	sTailNodes[sTailCount] = -1;
	sNode                  = 0;
	count                  = 0;
	countTail(sBoss02Models[1], &count, sTailNodes, sTailCount);
}

// 0x8005AEC8
extern "C" void freeBoss02Tail()
{
	if (sTailNodes != 0) {
		delete[] sTailNodes;
		sTailNodes = 0;
	}
}

// 0x8005AEFC
static int countTail(NJS_OBJECT* obj, int* index, int* out, int max)
{
	int more = 1;
	do {
		if (*index < max) {
			out[*index] = sNode;
		}
		if (sNode == sPick) {
			more = 0;
		} else {
			sNode++;
			if (!(obj->evalflags & 0x10)) {
				(*index)++;
				more = countTail(obj->child, index, out, max);
				if (more != 0) {
					(*index)--;
				}
			}
		}
		obj = obj->sibling;
	} while (more != 0 && obj != 0);
	return more;
}

// 0x8005AFC0
// Poses the body then bends each link of the tail from the one before
void TEneBoss02::poseTail()
{
	int nodes[3] = { 0x41, 0x42, -1 };
	Vec3f pts[2];
	f32 mtx[12];
	Vec3f prev;
	Vec3f d;
	Vec3f axis;
	f32 quat[4];
	GXQuaternion q;
	Vec3f* links;
	f32* mats;
	int index;
	int angle;
	int i;

	gjPushUnitMatrix();
	sNode     = 0;
	sPick     = 0;
	sPickList = nodes;
	sPickOut  = pts;
	sPickMtx  = mtx;
	index     = 0;
	bodyMatrix();
	bodyTilt();
	if (motionCtrlIsBlending(&mAnim) != 0) {
		f32 frame = motionCtrlBlendTime(&mAnim);
		njStartMotionLink((const NJS_MOTION_LINK*)motionCtrlGetBlend(&mAnim), frame);
		mTransform = njMotionLinkTransformEx;
		mNextNode  = njSetNextMotionLinkNode;
	} else {
		f32 frame = motionCtrlFrame(&mAnim);
		njStartMotionObj(motionCtrlGetMotion(&mAnim), frame);
		mTransform = njMotionTransformEx;
		mNextNode  = njSetNextMotionNodeEx;
	}
	walkTail(sBoss02Models[1], &index);
	njPopMatrixEx();
	mTail->update(pts, &mTip);
	njInvertMatrix(mtx);
	links = mTail->mPoints;
	mats  = mTail->mMatrices;
	njPushMatrix(mtx);
	prev = pts[1];
	njSubVector(&prev, &pts[0]);
	njCalcVector((f32*)0, (const Vec3f*)(const NJS_VECTOR*)&prev, (Vec3f*)(NJS_VECTOR*)&prev);
	if (njUnitVector(&prev) < gUnitVectorMin) {
		prev = gVecZero;
	}
	for (i = 0; i < 7; i++) {
		d = links[i + 1];
		njSubVector(&d, &links[i]);
		njCalcVector((f32*)0, (const Vec3f*)(const NJS_VECTOR*)&d, (Vec3f*)(NJS_VECTOR*)&d);
		if (njUnitVector(&d) < gUnitVectorMin) {
			d = gVecZero;
		}
		angle = (int)(10430.38043493439 * (f32)acos(njInnerProduct(&prev, &d)));
		njOuterProduct(&prev, &d, &axis);
		if (njUnitVector(&axis) < gUnitVectorMin) {
			axis = gVecZero;
		}
		njCreateQuaternion(angle, axis.x, axis.y, axis.z, (NJS_QUATERNION*)quat);
		q.w = quat[0];
		q.x = quat[1];
		q.y = quat[2];
		q.z = quat[3];
		PSMTXQuat(mats, &q);
		mats += 12;
		prev = d;
	}
	njPopMatrixEx();
}

// 0x8005B2D4
// Transforms the tail's nodes and skips the rest keeping the tip's matrix
int TEneBoss02::walkTail(NJS_OBJECT* obj, int* index)
{
	int more;
	do {
		if (sNode == sTailNodes[*index]) {
			(*index)++;
			mTransform(obj);
		} else {
			mNextNode();
		}
		if (pickNode() != 0 && sPick == 1) {
			njCalcVectorEx(&mTipLocal, &mTip);
			njGetMatrix(sPickMtx);
		}
		sNode++;
		more = pickDone() == 0;
		if (more && obj->child != 0) {
			more = walkTail(obj->child, index);
		}
		obj = obj->sibling;
	} while (more && obj != 0);
	return more;
}

// 0x8005B3BC
void TEneBoss02::armBite()
{
	mBossFlags |= BOSS_BITE;
	setShape(sBoss02BiteShapes);
}

// 0x8005B3F0
void TEneBoss02::armSlam()
{
	mBossFlags |= BOSS_SLAM;
	setShape(sBoss02SlamShapes);
}

// 0x8005B424
void TEneBoss02::disarm()
{
	mBossFlags &= ~(BOSS_BITE | BOSS_SLAM);
}

// 0x8005B434
// Takes the size of the attack shape from the table
void TEneBoss02::setShape(const void* shapes)
{
	f32* shape = (f32*)colliGetShape(mAttack, 0);
	shape[6]   = ((const f32*)shapes)[6];
	shape[7]   = ((const f32*)shapes)[7];
}

// 0x8005B478
void TEneBoss02::floatHigh()
{
	mBody.mPos.y = 36.0f + mBaseY;
}

// 0x8005B48C
void TEneBoss02::floatAt()
{
	mBody.mPos.y = mBaseY;
}

// 0x8005B498
void TEneBoss02::sink()
{
	mBody.mPos.y = -200.0f + mBaseY;
}

// 0x8005B4AC
void TEneBoss02::setLayer(int layer)
{
	if (mLayer != layer) {
		mLayer = layer;
		moveDrawTask(mDrawTask, (u16)layer);
	}
}

// 0x8005B4E4
void TEneBoss02::layerGlow()
{
	setLayer(0x2A);
}

// 0x8005B508
void TEneBoss02::layerNormal()
{
	setLayer(0x21);
}

// 0x8005B52C
void TEneBoss02::layerSuit()
{
	setLayer(0x3A);
}

// 0x8005B550
void TEneBoss02::glowOff()
{
	mBossFlags &= ~BOSS_GLOW;
	mGlowState = 2;
	mGlow      = 0.0f;
	mGlowWave  = 0.0f;
	mGlowAngle = 0;
	if (mGlowFx != 0) {
		mGlowFx->mFlags |= 1;
		mGlowFx = 0;
	}
}

// 0x8005B598
void TEneBoss02::glowOn()
{
	mBossFlags |= BOSS_GLOW;
	mGlowState = 0;
	mGlow      = 0.8f;
	mGlowWave  = 0.0f;
	mGlowAngle = 0;
	keepGlowFx();
}

// 0x8005B5E0
void TEneBoss02::keepGlowFx()
{
	if (mGlowFx == 0) {
		mGlowFx = particleGenByIdDefault(&mParts[2], 1, 0x45);
	}
}

// 0x8005B628
void TEneBoss02::glowFlash()
{
	mGlowState = 1;
	keepGlowFx();
	seStartAtFar(0x117, &mBody.mPos);
}

// 0x8005B668
void TEneBoss02::glowFade()
{
	mGlowState = 3;
	if (mGlowFx != 0) {
		mGlowFx->mFlags |= 1;
		mGlowFx = 0;
	}
}

// 0x8005B694
void TEneBoss02::updateGlow()
{
	switch (mGlowState) {
	case 0:
		break;
	case 1:
		mGlow += 0.02f;
		if (mGlow >= 0.8f) {
			mGlow      = 0.8f;
			mGlowState = 0;
		}
		break;
	case 2:
		break;
	case 3:
		mGlow -= 0.02f;
		if (mGlow <= 0.0f) {
			mGlow      = 0.0f;
			mGlowState = 2;
		}
		break;
	}
	if (mGlow > 0.0f) {
		mBossFlags |= BOSS_GLOW;
	} else {
		mBossFlags &= ~BOSS_GLOW;
	}
	mGlowAngle += 0x300;
	mGlowWave = 0.4f * njSin(mGlowAngle);
	if (mGlowFx != 0) {
		mGlowFx->mPos = mParts[2];
	}
}

// 0x8005B7BC
// The velocity to cover from to to in time frames flat
void TEneBoss02::startMove(f32 time, const Vec3f* from, const Vec3f* to)
{
	mBody.mVel = *to;
	njSubVector(&mBody.mVel, from);
	mBody.mVel.y = 0.0f;
	gjScaleVector(1.0f / time, &mBody.mVel, &mBody.mVel);
	mMoveFrom = *from;
}

// 0x8005B850
f32 TEneBoss02::swim(f32 time, f32 rate, f32 phase)
{
	f32 dy       = jumpHeightAt(time, rate, phase);
	mBody.mPos.y = mBaseY + dy;
	return dy;
}

// 0x8005B888
// Moves along the velocity from the start keeping the height
void TEneBoss02::moveStep(f32 time)
{
	f32 y = mBody.mPos.y;
	gjMulAddVector(time, &mBody.mPos, &mMoveFrom, &mBody.mVel);
	mBody.mPos.y = y;
}

// 0x8005B8D8
// Slows down past end
void TEneBoss02::moveEase(f32 time, f32 end, f32 rate)
{
	moveStep(time < end ? time : (time -= end, time -= time * (0.5f * rate * time), time += end));
}

// 0x8005B91C
// The turn from the facing to the velocity a step for each frame of time
void TEneBoss02::startTurn(f32 x, f32 z, f32 time)
{
	int from  = mBody.mRot.y;
	mTurnFrom = from;
	mTurnStep = angleDiff(from, toAngle(x, z)) / (int)time;
}

// 0x8005B998
void TEneBoss02::turnStep(f32 time)
{
	mBody.mRot.y = mTurnFrom + mTurnStep * (int)time;
}

// 0x8005B9C4
void TEneBoss02::setOpen()
{
	mBossFlags |= BOSS_OPEN;
	mBossFlags &= ~BOSS_GUARDED;
}

// 0x8005B9E0
void TEneBoss02::setNeutral()
{
	mBossFlags &= ~(BOSS_OPEN | BOSS_GUARDED);
}

// 0x8005B9F0
void TEneBoss02::setGuarded()
{
	mBossFlags &= ~BOSS_OPEN;
	mBossFlags |= BOSS_GUARDED;
}

// 0x8005BA0C
void TEneBoss02::onShoot()
{
}

// 0x8005BA10
void TEneBoss02::shotFx()
{
	particleOnceByIdDefault(&mParts[3], 1, 0x48);
	particleOnceByIdDefault(&mParts[3], 1, 0x49);
	particleOnceByIdDefault(&mParts[3], 1, 0x4C);
	seStartAtFar(0x114, &mParts[3]);
}

// 0x8005BA74
void TEneBoss02::endSplash()
{
	mBossFlags &= ~BOSS_SPLASHED;
}

// 0x8005BA84
// Once a dive
void TEneBoss02::splash()
{
	if (!(mBossFlags & BOSS_SPLASHED)) {
		mBossFlags |= BOSS_SPLASHED;
		particleOnceByIdDefault(&mTarget, 1, 0x43);
		particleOnceByIdDefault(&mTarget, 1, 0x4D);
		noOp__9TObjEnemyFv(4);
		seStartAtFar(0x112, &mBody.mPos);
	}
}

// 0x8005BAF4
void TEneBoss02::splashSoft()
{
	noOp__9TObjEnemyFv(4);
	seStartAtFar(0x113, &mBody.mPos);
}

// 0x8005BB30
void TEneBoss02::keepTrail()
{
	particleKeepAt((void**)&mTrail, (const Vec3f*)&mParts[0], 1, 0x42);
}

// 0x8005BB64
void TEneBoss02::dropTrail()
{
	if (mTrail != 0) {
		mTrail->mFlags |= 1;
		mTrail = 0;
	}
}

// 0x8005BB88
// A ripple dist ahead once for each flag
void TEneBoss02::ripple(f32 dist, u32 flag)
{
	if (!(mBossFlags & flag)) {
		Vec3f at;
		njSinCos(mBody.mRot.y, &at.x, &at.z);
		at.y = 0.0f;
		gjMulAddVector(dist, &at, &mBody.mPos, &at);
		particleOnceByIdDefault(&at, 1, 0x4A);
		fxSpawnFlat(5, (void*)1, &at);
		seStartAtFar(0x111, &mBody.mPos);
	}
}

// 0x8005BC28
void TEneBoss02::rippleBite()
{
	ripple(10.0f, BOSS_BITE);
}

// 0x8005BC50
void TEneBoss02::rippleSlam()
{
	ripple(30.0f, BOSS_SLAM);
}

// 0x8005BC78
void TEneBoss02::onActionPhase(int action, int phase)
{
	switch (action) {
	case ACT_APPEAR:
		actAppear(phase);
		break;
	case ACT_RISE:
		actRise(phase);
		break;
	case ACT_TURN:
		actTurn(phase);
		break;
	case ACT_WAIT:
		actWait(phase);
		break;
	case ACT_SHOOT:
		actShoot(phase);
		break;
	case ACT_SHOOT_GATE:
		actShootGate(phase);
		break;
	case ACT_ROAR:
		actRoar(phase);
		break;
	case ACT_STUN:
		actStun(phase);
		break;
	case ACT_SPIT:
		actSpit(phase);
		break;
	case ACT_DIVE:
	case ACT_DIVE_DEEP:
		actDive(phase, action);
		break;
	case ACT_LEAP:
		actLeap(phase);
		break;
	case ACT_SWIM:
		actSwim(phase);
		break;
	case ACT_SURFACE:
		actSurface(phase);
		break;
	case ACT_GATE_RISE:
		actGateRise(phase);
		break;
	case ACT_DIE:
		actDie(phase);
		break;
	}
}

// 0x8005BD6C
// Rises out of the middle in its demo roars and settles
void TEneBoss02::actAppear(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0x13);
		mBody.mPos.x = 0.0f;
		mBody.mPos.y = 610.2f;
		mBody.mPos.z = 0.0f;
		mBody.mRot.y = 0;
		mBossFlags |= BOSS_SHOWN | BOSS_DEMO_POSE;
		setCantAct();
		glowOn();
		clearFrameKey();
		mDemo  = 0;
		mCount = 0;
		mStep  = 0;
		break;
	case 1: {
		f32 frame;
		if (mCount == 0) {
			mCount = 1;
			playDemo(1, 1);
			new TEneBoss02_GateShot(this, *resBankGetMotion(&sBoss02Bank, 2), 0.0f, 1.0f);
		}
		lockOn();
		motionCtrlUpdate(&mAnim);
		waitMotion();
		frame = motionCtrlFrame(&mAnim);
		switch (mCount) {
		case 1:
			if (frame > 152.0f) {
				mCount = 2;
				seStartArg(0x2A4, 0x189);
			}
			break;
		case 2:
			if (frame > 158.0f) {
				mCount = 3;
			}
			break;
		case 3:
			if (frame > 265.0f) {
				mCount = 4;
			}
			mStep++;
			break;
		case 4:
			if (frame > 300.0f) {
				glowFade();
				mCount = 5;
			}
			break;
		case 5:
			motionCtrlPassed(355.0f, &mAnim);
			if (frame > 420.0f) {
				glowFlash();
				mCount = 6;
			}
			break;
		}
		break;
	}
	case 2:
		mBaseY       = mTarget.y;
		mBody.mPos.x = mTarget.x;
		mBody.mPos.y = mTarget.y;
		mBody.mPos.z = mTarget.z;
		floatHigh();
		mBossFlags &= ~(BOSS_SHOWN | BOSS_DEMO_POSE);
		clearCantAct();
		if (mDemo != 0) {
			mDemo->mFlags |= 1;
			mDemo = 0;
		}
		camSetLookFrom(0, &mBody.mPos);
		unlock();
		break;
	}
}

// 0x8005BFF4
// Bobs up out of the water and settles
void TEneBoss02::actRise(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 1);
		motionCtrlSetSpeed(&mAnim, mTurnRate);
		mStep = 0;
		floatHigh();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (mStep == 0) {
			f32 frame = motionCtrlFrame(&mAnim);
			f32 dy;
			if (frame < 5.0f) {
				dy = 36.0f;
			} else if (frame < 9.0f) {
				// A parabola reaching 46 at frame 9 10 units over the 4 frames
				f32 t = 9.0f - frame;
				f32 d = 4.0f;
				d *= d;
				t *= t;
				dy = 46.0f - (10.0f / d) * t;
			} else if (frame < 12.0f) {
				// Down from 46 to 0 over the next 3 frames
				f32 t = frame - 9.0f;
				f32 d = 3.0f;
				d *= d;
				t *= t;
				dy = 46.0f - (46.0f / d) * t;
			} else {
				dy    = 0.0f;
				mStep = 1;
				seStartAtFar(0x111, &mBody.mPos);
			}
			mBody.mPos.y = mBaseY + dy;
		} else {
			finishMotion();
		}
		break;
	case 2:
		floatAt();
		motionCtrlSetSpeed(&mAnim, 1.0f);
		break;
	}
}

// 0x8005C14C
// Fast between frames 4 and 10 and slowing down outside them
int turnToward(f32 frame, f32 rate, int target, int* angle, int* step)
{
	u8 done;
	if (frame > 4.0f && frame <= 10.0f) {
		*step = (int)(352.0f * rate);
	} else {
		f32 s = *step;
		s *= 0.95f;
		*step = (int)s;
		if (*step < 8) {
			*step = 8;
		}
	}
	*angle = (u16)angleApproach(*angle, target, *step);
	done   = 0;
	if (*angle == target && frame >= 17.5f) {
		done = 1;
	}
	return done;
}

// 0x8005C250
// Turns so the head points at the aim biting the water on the way
void TEneBoss02::actTurn(int phase)
{
	int* speed = &mTurnSpeed;
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 4);
		motionCtrlSetSpeed(&mAnim, mTurnRate);
		floatAt();
		mBossFlags &= ~(BOSS_FACING | BOSS_CLOSE);
		disarm();
		clearFrameKey();
		*speed = 0;
		break;
	case 1: {
		Vec3f d;
		f32 distSq;
		int target;
		int angle;
		f32 frame;
		motionCtrlUpdate(&mAnim);
		d = mAim;
		njSubVector(&d, &mBody.mPos);
		d.y    = 0.0f;
		distSq = njScalor2(&d);
		target = toAngle(d.x, d.z) & 0xFFFF;
		if (distSq < 10000.0f) {
			mBossFlags |= BOSS_CLOSE;
		} else {
			f32 t0;
			f32 t1;
			f32 bx;
			f32 bz;
			f32 dx;
			f32 dz;
			mBossFlags &= ~BOSS_CLOSE;
			bz = mNeck.z;
			dz = mNeckDir.z;
			bx = mNeck.x;
			dx = mNeckDir.x;
			quadRoots(1.0f, 2.0f * (bx * dx + bz * dz), (bx * bx + bz * bz) - distSq, &t0, &t1);
			target = angleDiff(toAngle(dx * t0 + bx, dz * t0 + bz), target) & 0xFFFF;
		}
		frame = motionCtrlFrame(&mAnim);
		angle = mBody.mRot.y;
		if (turnToward(frame, mTurnRate, target, &angle, speed) != 0) {
			mBossFlags |= BOSS_FACING;
		} else {
			mBossFlags &= ~BOSS_FACING;
		}
		mBody.mRot.y = angle;
		if (frame > 10.0f && frame < 15.0f) {
			rippleBite();
			armBite();
		} else {
			disarm();
		}
		break;
	}
	case 2:
		mBossFlags &= ~(BOSS_FACING | BOSS_CLOSE);
		disarm();
		motionCtrlSetSpeed(&mAnim, 1.0f);
		break;
	}
}

// 0x8005C4E0
int atShotFrame(void* anim)
{
	return motionCtrlPassed(4.0f, anim);
}

// 0x8005C504
// Spits a water shot at the aim
void TEneBoss02::actShoot(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 2);
		motionCtrlSetSpeed(&mAnim, mTurnRate);
		floatAt();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		setFrameKey(sBoss02FrameKeys[1]);
		if (motionCtrlPassed(1.0f, &mAnim) != 0) {
			onShoot();
		} else if (atShotFrame(&mAnim) != 0) {
			createWaterShot(this, &mParts[3], &mAim);
			shotFx();
		}
		finishMotion();
		break;
	case 2:
		motionCtrlSetSpeed(&mAnim, 1.0f);
		clearFrameKey();
		break;
	}
}

// 0x8005C5F8
// Spits a shot that opens a gate at the aim's slot
void TEneBoss02::actShootGate(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 2);
		motionCtrlSetSpeed(&mAnim, mTurnRate);
		floatAt();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		setFrameKey(sBoss02FrameKeys[1]);
		if (atShotFrame(&mAnim) != 0) {
			createWaterShotGate(this, &mParts[3], &mAim, mAimSlot);
			shotFx();
		}
		finishMotion();
		break;
	case 2:
		motionCtrlSetSpeed(&mAnim, 1.0f);
		clearFrameKey();
		break;
	}
}

// 0x8005C6D0
void TEneBoss02::actWait(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 3);
		motionCtrlSetSpeed(&mAnim, mTurnRate);
		floatAt();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		break;
	case 2:
		motionCtrlSetSpeed(&mAnim, 1.0f);
		break;
	}
}

// 0x8005C758
void TEneBoss02::actRoar(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xE);
		floatAt();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		finishMotion();
		break;
	case 2:
		break;
	}
}

// 0x8005C7C4
void TEneBoss02::actStun(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xD);
		floatAt();
		mFrameKey = 4;
		seStartAtFar(0x110, &mBody.mPos);
		mStep = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		mStep++;
		break;
	case 2:
		clearFrameKey();
		break;
	}
}

// 0x8005C860
void TEneBoss02::actSpit(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xF);
		floatAt();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		setFrameKey(sBoss02FrameKeys[2]);
		finishMotion();
		break;
	case 2:
		clearFrameKey();
		break;
	}
}

// 0x8005C8F0
// The script of a dive the motion then 1 for its length then 10
void TEneBoss02::planDive(f32 height, int motion, f32* time, f32* travel, f32* end, f32* tilt, Boss02Key* keys)
{
	const MotionEntry* entry = &sBoss02Motions[motion];
	f32 phase;
	f32 length;
	f32 swim;

	*time          = -motionEntryTimeAt(entry, height);
	length         = *time + motionEntryLength(entry);
	phase          = jumpSpeed(0.12f, 120.0f);
	swim           = jumpRiseTime(0.12f, phase);
	*travel        = 2.0f * swim;
	*end           = *travel;
	*tilt          = swim - 42.666668f;
	mSwimPhase     = phase;
	keys[0].mKind  = 0;
	keys[0].mValue = *time;
	keys[1].mKind  = motion;
	keys[1].mValue = 0.0f;
	keys[2].mKind  = 1;
	keys[2].mValue = length;
	keys[3].mKind  = 10;
	keys[3].mValue = 0.0f;
	keys[4].mKind  = 3;
}

// 0x8005CA00
// Dives through the water toward the target and under
void TEneBoss02::actDive(int phase, int action)
{
	f32* time = &mTime;
	f32* end  = &mTime1;
	f32* tilt = &mTime2;
	switch (phase) {
	case 0: {
		f32 travel;
		mBody.mRot.x = 0;
		mBossFlags &= ~BOSS_TILTED;
		endSplash();
		clearFrameKey();
		clearCantAct();
		mStep = 0;
		f32 speed;
		if (action == ACT_DIVE) {
			mCount = 9;
			speed  = 12.0f;
		} else {
			mCount = 8;
			speed  = 12.0f;
		}
		planDive(speed, mCount, time, &travel, end, tilt, mKeys);
		startMove(travel, &mBody.mPos, &mTarget);
		startTurn(mBody.mVel.x, mBody.mVel.z, travel);
		break;
	}
	case 1:
		motionKeyPlay((const MotionEntry*)sBoss02Motions, (MotionKey*)mKeys, &mBossMotion, *time, 0.1f);
		if (!(*time < 0.0f)) {
			f32 dy;
			f32 over;
			mView = 1;
			dy    = swim(*time, 0.12f, mSwimPhase);
			moveEase(*time, *end, 0.0005f);
			if (dy < 0.0f) {
				if (!(mBossFlags & BOSS_SPLASHED)) {
					createPoolPowGen(this, &mBody.mPos, 0x1E);
				}
				splash();
			}
			if (dy < -200.0f) {
				mBody.mVel.x = 0.0f;
				mBody.mVel.y = 0.0f;
				mBody.mVel.z = 0.0f;
				mHitFlags |= HIT_DONE;
			}
			over = *time - *tilt;
			if (over > 0.0f) {
				int x;
				mBossFlags |= BOSS_TILTED;
				x = (int)(-384.0f * over);
				if (x < -0x8000) {
					*tilt = 100000.0f;
					x     = -0x8000;
				}
				mBody.mRot.x = x;
			}
			if (*time < *end) {
				turnStep(*time);
			}
		}
		*time += mMoveRate;
		if (mBody.mPos.y <= 610.2f) {
			setCantAct();
		}
		break;
	case 2:
		sink();
		mBody.mVel.x = 0.0f;
		mBody.mVel.y = 0.0f;
		mBody.mVel.z = 0.0f;
		mBody.mRot.x = 0;
		mBossFlags &= ~BOSS_TILTED;
		clearCantAct();
		break;
	}
}

// 0x8005CC8C
// The script of a leap up through the gate over and down into the water
void TEneBoss02::planLeap(f32* time, f32* travel, f32* top, f32* fall, f32* land, Boss02Key* keys)
{
	const MotionEntry* rise;
	const MotionEntry* drop;
	f32 blend;
	f32 riseStart;
	f32 riseLength;
	f32 dropStart;
	f32 dropLength;
	f32 swim;
	f32 a;
	f32 b;
	f32 start;

	blend      = motionBlendLength(0.2f);
	rise       = &sBoss02Motions[11];
	riseStart  = motionEntryTimeAt(rise, 10.0f);
	riseLength = motionEntryLength(rise);
	drop       = &sBoss02Motions[5];
	dropStart  = motionEntryTimeAt(drop, 4.0f);
	dropLength = motionEntryLength(drop);
	mSwimPhase = jumpSpeed(0.12f, 150.0f);
	*travel    = jumpRiseTime(0.12f, mSwimPhase);
	jumpTimesAt(0.12f, mSwimPhase, -200.0f, &a, &b);
	*time           = b;
	start           = *travel - riseStart;
	*top            = start + riseLength;
	*fall           = 2.0f * *travel;
	*fall           = *fall + (*top - *travel);
	*land           = *fall + dropLength - dropStart;
	keys[0].mKind   = 1;
	keys[0].mValue  = *time;
	keys[1].mKind   = 10;
	keys[1].mValue  = 0.0f;
	keys[2].mKind   = 2;
	keys[2].mValue  = start - blend;
	keys[3].mKind   = 11;
	keys[3].mValue  = 0.2f;
	keys[4].mKind   = 0;
	keys[4].mValue  = start;
	keys[5].mKind   = 11;
	keys[5].mValue  = 0.0f;
	keys[6].mKind   = 0;
	keys[6].mValue  = *top;
	keys[7].mKind   = 7;
	keys[7].mValue  = 0.0f;
	keys[8].mKind   = 2;
	keys[8].mValue  = *fall - blend - dropStart;
	keys[9].mKind   = 5;
	keys[9].mValue  = 0.2f;
	keys[10].mKind  = 0;
	keys[10].mValue = *fall - dropStart;
	keys[11].mKind  = 5;
	keys[11].mValue = 0.0f;
	keys[12].mKind  = 3;
}

// 0x8005CEAC
// Leaps out of a gate onto the aim and slams down
void TEneBoss02::actLeap(int phase)
{
	f32* time   = &mTime;
	f32* travel = &mTime1;
	f32* top    = &mTime2;
	f32* fall   = &mTime3;
	f32* land   = &mTime4;
	switch (phase) {
	case 0:
		disarm();
		endSplash();
		clearFrameKey();
		setCantAct();
		mStep = 0;
		planLeap(time, travel, top, fall, land, mKeys);
		mTime5 = *fall - 6.0f;
		sink();
		mBody.mPos.x = mTarget.x;
		mBody.mPos.z = mTarget.z;
		startMove(1.0f, &mBody.mPos, &mAim);
		mBody.mRot.y = toAngle(mBody.mVel.x, mBody.mVel.z);
		createPoolPowGen(this, &mTarget, (int)(f32)__fabs(*time / mMoveRate) + 10);
		break;
	case 1: {
		int again;
		motionKeyPlay((const MotionEntry*)sBoss02Motions, (MotionKey*)mKeys, &mBossMotion, *time, 0.2f);
		do {
			again = 0;
			switch (mStep) {
			case 0:
				if (*time >= 0.0f) {
					startMove(*fall, &mBody.mPos, &mAim);
					startTurn(mBody.mVel.x, mBody.mVel.z, *fall);
					splashSoft();
					mStep = 1;
					clearCantAct();
					again = 1;
				} else {
					swim(*time, 0.12f, mSwimPhase);
					if (mBody.mPos.y >= 560.2f) {
						splash();
					}
				}
				break;
			case 1:
				if (*time >= *travel) {
					mStep = 2;
					again = 1;
				} else {
					moveStep(*time);
					swim(*time, 0.12f, mSwimPhase);
				}
				turnStep(*time);
				keepTrail();
				break;
			case 2:
				if (*time >= *top) {
					mStep = 3;
					again = 1;
				} else {
					moveStep(*time);
					turnStep(*time);
				}
				break;
			case 3:
				if (*time >= mTime5) {
					mTime5 = *fall;
					noOp__9TObjEnemyFv(2);
				}
				if (*time >= *fall) {
					mStep = 4;
					again = 1;
					floatAt();
					rippleSlam();
					armSlam();
					mFrameKey = 3;
					createPoolPow(&mBody.mPos, 10.0f, 60.0f, 30.0f);
				} else {
					swim(*travel + (*time - *top), 0.12f, mSwimPhase);
					moveStep(*time);
					turnStep(*time);
				}
				break;
			case 4:
				if (*time >= *land) {
					mHitFlags |= HIT_DONE;
				}
				break;
			}
		} while (again != 0);
		*time += mMoveRate;
		break;
	}
	case 2:
		floatAt();
		mBody.mVel.x = 0.0f;
		mBody.mVel.y = 0.0f;
		mBody.mVel.z = 0.0f;
		mBody.mRot.z = 0;
		disarm();
		dropTrail();
		clearFrameKey();
		clearCantAct();
		break;
	}
}

// 0x8005D288
void TEneBoss02::actSwim(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.03f, &mAnim, 0x10);
		motionCtrlSetSpeed(&mAnim, mMoveRate);
		floatAt();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		break;
	case 2:
		motionCtrlSetSpeed(&mAnim, 1.0f);
		break;
	}
}

// 0x8005D314
void TEneBoss02::actSurface(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0x11);
		motionCtrlSetSpeed(&mAnim, mMoveRate);
		floatAt();
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		finishMotion();
		setFrameKey(sBoss02FrameKeys[2]);
		break;
	case 2:
		motionCtrlSetSpeed(&mAnim, 1.0f);
		clearFrameKey();
		break;
	}
}

// 0x8005D3C8
// The script of rising through a gate onto the ring
void TEneBoss02::planGateRise(f32* time, f32* travel, Boss02Key* keys)
{
	f32 a;
	f32 b;
	f32 blend;
	f32 length;

	mSwimPhase = jumpSpeed(0.12f, 150.0f);
	jumpTimesAt(0.12f, mSwimPhase, 36.0f, &a, &b);
	*travel = a;
	blend   = motionBlendLength(0.2f);
	length  = motionEntryLength(&sBoss02Motions[12]);
	jumpTimesAt(0.12f, mSwimPhase, -200.0f, &a, &b);
	*time          = b;
	keys[0].mKind  = 1;
	keys[0].mValue = -20.0f;
	keys[1].mKind  = 10;
	keys[1].mValue = 0.0f;
	keys[2].mKind  = 2;
	keys[2].mValue = *travel - length - blend;
	keys[3].mKind  = 12;
	keys[3].mValue = 0.2f;
	keys[4].mKind  = 0;
	keys[4].mValue = *travel - length;
	keys[5].mKind  = 12;
	keys[5].mValue = 0.0f;
	keys[6].mKind  = 2;
	keys[6].mValue = *travel;
	keys[7].mKind  = 0;
	keys[7].mValue = 0.2f;
	keys[8].mKind  = 1;
	keys[8].mValue = *travel + blend;
	keys[9].mKind  = 0;
	keys[9].mValue = 0.0f;
	keys[10].mKind = 3;
}

// 0x8005D524
// Rises out of a gate and swims to the aim closing the gates
void TEneBoss02::actGateRise(int phase)
{
	f32* time   = &mTime;
	f32* travel = &mTime1;
	switch (phase) {
	case 0:
		endSplash();
		clearFrameKey();
		mStep = 0;
		planGateRise(time, travel, mKeys);
		sink();
		mBody.mPos.x = mTarget.x;
		mBody.mPos.z = mTarget.z;
		startMove(*travel, &mBody.mPos, &mAim);
		mBody.mRot.y = toAngle(mBody.mVel.x, mBody.mVel.z);
		startTurn(-mAim.x, -mAim.z, *travel);
		break;
	case 1: {
		int again;
		motionKeyPlay((const MotionEntry*)sBoss02Motions, (MotionKey*)mKeys, &mBossMotion, *time, 0.2f);
		do {
			again = 0;
			switch (mStep) {
			case 0:
				if (*time >= 0.0f) {
					splashSoft();
					mStep = 1;
					again = 1;
				} else {
					swim(*time, 0.12f, mSwimPhase);
					if (mBody.mPos.y >= 560.2f) {
						splash();
					}
				}
				break;
			case 1:
				if (*time >= *travel) {
					mStep = 2;
					closeBossGates();
					mHitFlags |= HIT_DONE;
					mView = 0;
				} else {
					swim(*time, 0.12f, mSwimPhase);
					moveStep(*time);
					turnStep(*time);
					keepTrail();
				}
				break;
			case 2:
				break;
			}
		} while (again != 0);
		*time += mMoveRate;
		break;
	}
	case 2:
		floatAt();
		mBody.mVel.x = 0.0f;
		mBody.mVel.y = 0.0f;
		mBody.mVel.z = 0.0f;
		closeBossGates();
		dropTrail();
		mView = 0;
		break;
	}
}

// 0x8005D75C
// Waits 18 frames then plays the death demo in its suit and goes at frame 480
void TEneBoss02::actDie(int phase)
{
	switch (phase) {
	case 0:
		mStep  = 0;
		mCount = 18;
		mTint.start();
		mEnemyFlags |= ENEMY_BLOCKED;
		colliDisable(mCollider);
		break;
	case 1:
		if (mStep == 0) {
			mCount--;
			if (mCount >= 0) {
				break;
			}
			motionCtrlPlay(&mAnim, 0x12);
			motionCtrlSetSpeed(&mAnim, 1.0f);
			floatHigh();
			glowOff();
			mFrameKey = 3;
			if (mTrail != 0) {
				mTrail->mFlags |= 1;
				mTrail = 0;
			}
			mBody.mPos.x = 0.0f;
			mBody.mPos.y = 610.2f;
			mBody.mPos.z = 0.0f;
			mBody.mRot.y = 0;
			mBossFlags &= ~BOSS_SUIT;
			mBossFlags |= BOSS_SHOWN | BOSS_DEMO_POSE;
			shadowDisable(&mShadow);
			mStep = 1;
			mTime = 0.0f;
			playDemo(0, 0);
			bossSuitSetFade(gBoss02Params->mSuitA, gBoss02Params->mSuitB, &mSuit);
			bossSuitSetSink(gBoss02Params->mSuitC, gBoss02Params->mSuitD, gBoss02Params->mSuitE,
			                gBoss02Params->mSuitTop - gBoss02Params->mSuitBase, gBoss02Params->mSuitBase, &mSuit);
			bossSuitSetFx(&mSuit, 1, 0x53, 0x52);
			seStartArg(0x2A5, 0x1E5);
			seCueSet(&mCue, 0x69, 0xA5);
		}
		motionCtrlUpdate(&mAnim);
		finishMotion();
		lockOn();
		switch (mStep) {
		case 1: {
			Vec3f at;
			at.x = mBody.mPos.x;
			at.y = mBody.mPos.y;
			at.z = mBody.mPos.z;
			at.y = 610.2f;
			if (motionCtrlPassed(100.0f, &mAnim) != 0) {
				particleOnceByIdDefault(&at, 1, 0x4A);
			} else if (motionCtrlPassed(115.0f, &mAnim) != 0) {
				particleOnceByIdDefault(&at, 1, 0x4A);
			}
			if (motionCtrlIsDone(&mAnim) != 0) {
				mStep = 2;
			}
			mTime = motionCtrlFrame(&mAnim);
			break;
		}
		case 2:
			mTime += 1.0f;
			break;
		}
		if (bossSuitUpdate(mTime, &mSuit, &mBody.mPos, sBoss02Models[2]) != 0) {
			mBossFlags |= BOSS_SUIT;
		} else {
			mBossFlags &= ~BOSS_SUIT;
		}
		seCueUpdate(&mCue, (int)mTime);
		if (mTime >= 480.0f) {
			Vec3f center;
			slain();
			sendBossDefeated(this);
			if (mDemo != 0) {
				((TCamera*)mDemo)->release();
			}
			if (mDemo != 0) {
				mDemo->mFlags |= 1;
				mDemo = 0;
			}
			center.x = 0.0f;
			center.y = 610.2f;
			center.z = 0.0f;
			camSetLookFrom(0, &center);
			mFlags |= 1;
		}
		break;
	case 2:
		motionCtrlSetSpeed(&mAnim, 1.0f);
		clearFrameKey();
		if (mTrail != 0) {
			mTrail->mFlags |= 1;
			mTrail = 0;
		}
		mBossFlags &= ~(BOSS_SHOWN | BOSS_DEMO_POSE);
		if (mDemo != 0) {
			mDemo->mFlags |= 1;
			mDemo = 0;
		}
		unlock();
		break;
	}
}

// 0x8005DB2C
void TEneBoss02::vfn_3C()
{
}

// 0x8005DB30
// Hidden off screen unless shown
void TEneBoss02::vfn_10()
{
	if (!(mBossFlags & BOSS_SHOWN)) {
		if (isAppearing() != 0) {
			mFlags |= 0x10;
			return;
		}
		if (isInView(1000.0f, 70.0f, &mBody.mPos, 0) == 0) {
			mFlags |= 0x10;
			return;
		}
	}
	mFlags &= ~0x10;
	if (mBossFlags & BOSS_GLOW) {
		layerGlow();
	} else if (mBossFlags & BOSS_SUIT) {
		layerSuit();
	} else {
		layerNormal();
	}
}

// 0x8005DBFC
void TEneBoss02::bodyMatrix()
{
	njTranslateEx(&mBody.mPos);
	gjRotateY(mBody.mRot.y);
}

// 0x8005DC34
void tiltMatrix(int angle)
{
	gjTranslate(0.0f, 30.0f, 0.0f);
	njScale(0, 2.0f, 2.0f, 2.0f);
	gjRotateX(angle);
}

// 0x8005DC88
void TEneBoss02::bodyTilt()
{
	tiltMatrix(mBody.mRot.x);
}

struct Boss02Color {
	u8 r;
	u8 g;
	u8 b;
	u8 a;
};

// 0x8005DCAC
void TEneBoss02::render()
{
	int glow = mGlow > 0.0f;
	int suit = mBossFlags & BOSS_SUIT;
	if (glow) {
		beginGlow();
	} else if (suit) {
		beginSuit();
	}
	njSetTexture(*sBoss02Textures);
	enemyEyeSet((NJS_TEXLIST*)*sBoss02Textures, 2, mFrameKey);
	if (glow) {
		drawGlow();
	} else if (suit) {
		drawSuit();
	} else if (mTint.isActive() != 0) {
		Boss02Color c;
		c.r       = mTint.mLevel;
		c.g       = 0;
		c.b       = 0;
		c.a       = 0;
		GXColor k = *(GXColor*)&c;
		beginKColor(&k);
	}
	njPushMatrixEx();
	bodyMatrix();
	if (glow) {
		endGlowDraw();
	} else if (suit) {
		suitLift();
	}
	shadowAlphaBegin();
	gjDrawActor(mPose);
	shadowAlphaEnd();
	njPopMatrixEx();
	if (glow) {
		endGlow();
	} else if (suit) {
		endSuit();
	} else if (mTint.isActive() != 0) {
		endKColor();
	}
	enemyEyeRestore();
	shadowSetAngle(&mShadow, 0);
}

// 0x8005DE18
void TEneBoss02::beginGlow()
{
	texFxWarpLoadTex(2);
}

// 0x8005DE3C
void TEneBoss02::endGlowDraw()
{
	texFxWarpLoadMtx(1, 2);
}

// 0x8005DE64
void TEneBoss02::drawGlow()
{
	texFxWarpBegin(mGlow, 0, 2, 0, 0, mTint.mLevel, mGlowWave);
}

// 0x8005DEA4
void TEneBoss02::endGlow()
{
	texFxWarpEnd();
}

// 0x8005DEC4
void TEneBoss02::beginSuit()
{
	texFxSuitLoadTex(2);
}

// 0x8005DEE8
void TEneBoss02::suitLift()
{
	texFxSuitScroll(mSuit.mHeight + gBoss02Params->mSuitBase, mSuit.mHeight, 1);
}

// 0x8005DF1C
void TEneBoss02::drawSuit()
{
	gjSetBlendOverride(1, 4, 5, 3);
	_nj_control_3d_flag_ |= 0x8000;
	texFxSuitBegin(0, mSuit.mAlpha);
}

// 0x8005DF70
void TEneBoss02::endSuit()
{
	texFxSuitEnd();
	_nj_control_3d_flag_ &= ~0x8000;
}
