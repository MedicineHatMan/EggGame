#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "camera/TCamera.h"
#include "enemy/EnemyGroupWave.h"
#include "enemy/EnemyAnim.h"
#include "enemy/ObjEnemy.h"
#include "engine/MotionPose.h"
#include "engine/FxHolder.h"
#include "io/ResBank.h"
#include "enemy/TEneGreenDemo.h"
#include "enemy/EnemyFx.h"
#include "stage/TEventCtrl.h"
#include "stage/SetManager.h"
#include "enemy/boss/TEneYellowBoss.h"
#include "egg/EggCreature.h"
#include "object/ObjShadow.h"
#include "animal/AniMount.h"
#include "engine/DrawState.h"
#include "Ninja/ninja.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"
#include "sound/SoundCtrl.h"
#include "engine/TParticle.h"
#include "core/TMainTask.h"

const char* gTEneGreenDemoName = "TEneGreenDemo";

extern "C" {
MotionEntry sGreenDemoMotionTable[] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 899.0f, 1.0f, 0.0f, 0 },
};
}

typedef char TEneGreenDemo_size_check[(sizeof(TEneGreenDemo) == 0x138) ? 1 : -1];

static ResBank sGreenDemoBank;           // 0x802868B8
static NJS_OBJECT** sGreenDemoModels;    // 0x802C5AE8
static NJS_TEXLIST** sGreenDemoTexLists; // 0x802C5AEC
static NJS_MOTION** sGreenDemoMotions;   // 0x802C5AF0

// 0x801B6864
// The enemy init list clears the bank
void greenDemoBankInit()
{
	resBankCtor(&sGreenDemoBank);
}

// 0x801B688C
void greenDemoBankExit()
{
	if (sGreenDemoBank.mArc != NULL) {
		resBankRemoveTextures(&sGreenDemoBank, 0);
		resBankRelease(&sGreenDemoBank);
		sGreenDemoModels   = NULL;
		sGreenDemoTexLists = NULL;
		sGreenDemoMotions  = NULL;
	}
}

// 0x801B68DC
// The event command load_green_ene_demo
int greenDemoBankLoad(void*)
{
	if (sGreenDemoBank.mArc == NULL) {
		resBankLoad(&sGreenDemoBank, "ar_ene_green_demo.arc");
		resBankAddTextures(&sGreenDemoBank, 0, "ene_green_demo.gvm");
		sGreenDemoModels   = resBankGetModels(&sGreenDemoBank);
		sGreenDemoTexLists = resBankGetTextures(&sGreenDemoBank);
		sGreenDemoMotions  = resBankGetMotion(&sGreenDemoBank, 0);
	}
	return 1;
}

// 0x801B6964
// TObjEggSuit starts the cutscene with this
void greenDemoSpawn()
{
	new TEneGreenDemo(gTL_05);
}

extern "C" {
extern const f32 sGreenDemoEffectFrames[] = {
	24.0f, 365.0f, 730.0f, 764.0f, 822.0f, -1.0f, 0.0f,
}; // the effect frames ending at 0
}

// 0x801B699C
TEneGreenDemo::TEneGreenDemo(TObject* parent)
    : TObject(parent)
{
	_00   = (u32)gTEneGreenDemoName;
	mSize = 0x138;
	motionCtrlSetPlayer(&mAnim, &mPlayer);
	motionCtrlInit(&mAnim, sGreenDemoMotionTable, sGreenDemoMotions);
	motionCtrlPlay(&mAnim, 0);
	mPose     = actorCreate(*sGreenDemoModels);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	shadowSetGround(&mShadow, &mPos, 0);
	mShadow._30 = 20.0f;
	mShadow._34 = 20.0f;
	shadowEnable(&mShadow);
	mScreen      = 0;
	mSavedParent = 0;
	mState       = 0;
	mFx          = 0;
	fxHolderStop(&mVoice);
}

// 0x801B6ABC
TEneGreenDemo::~TEneGreenDemo()
{
	if (mState == 1) {
		eventEnd();
	}
	if (mSavedParent != 0) {
		setSetEnemyParent(mSavedParent);
	}
	if (mScreen != 0) {
		mScreen->mFlags |= 1;
		mScreen = 0;
	}
	fxHolderStop(&mVoice);
	for (int i = 0; i < 1; i++) {
		if ((&mFx)[i] != 0) {
			(&mFx)[i]->mFlags |= 1;
			(&mFx)[i] = 0;
		}
	}
	removeDrawTask(mDrawTask);
	actorFree(&mPose);
}

