#include "stage/SetActor.h"
#include "object/ObjScript.h"
#include "object/ObjCommon.h"
#include "engine/DrawTask.h"
#include "sound/SoundHandle.h"
#include "engine/SpringBody.h"
#include "engine/MeshColli.h"
#include "engine/CharaColli.h"
#include "object/ObjArchive.h"
#include "object/TObjBridgeIta.h"
#include "egg/ObjEgg.h"
#include "engine/Colli.h"
#include "stage/StageObjUtil.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "core/Mem.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjAction.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"
#include "stl/math.h"
#include "egg/EggCreature.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "core/VecMath.h"
#include "object/TObjHangBridge.h"
struct MeshData;
struct NJS_OBJECT;
struct DrawState;
struct DrawStateSlot;
struct MeshObject;

extern "C" {
// 0x802C40D8
ObjClassName gTObjHangBridgeName = { "TObjHangBridge", 0 };
}

typedef char TObjHangBridge_size_check[(sizeof(TObjHangBridge) == 0x14C) ? 1 : -1];

// 0x800D51D0
extern "C" TObject* createTObjHangBridge(TObject* parent, SetDesc* desc)
{
	return new TObjHangBridge(parent, desc);
}

// 0x800D5224
void TObjHangBridge::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mCount     = desc->getInt(0) * 2 + 3;
	mVertCount = (mCount + 1) * 4;
	mTriCount  = mCount * 6;
	mAnchor0   = desc->getInt(1);
	mAnchor0   = mAnchor0 < 0 ? 0 : ((u32)mAnchor0 > mCount + 1 ? mCount + 1 : mAnchor0);
	mAnchor1   = desc->getInt(2);
	mAnchor1   = mAnchor1 < 0 ? 0 : ((u32)mAnchor1 > mCount + 1 ? mCount + 1 : mAnchor1);
}

// 0x800D5324
void TObjHangBridge::debugPrint(void*)
{
	njPrintC(0xA0013, "*2+3:DIV NUM");
	njPrintC(0xA0014, ":ADD ANCHOR IDX1");
	njPrintC(0xA0015, ":ADD ANCHOR IDX2");
}

// 0x800D5380
TObjHangBridge::TObjHangBridge(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjHangBridgeName.mName;
	mSize = 0x14C;
	initFromDesc(desc);
	mDraw  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mShape = 0;
	mShape = colliCreate(1, 1);
	if (mShape != 0) {
		colliSetOwner(mShape, (u16)mObjId);
		colliSetMasks(mShape, 4, 255, 255, 0xD6);
	}
	setup();
}

// 0x800D546C
TObjHangBridge::~TObjHangBridge()
{
	seStop3D(&mSnd);
	colliRetire(mShape);
	mShape = 0;
	meshColliFree(mPost1Mesh);
	meshColliFree(mPost0Mesh);
	meshColliFree(mMesh);
	delete mRope;
	delete[] mForces;
	delete[] mVerts;
	delete[] mTris;
	removeDrawTask(mDraw);
}

