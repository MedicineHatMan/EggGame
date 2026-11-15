#include "object/ObjModel.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "Ninja/ninja.h"
#include "stage/SetActor.h"
#include "egg/ObjEgg.h"
#include "engine/FlagCloth.h"
#include "engine/CharaColli.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCamera.h"
#include "ui/AeObjControl.h"
#include "animal/TAniRacePath.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "object/mission/TMsnObjGoalPole.h"
#include "object/TObjLauncher.h"
#include "ui/AeArc.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "engine/Color.h"
#include "core/MathUtil.h"
#include "engine/Colli.h"
#include "core/VecMath.h"
#include "stage/SetSlot.h"
#include "core/Mem.h"
#include "sound/SoundCtrl.h"
struct DrawTask;
struct CharaColli;
struct NJS_TEXLIST;
struct ColliShape;
struct SetLink;
struct NJS_QUATERNION;

// 0x801060D0
extern "C" void initGoalArc()
{
	sGoalArc         = 0;
	sGoalModel       = 0;
	sGoalTexList     = 0;
	sGoalFlagTexList = 0;
}

// 0x801060E8
extern "C" void releaseGoalArc()
{
	if (sGoalArc != 0) {
		releaseTextures(sGoalFlagTexList);
		releaseTextures(sGoalTexList);
		releaseResource(sGoalArc);
		sGoalArc         = 0;
		sGoalModel       = 0;
		sGoalTexList     = 0;
		sGoalFlagTexList = 0;
	}
}

// 0x8010613C
// The event script's load_goal
extern "C" int scriptLoadGoal(void*)
{
	loadGoalArc();
	return 1;
}

// 0x80106160
extern "C" void loadGoalArc()
{
	loadArc("geobj_goal.arc", &sGoalArc);
	sGoalModel       = (NJS_OBJECT*)findArcExport(sGoalArc, "model");
	sGoalTexList     = (NJS_TEXLIST*)findArcExport(sGoalArc, "texlist");
	sGoalFlagTexList = (NJS_TEXLIST*)findArcExport(sGoalArc, "texlist_flag");
	bindGvmTextures((u8*)findArcExport(sGoalArc, "texture"), sGoalTexList);
	bindGvmTextures((u8*)findArcExport(sGoalArc, "texture"), sGoalFlagTexList);
}

#define WGPIPE_F32 (*(volatile f32*)0xCC008000)

static const char* sClassName = "TMsnObjGoalPole";

typedef char TMsnObjGoalPole_size_check[(sizeof(TMsnObjGoalPole) == 0x1C8) ? 1 : -1];

enum {
	POLE_WAIT   = 0, // for the race
	POLE_RACE   = 1,
	POLE_SECOND = 2, // the player is in waiting for the racer
	POLE_DONE   = 3,
};

// 0x801061E8
extern "C" TObject* createTMsnObjGoalPole(TObject* parent, SetDesc* desc)
{
	return new TMsnObjGoalPole(parent, desc);
}

// 0x8010623C
// The record itself is only read by the base
void TMsnObjGoalPole::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mPlace.mFlags = 3;
	mPlace.mPos.x = mPos.x;
	mPlace.mPos.y = mPos.y;
	mPlace.mPos.z = mPos.z;
	njXYZAngleToQuaternion((const s32*)&mRot, (NJS_QUATERNION*)mPlace.mRot);
	mPlace.update();

	if (sGoalModel != 0) {
		mModel.setModel(sGoalModel);
		mModel.mTexList = sGoalTexList;
		mModel.mState   = getDefaultDrawState();
		mModel.mPlace   = &mPlace;
		GjModel* model  = sGoalModel->model;
		mCenter.x       = model->center.x;
		mCenter.y       = model->center.y;
		mCenter.z       = model->center.z;
		mRadius         = 30.0f + sGoalModel->model->r;
		vecAdd(&mCenter, &mCenter, &mPos);
	} else {
		mCenter.x = 0.0f;
		mCenter.y = 0.0f;
		mCenter.z = 0.0f;
		mRadius   = 1.0f;
	}
}

// 0x8010632C
void TMsnObjGoalPole::debugPrint(void*)
{
}

