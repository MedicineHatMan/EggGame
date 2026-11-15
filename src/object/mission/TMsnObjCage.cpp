#include "object/purple/TObjPurpleBlizzard.h"
#include "object/mission/TMsnObjCage.h"
#include "camera/TCamera.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "camera/EventCamRef.h"
#include "object/ObjScript.h"
#include "stage/StageObjUtil.h"
#include "object/ObjAction.h"
#include "io/NinjaFile.h"
#include "ui/AeArc.h"
#include "stage/TStageOrange.h"
#include "object/TObjLauncher.h"
#include "engine/Color.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCamPath.h"
#include "sound/Sound3DPlay.h"
#include "core/MathUtil.h"
#include "stage/SetSlot.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "object/TObjKey00.h"
#include "sound/SoundCtrl.h"
#include "engine/TParticle.h"
#include "core/VecMath.h"
#include "object/TObject.h"
#include "object/TObjSnowman.h"
#include "ui/TAeObjMissionIcon.h"
#include "Ninja/ninja.h"
struct DrawState;
struct MeshColli;
struct MeshData;
struct MeshObject;
struct EnemyLight;
struct NJS_QUATERNION;

typedef char TMsnObjCage_size_check[(sizeof(TMsnObjCage) == 0x198) ? 1 : -1];

extern "C" {

// The archive the model list comes from and the two words beside it

// Half-width height half-depth of the box fitModel builds
}

static const char* sClassName = "TMsnObjCage";

// The only message this object takes
static const u16 MSG_OPEN = 199;

static inline int clamp01(int value)
{
	if (value < 0) {
		return 0;
	}
	if (value > 1) {
		return 1;
	}
	return value;
}

inline void* TMsnObjCage::operator new(u32 size)
{
	return objHeapAlloc(gObjHeap, size);
}

inline void TMsnObjCage::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

// 0x800FC60C
// The set-init table calls this for TMsnObjCage
extern "C" void initCage()
{
	loadArc("geobj_cage.arc", &gCageArc);
	gCageShadowModel = (NJS_OBJECT*)findArcExport(gCageArc, "model");
	gCageTexList     = (NJS_TEXLIST*)findArcExport(gCageArc, "texlist");
	bindGvmTextures((u8*)findArcExport(gCageArc, "texture"), gCageTexList);
}

// 0x800FC66C
extern "C" void termCage()
{
	releaseTextures((NJS_TEXLIST*)gCageTexList);
	releaseResource(gCageArc);
}

// 0x800FC698
extern "C" TObject* createTMsnObjCage(TObject* parent, SetDesc* desc)
{
	return new TMsnObjCage(parent, desc);
}

// 0x800FC6EC
// Slot _30
void TMsnObjCage::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mKeyId = desc->getInt(0);

	// The file stores how much to add to 1
	mScale     = 1.0f + (f32)(int)desc->getFloat(0);
	mScale     = mScale < 1.0f ? 1.0f : (mScale > 3.0f ? 3.0f : mScale);
	mCamera    = desc->getInt(1);
	int camera = mCamera;
	if (camera < 0) {
		camera = 0;
	} else if (camera > 1) {
		camera = 1;
	}
	mCamera = camera;

	mPlace.mFlags = 7;
	mPlace.mPos   = mPos;
	njXYZAngleToQuaternion((const s32*)&mRot, (NJS_QUATERNION*)mPlace.mRot);
	mPlace.mScale.set(mScale, mScale, mScale);
	mPlace.update();

	mSaved = *getDefaultDrawState();

	if (gCageShadowModel != 0) {
		mShadow.setModel(gCageShadowModel);
		mShadow.mPlace   = &mPlace;
		mShadow.mTexList = gCageTexList;
		mShadow.mState   = &mSaved;
	}

	// 0 in the file draws to the mirror
	mMirror = 1 - CLAMP_RANGE(desc->getInt(2), 0, 1);
	mCamPos = CLAMP_RANGE(desc->getInt(3), 0, 1);
	if (mCamPos != 0) {
		mAim.set(desc->getFloat(1), desc->getFloat(2), desc->getFloat(3));
		vecRotateY(&mAim, &mAim, mRot.y);
		vecAdd(&mAim, &mAim, &mPos);
	} else {
		vecFromAngleXZ(120.0f, &mAim, mRot.y);
		vecAdd(&mAim, &mAim, &mPos);
		mAim.y += 60.0f;
	}

	mMark = mPos;
	mMark.y += 12.5f * mScale;
}

// 0x800FCA8C
// Slot _34 The lines are the set-file labels not the values
void TMsnObjCage::debugPrint(void*)
{
	njPrintC(0xA0013, ":KEY ID");
	if (mCamera != 0) {
		njPrintC(0xA0014, ":EVENT_CAMERA ON");
		njPrintC(0xA0016, ":SET CAM POS ");
		if (mCamPos != 0) {
			njPrintC(0xA0018, ":OFFSET-X");
			njPrintC(0xA0019, ":OFFSET-Y");
			njPrintC(0xA001A, ":OFFSET-Z");
		}
	} else {
		njPrintC(0xA0014, ":EVENT_CAMERA OFF");
	}
	njPrint(0xA0015, ":DRAW TO MIRROR %s", mMirror != 0 ? "ON" : "OFF");
	njPrintC(0xA0017, "+1.0:SCALE");
}