// 0x800D5548
void TObjHangBridge::setup()
{
	f32 mtx[12];
	f32 box[6];
	Vec3f lo;
	Vec3f hi;
	Vec3f pad;
	Vec3f from;
	Vec3f to;
	Vec3f at;
	ColliShape shape;
	MeshData* data;
	TObjBridgeIta* plank;
	u32 i;
	int half;
	int j;
	int k;

	mPostModel = (NJS_OBJECT*)findGeObjData("MODEL_TURIBASHI_HASHIRA02");
	mPostTex   = findGeObjData("TLS_MODEL_TURIBASHI_HASHIRA02");
	mForces    = 0;
	mForces    = new HitVec[mCount + 1];
	if (mForces != 0) {
		for (i = 0; i <= mCount; i++) {
			mForces[i].x = 0.0f;
			mForces[i].y = 0.0f;
			mForces[i].z = 0.0f;
		}
	}
	buildVerts();
	buildIndices();
	buildMesh();
	delete[] mIndices;

	mtxSetTransV(mMtx, getPos());
	mtxRotateY(mMtx, mMtx, mRot.y);
	mRopeFrom.x = 0.0f;
	mRopeFrom.y = 0.0f;
	mRopeFrom.z = 0.0f;
	mRopeFrom.z = mVerts[mCount + 1].z;
	mRopeFrom.y = mVerts[mCount + 1].y;
	mRopeTo.x   = 0.0f;
	mRopeTo.y   = 0.0f;
	mRopeTo.z   = 0.0f;
	mRopeTo.z   = mVerts[mCount * 2 + 1].z;
	mRopeTo.y   = mVerts[mCount * 2 + 1].y;
	mtxMultVec(mMtx, &mRopeFrom, &mRopeFrom);
	mtxMultVec(mMtx, &mRopeTo, &mRopeTo);
	mRope = new SpringRope(&mRopeFrom, &mRopeTo, mCount, 0.5f, 0.95f, 1.0f);
	if (mRope != 0) {
		mRope->mWind.x = 0.0f;
		mRope->mWind.y = -0.0032666668f;
		mRope->mWind.z = 0.0f;
		mRope->mForces    = mForces;
	}

	// The two ends are the posts
	half = (int)mCount / 2 + 1;
	for (j = -half; j <= half; j++) {
		at.x = 0.0f;
		at.y = 0.0f;
		at.z = 0.0f;
		at.z = (f32)j * gBridgePlankLength;
		if (j == -half) {
			at.z += 2.2f;
		} else if (j == half) {
			at.z -= 2.2f;
		}
		mtxMultVec(mMtx, &at, &at);
		if (j == -half) {
			mPost0 = at;
		} else if (j == half) {
			mPost1 = at;
		} else {
			plank           = new TObjBridgeIta(this);
			k               = j + half;
			plank->mPos     = at;
			plank->mPrevPos = plank->mPos;
			plank->mRotY    = mRot.y;
			plank->setPlankKind(k % 3);
			plank->setRope(k % 2);
		}
	}

	colliShapeInit(&shape);
	shape.mFlags |= 0x10;
	shape.mType  = 1;
	shape.mSizeX = 0.5f * gBridgeWidth;
	shape.mSizeY = 18.0f;
	shape.mSizeZ = 0.5f * (f32)mCount * gBridgePlankLength;
	colliCopyShapes(mShape, &shape);
	colliPlaceShapesRotY(mShape, &mPos, mRot.y, &shape);

	mMesh = (MeshObject*)meshColliCreate();
	meshColliSetData(mMesh, &mMeshData);
	meshColliSetMatrix(mMesh, mMtx);
	fitBox();

	lo.x  = -48.395267f;
	lo.y  = -48.395267f;
	lo.z  = -48.395267f;
	hi.x  = 48.395267f;
	hi.y  = 48.395267f;
	hi.z  = 48.395267f;
	pad.x = 1.499998f;
	pad.y = 8.660439f;
	pad.z = 7.453383f;
	vecSub(&lo, &lo, &pad);
	vecAdd(&hi, &hi, &pad);

	data       = (MeshData*)findStgObjData("COL_MODEL_BRI_END0");
	mPost0Mesh = (MeshObject*)meshColliCreate();
	meshSetState100(mPost0Mesh, 1);
	meshColliSetData(mPost0Mesh, data);
	mtxSetTransV(mtx, &mPost0);
	mtxRotateY(mtx, mtx, mRot.y);
	meshColliSetMatrix(mPost0Mesh, mtx);
	vecAdd(&from, &lo, &mPost0);
	vecAdd(&to, &hi, &mPost0);
	boxAroundLine(box, &from, &to);
	meshColliSetBox(mPost0Mesh, box);

	data       = (MeshData*)findStgObjData("COL_MODEL_BRI_END1");
	mPost1Mesh = (MeshObject*)meshColliCreate();
	meshSetState100(mPost1Mesh, 1);
	meshColliSetData(mPost1Mesh, data);
	mtxSetTransV(mtx, &mPost1);
	mtxRotateY(mtx, mtx, mRot.y);
	meshColliSetMatrix(mPost1Mesh, mtx);
	vecAdd(&from, &lo, &mPost1);
	vecAdd(&to, &hi, &mPost1);
	boxAroundLine(box, &from, &to);
	meshColliSetBox(mPost1Mesh, box);

	mRope->step();
	updateDeck();
}