// 0x80106330
// Builds the flag from the loaded model then two shapes the pole and the cloth
extern "C" void goalPoleBuild(TMsnObjGoalPole* self)
{
	self->mState = 0;
	if (sGoalModel != 0) {
		NJS_OBJECT* pole = sGoalModel->child;
		pole->evalflags &= ~2u;
	}

	ClothMatrix matrix;
	mtxSetTransV(matrix.m, self->getPos());
	mtxTranslate((float*)matrix.m, (const float*)matrix.m, 0.0f, 50.0f, 0.0f);
	mtxRotateY(matrix.m, matrix.m, self->mRot.y);

	self->mCloth = new FlagCloth(matrix, 3, 3, 30.0f, 20.0f, 0.5f, 0.99f, 1.0f);
	if (self->mCloth != 0) {
		self->mCloth->pin(0, 0);
		self->mCloth->pin(0, 3);
	}
	if (self->mCloth != 0) {
		Vec3f aim;
		aim.x = 0.0f;
		aim.y = 0.0f;
		aim.z = 1.0f;
		vecRotateY(&aim, &aim, self->mRot.y);
		self->mCloth->setWind(&aim, 0, 2.5f);
	}

	self->mShape = colliCreate(1, 1);
	colliSetOwner(self->mShape, (u16)self->mObjId);
	colliSetMasks(self->mShape, 4, 255, 255, 0xDE);
	colliCopyShapes(self->mShape, sMsnGoalPoleShape);
	ColliShape* part = colliGetShape(self->mShape, 0);
	part->mFlags |= 0x10;
	colliPlaceShapes(self->mShape, &self->mPos, sMsnGoalPoleShape);

	self->mClothColli = colliCreate(1, 1);
	colliSetOwner(self->mClothColli, (u16)self->mObjId);
	colliSetMasks(self->mClothColli, 4, 255, 0xF6, 0xF6);
	colliShapeMakeCylinder(colliGetShapes(self->mClothColli), &self->mPos, 1, 2.0f, 0.5f * self->mRadius);
}

// 0x80106554
TMsnObjGoalPole::TMsnObjGoalPole(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)sClassName;
	mSize = 0x1C8;
	initFromDesc(desc);
	setLinkDone((SetLink*)&mLink);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	goalPoleBuild(this);
}

// 0x80106614
TMsnObjGoalPole::~TMsnObjGoalPole()
{
	colliRetire(mClothColli);
	colliRetire(mShape);
	mShape = 0;
	delete mCloth;
	removeDrawTask(mDrawTask);
}

// 0x801066C0
// The player reaching the pole wins
void TMsnObjGoalPole::checkFirst()
{
	for (ColliContact* contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		if (colliContactNoBit10(contact) == 0) {
			continue;
		}
		if (colliContactInGroup1(contact) != 0) {
			sePlay(0x38);
			aeCreateGoal();
			aniRacePlaceSecond();
			mState = POLE_SECOND;
		} else if (colliInGroup(contact->mOther, 0x20) != 0) {
			sePlay(0x38);
			aniRacePlaceFirst();
			aniRacePlaced();
			mState = POLE_DONE;
		}
	}
}

// 0x80106768
void TMsnObjGoalPole::checkSecond()
{
	for (ColliContact* contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		if (colliContactNoBit10(contact) != 0 && colliInGroup(contact->mOther, 0x20) != 0) {
			aniRacePlaced();
			mState = POLE_DONE;
		}
	}
}

// 0x801067D8
void TMsnObjGoalPole::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (sGoalArc == 0) {
		return;
	}
	switch (mState) {
	case POLE_WAIT:
		if (aniRaceHasState()) {
			mState = POLE_RACE;
		}
		break;
	case POLE_RACE:
		checkFirst();
		break;
	case POLE_SECOND:
		checkSecond();
		break;
	case 3:
		break;
	}

	static int sTick = 0;
	if (30.0f == (f32)sTick++) {
		int turn = (int)(8192.0f * ((1.0f / 32768.0f) * (f32)rand()));
		if (turn >= 0x2000) {
			turn = 0x1FFF;
		}
		mWindTarget = turn - 0x1000;
		sTick       = 0;
	}
	mWindAngle = angleApproach(mWindAngle, mWindTarget, 0x40);
	sGoalModel->child->ang[1] += mWindAngle;

	Vec3f wind;
	wind.x = 0.0f;
	wind.y = 0.0f;
	wind.z = 1.0f;
	vecRotateY(&wind, &wind, mWindTarget + mRot.y);
	mCloth->setWind(&wind, 0, 2.5f);
	mCloth->step();
	calcNormals();
}