// 0x801B6BAC
void TEneGreenDemo::draw()
{
	switch (mState) {
	case 0:
		if (eventStartByObj(this) != 0) {
			void* first = *resBankGetObjects(&sGreenDemoBank);
			mScreen     = createCamPath(0, 3, first, *resBankGetMotion(&sGreenDemoBank, 1), 0, 1.0f, 1.0f);
			sePlayFor(0x2BC, 0);
			mState = 1;
			pose();
			placeNodes();
		}
		break;
	case 1: {
		motionCtrlUpdate(&mAnim);
		pose();
		placeNodes();
		if (mSavedParent == 0) {
			mSavedParent = setSetEnemyParent(gTL_04);
		}
		for (int i = 0; sGreenDemoEffectFrames[i] > 0.0f; i++) {
			if (motionCtrlPassed(sGreenDemoEffectFrames[i], &mAnim) != 0) {
				Vec3f at;
				at.x = mNodes[3].x;
				at.y = mNodes[3].y;
				at.z = mNodes[3].z;
				njAddVector(&at, &mNodes[4]);
				gjScaleVector(0.5f, &at, &at);
				particleOnceByIdDefault(&at, 0, 0x66);
				break;
			}
		}
		f32 now = motionCtrlFrame(&mAnim);
		if (now >= 505.0f && now <= 670.0f) {
			particleKeepAt((void**)&mFx, &mPos, 0, 0x67);
		} else if (mFx != 0) {
			mFx->mFlags |= 1;
			mFx = 0;
		}
		if (motionCtrlPassed(560.0f, &mAnim) != 0) {
			enemyGroupStartWave(1, 0, 0);
		}
		if (motionCtrlIsDone(&mAnim) != 0) {
			mState = 2;
			if (mScreen != 0) {
				mScreen->mFlags |= 1;
				mScreen = 0;
			}
			eventEnd();
			if (mSavedParent != 0) {
				setSetEnemyParent(mSavedParent);
			}
			mFlags |= 1;
		}
		break;
	}
	case 2:
		break;
	}
}

// 0x801B6E04
void TEneGreenDemo::startNodeCapture()
{
	nodeTrackBeginOrigins(paramsTEneYellowBoss(), mNodes);
}

// 0x801B6E38
void TEneGreenDemo::endNodeCapture()
{
	nodeTrackEnd();
}

// 0x801B6E58
// Takes the place of its root from the motion then poses the rest around it
void TEneGreenDemo::pose()
{
	MotionNodePose root;
	NJS_MOTION* motion = motionCtrlGetMotion(&mAnim);
	motionGetNodePose(&root, *(NJS_OBJECT**)mPose, motion, motionCtrlFrame(&mAnim));
	motionPoseGetPos(&root, &mPos);
	startNodeCapture();
	gjPushUnitMatrix();
	gjTranslate(-mPos.x, -mPos.y, -mPos.z);
	actorApplyMotion(mPose, &mAnim);
	njPopMatrixEx();
	endNodeCapture();
}

// 0x801B6EF8
void TEneGreenDemo::placeNodes()
{
	gjPushUnitMatrix();
	applyMatrix();
	for (int i = 0; i < 5; i++) {
		njCalcPointEx(&mNodes[i], &mNodes[i]);
	}
	njPopMatrixEx();
}

// 0x801B6F58
void TEneGreenDemo::applyMatrix()
{
	njTranslateEx(&mPos);
}

// 0x801B6F7C
void TEneGreenDemo::vfn_10()
{
	if (mState != 1) {
		mFlags |= 0x10;
	} else if (cameraIsVisible(3000.0f, 200.0f, &mPos, 0) == 0) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
	}
}

// 0x801B6FF8
void TEneGreenDemo::render()
{
	njSetTexture(*sGreenDemoTexLists);
	njPushMatrixEx();
	applyMatrix();
	shadowAlphaBegin();
	drawActorDefault(mPose);
	shadowAlphaEnd();
	njPopMatrixEx();
}
