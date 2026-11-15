#include "core/types.h"
struct DrawTask;
#include "object/ObjModel.h"
#include "camera/TCamera.h"
#include "engine/DrawTask.h"
#include "camera/EventCamArc.h"
#include "ui/AeArc.h"
#include "object/ObjScript.h"
#include "stage/SetActor.h"
#include "camera/EventCamRef.h"
#include "engine/MotionPlayer.h"
#include "object/ObjCommon.h"
#include "egg/ObjEgg.h"
#include "sound/SoundHandle.h"
#include "engine/CharaColli.h"
#include "engine/Light.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "object/ObjShadow.h"
#include "camera/TCameraCtrl.h"
#include "object/mission/TMsnObjEmblem.h"
#include "object/TObjLauncher.h"
#include "engine/Color.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "object/TObjEggRing.h"
#include "object/ObjAction.h"
#include "engine/FxSystem.h"
#include "sound/Sound3DPlay.h"
#include "engine/Colli.h"
#include "engine/GroundQuery.h"
#include "Ninja/ninja.h"
#include "engine/FrameTex.h"
#include "egg/EggCreature.h"
#include "stage/SetSlot.h"
#include "object/TObjKey00.h"
#include "engine/TParticle.h"
#include "sound/SoundCtrl.h"
#include "object/TObjPlayer.h"
#include "object/mission/TMsnObjDarkGate.h"
struct TObject;
struct CamView;
struct SetLink;
struct NJS_QUATERNION;
struct Fx;

// 0x86 collects it The model is drawn with a light-projected shine

extern "C" {
// 0x8025CF38
MotionEntry sMsnEmblemMotions[3] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 59.0f, 0.2f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 30.0f, 0.2f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 419.0f, 1.0f, 0.0f, 0 },
};
ColliShape sMsnEmblemShape[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 10.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

static const char* sClassName = "TMsnObjEmblem";
const char* gTEmblemGetName   = "TEmblemGet";

extern "C" {
extern const int sEmblemCameras[] = {
	4,
	5,
	6,
}; // the camera of each camera id
}

// The field of view and aspect of the current view
struct EmblemCamera {
	u8 _00[0x44];
	int mFov; // _44
	u8 _48[4];
	f32 mAspect; // _4C
};

// Message 0x86 take the emblem
struct EmblemTakeMsg : public ObjMsg {
	u16 _02;   // _02
	u16 mFrom; // _04
	u16 _06;   // _06
};

enum {
	EMBLEM_HIDDEN = 0, // waiting for the event camera
	EMBLEM_APPEAR = 1,
	EMBLEM_SHOW   = 2,
	EMBLEM_TAKEN  = 3,
};

typedef char TMsnObjEmblem_size_check[(sizeof(TMsnObjEmblem) == 0x210) ? 1 : -1];

// 0x8010487C
extern "C" void loadEmblemArc()
{
	loadArc("geobj_emblem.arc", &gEmblemArc);
	gEmblemModel   = (NJS_OBJECT*)findArcExport(gEmblemArc, "model");
	gEmblemMotions = (NJS_MOTION**)findArcExport(gEmblemArc, "motions");
	gEmblemTexList = (NJS_TEXLIST*)findArcExport(gEmblemArc, "texlist");
	bindGvmTextures((u8*)findArcExport(gEmblemArc, "texture"), gEmblemTexList);
}

// 0x801048EC
extern "C" void releaseEmblemArc()
{
	releaseTextures(gEmblemTexList);
	releaseResource(gEmblemArc);
}

// 0x80104918
extern "C" TObject* createTMsnObjEmblem(TObject* parent, SetDesc* desc)
{
	return new TMsnObjEmblem(parent, desc);
}

// 0x8010496C
// Int 0 picks the kind 0 normal or 1 a flag
void TMsnObjEmblem::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mKind     = desc->getInt(0);
	int value = mKind;
	if (value < 0) {
		value = 0;
	} else if (value > 1) {
		value = 1;
	}
	mKind     = value;
	mCameraId = CLAMP_RANGE(desc->getInt(2), 0, 3);
	switch (mKind) {
	case 0:
		break;
	case 1:
		mSwitch = desc->getInt(1);
		int sw  = mSwitch;
		if (sw < 0) {
			sw = 0;
		} else if (sw > 0x7FF) {
			sw = 0x7FF;
		}
		mSwitch = sw;
		break;
	}

	mPlace.mFlags = 3;
	mPlace.mPos.x = mPos.x;
	mPlace.mPos.y = mPos.y;
	mPlace.mPos.z = mPos.z;
	njXYZAngleToQuaternion((const s32*)&mRot, (NJS_QUATERNION*)mPlace.mRot);
	mPlace.update();
}

