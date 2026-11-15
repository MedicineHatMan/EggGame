#include "stage/SetManager.h"
struct DrawTask;
#include "stage/SetActor.h"
#include "object/ObjAction.h"
#include "object/ObjModel.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "enemy/EnemyLight.h"
#include "Ninja/ninja.h"
#include "object/ObjArchive.h"
#include "stage/StageObjUtil.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "sound/Sound3DPlay.h"
#include "camera/TCamPath.h"
#include "engine/Colli.h"
#include "core/Pad.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "stage/SetSlot.h"
#include "core/VecMath.h"
#include "engine/TParticle.h"
#include "object/yellow/TObjYellowPillar.h"
struct CharaColli;
struct ColliShape;
struct SetLink;
struct MeshColli;
struct MeshData;
struct MeshObject;
struct ColliContact;
struct DrawState;

static inline Vec3f colliNeg(const Vec3f& v)
{
	Vec3f r;
	r.x = -v.x;
	r.y = -v.y;
	r.z = -v.z;
	return r;
}

typedef char TObjYellowPillar_size_check[(sizeof(TObjYellowPillar) == 0x14C) ? 1 : -1];

const f32 gYellowPillarSetRange = 800.0f;

// 0x8016E50C
extern "C" TObject* createTObjYellowPillar(TObject* parent, SetDesc* desc)
{
	return new TObjYellowPillar(parent, desc);
}

const char* gTObjYellowPillarName = "TObjYellowPillar";

// 0x8016E560
TObjYellowPillar::TObjYellowPillar(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjYellowPillarName;
	mSize = 0x14C;

	mModelData = getGeObjModels()[5];
	mTexList   = getGeObjTextures()[5];
	mColliData = (MeshData*)gGeObjCommon->mSet.mMeshes[2];
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	createShapes();
	initColli();
	initFromDesc(desc);
	vfn_74();
}

// 0x8016E64C
TObjYellowPillar::~TObjYellowPillar()
{
	freeColli();
	freeShapes();
	removeDrawTask(mDrawTask);
}

// 0x8016E6D8
void TObjYellowPillar::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mPower        = desc->getInt(0);
	mTerritoryRad = desc->getFloat(0);
}

// 0x8016E740
void TObjYellowPillar::debugPrint(void*)
{
	njPrintC(0xA0013, ":POEWR");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, ":TERRITORY RAD");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x8016E7E0
int TObjYellowPillar::onHit(const TObjAction*, int, int* info)
{
	HitInfo* hit = (HitInfo*)info;
	resetHitInfo(hit);
	hit->mDamage = mPower;
	hit->mFlags.setBit(13);
	return 1;
}

// 0x8016E82C
void TObjYellowPillar::vfn_74()
{
	updateMatrix();
	updateShapes();
	updateColli();
	mModel.setModel(mModelData);
	mModel.mTexList = mTexList;
	mModel.mState   = &mState;
	setLinkFree((SetLink*)&mLink);
	mLight.mTimer = 0;
	mAct          = 0;
}

// 0x8016E898
void TObjYellowPillar::setColliBox(void* colli, const Vec3f* center, f32 radius)
{
	Vec3f size;
	size.x = 1.0f;
	size.y = 1.0f;
	size.z = 1.0f;
	gjScaleVector(radius, &size, &size);

	Vec3f high;
	high.x = size.x;
	high.y = size.y;
	high.z = size.z;
	Vec3f low;
	Vec3f t = colliNeg(size);
	low.x   = t.x;
	low.y   = t.y;
	low.z   = t.z;

	gjPushUnitMatrix();
	njTranslateEx(center);
	njCalcPointEx(&high, &high);
	njCalcPointEx(&low, &low);
	njPopMatrixEx();

	f32 box[6];
	boxAroundLine(box, &low, &high);
	meshColliSetBox((MeshObject*)colli, (const f32*)box);
}

// 0x8016E984
void TObjYellowPillar::updateMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	mtxRotateX(mMatrix, mMatrix, mRot.x);
	GjModel* model = mModelData->model;
	mCenter        = *(Vec3f*)&model->center;
	mtxMultVec(mMatrix, &mCenter, &mCenter);
	mRadius = mModelData->model->r;
}

// 0x8016EA18
void TObjYellowPillar::createShapes()
{
	mShape = colliCreate(7, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 255, 255, 214);

	ColliShape shapes[7];
	for (int i = 0; i < 7; i++) {
		Vec3f pos;
		pos.x = mPos.x;
		pos.y = mPos.y;
		pos.z = mPos.z;
		pos.z -= 12.5f;
		f32 off = 12.5f * (f32)i;
		pos.y += off;
		colliShapeMakeSphere(&shapes[i], &pos, 1, 19.0f);
	}
	colliCopyShapes(mShape, shapes);

	mTerritory = colliCreate(1, 1);
	colliSetOwner(mTerritory, (u16)mObjId);
	colliSetMasks(mTerritory, 4, 255, 255, 254);
	colliShapeMakeCylinder(shapes, &mPos, 1, 1.0f, 50.0f);
	colliCopyShapes(mTerritory, (const ColliShape*)shapes);
}

