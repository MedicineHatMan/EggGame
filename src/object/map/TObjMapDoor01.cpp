#include "object/ObjAction.h"
struct DrawTask;
#include "camera/TCamera.h"
#include "stage/SetManager.h"
#include "engine/DrawTask.h"
#include "object/ObjScript.h"
#include "object/ObjModel.h"
#include "camera/EventCamRef.h"
#include "enemy/EnemyLight.h"
#include "object/ObjArchive.h"
#include "stage/StageObjUtil.h"
#include "engine/TParticle.h"
#include "core/Vibration.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "engine/FxSystem.h"
#include "sound/Sound3DPlay.h"
#include "camera/TCamPath.h"
#include "ui/AeArc.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "core/Pad.h"
#include "stage/SetSlot.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "object/TObjKey00.h"
#include "stage/TEventCtrl.h"
#include "sound/SoundCtrl.h"
#include "core/VecMath.h"
#include "engine/LightSet.h"
#include "object/map/TObjMapDoor01.h"
struct EnemyLight;
struct SetLink;
struct DrawStateSlot;
struct NJS_OBJECT;
struct DrawState;
struct Fx;
struct MeshColli;
struct MeshData;
struct MeshObject;

extern "C" {

const f32 gDoorASetRange = 1000.0f;

// 0x802C3DD8
ObjClassName gTObjMapDoor01Name = { "TObjMapDoor01", 0 };

// The event camera handle See TObjYellowSlideDoor.cpp

}

class TObjMapDoor01;

#define sDoors sDoorListObj

typedef char TObjMapDoor01_size_check[(sizeof(TObjMapDoor01) == 0x180) ? 1 : -1];

// 0x80070A50
extern "C" void initDoors()
{
	sDoors = new DoorList;
}

// 0x80070A88
extern "C" void termDoors()
{
	delete sDoors;
}

static inline TObjMapDoor01* makeDoor(TObject* parent, SetDesc* desc)
{
	return new TObjMapDoor01(parent, desc);
}

// Everything but the door with the second model
#define CREATE_DOOR01(parent, desc, node, sound, frameColli)         	TObjMapDoor01* door = makeDoor(parent, desc);                    	if (door == 0) {                                                 		return 0;                                                    	}                                                                	door->mColliNode      = node;                                    	door->mDustId         = 0x1A8;                                   	door->mSoundId        = sound;                                   	door->mModel          = (NJS_OBJECT*)findGeObjData("MODEL_DOOR01");           	door->mFrameModel     = (NJS_OBJECT*)findGeObjData("MODEL_MON01");            	door->mTexList        = (NJS_TEXLIST*)findGeObjData("TLS_MODEL_DOOR01");       	door->mFrameTexList   = findGeObjData("TLS_MODEL_MON01");        	door->mMotion         = findGeObjData("MOTION_DOOR01");          	door->mColliData      = (MeshData*)findStgObjData("COL_MODEL_DOOR01");      	door->mFrameColliData = (MeshData*)frameColli;                              	door->mBoxLow.x       = -70.0f;                                  	door->mBoxLow.y       = -10.0f;                                  	door->mBoxLow.z       = -20.0f;                                  	door->mBoxHigh.x      = 70.0f;                                   	door->mBoxHigh.y      = 90.0f;                                   	door->mBoxHigh.z      = 20.0f;                                   	door->createColli(0);                                            	door->mRadius = 100.0f;                                          	door->initOpen();                                                	return door;

// 0x80070AB0
extern "C" TObject* createDoorA(TObject* parent, SetDesc* desc)
{
	CREATE_DOOR01(parent, desc, 1, 0x278, findStgObjData("COL_MODEL_MON01"))
}

// 0x80070BD4
extern "C" TObject* createDoorB(TObject* parent, SetDesc* desc)
{
	CREATE_DOOR01(parent, desc, 1, 0x20F, findStgObjData("COL_MODEL_MON01"))
}

// 0x80070CF8
extern "C" TObject* createDoorC(TObject* parent, SetDesc* desc)
{
	CREATE_DOOR01(parent, desc, 0, 0x224, findStgObjData("COL_MODEL_MON01"))
}

