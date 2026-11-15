#include "object/TObjLauncher.h"
#include "stage/StageObjUtil.h"
#include "engine/CharaColli.h"
#include "object/ObjModel.h"
#include "stage/SetManager.h"
#include "Ninja/ninja.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/Colli.h"
#include "object/ObjAction.h"
#include "engine/Color.h"
#include "engine/DrawState.h"
#include "egg/EggCreature.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "ui/AeArc.h"
#include "core/MathUtil.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "core/VecMath.h"
#include "object/TObjEggFeed.h"
#include "core/TMainTask.h"
struct MeshObject;
struct DrawStateSlot;
struct TObjEggFeed;

// As in njMotion.c
// As in njMisc.c
typedef struct NJS_POLYGON_VTX {
	f32 x, y, z;
	u32 col;
} NJS_POLYGON_VTX;

// The library's sqrtf inlined
static inline f32 plateSqrt(f32 x)
{
	if (x > 0.0f) {
		double g = __frsqrte(x);
		g = 0.5 * g * (3.0 - g * g * x);
		g = 0.5 * g * (3.0 - g * g * x);
		g = 0.5 * g * (3.0 - g * g * x);
		volatile f32 y = (f32)(x * g);
		return y;
	}
	return x;
}

#define randUnit() (3.0517578e-5f * rand())

static inline f32 randRange(f32 lo, f32 hi)
{
	return lo + (hi - lo) * randUnit();
}

// 0x80051DB0
ContactQuery::ContactQuery()
{
	reset();
}

// 0x80051DEC
ContactQuery::~ContactQuery() { }

// 0x80051E28
void ContactQuery::reset()
{
	mOwner = _02 = mHitOwner = 0xFFFF;
	mContact   = 0;
	mPos.x     = 0.0f;
	mPos.y     = 0.0f;
	mPos.z     = 0.0f;
	mHitPos.x  = 0.0f;
	mHitPos.y  = 0.0f;
	mHitPos.z  = 0.0f;
	mGroup     = 0;
	mHitGroup  = 0;
	resetHitInfo(&mHit);
	mFlags = 0;
}

// 0x80051EA0
int ContactQuery::findGroup(CharaColli* colli, u8 group)
{
	reset();
	for (ColliContact* contact = colli->mContacts; contact != 0; contact = contact->mNext) {
		if (colliInGroup(contact->mOther, group)) {
			mContact = contact;
			mOwner   = colliContactOwner(contact);
			mPos     = contact->mPos;
			mGroup   = colliGetGroup(contact->mOther);
			return 1;
		}
	}
	return 0;
}

// 0x80051F40
int ContactQuery::findHit(TObjAction* attacker, CharaColli* colli)
{
	reset();
	for (ColliContact* contact = colli->mContacts; contact != 0; contact = contact->mNext) {
		mOwner = colliContactOwner(contact);
		if (colliContactHasBit8(contact)) {
			TObjAction* obj = findObjById(mOwner);
			if (obj != 0 && obj->onHit(attacker, colliContactTag(contact), (int*)&mHit)) {
				mFlags |= 2;
				mContact   = contact;
				mHitOwner  = mOwner;
				mHitPos    = contact->mPos;
				mHitGroup  = colliGetGroup(contact->mOther);
				return 1;
			}
		}
	}
	return 0;
}

// 0x8005202C
MotionTrack::MotionTrack()
{
	mMotion = 0;
}

// 0x80052038
MotionTrack::~MotionTrack() { }

// 0x80052074
void MotionTrack::set(void* motion, int loop, f32 speed)
{
	if (motion != 0) {
		u32 last = ((NJS_MOTION*)motion)->nbFrame;
		if (!loop) {
			last--;
		}
		setRange(motion, 0.0f, last, loop, speed);
	}
}

// 0x800520CC
void MotionTrack::setRange(void* motion, f32 start, f32 end, int loop, f32 speed)
{
	if (motion == 0) {
		return;
	}
	mMotion = (NJS_MOTION*)motion;
	mStart  = start;
	mEnd    = end;
	mLoop   = loop;
	mSpeed  = speed;
	mFrame  = mStart;
	mRate   = 1.0f;
}