// 0x80104AD0
// The lines are the set-file labels
void TMsnObjEmblem::debugPrint(void*)
{
	switch (mKind) {
	case 0:
		njPrintC(0xA0013, ":NORMAL TYPE");
		break;
	case 1:
		njPrintC(0xA0013, ":FLAG TYPE");
		njPrintC(0xA0014, ":ID");
		break;
	}
	njPrintC(0xA0015, ":CAMERA ID");
}

// 0x80104B5C
TMsnObjEmblem::TMsnObjEmblem(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)sClassName;
	mSize = 0x210;
	initFromDesc(desc);
	setLinkDone((SetLink*)&mLink);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 42, 0);
	mtxIdentity(mView);
	setup();
}

// 0x80104C30
TMsnObjEmblem::~TMsnObjEmblem()
{
	fxRemove(mFx);
	colliRetire(mShape);
	mShape = 0;
	if (mCamera != 0) {
		mCamera->mFlags |= 1;
		mCamera = 0;
	}
	seStop3D(&mSound);
	if (mGenA != 0) {
		mGenA->mFlags |= 1;
		mGenA = 0;
	}
	if (mGenB != 0) {
		mGenB->mFlags |= 1;
		mGenB = 0;
	}
	removeDrawTask(mDrawTask);
}

// 0x80104D38
void TMsnObjEmblem::setup()
{
	_15C          = 0;
	sEmblemTint.a = 0x60;
	mGenA         = 0;
	mGenB         = 0;
	mShape        = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 255, 255, 0xFE);
	colliCopyShapes(mShape, sMsnEmblemShape);
	colliPlaceShapesRotY(mShape, &mPos, mRot.y, sMsnEmblemShape);
	mMotion.init(sMsnEmblemMotions, gEmblemMotions);
	mMotion.play(0);
	mModel.setModelNodes(gEmblemModel, 1);
	mModel.mState   = (DrawState*)getDefaultDrawState();
	mModel.mTexList = gEmblemTexList;
	mModel.mMotion  = &mMotion;
	mModel.calcMotion();
	placeGenerators();
	mCamera = 0;
	start();

	EggHits hits;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)getPos(), 0xD) != 0 && (hits.mMask & 1) != 0) {
		shadowSetFromContact(&mShadow, (u8*)&hits);
		shadowDisable(&mShadow);
	}
}

// 0x80104EA8
// The event camera message
int TMsnObjEmblem::takeCamera(ObjMsg* msg)
{
	mCamRef.set(msg);
	return 1;
}

// 0x80104ED0
// A normal emblem or a flag one whose switch is set shows at once
void TMsnObjEmblem::start()
{
	if (keyBitIsSet((u16)mSwitch) == 1 || mKind == 0) {
		mGenA = particleGenByIdDefault(getPos(), 0, 0x19B);
		mGenB = particleGenByIdDefault(getPos(), 0, 0x19A);
		mMotion.play(1);
		mtxSetTransV(mFxMatrix, getPos());
		mFx = fxSpawnFollow(1, 7, 0x28, mFxMatrix);
		sePlay3DHeld(400.0f, &mSound, 0x6B, 0, getPos());
		mState = EMBLEM_SHOW;
	} else {
		mFx = 0;
		mtxIdentity(mFxMatrix);
		mState = EMBLEM_HIDDEN;
	}
}

// 0x80104FF4
void TMsnObjEmblem::openCamera()
{
	mCamera = eventCamArcPlay(0.2f, 0.0f, sEmblemCameras[mCameraId]);
	if (mCamera != 0) {
		((TCamera*)mCamera)->setCut();
		mCamera->attachToMtx();
		mtxCopy(mCamera->getMtx(), mPlace.mMatrix);
	}
}

// 0x80105068
void TMsnObjEmblem::closeCamera()
{
	if (mCamera != 0) {
		mCamera->mFlags |= 1;
		mCamera = 0;
	}
}