// 0x800D5B38
// Whoever stands on the deck pulls the rope down near them and the first one sets off the creak
void TObjHangBridge::draw()
{
	ObjMsg msg;
	ObjMsg msg2;
	Vec3f d;
	ColliContact* contact;
	TObjAction* other;
	SpringPoint* points;
	u32 i;
	int n;
	int j;
	int creak;
	u32 reply;
	f32 dist;

	if (!isInRange((void*)this, (const Vec3f*)&mPos, 900.0f)) {
		mFlags |= 1;
		return;
	}
	if (objDebugEnabled(this)) {
		return;
	}
	for (i = 0; i < mCount + 1; i++) {
		mForces[i].x = 0.0f;
		mForces[i].y = 0.0f;
		mForces[i].z = 0.0f;
	}
	creak = 0;
	for (contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		if (!colliContactNoBit10(contact)) {
			continue;
		}
		other = findObjById(colliContactOwner(contact));
		if (other == 0) {
			continue;
		}
		if (creak == 0) {
			if (colliContactInGroup1(contact)) {
				msg.mType = 0x3C;
				reply     = other->sendToHandle(&msg);
				if ((reply & 0x8000) && (reply & 1)) {
					sePlay3DHeld(400.0f, &mSnd, 0x22E, 0, (const Vec3f*)other->getPos());
					creak     = 1;
					mSnd.mPos = *other->getPos();
				}
			} else if (colliContactInGroup8(contact)) {
				msg2.mType = 0x41;
				reply      = other->sendToHandle(&msg2);
				if ((reply & 0x390) && !(reply & 8)) {
					sePlay3DHeld(400.0f, &mSnd, 0x22E, 0, (const Vec3f*)other->getPos());
					creak     = 1;
					mSnd.mPos = *other->getPos();
				}
			}
		}
		points = mRope->mPoints;
		n      = mCount + 1;
		for (j = 0; j < n; j++) {
			vecSub(&d, &contact->mPos, &points[j].mPos);
			dist = 1.0f + vecLength(&d);
			if (dist < 50.0f) {
				mForces[j].y -= 0.01f * (1.0f - dist / 50.0f);
			}
		}
	}
	if (creak == 0) {
		seStop3D(&mSnd);
	}
	mRope->step();
	updateDeck();
}

// 0x800D5E34
void TObjHangBridge::vfn_10() {}

// 0x800D5E38
// The two posts the far one mirrored
void TObjHangBridge::render()
{
	int cull;
	DrawStateSlot* slot;

	getCullMode(&cull);
	setCullMode(0);
	slot = getDrawStateSlot();
	njSetTexture(mPostTex);
	beginDrawState(slot, getDefaultDrawState());
	njPushMatrixEx();
	njTranslateEx(&mPost0);
	gjRotateY(mRot.y);
	gjDrawObject(mPostModel);
	njPopMatrixEx();
	njPushMatrixEx();
	njTranslateEx(&mPost1);
	gjRotateY(mRot.y);
	njScale(0, 1.0f, 1.0f, -1.0f);
	gjDrawObject(mPostModel);
	njPopMatrixEx();
	endDrawState(slot);
	setCullMode(cull);
}