// 0x800520FC
void MotionTrack::update()
{
	if (mMotion == 0) {
		return;
	}
	mFrame += mSpeed * mRate;
	if (mFrame > mEnd) {
		if (mLoop) {
			mFrame = mStart + (mFrame - mEnd);
		} else {
			mFrame = mEnd;
		}
	}
}

// 0x80052154
int MotionTrack::isDone()
{
	if (mLoop) {
		return 0;
	}
	return mFrame == mEnd;
}

// 0x80052180
ModelDraw::ModelDraw()
{
	mModel     = 0;
	mDrawState = 0;
	mTexList   = 0;
	mTrack     = 0;
	mMatrix    = 0;
	mNodeFunc  = 0;
	mNodeArg   = 0;
}

// 0x800521A4
ModelDraw::~ModelDraw() { }

// 0x800521E0
void ModelDraw::setModel(void* model)
{
	mModel = (NJS_OBJECT*)model;
}

// 0x800521E8
void ModelDraw::setNodeFunc(void* func, void* arg)
{
	mNodeFunc = func;
	mNodeArg  = arg;
}

// 0x800521F4
void ModelDraw::walkNodes()
{
	if (mModel != 0 && mTrack != 0 && mNodeFunc != 0) {
		if (mMatrix != 0) {
			njPushMatrix(mMatrix);
		} else {
			gjPushUnitMatrix();
		}
		njStartMotionObj(mTrack->getMotion(), mTrack->getFrame());
		walkNode(mModel, 0);
		njPopMatrixEx();
	}
}

// 0x8005227C
int ModelDraw::walkNode(NJS_OBJECT* node, int index)
{
	do {
		njPushMatrixEx();
		njMotionTransformEx(node);
		((ModelNodeFunc)mNodeFunc)(index, mNodeArg);
		index++;
		if (node->child != 0) {
			index = walkNode(node->child, index);
		}
		njPopMatrixEx();
		node = node->sibling;
	} while (node != 0);
	return index;
}

// 0x80052300
void ModelDraw::draw()
{
	if (mModel == 0) {
		return;
	}
	njPushMatrixEx();
	if (mMatrix != 0) {
		njMultiMatrix(0, mMatrix);
	}
	if (mTexList != 0) {
		njSetTexture(mTexList);
	}
	beginDrawState(getDrawStateSlot(), mDrawState);
	if (mTrack != 0) {
		gjDrawMotion(mModel, mTrack->mMotion, mTrack->mFrame);
	} else {
		gjDrawObject(mModel);
	}
	endDrawState(getDrawStateSlot());
	njPopMatrixEx();
}

// 0x8005239C
MeshPlate::MeshPlate(u32 mask, f32 width, f32 depth)
{
	mWidth  = width;
	mDepth  = depth;
	mRadius = 0.5f * plateSqrt(width * width + depth * depth) + 1.0f;
	mMask   = mask;

	mData.mVerts    = mVerts;
	mData._04       = 4;
	mData.mTris     = mTris;
	mData.mTriCount = 2;
	setVerts();
	setTris();

	mColli = (MeshObject*)meshColliCreate();
	if (mColli != 0) {
		meshColliSetData(mColli, &mData);
	}
}

// 0x800524B4
MeshPlate::~MeshPlate()
{
	meshColliFree(mColli);
}

// 0x8005250C
void MeshPlate::setMatrix(const f32* mtx)
{
	if (mColli != 0) {
		meshColliSetMatrix(mColli, mtx);
		updateBox();
	}
}

// 0x80052558
void MeshPlate::debugDraw()
{
	if (mColli != 0) {
		meshDebugDraw(mColli, &mData);
	}
}

// 0x80052588
void MeshPlate::setVerts()
{
	f32 halfW = 0.5f * mWidth;
	f32 halfD = 0.5f * mDepth;

	mVerts[0].x = -halfW;
	mVerts[0].y = 0.0f;
	mVerts[0].z = -halfD;
	mVerts[1].x = -halfW;
	mVerts[1].y = 0.0f;
	mVerts[1].z = halfD;
	mVerts[2].x = halfW;
	mVerts[2].y = 0.0f;
	mVerts[2].z = -halfD;
	mVerts[3].x = halfW;
	mVerts[3].y = 0.0f;
	mVerts[3].z = halfD;
}

