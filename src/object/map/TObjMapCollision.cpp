#include "core/types.h"
#include "object/ObjAction.h"
#include "object/ObjScript.h"
#include "stage/SetManager.h"
#include "stage/StageObjUtil.h"
#include "engine/MeshColli.h"
#include "camera/TCamera.h"
#include "core/Mem.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "stage/SetSlot.h"
#include "object/TObjKey00.h"
#include "object/map/TObjMapCollision.h"
#include "animal/RacerMover.h"
struct MeshObject;
struct SetLink;

extern "C" {
// 0x802C3DC0
ObjClassName gTObjMapCollisionName = { "TObjMapCollision", 0 };
}

class TObjMapCollision;

typedef char TObjMapCollision_size_check[(sizeof(TObjMapCollision) == 0xB4) ? 1 : -1];

// 0x80069948
// The four corners of an upright wall width wide and height high centred on x
extern "C" void mapColliWallVerts(Vec3f* verts, f32 width, f32 height, f32 z)
{
	f32 right  = 0.5f * width;
	verts[0].x = right;
	verts[0].y = height;
	verts[0].z = z;
	verts[1].x = right;
	verts[1].y = 0.0f;
	verts[1].z = z;
	f32 left   = -0.5f * width;
	verts[2].x = left;
	verts[2].y = height;
	verts[2].z = z;
	verts[3].x = left;
	verts[3].y = 0.0f;
	verts[3].z = z;
}

// 0x80069990
// Fills a collision triangle from three of verts with its normal and bounding sphere
extern "C" void mapColliSetTri(Vec3f* verts, int a, int b, int c, MeshTri* tri)
{
	Vec3f p0;
	Vec3f p1;
	Vec3f p2;
	Vec3f e0;
	Vec3f e1;
	Vec3f normal;
	Vec3f center;

	tri->mVerts[0] = a;
	tri->mVerts[1] = b;
	tri->mVerts[2] = c;
	p0             = verts[a];
	p1             = verts[b];
	p2             = verts[c];
	tri->mMask     = 1;
	tri->_06       = 0;

	e0 = p1;
	njSubVector(&e0, &p0);
	e1 = p2;
	njSubVector(&e1, &p0);
	njOuterProduct(&e0, &e1, &normal);
	if (njUnitVector(&normal) < gUnitVectorMin) {
		normal = gVecZero;
	}
	tri->mNormal = normal;

	center = p0;
	njAddVector(&center, &p1);
	njAddVector(&center, &p2);
	gjScaleVector(1.0f / 3.0f, &center, &center);

	njSubVector(&p0, &center);
	f32 radius = njScalor(&center);
	njSubVector(&p1, &center);
	f32 len = njScalor(&center);
	if (len > radius) {
		radius = len;
	}
	njSubVector(&p2, &center);
	len = njScalor(&center);
	if (len > radius) {
		radius = len;
	}
	tri->mCenter = center;
	tri->mRadius = radius;
}

// 0x80069B98
// Two triangles a b c and d c b
extern "C" void mapColliSetQuad(Vec3f* verts, MeshTri* tris, int a, int b, int c, int d)
{
	mapColliSetTri(verts, a, b, c, tris);
	mapColliSetTri(verts, d, c, b, tris + 1);
}

// 0x80069BFC
extern "C" TObject* createTObjMapCollision(TObject* parent, SetDesc* desc)
{
	return new TObjMapCollision(parent, desc);
}

// 0x80069C50
void TObjMapCollision::initFromDesc(SetDesc* desc)
{
	desc->getPos(&mPos);
	desc->getRot(&mRot);
	mtxSetTransV(mMtx, &mPos);
	mtxRotateZYX((f32*)mMtx, (const f32*)mMtx, (const int*)&mRot);
	mSwitch  = desc->getInt(0);
	mModel   = desc->getInt(1);
	mModel   = CLAMP_RANGE(mModel, 0, 1);
	mScale.x = 1.0f;
	mScale.y = 1.0f;
	mScale.z = 1.0f;
	switch (mModel) {
	case 0:
		mScale.x = 10.0f + desc->getFloat(1);
		mScale.y = 10.0f + desc->getFloat(2);
		break;
	case 1:
		mScale.x = 10.0f + desc->getFloat(1);
		mScale.y = 10.0f + desc->getFloat(2);
		mScale.z = 10.0f + desc->getFloat(3);
		break;
	}
}