// 0x8010508C
// Moves the generators and the shape to the model's node
void TMsnObjEmblem::placeGenerators()
{
	f32 mtx[12];
	Vec3f pos;
	mtxConcat(mtx, mPlace.mMatrix, mModel.mNodes->mMatrix);
	mtxGetPos(mtx, &pos);
	if (mGenA != 0) {
		mGenA->mPos = pos;
		mGenB->mPos = pos;
	}
	colliPlaceShapes(mShape, &pos, sMsnEmblemShape);
}

// 0x80105128
void TMsnObjEmblem::checkTaken()
{
	for (ColliContact* contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		if (colliContactInGroup1(contact) == 0 || colliContactNoBit10(contact) == 0) {
			continue;
		}
		TObjAction* other = findObjById(colliContactOwner(contact));
		if (other == 0) {
			continue;
		}
		EmblemTakeMsg msg;
		msg.mType = 0x86;
		msg.mFrom = mObjId;
		if (other->sendToHandle(&msg) == 0) {
			continue;
		}
		if (mGenA != 0) {
			mGenA->mFlags |= 1;
			mGenA = 0;
		}
		if (mGenB != 0) {
			mGenB->mFlags |= 1;
			mGenB = 0;
		}
		if (mFx != 0) {
			fxRemove(mFx);
			mFx = 0;
		}
		seStop3D(&mSound);
		const Vec3f* pos = (const Vec3f*)colliGetShapes(mShape);
		Vec3f at;
		at.set(pos->x, pos->y, pos->z);
		sePlay3D(0x72, &at);
		mState = EMBLEM_TAKEN;
		return;
	}
}

// 0x80105264
void TMsnObjEmblem::waitCamera()
{
	if (mCamRef.isActive()) {
		openCamera();
		if (mGenA == 0) {
			mGenA = particleGenByIdDefault(getPos(), 0, 0x19B);
		}
		if (mGenB == 0) {
			mGenB = particleGenByIdDefault(getPos(), 0, 0x19A);
		}
		mMotion.play(0);
		_15C = 3;
		sePlayFor(0x6A, 600);
		mState = EMBLEM_APPEAR;
	}
}

// 0x8010532C
void TMsnObjEmblem::appear()
{
	if (mCamera == 0) {
		openCamera();
	} else if (mMotion.isDone() && mCamera->isDone() != 0) {
		mCamRef.release();
		closeCamera();
		mMotion.play(1);
		mtxSetTransV(mFxMatrix, &mPos);
		mFx = fxSpawnFollow(1, 7, 0x28, mFxMatrix);
		sePlay3DHeld(400.0f, &mSound, 0x6B, 0, getPos());
		mState = EMBLEM_SHOW;
	} else {
		mMotion.update();
		mModel.calcMotion();
		placeGenerators();
	}
}

// 0x80105418
void TMsnObjEmblem::show()
{
	checkTaken();
	mMotion.update();
	mModel.calcMotion();
	placeGenerators();
	if (mFx != 0) {
		Vec3f pos;
		mtxGetPos(mModel.mNodes->mMatrix, &pos);
		mtxTranslateV(mFxMatrix, mPlace.mMatrix, &pos);
	}
}

// 0x80105488
void TMsnObjEmblem::taken()
{
}

// 0x8010548C
void TMsnObjEmblem::draw()
{
	if (objDebugEnabled(this) != 0) {
		mState = EMBLEM_SHOW;
	} else {
		switch (mState) {
		case EMBLEM_HIDDEN:
			waitCamera();
			break;
		case EMBLEM_SHOW:
			show();
			break;
		case EMBLEM_APPEAR:
			appear();
			break;
		case EMBLEM_TAKEN:
			taken();
			break;
		}
	}
}

// 0x80105524
void TMsnObjEmblem::vfn_10()
{
	*(Mtx34*)mView = *(Mtx34*)camGetDrawMtx();
	if (mState == EMBLEM_APPEAR) {
		mDrawTask->mFlags &= ~1;
		shadowDisable(&mShadow);
	} else if (mState == EMBLEM_HIDDEN || mState == EMBLEM_TAKEN) {
		mDrawTask->mFlags |= 1;
		shadowDisable(&mShadow);
	} else if (cameraIsVisible(1000.0f, 15.0f, &mPos, 0) != 0) {
		mDrawTask->mFlags &= ~1;
		shadowEnable(&mShadow);
	} else {
		mtxIdentity(mView);
		mDrawTask->mFlags |= 1;
		shadowDisable(&mShadow);
	}
}