// 0x80070E1C
extern "C" TObject* createDoorD(TObject* parent, SetDesc* desc)
{
	TObjMapDoor01* door = makeDoor(parent, desc);
	if (door == 0) {
		return 0;
	}
	door->mColliNode      = 0;
	door->mSoundId        = 0x239; // the one door that sets these two the other way round
	door->mDustId         = 0x1A8;
	door->mModel          = (NJS_OBJECT*)findGeObjData("MODEL_DOOR01");
	door->mFrameModel     = (NJS_OBJECT*)findGeObjData("MODEL_MON01");
	door->mTexList        = (NJS_TEXLIST*)findGeObjData("TLS_MODEL_DOOR01");
	door->mFrameTexList   = findGeObjData("TLS_MODEL_MON01");
	door->mMotion         = findGeObjData("MOTION_DOOR01");
	door->mColliData      = (MeshData*)findStgObjData("COL_MODEL_DOOR01");
	door->mFrameColliData = (MeshData*)findStgObjData("COL_MODEL_MON01");
	door->mBoxLow.x       = -70.0f;
	door->mBoxLow.y       = -10.0f;
	door->mBoxLow.z       = -20.0f;
	door->mBoxHigh.x      = 70.0f;
	door->mBoxHigh.y      = 90.0f;
	door->mBoxHigh.z      = 20.0f;
	door->createColli(0);
	door->mRadius = 100.0f;
	door->initOpen();
	return door;
}

// 0x80070F40
extern "C" TObject* createDoorE(TObject* parent, SetDesc* desc)
{
	CREATE_DOOR01(parent, desc, 0, 0x24E, findStgObjData("COL_MODEL_MON01"))
}

// 0x80071064
// The second door model which has no frame
extern "C" TObject* createDoor02(TObject* parent, SetDesc* desc)
{
	TObjMapDoor01* door = makeDoor(parent, desc);
	if (door == 0) {
		return 0;
	}
	door->mModel      = (NJS_OBJECT*)findGeObjData("MODEL_DOOR02");
	door->mFrameModel = 0;
	door->mTexList    = (NJS_TEXLIST*)findGeObjData("TLS_MODEL_DOOR02");
	door->mFrameTexList = 0;
	door->mMotion     = findGeObjData("MOTION_DOOR02");
	door->mColliData  = (MeshData*)findGeObjData("COL_MODEL_DOOR02");
	door->mFrameColliData = 0;
	door->mColliNode  = 0;
	door->mBoxLow.x   = -100.0f;
	door->mBoxLow.y   = -10.0f;
	door->mBoxLow.z   = -20.0f;
	door->mBoxHigh.x  = 100.0f;
	door->mBoxHigh.y  = 100.0f;
	door->mBoxHigh.z  = 20.0f;
	door->createColli(0);
	door->initOpen();
	door->mBurstId = 1;
	door->mSoundId = 0x24E;
	door->mRadius  = 150.0f;
	return door;
}

// 0x80071174
// No collision for the frame
extern "C" TObject* createDoorF(TObject* parent, SetDesc* desc)
{
	CREATE_DOOR01(parent, desc, 0, 0x263, 0)
}

// 0x80071294
extern "C" TObject* createDoorG(TObject* parent, SetDesc* desc)
{
	CREATE_DOOR01(parent, desc, 0, 0x28D, findStgObjData("COL_MODEL_MON01"))
}

// 0x800713B8
void TObjMapDoor01::initFromDesc(SetDesc* desc)
{
	desc->getPos(&mPos);
	desc->getRot(&mRot);
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	mSoundPos = mPos;

	mHasCamera = desc->getInt(0);
	int camera = mHasCamera;
	if (camera < 0) {
		camera = 0;
	} else if (camera > 1) {
		camera = 1;
	}
	mHasCamera = camera;
	mDoorId    = desc->getInt(3) % 256;

	mSetCamPos = desc->getInt(1) < 0 ? 0 : (desc->getInt(1) > 1 ? 1 : desc->getInt(1));

	if (mSetCamPos != 0) {
		f32 z = desc->getFloat(2);
		f32 y = desc->getFloat(1);
		mCamPos.x = desc->getFloat(0);
		mCamPos.y = y;
		mCamPos.z = z;
		vecRotateY(&mCamPos, &mCamPos, mRot.y);
		vecAdd(&mCamPos, &mCamPos, &mPos);
		mCamTarget = mPos;
		mCamTarget.y += desc->getFloat(3);
	} else {
		mCamTarget = mPos;
		mCamTarget.y += 50.0f;
		vecFromAngleXZ(165.0f, &mCamPos, mRot.y);
		vecAdd(&mCamPos, &mCamPos, &mPos);
		mCamPos.y += 100.0f;
	}
}