// 0x80069E0C
void TObjMapCollision::debugPrint(void*)
{
	switch (mModel) {
	case 0:
	case 1:
		njPrintC(0xA0018, "+1.0:SCALE X");
		njPrintC(0xA0019, "+1.0:SCALE Y");
		njPrintC(0xA001A, "+1.0:SCALE Z");
		break;
	}
	njPrintC(0xA0013, "KEY ID");
	njPrintC(0xA0014, ":MODEL TYPE");
	meshDebugDraw(mActor, &mData);
}

// 0x80069EB0
TObjMapCollision::TObjMapCollision(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
{
	_00   = (u32)gTObjMapCollisionName.mName;
	mSize = 0xB4;
	setObjId(this, desc->mObjId);
	initFromDesc((SetDesc*)desc);
	mLink.get()->attach((SetDesc*)desc);
	buildMesh();
}

// 0x80069F40
TObjMapCollision::~TObjMapCollision()
{
	meshColliFree(mActor);
	delete[] mVerts;
	delete[] mTris;
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	releaseObjId(this);
}

// 0x80069FD8
void TObjMapCollision::buildMesh()
{
	buildVerts();
	buildTris();
	mData.mVerts = mVerts;
	mData.mTris  = mTris;
	mActor       = (MeshObject*)meshColliCreate();
	meshColliSetData(mActor, &mData);
	meshColliSetMatrix(mActor, mMtx);
	setBox();
}

// 0x8006A044
void TObjMapCollision::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (!isInRange(this, &mPos, 500.0f)) {
		mFlags |= 1;
	} else if (keyBitIsSet((u16)mSwitch) == 1) {
		mFlags |= 1;
	}
}

// 0x8006A0C0
// A wall has four corners
int TObjMapCollision::buildVerts()
{
	int count;
	switch (mModel) {
	case 0:
		count = 4;
		break;
	case 1:
		count = 8;
		break;
	}
	mData._04    = count;
	mVerts       = new Vec3f[count];
	Vec3f* verts = mVerts;
	if (verts == 0) {
		return 0;
	}
	switch (mModel) {
	case 0:
		mapColliWallVerts(verts, mScale.x, mScale.y, 0.0f);
		break;
	case 1:
		mapColliWallVerts(verts, mScale.x, mScale.y, -0.5f * mScale.z);
		mapColliWallVerts(verts + 4, mScale.x, mScale.y, 0.5f * mScale.z);
		break;
	}
	return 1;
}

// 0x8006A1A4
// Two triangles a face one face for a wall six for a box
int TObjMapCollision::buildTris()
{
	int count;
	switch (mModel) {
	case 0:
		count = 2;
		break;
	case 1:
		count = 12;
		break;
	}
	mData.mTriCount = count;
	mTris           = new MeshTri[count];
	MeshTri* tris   = mTris;
	if (tris == 0) {
		return 0;
	}
	Vec3f* verts = mVerts;
	switch (mModel) {
	case 0:
		mapColliSetQuad(verts, tris, 0, 1, 2, 3);
		break;
	case 1:
		mapColliSetQuad(verts, tris, 0, 1, 2, 3);
		mapColliSetQuad(verts, tris + 2, 4, 0, 6, 2);
		mapColliSetQuad(verts, tris + 4, 2, 3, 6, 7);
		mapColliSetQuad(verts, tris + 6, 4, 5, 0, 1);
		mapColliSetQuad(verts, tris + 8, 6, 7, 4, 5);
		mapColliSetQuad(verts, tris + 10, 1, 5, 3, 7);
		break;
	}
	return 1;
}

// 0x8006A304
// The culling box 10 bigger than the shape every way
void TObjMapCollision::setBox()
{
	f32 box[6];
	Vec3f lo;
	Vec3f hi;
	Vec3f pad;

	lo.set(-0.5f * mScale.x, 0.0f, -0.5f * mScale.z);
	hi.set(0.5f * mScale.x, mScale.y, 0.5f * mScale.z);
	pad.set(10.0f, 10.0f, 10.0f);
	njSubVector(&lo, &pad);
	njAddVector(&hi, &pad);
	boxAroundBox(box, mMtx, &lo, &hi);
	meshColliSetBox(mActor, box);
}

// 0x8006A3B4
int TObjMapCollision::getKind()
{
	return 3;
}

// 0x8006A3BC
SetLink* TObjMapCollision::getLink()
{
	return (SetLink*)&mLink;
}