// 0x800FCB80
void TMsnObjCage::loadModel()
{
	mTimer = 0;
	mPhase = PHASE_WAIT;
	mBeam  = 0;

	// Already collected and not in the state that keeps the prop on screen delete it
	if (keyBitIsSet((u16)mKeyId) == 1 && debugEditActive() == 0) {
		mModel = 0;
		mFlags |= 1;
		return;
	}
	if (gCageArc != 0) {
		void** list        = (void**)findArcExport(gCageArc, "dymodels");
		MeshData* resource = ((MeshData**)list)[(int)mScale - 1];
		mModel             = (MeshObject*)meshColliCreate();
		meshColliSetData(mModel, resource);

		f32 fit = 1.0f / mScale;
		f32 matrix[12];
		mtxScale(fit, fit, fit, matrix, mPlace.mMatrix);
		meshColliSetMatrix(mModel, matrix);
		fitModel();
	} else {
		mModel = 0;
	}
}

// 0x800FCC78
// Arms the watch and reports that the message was taken
int TMsnObjCage::acceptMessage(ObjMsg* msg)
{
	mWatch.set(msg);
	return 1;
}

// 0x800FCCA0
// The box is -15 0 -15 to 15 40 15 once the static initializer has run
void TMsnObjCage::fitModel()
{
	Vec3f neg;
	Vec3f pos;
	f32 box[6];
	neg.set(-sCageExtent.x, 0.0f, -sCageExtent.z);
	pos.set(sCageExtent.x, 2.0f * sCageExtent.y, sCageExtent.z);
	boxAroundBox(box, mPlace.mMatrix, &neg, &pos);
	meshColliSetBox(mModel, box);
}

// 0x800FCD28
TMsnObjCage::TMsnObjCage(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)sClassName;
	mSize = 0x198;
	initFromDesc(desc);
	setLinkDone((SetLink*)&mLink);
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	loadModel();
}

// 0x800FCDE8
// The placement is not destroyed
TMsnObjCage::~TMsnObjCage()
{
	clearBeam();
	if (mModel != 0) {
		meshColliFree(mModel);
	}
	removeDrawTask(mTask);
}

// 0x800FCE88
void TMsnObjCage::spawnBeam()
{
	if (mBeam != 0) {
		return;
	}
	mBeam = createTCamFix(0, 2, &mMark, &mAim);
	if (mBeam != 0) {
		mBeam->setCut();
	}
}

// 0x800FCEE4
void TMsnObjCage::clearBeam()
{
	if (mBeam == 0) {
		return;
	}
	mBeam->mFlags |= 1;
	mBeam = 0;
}

// 0x800FCF08
void TMsnObjCage::checkOpen()
{
	if (mCamera != 0) {
		if (mWatch.isActive() == 0) {
			return;
		}
		if (mBeam == 0) {
			spawnBeam();
			return;
		}
		sePlay(0x2C);
		mTimer = 60;
		mPhase = PHASE_OPEN;
	} else {
		if (keyBitIsSet((u16)mKeyId) == 1) {
			sePlay(0x2C);
			mTimer = 60;
			mPhase = PHASE_OPEN;
		}
	}
}

// 0x800FCFAC
void TMsnObjCage::playOpen()
{
	if (mTimer == 0) {
		shatterModel(this, &mPos, mRot.y, gCageShadowModel, gCageTexList, mScale, 1.5f);
		particleOnceByIdDefault(getPos(), 0, 0xCF);
		sePlay3D(0x6D, &mPos);
		mPhase = PHASE_DONE;
	} else {
		mTimer--;
	}
}

// 0x800FD03C
void TMsnObjCage::finish()
{
	if (objCountChildren(this) > 0) {
		return;
	}
	if (mCamera != 0) {
		mWatch.release();
		clearBeam();
	}
	setLinkDone((SetLink*)&mLink);
	mFlags |= 1;
}

// 0x800FD0A0
// Slot _0C the per-frame update
void TMsnObjCage::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	switch (mPhase) {
	case PHASE_WAIT:
		checkOpen();
		break;
	case PHASE_OPEN:
		playOpen();
		break;
	case PHASE_DONE:
		finish();
		break;
	}
}

// 0x800FD11C
inline void TMsnObjCage::setHidden(int hidden)
{
	u16* flag = &mTask->mFlags;
	if (hidden) {
		*flag |= 1;
	} else {
		*flag &= ~1;
	}
}

void TMsnObjCage::vfn_10()
{
	// A mirror pass and this cage does not draw into the mirror
	if (mMirror == 0 && mirrorIsReflecting() != 0) {
		setHidden(1);
		return;
	}
	if (mLight.isVisible(mTask, 1000.0f, &mPos, 25.0f * mScale, 0) != 0 && mPhase != PHASE_DONE) {
		setHidden(0);
	} else {
		setHidden(1);
	}
}

// 0x800FD1CC
void TMsnObjCage::render()
{
	if (gCageShadowModel == 0) {
		f32 matrix[12];
		mtxConcat(matrix, gjGetMatrix(), mPlace.mMatrix);
		mtxTranslate((float*)matrix, (const float*)matrix, 0.0f, 12.5f, 0.0f);
		mtxScale(25.0f, 25.0f, 25.0f, matrix, matrix);
		debugWireBox(matrix, &gColorBlack);
		return;
	}
	setDrawAmbient(&mLight, &mSaved);
	mShadow.draw();
}

// 0x800FD268
// A placed object
int TMsnObjCage::getKind()
{
	return 3;
}

// 0x800FD270
SetLink* TMsnObjCage::getLink()
{
	return (SetLink*)&mLink;
}

// 0x800FD278
int TMsnObjCage::sendToHandle(ObjMsg* msg)
{
	switch (msg->mType) {
	case MSG_OPEN:
		return acceptMessage(msg);
		break;
	}
	return 0;
}

// The box fitModel reads set by the static initializer at 0x800FD2B0
InitVec sCageExtent(15.0f, 20.0f, 15.0f);