// 0x80071664
void TObjMapDoor01::debugPrint(void*)
{
	if (mHasCamera != 0) {
		njPrintC(0xA0013, ":EVENT_CAMERA ON");
		njPrintC(0xA0014, ":SET CAM POS");
		if (mSetCamPos != 0) {
			njPrintC(0xA0017, ":OFFSET-X");
			njPrintC(0xA0018, ":OFFSET-Y");
			njPrintC(0xA0019, ":OFFSET-Z");
			njPrintC(0xA001A, ":TGT-OFFSET-Y");
		}
	} else {
		njPrintC(0xA0013, ":EVENT_CAMERA OFF");
	}
	njPrintC(0xA0016, ":DOOR_ID(0~255)");
}

// 0x80071730
TObjMapDoor01::TObjMapDoor01(TObject* parent, SetDesc* desc)
    : TObjAction(parent)
{
	_00   = (u32)gTObjMapDoor01Name.mName;
	mSize = 0x180;
	setObjId(this, 0xFFFF);
	mLink.get()->attach(desc);
	initFromDesc(desc);
	setup();
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
}

// 0x80071800
TObjMapDoor01::~TObjMapDoor01()
{
	sDoors->remove(&mNode);
	endCamera();
	endEffects();
	if (mFrameColli != 0) {
		meshColliFree(mFrameColli);
	}
	if (mColli != 0) {
		meshColliFree(mColli);
	}
	removeDrawTask(mDrawTask);
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	releaseObjId(this);
}

// 0x800718DC
void TObjMapDoor01::setup()
{
	mNode.mObj = this;
	sDoors->add(&mNode);
	mFrameColliData = 0;
	mColliData      = 0;
	mFrameModel     = 0;
	mModel          = 0;
	mFrameTexList   = 0;
	mTexList        = 0;
	mColli          = 0;
	mFrameColli     = 0;
	mDust           = 0;
	mCamera         = 0;
	_124            = 0;
	mDustId         = -1;
	mBurstId        = -1;
	mAct            = 0;
	mTimer          = 0;
	mFrame          = 0.0f;
	mFrameStep      = 0.25f;
	mLight.setFadeDist(0.0f);
	mLight.mTimer = 0;
	mLightSet = lightSetFindArea(&mPos);
}

// 0x80071984
int TObjMapDoor01::onCameraMessage(ObjMsg* msg)
{
	mCam.set(msg);
	return 1;
}

// 0x800719AC
// A door whose switch is already on starts open at the motion's last frame
void TObjMapDoor01::initOpen()
{
	if (keyBitIsSet(mDoorId) == 1) {
		mFrame = (f32)(((NJS_MOTION*)mMotion)->nbFrame - 1);
		pose();
		mAct = 3;
	}
}

// 0x80071A20
void TObjMapDoor01::startCamera()
{
	if (mCamera == 0) {
		mCamera = createTCamFix(0, 2, &mCamTarget, &mCamPos);
		if (mCamera != 0) {
			mCamera->setCut();
		}
	}
}

// 0x80071A7C
void TObjMapDoor01::endCamera()
{
	if (mCamera != 0) {
		mCamera->mFlags |= 1;
		mCamera = 0;
	}
}