// 0x80105620
// Without the model it draws a black box
void TMsnObjEmblem::render()
{
	if (gEmblemModel == 0) {
		f32 mtx[12];
		f32 view[12];
		mtxConcat(view, (f32*)gjGetMatrix(), mPlace.mMatrix);
		mtxRotateX(mtx, view, 0x4000);
		mtxScale(5.0f, 2.0f, 5.0f, mtx, mtx);
		debugWireCylinder(mtx, &gColorBlack);
	} else {
		njPushMatrixEx();
		njMultiMatrix(0, mPlace.mMatrix);
		emblemShineBegin();
		mModel.draw();
		emblemShineEnd();
		njPopMatrixEx();
	}
}

// 0x801056C4
// Projects texture 0 from the view onto the model as a post stage
extern "C" void emblemShineBegin()
{
	f32 light[12];
	frameTexSetCur();
	gjLoadTextureNum(0, 0, 0, 1);
	EmblemCamera* camera = (EmblemCamera*)camGetDrawView();
	C_MTXLightPerspective((float*)light, (f32)(0.75 * (0.0054931640625 * (f64)camera->mFov)), camera->mAspect, -0.55f, -0.55f, 0.55f,
	                      0.55f);
	PSMTXConcat(light, _gj_matrix_, light);
	GXLoadTexMtxImm(light, GX_TEXMTX5, GX_MTX3x4);
	gjSetNumPostTexGens(1);
	gjSetNumPostTevStages(1);
	gjSetPostTevStageCallback((void*)emblemShineTev);
	gjSetPostTexGenCallback((void*)emblemShineTexGen);
	_nj_control_3d_flag_ |= 0x80000;
	shadowAlphaBegin();
}

// 0x801057A8
extern "C" void emblemShineEnd()
{
	shadowAlphaEnd();
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	_nj_control_3d_flag_ &= ~0x80000;
}