// 0x800D5EF8
// Moves the deck's points and the planks to the rope
void TObjHangBridge::updateDeck()
{
	f32 mtx[12];
	Vec3f back;
	Vec3f origin;
	Vec3f local;
	Vec3f at;
	Vec3f a;
	Vec3f b;
	Vec3f dir;
	Vec3f slope;
	SpringPoint* points;
	TObjBridgeIta* plank;
	u32 i;
	u32 j;
	u32 v;
	int rot;
	f32 y;

	points = mRope->mPoints;
	for (i = 0; i < mCount + 1; i++) {
		gjScaleVector(-1.0f, &back, getPos());
		origin = back;
		rot    = 0x10000 - getRotY();
		mtxSetTransV(mtx, &origin);
		mtxRotateY(mtx, mtx, rot);
		mtxMultVecSR(mtx, &points[i].mPos, &local);
		y = local.y - getPos()->y;
		for (j = i; j < mVertCount; j += mCount + 1) {
			mVerts[j].y = y;
			if (j < mVertCount / 4 || j >= mVertCount * 3 / 4) {
				mVerts[j].y += 18.0f;
			}
		}
	}
	plank = (TObjBridgeIta*)mChild;
	for (i = 0; i < mCount; i++) {
		v    = mCount + 1 + i;
		at   = plank->mPos;
		at.y += 0.5f * (mVerts[v].y + mVerts[v + 1].y);
		plank->setPos(&at);
		a = mVerts[v];
		b = mVerts[v + 1];
		gjSubVector(&dir, &b, &a);
		slope        = dir;
		plank->mTilt = pitchOf(&slope);
		plank        = (TObjBridgeIta*)plank->mNext;
	}
	refreshTris();
	fitBox();
}

static inline Vec3f negated(const Vec3f* v)
{
	Vec3f out;

	out.x = -v->x;
	out.y = -v->y;
	out.z = -v->z;
	return out;
}

// 0x800D6174
// The pitch of dir as an angle
int TObjHangBridge::pitchOf(Vec3f* dir)
{
	Vec3f up;

	up     = negated(dir);
	dir->y = 0.0f;
	dir->z = njScalor(dir);
	return (int)(10430.38043493439 * (f32)atan2(up.y, dir->z));
}

// 0x800D6210
// The normals and bounds of the deck's triangles after the points moved
void TObjHangBridge::refreshTris()
{
	Vec3f* verts = mMeshData.mVerts;
	MeshTri* tris = mMeshData.mTris;
	MeshTri* tri;
	Vec3f a;
	Vec3f b;
	Vec3f c;
	Vec3f ab;
	Vec3f ac;
	Vec3f n;
	Vec3f mid;
	f32 r;
	f32 d;
	u32 i;

	for (i = 0; i < mTriCount; i++) {
		tri = &tris[i];
		a   = verts[tri->mVerts[0]];
		b   = verts[tri->mVerts[1]];
		c   = verts[tri->mVerts[2]];
		ab  = b;
		njSubVector(&ab, &a);
		ac = c;
		njSubVector(&ac, &a);
		njOuterProduct(&ab, &ac, &n);
		if (njUnitVector(&n) < 0.000001f) {
			n.y = gVecZero.y;
			n.x = gVecZero.x;
			n.z = gVecZero.z;
		}
		tri->mNormal = n;
		mid          = a;
		njAddVector(&mid, &b);
		njAddVector(&mid, &c);
		gjScaleVector(0.33333334f, &mid, &mid);
		njSubVector(&a, &mid);
		r = njScalor(&mid);
		njSubVector(&b, &mid);
		d = njScalor(&mid);
		if (d > r) {
			r = d;
		}
		njSubVector(&c, &mid);
		d = njScalor(&mid);
		if (d > r) {
			r = d;
		}
		tri->mCenter = mid;
		tri->mRadius = r;
	}
}

// 0x800D6440
// Four rows of mCount+1 points 40 apart
int TObjHangBridge::buildVerts()
{
	u32 i;
	f32 half;
	f32 z;

	mVerts = new Vec3f[mVertCount];
	if (mVerts == 0) {
		return 0;
	}
	half = 40.0f * (f32)(mVertCount / 8) - 20.0f;
	z    = -half;
	for (i = 0; i < mVertCount; i++) {
		mVerts[i].x = 0.0f;
		mVerts[i].y = 0.0f;
		mVerts[i].z = 0.0f;
		if (i < mVertCount / 2) {
			mVerts[i].x = -28.0f;
		} else {
			mVerts[i].x = 31.0f;
		}
		mVerts[i].z = z;
		z += 40.0f;
		if (z > half) {
			z = -half;
		}
		if (i < mVertCount / 4 || i >= mVertCount * 3 / 4) {
			mVerts[i].y += 18.0f;
		}
	}
	return 1;
}