// 0x80071AA0
void TObjMapDoor01::startEffects()
{
	if (mDustId > -1 && mDust == 0) {
		Vec3f* pos = getPos();
		mDust = particleGenById(this, pos, 0, mDustId);
		if (mDust != 0) {
			mDust->setRot(0, mRot.y);
		}
	}
	if (mBurstId > -1) {
		f32 mtx[12];
		mtxSetTransV(mtx, &mPos);
		mtxRotateY(mtx, mtx, mRot.y);
		mtxTranslate((float*)mtx, (const float*)mtx, 0.0f, 0.0f, 20.0f);
		fxSpawnAt(1, 8, mBurstId, mtx);
	}
}

// 0x80071B74
void TObjMapDoor01::endEffects()
{
	if (mDust != 0) {
		mDust->mFlags |= 1;
		mDust = 0;
	}
}

// 0x80071B98
void TObjMapDoor01::createColli(int flag)
{
	f32 box[7];
	boxAroundBox(box, mMatrix, &mBoxLow, &mBoxHigh);

	mColli = (MeshColli*)meshColliCreate();
	meshSetFlag2((MeshObject*)mColli, 1);
	meshSetState100((MeshObject*)mColli, flag);
	meshColliSetData((MeshObject*)mColli, mColliData);
	meshColliSetMatrix(mColli, (const f32*)mMatrix);
	meshColliSetBox((MeshObject*)mColli, (const f32*)box);

	if (mFrameColliData != 0) {
		mFrameColli = (MeshObject*)meshColliCreate();
		meshColliSetData(mFrameColli, mFrameColliData);
		meshColliSetMatrix(mFrameColli, (const f32*)mMatrix);
		meshColliSetBox(mFrameColli, (const f32*)box);
	}
}

// 0x80071C54
// The other doors with this id open without a camera of their own
void TObjMapDoor01::openOthers()
{
	for (DoorListNode* node = sDoors->mHead; node != 0; node = node->mNext) {
		TObjMapDoor01* other = node->mObj;
		if (other != 0 && mDoorId == other->mDoorId && other != this) {
			eventHideObj((u8*)other);
			node->mObj->mAct = 2;
			node->mObj->startEffects();
		}
	}
}

// 0x80071CD0
// Walks the motion tree and moves the door's collision with the node it follows
void TObjMapDoor01::updateNodes(DoorNode* node, int index)
{
	do {
		njPushMatrixEx();
		njMotionTransformEx((NJS_OBJECT*)node);
		if (node->mModel != 0 && index == mColliNode && mColli != 0) {
			f32 mtx[12];
			njGetMatrix(mtx);
			meshColliSetMatrix(mColli, (const f32*)mtx);
		}
		index++;
		if (node->mChild != 0) {
			updateNodes(node->mChild, index);
		}
		njPopMatrixEx();
		node = node->mNext;
	} while (node != 0);
}

// 0x80071D70
int TObjMapDoor01::stepOpen()
{
	u32 last = ((NJS_MOTION*)mMotion)->nbFrame - 1;
	if (mFrame < last) {
		mFrame += mFrameStep;
		pose();
		return 0;
	}
	mFrame = last;
	pose();
	return 1;
}

// 0x80071DF0
void TObjMapDoor01::pose()
{
	njPushMatrix(mMatrix);
	njStartMotionObj(mMotion, mFrame);
	updateNodes((DoorNode*)mModel, 0);
	njPopMatrixEx();
}

// 0x80071E40
int TObjMapDoor01::stepClose()
{
	if (mFrame > 0.0f) {
		mFrame -= mFrameStep;
		pose();
		return 0;
	}
	mFrame = 0.0f;
	pose();
	return 1;
}

// 0x80071E90
void TObjMapDoor01::shakeCamera()
{
	padVibStart(0, (int)this, 10, 0, 0x1F);
}

// 0x80071EC4
void TObjMapDoor01::quake()
{
	vibStopOwner(0, this);
}