// 0x801057E4
// Adds the shine scaled by the colour to the colour
extern "C" void emblemShineTev(u8 stage)
{
	GXSetTevColor(GX_TEVREG0, *(GXColor*)&sEmblemTint);
	int st = stage;
	GXSetTevColorIn(st, GX_CC_ZERO, GX_CC_TEXC, GX_CC_A0, GX_CC_CPREV);
	GXSetTevColorOp(st, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(st, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(st, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(st, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
}

// 0x801058A0
extern "C" void emblemShineTexGen(u8 gen)
{
	GXSetTexCoordGen2(gen, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX5, GX_FALSE, GX_PTIDENTITY);
}

// ---- TEmblemGet the same file ----

typedef char TEmblemGet_size_check[(sizeof(TEmblemGet) == 0x1D4) ? 1 : -1];

// 0x801058D8
extern "C" TEmblemGet* createEmblemGet(TObject* parent, const f32* matrix)
{
	return new TEmblemGet(parent, matrix);
}

// 0x8010592C
// Builds one at a place turned by rotY
extern "C" TEmblemGet* spawnEmblemGet(TObject* parent, const Vec3f* pos, int rotY)
{
	f32 matrix[12];
	mtxSetTransV(matrix, pos);
	mtxRotateY(matrix, matrix, rotY);
	return createEmblemGet(parent, matrix);
}

// 0x8010597C
TEmblemGet::TEmblemGet(TObject* parent, const f32* matrix)
    : TObject(parent)
{
	_00   = (u32)gTEmblemGetName;
	mSize = 0x1D4;
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x2A, 0);
	mtxCopy(mMatrix, matrix);
	mtxIdentity(mView);
	mAnim.init(sMsnEmblemMotions, gEmblemMotions);
	mAnim.play(2);
	mModel.setModelNodes(gEmblemModel, 1);
	mModel.mState   = &mState;
	mModel.mTexList = gEmblemTexList;
	mModel.mMotion  = &mAnim;
	mModel.calcMotion();
	mFx = 0;
	mtxIdentity(mFxMatrix);

	Vec3f at;
	mtxGetPos(mMatrix, &at);
	mGenA = particleGenById(this, &at, 0, 0x19B);
	mGenB = particleGenById(this, &at, 0, 0x19A);

	mCamera         = 0;
	mState.mFlags   = DRAWSTATE_AMBIENT;
	mState.mAmbient = 0xFFFFFFFF;
	mFading         = 0;
	mFadeStep       = 0;
	_14C            = 0;
	mSounded        = 0;
	mLight.mFlags   = 2;

	Vec3f node;
	mtxGetPos(mModel.mNodes->mMatrix, &node);
	mtxMultVec(mMatrix, &node, &node);
	mLight.mPos   = node;
	mLight.mKa[0] = 1.0f;
	mLight.mKa[1] = 0.001f;
	mLight.mKa[2] = 0.001f;
	mLight.mColor = 0xFF2020FF;
	playerAddLight(playerGetDrawView(), &mLight);
}

// 0x80105B50
TEmblemGet::~TEmblemGet()
{
	playerRemoveLight(playerGetDrawView(), &mLight);
	fxRemove((Fx*)mFx);
	if (mCamera != 0) {
		mCamera->mFlags |= 1;
		mCamera = 0;
	}
	if (mGenA != 0) {
		mGenA->mFlags |= 1;
		mGenA = 0;
	}
	if (mGenB != 0) {
		mGenB->mFlags |= 1;
		mGenB = 0;
	}
	removeDrawTask(mTask);
}

// 0x80105C60
void TEmblemGet::draw()
{
	if (mCamera == 0) {
		startCamera();
		return;
	}
	if (mSounded == 0) {
		sePlayFor(0x29A, 0x2D0);
		mSounded = 1;
	}
	if (mAnim.isDone() && mCamera->isDone()) {
		return;
	}
	if (mAnim.passedFrame(130.0f)) {
		Vec3f node;
		mtxGetPos(mModel.mNodes->mMatrix, &node);
		mtxTranslateV(mFxMatrix, mMatrix, &node);
		mFx = fxSpawnFollow(1, 7, 0x2A, mFxMatrix);
	} else if (mAnim.passedFrame(170.0f)) {
		if (mFx != 0) {
			fxRemove(mFx);
			mFx = 0;
		}
		if (mGenA != 0) {
			mGenA->mFlags |= 1;
			mGenA = 0;
		}
		if (mGenB != 0) {
			mGenB->mFlags |= 1;
			mGenB = 0;
		}
		mFading   = 1;
		mFadeStep = 5;
	} else if (mAnim.passedFrame(215.0f)) {
		setColorAlpha(&mState.mAmbient, 0);
		mFading       = 0;
		mLight.mColor = gColorBlack;
	}
	if (mFading != 0) {
		addColorAlpha(&mState.mAmbient, -mFadeStep);
		int step   = -mFadeStep;
		u32 colour = mLight.mColor;
		addColorRed(&colour, step);
		addColorGreen(&colour, step);
		addColorBlue(&colour, step);
		mLight.mColor = colour;
	}
	mModel.calcMotion();
	updateEffects();
	if (mFx != 0) {
		Vec3f node;
		mtxGetPos(mModel.mNodes->mMatrix, &node);
		mtxTranslateV(mFxMatrix, mMatrix, &node);
	}
	mAnim.update();
}

// 0x80105E78
struct EmblemMtx {
	f32 m[12];
};

void TEmblemGet::vfn_10()
{
	*(EmblemMtx*)mView = *(EmblemMtx*)camGetDrawMtx();
}

// 0x80105EC8
void TEmblemGet::render()
{
	if (gEmblemModel == 0) {
		f32 shadow[12];
		f32 matrix[12];
		mtxConcat(matrix, (const f32*)gjGetMatrix(), mMatrix);
		mtxRotateX(shadow, matrix, 0x4000);
		mtxScale(5.0f, 2.0f, 5.0f, shadow, shadow);
		debugWireCylinder(shadow, &gColorBlack);
	} else {
		f32 matrix[12];
		mtxConcat(matrix, (const f32*)gjGetMatrix(), mMatrix);
		njPushMatrix(matrix);
		emblemShineBegin();
		mModel.draw();
		emblemShineEnd();
		njPopMatrixEx();
	}
}

// 0x80105F78
void TEmblemGet::startCamera()
{
	mCamera = eventCamArcPlay(1.0f, 0.0f, 0);
	mCamera->attachToMtx();
	*(EmblemMtx*)mCamera->getMtx() = *(EmblemMtx*)mMatrix;
}

// 0x80105FE8
void TEmblemGet::updateEffects()
{
	Vec3f at;
	mtxGetPos(mModel.mNodes->mMatrix, &at);
	mtxMultVec(mMatrix, &at, &at);
	if (mGenA != 0) {
		mGenA->mPos = at;
	}
	if (mGenB != 0) {
		mGenB->mPos = at;
	}
	mLight.mPos = at;
}