// 0x80106998
void TMsnObjGoalPole::vfn_10()
{
	if (cameraIsVisible(1000.0f, mRadius, &mCenter, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80106A00
// Without the model it draws a black box
void TMsnObjGoalPole::render()
{
	if (sGoalModel == 0) {
		f32 view[12];
		f32 mtx[12];
		mtxConcat(view, (f32*)gjGetMatrix(), mPlace.mMatrix);
		mtxScale(15.0f, 2.0f, 15.0f, mtx, view);
		debugWireCylinder(mtx, &gColorBlack);
	} else {
		mModel.draw();
		drawCloth();
	}
}

// 0x80106A80
// The cloth as one strip a row in world space textured across and down
void TMsnObjGoalPole::drawCloth()
{
	if (sGoalFlagTexList == 0) {
		return;
	}
	SpringPoint* points = mCloth->mPoints;
	u16 cols            = mCloth->mCols;
	u16 rows            = mCloth->mRows;

	GXSetZCompLoc(GX_FALSE);
	gjSetZBufferMode(1);
	gjGXSetBlendMode(0);
	njPushMatrixEx();
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	njSetTexture(sGoalFlagTexList);
	gjLoadTextureNum(0, 0, 0, 0);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	f32 identity[12];
	PSMTXIdentity(identity);
	GXLoadTexMtxImm(identity, GX_TEXMTX0, GX_MTX2x4);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_ALPHA0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXColor white = { 0xFF, 0xFF, 0xFF, 0xFF };
	GXSetChanMatColor(GX_COLOR0, white);
	GXSetNumTexGens(1);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GXSetCullMode(GX_CULL_NONE);

	f32 du = 1.0f / (f32)(cols - 1);
	f32 dv = 1.0f / (f32)(rows - 1);
	for (u16 y = 0; (int)y < rows - 1; y++) {
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, cols * 2);
		f32 v     = 1.0f - dv * (f32)y;
		f32 vNext = v - dv;
		for (u16 x = 0; x < cols; x++) {
			int i              = x + y * cols;
			f32 u              = du * (f32)x;
			SpringPoint* below = &points[i + cols];
			WGPIPE_F32         = below->mPos.x;
			WGPIPE_F32         = below->mPos.y;
			WGPIPE_F32         = below->mPos.z;
			WGPIPE_F32         = mNormals[y + 1][x].x;
			WGPIPE_F32         = mNormals[y + 1][x].y;
			WGPIPE_F32         = mNormals[y + 1][x].z;
			WGPIPE_F32         = u;
			WGPIPE_F32         = vNext;
			SpringPoint* here  = &points[i];
			WGPIPE_F32         = here->mPos.x;
			WGPIPE_F32         = here->mPos.y;
			WGPIPE_F32         = here->mPos.z;
			WGPIPE_F32         = mNormals[y][x].x;
			WGPIPE_F32         = mNormals[y][x].y;
			WGPIPE_F32         = mNormals[y][x].z;
			WGPIPE_F32         = u;
			WGPIPE_F32         = v;
		}
	}
	njPopMatrixEx();
}

// 0x80106E24
// Two face normals per cell of the 4x4 cloth then each point's normal from the faces around it
void TMsnObjGoalPole::calcNormals()
{
	static HitVec sFaces[3][6];
	SpringPoint* points = mCloth->mPoints;
	u16 cols            = mCloth->mCols;
	u16 rows            = mCloth->mRows;

	for (u16 y = 0; y < 3; y++) {
		HitVec* row = sFaces[y];
		for (u16 x = 0; x < 3; x++) {
			u16 i = x + y * cols;
			Vec3f a;
			a.x     = points[i].mPos.x;
			a.y     = points[i].mPos.y;
			a.z     = points[i].mPos.z;
			Vec3f b = points[i + cols + 1].mPos;
			Vec3f c = points[i + cols].mPos;
			Vec3f e1;
			Vec3f e2;
			vecSub(&e1, &b, &a);
			vecSub(&e2, &c, &a);
			njOuterProduct(&e2, &e1, &row[x * 2]);
			vecNormalize(&row[x * 2], &row[x * 2]);
			c = points[i + 1].mPos;
			Vec3f e3;
			Vec3f e4;
			vecSub(&e3, &c, &a);
			vecSub(&e4, &b, &a);
			njOuterProduct(&e4, &e3, &row[x * 2 + 1]);
			vecNormalize(&row[x * 2 + 1], &row[x * 2 + 1]);
		}
	}

	int lastX = cols - 1;
	int lastY = rows - 1;
	for (u16 y = 0; y < rows; y++) {
		HitVec* cur  = sFaces[y];
		HitVec* prev = sFaces[y - 1];
		for (u16 x = 0; x < cols; x++) {
			Vec3f n;
			n.x      = 0.0f;
			n.y      = 0.0f;
			n.z      = 0.0f;
			int face = x * 2;
			if ((int)x < lastX && (int)y < lastY) {
				vecAdd(&n, &n, &cur[face]);
				vecAdd(&n, &n, &cur[face + 1]);
			}
			if (x != 0 && y != 0) {
				vecAdd(&n, &n, &prev[face - 1]);
				vecAdd(&n, &n, &prev[face - 2]);
			}
			if ((int)x < lastX && y != 0) {
				vecScaleAdd(2.0f, &n, &prev[face], &n);
			}
			if (x != 0 && (int)y < lastY) {
				vecScaleAdd(2.0f, &n, &cur[face - 1], &n);
			}
			vecNormalize(&n, &n);
			mNormals[y][x] = n;
		}
	}
}

// 0x80107184
int TMsnObjGoalPole::getKind()
{
	return 3;
}