// 0x80071EEC
void TObjMapDoor01::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (!isInRange(this, &mPos, 1050.0f) && mCam.isActive() == 0) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
		endEffects();
		if (mHasCamera != 0) {
			if (mCam.isActive() != 0) {
				sePlay(0x2C);
				mTimer = 60;
				mAct   = 1;
			}
		} else if (keyBitIsSet(mDoorId) == 1) {
			sePlay(0x2C);
			mTimer = 60;
			mAct   = 1;
		}
		break;
	case 1:
		if (mHasCamera != 0) {
			if (mCamera == 0) {
				startCamera();
			} else if (mTimer == 0) {
				startEffects();
				sePlay(mSoundId);
				openOthers();
				shakeCamera();
				mAct = 2;
			} else {
				mTimer--;
			}
		} else if (mTimer == 0) {
			startEffects();
			sePlay3D(mSoundId, &mSoundPos);
			shakeCamera();
			mAct = 2;
		} else {
			mTimer--;
		}
		break;
	case 2:
		if (keyBitIsSet(mDoorId) == 0) {
			mAct = 4;
		}
		if (stepOpen() != 0) {
			if (mHasCamera != 0) {
				endCamera();
				_124 = 0;
				mCam.release();
			}
			quake();
			mAct = 3;
		}
		break;
	case 3:
		if (mCam.isActive() != 0) {
			mFrame = 0.0f;
			pose();
			mAct = 0;
		} else {
			endEffects();
			if (keyBitIsSet(mDoorId) == 0) {
				mAct = 4;
			}
		}
		break;
	case 4:
		if (keyBitIsSet(mDoorId) == 1) {
			mAct = 2;
		}
		if (stepClose() != 0) {
			quake();
			mAct = 0;
		}
		break;
	}
}

// 0x80072194
void TObjMapDoor01::vfn_10()
{
	if (mLight.isVisible(mDrawTask, 900.0f, getPos(), mRadius, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80072214
void TObjMapDoor01::render()
{
	int saved[1];
	getCullMode(saved);
	setCullMode(0);
	DrawStateSlot* slot = getDrawStateSlot();
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);

	ObjDrawState state;
	setDrawAmbient(&mLight, &state);
	lightSetSelect(mLightSet);
	beginDrawState(slot, (const DrawState*)&state);

	njSetTexture(mTexList);
	gjDrawMotion(mModel, mMotion, mFrame);
	if (mFrameModel != 0 && mFrameTexList != 0) {
		njSetTexture(mFrameTexList);
		gjDrawObject(mFrameModel);
	}
	endDrawState((DrawStateSlot*)slot);
	njPopMatrixEx();
	setCullMode(saved[0]);
}

// 0x800722E0
DoorList::DoorList()
{
	mHead  = 0;
	mTail  = 0;
	mCount = 0;
}

// 0x800722F4
DoorList::~DoorList()
{
	clear();
}

// 0x8007233C
void DoorList::add(DoorListNode* node)
{
	if (mHead == 0) {
		mTail = node;
		mHead = node;
	} else {
		node->mPrev  = mTail;
		mTail->mNext = node;
		mTail        = node;
	}
	node->mList = this;
	mCount++;
}

// 0x8007237C
void DoorList::remove(DoorListNode* node)
{
	if (node->mPrev != 0) {
		node->mPrev->mNext = node->mNext;
	} else {
		mHead = node->mNext;
	}
	if (node->mNext != 0) {
		node->mNext->mPrev = node->mPrev;
	} else {
		mTail = node->mPrev;
	}
	node->mNext = 0;
	node->mPrev = 0;
	node->mList = 0;
	mCount--;
}

// 0x800723DC
void DoorList::clear()
{
	DoorListNode* node = mHead;
	while (node != 0) {
		DoorListNode* next = node->mNext;
		node->mList = 0;
		node->mNext = 0;
		node->mPrev = 0;
		node = next;
	}
}

// 0x80072408
Vec3f* TObjMapDoor01::getPos()
{
	return &mPos;
}

// 0x80072410
int TObjMapDoor01::getKind()
{
	return 3;
}

// 0x80072418
int TObjMapDoor01::getRotY()
{
	return mRot.y;
}

// 0x80072420
SetLink* TObjMapDoor01::getLink()
{
	return mLink.get();
}

// 0x80072428
int TObjMapDoor01::sendToHandle(ObjMsg* msg)
{
	switch (*(u16*)msg) {
	case 0xC7:
		return onCameraMessage(msg);
	default:
		return 0;
	}
}

// 0x80072460
extern "C" void doorUnused1()
{
}

// 0x80072464
extern "C" void doorUnused2()
{
}