// 0x8016EB74
void TObjYellowPillar::updateShapes()
{
	for (int i = 0; i < colliGetShapeCount(mShape); i++) {
		ColliShape* shape = colliGetShape(mShape, i);
		shape->mPos.x     = 0.0f;
		f32 y             = 12.5f * (f32)i;
		shape->mPos.y     = 12.5f + y;
		shape->mPos.z     = -12.5f;
		njCalcPoint(mMatrix, &shape->mPos, &shape->mPos);
	}
	ColliShape* shape = colliGetShape(mTerritory, 0);
	shape->mPos       = mPos;
	shape->mPos.y += 50.0f;
	shape->mSizeX = mTerritoryRad;
}

// 0x8016EC8C
void TObjYellowPillar::freeShapes()
{
	colliRetire(mTerritory);
	colliRetire(mShape);
	mShape = 0;
}

// 0x8016ECCC
void TObjYellowPillar::initColli()
{
	mColli = (MeshObject*)meshColliCreate();
	meshColliSetData(mColli, mColliData);
	meshSetFlag2(mColli, 1);
	meshSetState100(mColli, 1);
}

// 0x8016ED20
void TObjYellowPillar::updateColli()
{
	setColliBox(mColli, &mCenter, 20.0f + mRadius);
	meshColliSetMatrix(mColli, (const f32*)mMatrix);
}

// 0x8016ED6C
void TObjYellowPillar::freeColli()
{
	meshColliFree(mColli);
}

// 0x8016ED90
void TObjYellowPillar::disableShapes()
{
	for (int i = 0; i < colliGetShapeCount(mShape); i++) {
		colliGetShape(mShape, i)->mFlags |= 8;
	}
}

// 0x8016EDEC
void TObjYellowPillar::enableShapes()
{
	for (int i = 0; i < colliGetShapeCount(mShape); i++) {
		colliGetShape(mShape, i)->mFlags &= ~8;
	}
}

// 0x8016EE48
void TObjYellowPillar::breakApart(const Vec3f* hit)
{
	Vec3f pos = *hit;
	pos.y -= mRadius;
	NJS_TEXLIST** textures = getGeObjTextures();
	NJS_OBJECT** models    = getGeObjModels();
	shatterModelRot(0, &pos, &mPos, &mRot, models[6], textures[6], 1.0f, 2.0f);
	sePlay3D(0x4C, &mCenter);
}

// 0x8016EED4
void TObjYellowPillar::draw()
{
	if (objDebugEnabled(this) != 0) {
		updateMatrix();
		updateShapes();
		updateColli();
		return;
	}
	if (!isInRange(this, &mPos, 850.0f)) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
		stateWait();
		break;
	case 1:
		stateFall();
		break;
	case 2:
		stateLanded();
		break;
	}
}

// 0x8016EF94
void TObjYellowPillar::stateWait()
{
	for (ColliContact* hit = mTerritory->mContacts; hit != 0; hit = hit->mNext) {
		if (colliContactNoBit10(hit) != 0) {
			colliDisable(mTerritory);
			meshColliRefresh(mColli);
			disableShapes();
			mAct = 1;
			break;
		}
	}
}

// 0x8016F008
// The rotation speeds up with the angle so the pillar falls like one
void TObjYellowPillar::stateFall()
{
	mRot.x += (*(volatile s32*)&mRot.x >> 4) + 0x10;
	if (mRot.x > 0x4000) {
		Vec3f pos;
		pos.x = 0.0f;
		pos.y = 0.0f;
		pos.z = 50.0f;
		vecRotateY(&pos, &pos, mRot.y);
		vecAdd(&pos, &pos, &mPos);
		particleOnceByIdRot((TObject*)this, (const Vec3f*)&pos, 1, 2, 0, mRot.y, 0.0f);
		mRot.x = 0x4000;
		meshClearState400(mColli);
		enableShapes();
		sePlay3D(0x25B, &mCenter);
		padVibStart(0, (int)this, 0xFF, 0, 0x19);
		mAct = 2;
	}
	updateMatrix();
	updateShapes();
	updateColli();
}

// 0x8016F104
struct PillarMsg {
	u16 mType; // _00
	u8 _02[6];
};

void TObjYellowPillar::stateLanded()
{
	for (ColliContact* hit = mShape->mContacts; hit != 0; hit = hit->mNext) {
		if (colliContactInGroup1(hit) == 0) {
			TObjAction* other = (TObjAction*)findObjById((u16)colliContactOwner(hit));
			HitInfo info;
			if (colliContactHasBit8(hit) != 0 && (colliContactInGroup8(hit) != 0 || other->onHit(this, 0, (int*)&info) != 0)) {
				breakApart(&hit->mPos);
				TObjAction* target = (TObjAction*)findObjById((u16)colliContactOwner(hit));
				if (target != 0) {
					PillarMsg first;
					first.mType = 0x2A;
					ObjMsg second;
					second.mType = 0x74;
					target->sendToHandle((ObjMsg*)&first);
					target->sendToHandle(&second);
				}
				setLinkForever((SetLink*)&mLink);
				mFlags |= 1;
				return;
			}
		}
	}
}

// 0x8016F234
void TObjYellowPillar::vfn_10()
{
	if (mLight.isVisible(mDrawTask, 700.0f, &mCenter, mRadius, 0) != 0) {
		setDrawAmbient(&mLight, &mState);
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8016F2B0
void TObjYellowPillar::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel.draw();
	njPopMatrixEx();
	if (objDebugEnabled(this) != 0) {
		colliDrawShapes(mShape);
		colliDrawShapes(mTerritory);
	}
}