// 0x800525DC
void MeshPlate::setTris()
{
	mTris[0].mVerts[0]  = 0;
	mTris[0].mVerts[1]  = 1;
	mTris[0].mVerts[2]  = 2;
	mTris[0].mMask      = mMask;
	mTris[0].mNormal.x  = 0.0f;
	mTris[0].mNormal.y  = 1.0f;
	mTris[0].mNormal.z  = 0.0f;
	mTris[0].mCenter.x  = 0.0f;
	mTris[0].mCenter.y  = 0.0f;
	mTris[0].mCenter.z  = 0.0f;
	mTris[0].mRadius    = mRadius;

	mTris[1].mVerts[0]  = 3;
	mTris[1].mVerts[1]  = 2;
	mTris[1].mVerts[2]  = 1;
	mTris[1].mMask      = mMask;
	mTris[1].mNormal.x  = 0.0f;
	mTris[1].mNormal.y  = 1.0f;
	mTris[1].mNormal.z  = 0.0f;
	mTris[1].mCenter.x  = 0.0f;
	mTris[1].mCenter.y  = 0.0f;
	mTris[1].mCenter.z  = 0.0f;
	mTris[1].mRadius    = mRadius;
}

// 0x80052660
void MeshPlate::updateBox()
{
	f32 box[8];
	Vec3f reach;
	Vec3f lo;
	Vec3f hi;
	Vec3f pos;

	if (mColli == 0) {
		return;
	}
	reach.x = mRadius;
	reach.y = mRadius;
	reach.z = mRadius;
	mtxGetPos(mColli->mMatrix, &pos);
	vecScaleAdd(-1.0f, &lo, &reach, &pos);
	vecAdd(&hi, &reach, &pos);
	boxAroundLine(box, &lo, &hi);
	meshColliSetBox(mColli, box);
}

// 0x800526F0
void shatterBank(f32 scale, TObjAction* obj, const Vec3f* hit, const Vec3f* size, const ShatterBank* pieces)
{
	Vec3f center = *obj->getPos();
	int rotY     = obj->getRotY();
	f32 floorY   = center.y - 100.0f;
	TObject* parent;
	if (obj != 0) {
		parent = obj;
	} else {
		parent = gTL_06;
	}

	for (int i = 0; i < pieces->mCount; i++) {
		Vec3f pos;
		Vec3f vel;

		pos.x = randRange(-size->x, size->x);
		pos.y = randRange(0.2f * size->y, 1.8f * size->y);
		pos.z = randRange(-size->z, size->z);
		vecRotateY(&pos, &pos, rotY);
		vecAdd(&pos, &pos, &center);

		vecSub(&vel, &pos, hit);
		vel.x *= randRange(0.5f, 1.5f);
		vel.z *= randRange(0.5f, 1.5f);
		vecNormalize(&vel, &vel);
		f32 lift = 0.2f * randUnit();
		vel.y = vel.y + lift;
		vecScale(randRange(0.5f * pieces->mSpeed, pieces->mSpeed), &vel, &vel);

		int pick = rand() % pieces->mModelCount;
		new TObjLauncher(parent, floorY, pieces->mScale * scale, &pos, &vel, pieces->mBank, pieces->mFirst + pick, 0);
	}
}

// 0x80052A8C
void shatterListAt(TObjAction* obj, const Vec3f* hit, const Vec3f* size, const ShatterList* pieces)
{
	obj->getPos();
	obj->getRotY();
	shatterList(obj, obj->getPos(), obj->getRotY(), hit, size, pieces);
}