// 0x800D6588
// Two triangles a quad each one the one six back moved along a point skipping the row ends
int TObjHangBridge::buildIndices()
{
	u32 count = mTriCount * 3;
	u32 row   = mCount * 6;
	u32 i;

	mIndices = new u16[count];
	if (mIndices == 0) {
		return 0;
	}
	mIndices[0] = 0;
	mIndices[1] = 1;
	mIndices[2] = mVertCount / 4;
	mIndices[3] = mVertCount / 4;
	mIndices[4] = 1;
	mIndices[5] = mVertCount / 4 + 1;
	for (i = 6; i < count; i++) {
		mIndices[i] = mIndices[i - 6] + 1;
		if (i >= row && i < row + 6) {
			mIndices[i]++;
		} else if (i >= row * 2 && i < row * 2 + 6) {
			mIndices[i]++;
		}
	}
	return 1;
}

// 0x800D66BC
int TObjHangBridge::buildMesh()
{
	mTris = new MeshTri[mTriCount];
	if (mTris == 0) {
		return 0;
	}
	buildTris(mVerts, mTris);
	mMeshData.mVerts    = mVerts;
	mMeshData._04       = mVertCount;
	mMeshData.mTris     = mTris;
	mMeshData.mTriCount = mTriCount;
	return 1;
}

// 0x800D6738
void TObjHangBridge::buildTris(Vec3f* verts, MeshTri* tris)
{
	u16* idx = mIndices;
	MeshTri* tri;
	Vec3f a;
	Vec3f b;
	Vec3f c;
	Vec3f ab;
	Vec3f ac;
	Vec3f n;
	Vec3f mid;
	f32 r;
	f32 d;
	s16 i;

	for (i = 0; i < mTriCount; i++) {
		tri            = &tris[i];
		tri->mVerts[0] = idx[0];
		a              = verts[idx[0]];
		tri->mVerts[1] = idx[1];
		b              = verts[idx[1]];
		tri->mVerts[2] = idx[2];
		c              = verts[idx[2]];
		idx += 3;
		tri->mMask = 1;
		tri->_06   = 0;
		ab         = b;
		njSubVector(&ab, &a);
		ac = c;
		njSubVector(&ac, &a);
		njOuterProduct(&ab, &ac, &n);
		if (njUnitVector(&n) < 0.000001f) {
			n.y = gVecZero.y;
			n.x = gVecZero.x;
			n.z = gVecZero.z;
		}
		tri->mNormal = n;
		mid          = a;
		njAddVector(&mid, &b);
		njAddVector(&mid, &c);
		gjScaleVector(0.33333334f, &mid, &mid);
		njSubVector(&a, &mid);
		r = njScalor(&mid);
		njSubVector(&b, &mid);
		d = njScalor(&mid);
		if (d > r) {
			r = d;
		}
		njSubVector(&c, &mid);
		d = njScalor(&mid);
		if (d > r) {
			r = d;
		}
		tri->mCenter = mid;
		tri->mRadius = r;
	}
}

// 0x800D699C
// The deck's box from the sag of the rope's middle
void TObjHangBridge::fitBox()
{
	f32 box[6];
	Vec3f lo;
	Vec3f hi;
	Vec3f pad;
	u32 count = mCount;
	SpringPoint* points = mRope->mPoints;

	lo.set(-0.5f * gBridgeWidth, points[count / 2].mPos.y - points[0].mPos.y, -0.5f * gBridgePlankLength * (f32)count);
	hi.set(0.5f * gBridgeWidth, 0.0f, 0.5f * gBridgePlankLength * (f32)count);
	pad.set(18.0f, 18.0f, 18.0f);
	njSubVector(&lo, &pad);
	njAddVector(&hi, &pad);
	boxAroundBox(box, mMtx, &lo, &hi);
	meshColliSetBox(mMesh, box);
}