// 0x80052B2C
void shatterList(TObjAction* obj, const Vec3f* center, int rotY, const Vec3f* hit, const Vec3f* size,
                 const ShatterList* pieces)
{
	f32 floorY = center->y - 100.0f;
	TObject* parent;
	if (obj != 0) {
		parent = obj;
	} else {
		parent = gTL_06;
	}

	for (int i = 0; i < pieces->mCount; i++) {
		Vec3f pos;
		Vec3f vel;

		pos.x = randRange(-size->x, size->x);
		pos.y = randRange(0.2f * size->y, 1.8f * size->y);
		pos.z = randRange(-size->z, size->z);
		vecRotateY(&pos, &pos, rotY);
		vecAdd(&pos, &pos, center);

		vecSub(&vel, &pos, hit);
		vel.x *= randRange(0.5f, 1.5f);
		vel.z *= randRange(0.5f, 1.5f);
		vecNormalize(&vel, &vel);
		f32 lift = 0.2f * randUnit();
		vel.y = vel.y + lift;
		vecScale(randRange(0.5f * pieces->mSpeed, pieces->mSpeed), &vel, &vel);

		int pick = rand() % pieces->mModelCount;
		new TObjLauncher(parent, floorY, pieces->mScale, &pos, &vel, pieces->mModels[pieces->mFirst + pick],
		                 pieces->mTexture);
	}
}

// 0x80052E78
void shatterModel(TObjAction* obj, const Vec3f* hit, int rotY, NJS_OBJECT* model, void* texList, f32 scale, f32 speed)
{
	f32 floorY = hit->y - 100.0f;
	Vec3f center;

	if (obj != 0) {
		center = *obj->getPos();
	} else {
		center = *hit;
		obj    = (TObjAction*)gTL_06;
	}

	// The model argument walks its own children
	model = model->child;
	while (model != 0) {
		Vec3f pos;
		Vec3f vel;
		Vec3f size;
		SetRot rot;

		rot.set(model->ang[0], model->ang[1], model->ang[2]);
		vel.set(model->pos[0], model->pos[1], model->pos[2]);
		vecScale(scale, &vel, &vel);
		vecRotateY(&vel, &vel, rotY);
		vecAdd(&pos, &center, &vel);

		vecSub(&vel, &pos, hit);
		vel.x *= randRange(0.5f, 1.5f);
		vel.z *= randRange(0.5f, 1.5f);
		vecNormalize(&vel, &vel);
		f32 lift = 0.2f * randUnit();
		vel.y = vel.y + lift;
		vecScale(randRange(0.5f * speed, speed), &vel, &vel);

		size.x = model->scl[0];
		size.y = model->scl[1];
		size.z = model->scl[2];
		vecScale(scale, &size, &size);

		new TObjLauncher(obj, floorY, &pos, &rot, &vel, size, model->model, texList);

		if (model->sibling != 0) {
			model = model->sibling;
		} else if (model->child != 0) {
			model = model->child;
		} else {
			model = 0;
		}
	}
}

// 0x8005319C
void shatterModelRot(TObjAction* obj, const Vec3f* hit, const Vec3f* center, const SetRot* rot, NJS_OBJECT* model, void* texList, f32 scale, f32 speed)
{
	f32 floorY = hit->y - 100.0f;
	TObject* parent;
	f32 mtx[12];

	if (obj != 0) {
		parent = obj;
	} else {
		parent = gTL_06;
	}
	mtxIdentity(mtx);
	mtxRotateZYX((f32*)mtx, (const f32*)mtx, (const int*)rot);

	for (NJS_OBJECT* node = model->child; node != 0; node = node->sibling) {
		Vec3f pos;
		Vec3f vel;
		Vec3f size;
		SetRot angle = *(SetRot*)node->ang;

		angle.x += rot->x;
		angle.y += rot->y;
		angle.z += rot->z;

		pos.set(node->pos[0], node->pos[1], node->pos[2]);
		vecScale(scale, &pos, &pos);
		mtxMultVec(mtx, &pos, &pos);
		vecAdd(&pos, &pos, center);

		vecSub(&vel, &pos, hit);
		vel.x *= randRange(0.5f, 1.5f);
		vel.z *= randRange(0.5f, 1.5f);
		vecNormalize(&vel, &vel);
		f32 lift = 0.2f * randUnit();
		vel.y = vel.y + lift;
		vecScale(randRange(0.5f * speed, speed), &vel, &vel);

		size.set(node->scl[0], node->scl[1], node->scl[2]);
		vecScale(scale, &size, &size);

		new TObjLauncher(parent, floorY, &pos, &angle, &vel, size, node->model, texList);
	}
}

// 0x800534A0
void dropItem(u8 item, Vec3f* pos)
{
	if (item == 0) {
		return;
	}
	if (item > 0 && item <= 100) {
		pos->y += 20.0f;
	} else if (item > 100 && item <= 107) {
		createTObjEggFeedAt(item - 101, pos, 0);
	} else if (item > 110 && item <= 117) {
		createTObjEggFeedAt(item - 111, pos, 1);
	}
}

// 0x80053528
int fadeAmbient(DrawState* state, int target, int step)
{
	u8 alpha = state->mAmbient;
	if ((u8)(alpha - target) > (u8)step) {
		setColorAlpha(&state->mAmbient, alpha - step);
	} else {
		setColorAlpha(&state->mAmbient, target);
		return 1;
	}
	return 0;
}

// 0x80053584
int colliFindContact(CharaColli* colli, int group, ContactFilter filter, void* arg)
{
	int found = 0;
	for (ColliContact* contact = colli->mContacts; contact != 0; contact = contact->mNext) {
		if (colliInGroup(contact->mOther, group)) {
			found = 1;
			if (filter == 0) {
				return 1;
			}
			if (filter(contact, arg) == 0) {
				return 1;
			}
		}
	}
	return found;
}

// 0x8005361C
void colliSetSphere(CharaColli* colli, const Vec3f* pos, u32 flags, const Vec3f* offset, f32 radius)
{
	if (colli != 0) {
		ColliShape shape;
		colliShapeInit(&shape);
		shape.mPos    = *offset;
		shape.mType   = COLLISHAPE_SPHERE;
		shape.mSizeX  = radius;
		shape.mFlags |= flags;
		colliCopyShapes(colli, &shape);
		colliPlaceShapes(colli, pos, &shape);
	}
}

// 0x800536BC
void colliSetCylinder(CharaColli* colli, const Vec3f* pos, u32 flags, const Vec3f* offset, f32 radius, f32 height)
{
	if (colli != 0) {
		ColliShape shape;
		colliShapeInit(&shape);
		shape.mPos    = *offset;
		shape.mType   = COLLISHAPE_CYLINDER;
		shape.mSizeX  = radius;
		shape.mSizeY  = height;
		shape.mFlags |= flags;
		colliCopyShapes(colli, &shape);
		colliPlaceShapes(colli, pos, &shape);
	}
}

// 0x80053774
void modelBoundsWalk(NJS_OBJECT* node, Vec3f* min, Vec3f* max, const Vec3f* base)
{
	for (; node != 0; node = node->sibling) {
		Vec3f pos;
		Vec3f at;
		Vec3f local;
		Vec3f center;

		local.set(node->pos[0], node->pos[1], node->pos[2]);
		vecAdd(&pos, &local, base);

		GjModel* model = node->model;
		if (model != 0) {
			f32 r = model->r;
			center.set(model->center.x, model->center.y, model->center.z);
			vecAdd(&at, &pos, &center);
			if (at.x - r < min->x) {
				min->x = at.x - r;
			}
			if (at.y - r < min->y) {
				min->y = at.y - r;
			}
			if (at.z - r < min->z) {
				min->z = at.z - r;
			}
			if (at.x + r > max->x) {
				max->x = at.x + r;
			}
			if (at.y + r > max->y) {
				max->y = at.y + r;
			}
			if (at.z + r > max->z) {
				max->z = at.z + r;
			}
		}
		modelBoundsWalk(node->child, min, max, &pos);
	}
}

// 0x800538CC
void modelBounds(f32 margin, NJS_OBJECT* model, Vec3f* min, Vec3f* max)
{
	Vec3f grow;

	modelBoundsWalk(model, min, max, &gVecZero);
	grow.x = margin;
	grow.y = margin;
	grow.z = margin;
	vecAdd(max, max, &grow);
	vecNegate(&grow, &grow);
	vecAdd(min, min, &grow);
}

// 0x80053948
void modelSphere(f32 margin, NJS_OBJECT* model, Vec3f* min, Vec3f* max, Vec3f* center, f32* radius)
{
	modelBounds(margin, model, min, max);
	vecSub(center, max, min);
	vecScale(0.5f, center, center);
	*radius = vecLength(center);
	vecAdd(center, center, min);
}

// 0x800539BC
void boxAroundBox(f32* box, const f32* mtx, const Vec3f* lo, const Vec3f* hi)
{
	Vec3f corners[8];
	Vec3f min;
	Vec3f max;

	corners[0].x = corners[1].x = corners[4].x = corners[5].x = lo->x;
	corners[2].x = corners[3].x = corners[6].x = corners[7].x = hi->x;
	corners[0].y = corners[1].y = corners[2].y = corners[3].y = hi->y;
	corners[4].y = corners[5].y = corners[6].y = corners[7].y = lo->y;
	corners[0].z = corners[2].z = corners[4].z = corners[6].z = lo->z;
	corners[1].z = corners[3].z = corners[5].z = corners[7].z = hi->z;

	mtxMultVec(mtx, &corners[0], &corners[0]);
	min = corners[0];
	max = corners[0];
	for (s8 i = 1; i < 8; i++) {
		mtxMultVec(mtx, &corners[i], &corners[i]);
		if (min.x > corners[i].x) {
			min.x = corners[i].x;
		}
		if (min.y > corners[i].y) {
			min.y = corners[i].y;
		}
		if (min.z > corners[i].z) {
			min.z = corners[i].z;
		}
		if (max.x < corners[i].x) {
			max.x = corners[i].x;
		}
		if (max.y < corners[i].y) {
			max.y = corners[i].y;
		}
		if (max.z < corners[i].z) {
			max.z = corners[i].z;
		}
	}
	boxAroundLine(box, &min, &max);
}

// 0x80053B54
// The arrow drawn for a triangle its normal twice as long
static inline Vec3f triArrow(const MeshTri* tri)
{
	Vec3f arrow;
	Vec3f normal;
	normal.set(tri->mNormal.x, tri->mNormal.y, tri->mNormal.z);
	gjScaleVector(2.0f, &arrow, &normal);
	return arrow;
}

void meshDebugDraw(MeshColli* mesh, const MeshData* data)
{
	NJS_POLYGON_VTX poly[3];
	Vec3f from;
	Vec3f to;
	u32 colors[3];
	Vec3f* verts;
	MeshTri* tri;
	int count;
	int i;

	colors[0] = 0xFFA00000;
	colors[1] = 0xFF00A000;
	colors[2] = 0xFF0000A0;
	pushCullMode(0x18000000);
	njPushMatrixEx();
	njMultiMatrix(0, mesh->mMatrix);
	verts = data->mVerts;
	tri   = data->mTris;
	count = data->mTriCount;

	njDrawPolygon3DExStart(0);
	for (i = 0; i < count; i++) {
		for (int j = 0; j < 3; j++) {
			Vec3f* v    = &verts[tri->mVerts[j]];
			poly[j].x   = v->x;
			poly[j].y   = v->y;
			poly[j].z   = v->z;
			poly[j].col = colors[j];
		}
		njDrawPolygon3DExSetData(poly, 3);
		tri++;
	}
	njDrawPolygon3DExEnd();

	tri = data->mTris;
	for (i = 0; i < count; i++) {
		from = verts[tri->mVerts[0]];
		to   = verts[tri->mVerts[1]];
		debugLine(&from, &to, 0);
		from = verts[tri->mVerts[1]];
		to   = verts[tri->mVerts[2]];
		debugLine(&from, &to, 0);
		from = verts[tri->mVerts[2]];
		to   = verts[tri->mVerts[0]];
		debugLine(&from, &to, 0);

		debugArrow(&tri->mCenter, triArrow(tri), 0);
		tri++;
	}
	njPopMatrixEx();
	popCullMode();
}
