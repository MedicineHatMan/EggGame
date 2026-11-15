#include "stage/Stage.h"
#include "engine/CharaColli.h"
#include "engine/TCharaColliCtrl.h"
#include "stl/math.h"
#include "object/ObjAction.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "core/Mem.h"
#include "core/TMainTask.h"

extern "C" {

void colliAddContact(CharaColli* colli, CharaColli* other, int otherShape, int shape, Vec3f* push,
                     Vec3f* pos);
}

static inline f32 colliSqrt(f32 x) { return __fres((f32)__frsqrte(x)); }

static inline f32 colliAbs(f32 x) { return (f32)__fabs(x); }

static inline f32 colliSqrtFull(f32 x)
{
	static const f64 _half  = 0.5;
	static const f64 _three = 3.0;
	volatile f32 y;
	if (x > 0.0f) {
		f64 guess = __frsqrte((f64)x);
		guess     = _half * guess * (_three - guess * guess * x);
		guess     = _half * guess * (_three - guess * guess * x);
		guess     = _half * guess * (_three - guess * guess * x);
		y         = (f32)(x * guess);
		return y;
	}
	return x;
}

static CharaColli* sColliLists[4]; // 0x80279780
static Vec3f sColliPush;          // 0x80279790 how far the last shape test pushes the first shape out
static Vec3f sColliPushA;         // 0x8027979C the first collider's push summed over its shapes
static Vec3f sColliPushB;         // 0x802797A8 the second collider's

extern "C" {
CharaColli* gColliPool;      // 0x802C4B48
CharaColli* gColliFree;      // 0x802C4B4C
CharaColli* gColliRetired;   // 0x802C4B50
int gColliContactCount;      // 0x802C4B54
ColliContact* gColliContacts; // 0x802C4B58
u32 gColliDebug;             // 0x802C4B5C

// 0x80007F80
void colliShapeInit(ColliShape* shape)
{
	shape->mPos.x = 0.0f;
	shape->mPos.y = 0.0f;
	shape->mPos.z = 0.0f;
	shape->_0C    = 0;
	shape->mRotY  = 0;
	shape->_14    = 0;
	shape->mSizeX = 0.0f;
	shape->mSizeY = 0.0f;
	shape->mSizeZ = 0.0f;
	shape->mFlags = 0;
	shape->mType  = 0;
	shape->mTag   = 0;
}

// 0x80007FBC
// With onGround the sphere sits on pos instead of around it
void colliShapeMakeSphere(ColliShape* shape, const Vec3f* pos, int onGround, f32 radius)
{
	colliShapeInit(shape);
	colliShapeSetSphere(shape);
	colliShapeSetPos(shape, pos);
	colliShapeSetRadius(shape, radius);
	if (onGround) {
		shape->mPos.y += radius;
	}
}

// 0x80008038
void colliShapeMakeCylinder(ColliShape* shape, const Vec3f* pos, int onGround, f32 radius, f32 height)
{
	colliShapeInit(shape);
	colliShapeSetCylinder(shape);
	colliShapeSetPos(shape, pos);
	colliShapeSetCylRadius(shape, radius);
	colliShapeSetCylHeight(shape, height);
	if (onGround) {
		shape->mPos.y += height;
	}
}

// 0x800080D4
void colliShapeMakeBox(ColliShape* shape, const Vec3f* pos, int ang, f32 x, f32 y, f32 z, int onGround)
{
	colliShapeInit(shape);
	colliShapeSetBox(shape);
	colliShapeSetPos(shape, pos);
	colliShapeSetBoxRot(shape, ang);
	colliShapeSetBoxX(shape, x);
	colliShapeSetBoxY(shape, y);
	colliShapeSetBoxZ(shape, z);
	if (onGround) {
		shape->mPos.y += y;
	}
}

// 0x800081A0
void colliShapeSetSphere(ColliShape* shape) { shape->mType = COLLISHAPE_SPHERE; }

// 0x800081AC
void colliShapeSetCylinder(ColliShape* shape) { shape->mType = COLLISHAPE_CYLINDER; }

// 0x800081B8
void colliShapeSetBox(ColliShape* shape) { shape->mType = COLLISHAPE_BOX; }

// 0x800081C4
void colliShapeSetPos(ColliShape* shape, const Vec3f* pos)
{
	shape->mPos.x = pos->x;
	shape->mPos.y = pos->y;
	shape->mPos.z = pos->z;
}

// 0x800081E0
void colliShapeSetRadius(ColliShape* shape, f32 radius) { shape->mSizeX = radius; }

// 0x800081E8
void colliShapeSetCylRadius(ColliShape* shape, f32 radius) { shape->mSizeX = radius; }

// 0x800081F0
void colliShapeSetCylHeight(ColliShape* shape, f32 height) { shape->mSizeY = height; }

// 0x800081F8
void colliShapeSetBoxX(ColliShape* shape, f32 x) { shape->mSizeX = x; }

// 0x80008200
void colliShapeSetBoxY(ColliShape* shape, f32 y) { shape->mSizeY = y; }

// 0x80008208
void colliShapeSetBoxZ(ColliShape* shape, f32 z) { shape->mSizeZ = z; }

// 0x80008210
void colliShapeSetBoxRot(ColliShape* shape, int ang) { shape->mRotY = ang; }

// 0x80008218
void colliShapeDebugDraw(ColliShape* shape, ColliColor* color)
{
	switch (shape->mType) {
	case COLLISHAPE_SPHERE:
		debugSphere((const Vec3f*)&shape->mPos, (GXColor*)color, shape->mSizeX);
		break;
	case COLLISHAPE_CYLINDER:
		debugCylinder((const Vec3f*)&shape->mPos, (GXColor*)color, shape->mSizeX, shape->mSizeY);
		break;
	case COLLISHAPE_BOX:
		njPushMatrixEx();
		njTranslateEx(&shape->mPos);
		njRotateY(0, shape->mRotY);
		Vec3f size;
		size.x = 2.0f * shape->mSizeX;
		size.y = 2.0f * shape->mSizeY;
		size.z = 2.0f * shape->mSizeZ;
		debugCube(0, (const Vec3f*)&size, (GXColor*)color);
		njPopMatrixEx();
		break;
	}
}

// 0x800082DC
void colliContactInit(ColliContact* contact)
{
	contact->mNext       = 0;
	contact->mOther      = 0;
	contact->mOtherShape = 0;
	contact->mPos.x      = 0.0f;
	contact->mPos.y      = 0.0f;
	contact->mPos.z      = 0.0f;
	contact->mShape      = 0;
	contact->mPush.x     = 0.0f;
	contact->mPush.y     = 0.0f;
	contact->mPush.z     = 0.0f;
}

// 0x80008310
// The other collider's shape that touched
ColliShape* colliContactShape(const ColliContact* contact)
{
	return colliGetShape(contact->mOther, contact->mOtherShape);
}

// 0x8000833C
int colliContactHasBit8(ColliContact* contact) { return colliContactShape(contact)->mFlags & 8; }

// 0x80008364
int colliContactInGroup1(const ColliContact* contact) { return colliInGroup(contact->mOther, 1); }

// 0x8000838C
int colliContactInGroup8(const ColliContact* contact) { return colliInGroup(contact->mOther, 8); }

// 0x800083B4
int colliContactHasBit40(ColliContact* contact) { return colliContactShape(contact)->mFlags & 0x40; }

// 0x800083DC
int colliContactInGroup2(const ColliContact* contact) { return colliInGroup(contact->mOther, 2); }

// 0x80008404
int colliContactNoBit10(ColliContact* contact) { return !(colliContactShape(contact)->mFlags & 0x10); }

// 0x80008434
// The id of the object that owns the other side of a touch
u32 colliContactOwner(const ColliContact* contact) { return (u16)colliGetOwner(contact->mOther); }

// 0x8000845C
int colliContactTag(ColliContact* contact) { return colliContactShape(contact)->mTag; }

// 0x80008480
void colliInitPush()
{
	sColliPush.set(0.0f, 0.0f, 0.0f);
	sColliPushA.set(0.0f, 0.0f, 0.0f);
	sColliPushB.set(0.0f, 0.0f, 0.0f);
}

// 0x800084C0
void colliExitPush() { }
}

// 0x800084C4
CharaColli::CharaColli()
{
	mPrev = 0;
	mNext = 0;
	colliReset(this);
}

// 0x80008500
CharaColli::~CharaColli() { }

extern "C" {
// 0x8000853C
void colliReset(CharaColli* colli)
{
	colli->mShapeCount = 0;
	colliShapeInit(&colli->mInline);
	colli->mShapes = 0;
	colli->mOwner  = -1;
	colliShapeInit(&colli->mBounds);
	colli->mBounds.mType = COLLISHAPE_SPHERE;
	colli->mContactCount = 0;
	colli->mContacts     = 0;
	colli->mLastContact  = 0;
	colli->mState        = 0;
	colli->mGroup        = 1;
	colli->mFirmMask     = 0;
	colli->mSensorMask   = 0;
	colli->mIgnoreMask   = 0;
}

// 0x800085B8
// One shape uses mInline
void colliAllocShapes(CharaColli* colli, int count)
{
	colli->mShapeCount = count;
	if (colli->mShapeCount == 1) {
		colli->mShapes = &colli->mInline;
	} else {
		colli->mShapes = new ColliShape[colli->mShapeCount];
	}
}

// 0x80008608
void colliFreeShapes(CharaColli* colli)
{
	if (colli->mShapes != 0 && colli->mShapes != &colli->mInline) {
		delete colli->mShapes;
	}
	colli->mShapes     = 0;
	colli->mShapeCount = 0;
}

// 0x80008658
void colliClear(CharaColli* colli)
{
	colliFreeShapes(colli);
	colliReset(colli);
}

// 0x8000868C
int colliHasShape(CharaColli* colli, int shape)
{
	u8 has = 0;
	if (shape >= 0 && shape < colli->mShapeCount) {
		has = 1;
	}
	return has;
}

// 0x800086B0
int colliGetContactCount(CharaColli* colli) { return colli->mContactCount; }

// 0x800086B8
ColliContact* colliGetContact(CharaColli* colli, int index)
{
	if (index < 0 || index >= colli->mContactCount) {
		return 0;
	}
	ColliContact* contact = colli->mContacts;
	for (int i = 0; i < index; i++) {
		if (contact == 0) {
			return 0;
		}
		contact = contact->mNext;
	}
	return contact;
}

// 0x80008700
int colliGetShapeCount(CharaColli* colli) { return colli->mShapeCount; }

// 0x80008708
ColliShape* colliGetShapes(CharaColli* colli) { return colli->mShapes; }

// 0x80008710
ColliShape* colliGetShape(CharaColli* colli, int shape) { return &colli->mShapes[shape]; }

// 0x80008720

void colliCopyShapes(CharaColli* colli, const ColliShape* src)
{
	if (colli->mShapes == 0 && src == 0) {
		return;
	}
	for (int i = 0; i < colli->mShapeCount; i++) {
		*(ColliShapeCopy*)&colli->mShapes[i] = *(const ColliShapeCopy*)&src[i];
	}
}

// 0x800087C4
void colliCopyShapesTagged(CharaColli* colli, const ColliShape* src, u32 tag)
{
	if (colli->mShapes == 0 && src == 0) {
		return;
	}
	for (int i = 0; i < colli->mShapeCount; i++) {
		*(ColliShapeCopy*)&colli->mShapes[i] = *(const ColliShapeCopy*)&src[i];
		colli->mShapes[i].mTag = tag;
	}
}

// 0x80008874
// Places the shapes of src turned by ang about pos
void colliPlaceShapesRotY(CharaColli* colli, const Vec3f* pos, int ang, const ColliShape* src)
{
	if (colli->mShapes == 0 || src == 0) {
		return;
	}
	gjPushUnitMatrix();
	njTranslateEx(pos);
	gjRotateY(ang);
	for (int i = 0; i < colli->mShapeCount; i++) {
		njCalcPointEx(&src[i].mPos, &colli->mShapes[i].mPos);
		colli->mShapes[i].mRotY = ang;
	}
	njPopMatrixEx();
}

static inline Vec3f colliAdd(const Vec3f* a, const Vec3f* b)
{
	Vec3f sum;
	Vec3f in;
	in.set(a->x, a->y, a->z);
	gjAddVector(&sum, &in, b);
	return sum;
}

// 0x80008914
// Places the shapes of src at pos
void colliPlaceShapes(CharaColli* colli, const Vec3f* pos, const ColliShape* src)
{
	if (colli->mShapes == 0 || src == 0) {
		return;
	}
	for (int i = 0; i < colli->mShapeCount; i++) {
		colli->mShapes[i].mPos = colliAdd(&src[i].mPos, pos);
	}
}

// 0x800089E0
// colliPlaceShapesRotY and bounds goes to mBounds the same way
void colliPlaceShapesBounds(CharaColli* colli, const Vec3f* pos, int ang, const ColliShape* src, const Vec3f* bounds)
{
	if (colli->mShapes == 0 || src == 0) {
		return;
	}
	gjPushUnitMatrix();
	njTranslateEx(pos);
	gjRotateY(ang);
	for (int i = 0; i < colli->mShapeCount; i++) {
		njCalcPointEx(&src[i].mPos, &colli->mShapes[i].mPos);
		colli->mShapes[i].mRotY = ang;
	}
	njCalcPointEx(bounds, &colli->mBounds.mPos);
	njPopMatrixEx();
}

// 0x80008A90
void colliSetBounds(CharaColli* colli, const ColliShape* bounds)
{
	colli->mBounds = *bounds;
	colliBoundsOn(colli);
}

// 0x80008B10
// mBounds when it is on else the first shape
ColliShape* colliGetBounds(CharaColli* colli)
{
	if (colli->mState & 2) {
		return &colli->mBounds;
	}
	return colli->mShapes;
}

// 0x80008B2C
// A sphere around count shapes centred on their average
void colliCalcBounds(ColliShape* out, ColliShape* shapes, int count)
{
	if (shapes == 0) {
		return;
	}
	Vec3f center;
	Vec3f d;
	Vec3f pos;
	center.x = 0.0f;
	center.y = 0.0f;
	center.z = 0.0f;
	int i;
	for (i = 0; i < count; i++) {
		pos.x = shapes[i].mPos.x;
		pos.y = shapes[i].mPos.y;
		pos.z = shapes[i].mPos.z;
		njAddVector(&center, &pos);
	}
	gjScaleVector(1.0f / count, &center, &center);

	f32 maxRadius = 0.0f;
	f32 radius;
	for (i = 0; i < count; i++) {
		ColliShape* shape = &shapes[i];
		d.x = shape->mPos.x;
		d.y = shape->mPos.y;
		d.z = shape->mPos.z;
		njSubVector(&d, &center);
		radius = njScalor(&d);
		switch (shape->mType) {
		case COLLISHAPE_SPHERE:
			radius += shape->mSizeX;
			break;
		case COLLISHAPE_CYLINDER:
			radius = radius + colliSqrt(shape->mSizeX * shape->mSizeX + shape->mSizeY * shape->mSizeY);
			break;
		case COLLISHAPE_BOX:
			radius = radius + colliSqrt(shape->mSizeX * shape->mSizeX + shape->mSizeY * shape->mSizeY + shape->mSizeZ * shape->mSizeZ);
			break;
		}
	}
	if (radius > maxRadius) {
		maxRadius = radius;
	}
	out->mPos.x  = center.x;
	out->mPos.y  = center.y;
	out->mPos.z  = center.z;
	out->mSizeX  = 3.0f + maxRadius;
}

// 0x80008D0C
void colliRefreshBounds(CharaColli* colli)
{
	colliCalcBounds(&colli->mBounds, colli->mShapes, colli->mShapeCount);
	colliBoundsOn(colli);
}

// 0x80008D4C
// Also moves the collider to the list its group belongs in
void colliSetMasks(CharaColli* colli, int group, int firm, int sensor, int ignore)
{
	colli->mGroup      = group;
	colli->mFirmMask   = firm;
	colli->mSensorMask = sensor;
	colli->mIgnoreMask = ignore;
	if (colli->mGroup & 9) {
		colliLink(colli, sColliLists[0]);
	} else if (colli->mGroup & 4) {
		colliLink(colli, sColliLists[2]);
	} else if (colli->mGroup & 0x10) {
		colliLink(colli, sColliLists[3]);
	}
}

// 0x80008DCC
void colliAddMasks(CharaColli* colli, int firm, int sensor, int ignore)
{
	colli->mFirmMask |= firm;
	colli->mSensorMask |= sensor;
	colli->mIgnoreMask |= ignore;
}

// 0x80008DF4
void colliClearMasks(CharaColli* colli, int firm, int sensor, int ignore)
{
	colli->mFirmMask &= ~firm;
	colli->mSensorMask &= ~sensor;
	colli->mIgnoreMask &= ~ignore;
}

// 0x80008E1C
int colliInGroup(CharaColli* colli, int group) { return colli->mGroup & (u8)group; }

// 0x80008E2C
int colliIsFirm(CharaColli* colli, int group) { return colli->mFirmMask & (u8)group; }

// 0x80008E3C
int colliIsSensor(CharaColli* colli, int group) { return colli->mSensorMask & (u8)group; }

// 0x80008E4C
u8 colliGetGroup(CharaColli* colli) { return colli->mGroup; }

// 0x80008E54
void colliSetOwner(CharaColli* colli, u32 id) { colli->mOwner = id; }

// 0x80008E5C
u32 colliGetOwner(CharaColli* colli) { return colli->mOwner; }

// 0x80008E64
void colliEnable(CharaColli* colli) { colli->mState &= ~1; }

// 0x80008E74
void colliDisable(CharaColli* colli) { colli->mState |= 1; }

// 0x80008E84
int colliIsDisabled(CharaColli* colli) { return colli->mState & 1; }

// 0x80008E90
void colliBoundsOn(CharaColli* colli) { colli->mState |= 2; }

// 0x80008EA0
void colliBoundsOff(CharaColli* colli) { colli->mState &= ~2; }

// 0x80008EB0
int colliHasBounds(CharaColli* colli) { return colli->mState & 2; }

// 0x80008EBC
void colliShapeSetBit8(CharaColli* colli, int shape)
{
	if (colliHasShape(colli, shape)) {
		((u32*)&colli->mShapes[shape])[9] |= 8;
	}
}

// 0x80008F0C
void colliShapeClearBit8(CharaColli* colli, int shape)
{
	if (colliHasShape(colli, shape)) {
		((u32*)&colli->mShapes[shape])[9] &= ~8;
	}
}

// 0x80008F5C
void colliShapeSetBit10(CharaColli* colli, int shape)
{
	if (colliHasShape(colli, shape)) {
		((u32*)&colli->mShapes[shape])[9] |= 0x10;
	}
}

// 0x80008FAC
void colliShapeClearBit10(CharaColli* colli, int shape)
{
	if (colliHasShape(colli, shape)) {
		((u32*)&colli->mShapes[shape])[9] &= ~0x10;
	}
}

// 0x80008FFC
void colliShapeSetTag(CharaColli* colli, int shape, u32 tag)
{
	if (colliHasShape(colli, shape)) {
		((u32*)&colli->mShapes[shape])[11] = tag;
	}
}

// 0x80009048
// Tests two shapes and when they touch marks both and adds the push to sColliPushA and sColliPushB
// mode 0 only tests 1 moves the first shape all the way 2 the second 3 each half way
int colliTestShapes(ColliShape* a, ColliShape* b, int mode)
{
	int hit = 0;
	int test;
	f32 weightA;
	f32 weightB;
	switch (mode) {
	case 0:
	default:
		weightA = 0.0f;
		test    = 1;
		weightB = weightA;
		break;
	case 1:
		weightA = 1.0f;
		test    = 0;
		weightB = 0.0f;
		break;
	case 2:
		weightA = 0.0f;
		test    = 0;
		weightB = -1.0f;
		break;
	case 3:
		weightA = 0.5f;
		test    = 0;
		weightB = -0.5f;
		break;
	}

	switch (a->mType) {
	case COLLISHAPE_SPHERE:
		switch (b->mType) {
		case COLLISHAPE_SPHERE:
			hit = colliSphereSphere(a, b, test);
			break;
		case COLLISHAPE_CYLINDER:
			hit = colliSphereCylinder(a, b, test);
			break;
		case COLLISHAPE_BOX:
			hit = colliSphereBox(a, b, test);
			break;
		}
		break;
	case COLLISHAPE_CYLINDER:
		switch (b->mType) {
		case COLLISHAPE_SPHERE:
			weightA = -weightA;
			weightB = -weightB;
			hit     = colliSphereCylinder(b, a, test);
			break;
		case COLLISHAPE_CYLINDER:
			hit = colliCylinderCylinder(a, b, test);
			break;
		case COLLISHAPE_BOX:
			hit = colliCylinderBox(a, b, test);
			break;
		}
		break;
	case COLLISHAPE_BOX:
		switch (b->mType) {
		case COLLISHAPE_SPHERE:
			weightA = -weightA;
			weightB = -weightB;
			hit     = colliSphereBox(b, a, test);
			break;
		case COLLISHAPE_CYLINDER:
			weightA = -weightA;
			weightB = -weightB;
			hit     = colliCylinderBox(b, a, test);
			break;
		}
		break;
	}

	if (hit != 0) {
		a->mFlags |= 2;
		b->mFlags |= 2;
		if (mode & 1) {
			gjMulAddVector(weightA, &sColliPushA, &sColliPushA, &sColliPush);
		}
		if (mode & 2) {
			gjMulAddVector(weightB, &sColliPushB, &sColliPushB, &sColliPush);
		}
	}
	return hit;
}

// 0x800092B8
int colliSphereSphere(ColliShape* a, ColliShape* b, int test)
{
	int hit = 0;
	Vec3f d;
	d.x = a->mPos.x;
	d.y = a->mPos.y;
	d.z = a->mPos.z;
	d.x -= b->mPos.x;
	d.y -= b->mPos.y;
	d.z -= b->mPos.z;
	f32 reach = a->mSizeX + b->mSizeX;
	reach *= reach;
	if (njScalor2(&d) < reach) {
		hit = 1;
		if (test == 0) {
			f32 dy   = d.y * d.y;
			d.y      = 0.0f;
			f32 flat = colliSqrt(reach - dy);
			f32 len  = normalizeVector(&d);
			if (0.0f == len) {
				sColliPush.x = 0.0f;
				sColliPush.y = 0.0f;
				sColliPush.z = flat;
			} else {
				f32 k        = flat - len;
				sColliPush.x = d.x * k;
				sColliPush.y = 0.0f;
				sColliPush.z = d.z * k;
			}
		}
	}
	return hit;
}

// 0x800093D0
int colliSphereCylinder(ColliShape* sphere, ColliShape* cyl, int test)
{
	int hit    = 0;
	f32 radius = sphere->mSizeX;
	f32 dy     = sphere->mPos.y - cyl->mPos.y;
	f32 height = cyl->mSizeY;
	f32 cylRad = cyl->mSizeX;
	f32 reachY = height + radius;
	f32 reach  = cylRad + radius;
	f32 absY   = colliAbs(dy);
	if (absY < reachY) {
		Vec3f d;
		d = sphere->mPos;
		d.x -= cyl->mPos.x;
		d.y = 0.0f;
		d.z -= cyl->mPos.z;
		f32 len = normalizeVector(&d);
		if (absY < height) {
			if (len < reach) {
				hit = 1;
				if (test == 0) {
					sColliPush.x = d.x * (reach - len);
					sColliPush.y = 0.0f;
					sColliPush.z = d.z * (reach - len);
				}
			}
		} else if (len < cylRad) {
			hit = 2;
			if (test == 0) {
				sColliPush.x = 0.0f;
				sColliPush.z = 0.0f;
				if (dy >= 0.0f) {
					sColliPush.y = reachY - dy;
				} else {
					sColliPush.y = -reachY - dy;
				}
			}
		} else if (len < reach) {
			// Squared the distance out from the rim and the height above the top or below the bottom
			f32 out = len - cylRad;
			absY -= height;
			radius *= radius;
			f32 out2 = out * out;
			absY *= absY;
			if (out2 + absY < radius) {
				hit = 1;
				if (test == 0) {
					f32 k        = colliSqrtFull(radius - absY);
					sColliPush.x = d.x * (k - out);
					sColliPush.y = 0.0f;
					sColliPush.z = d.z * (k - out);
				}
			}
		}
	}
	return hit != 0;
}

// 0x8000963C
int colliSphereBox(ColliShape* sphere, ColliShape* box, int test)
{
	f32 sn;
	f32 cs;
	f32 radius = sphere->mSizeX;
	f32 sizeX  = box->mSizeX;
	f32 sizeY  = box->mSizeY;
	f32 sizeZ  = box->mSizeZ;
	f32 lx;
	f32 lz;
	f32 absX;
	f32 absZ;
	if (box->mRotY != 0) {
		f32 dx;
		f32 dz = sphere->mPos.z - box->mPos.z;
		dx     = sphere->mPos.x - box->mPos.x;
		sn     = njSin(box->mRotY);
		cs     = njCos(box->mRotY);
		lz     = cs * dz + sn * dx;
		lx     = -sn * dz + cs * dx;
		absZ   = colliAbs(lz);
		absX   = colliAbs(lx);
	} else {
		lz   = sphere->mPos.z - box->mPos.z;
		sn   = 0.0f;
		lx   = sphere->mPos.x - box->mPos.x;
		cs   = 1.0f;
		absZ = colliAbs(lz);
		absX = colliAbs(lx);
	}
	f32 dy   = sphere->mPos.y - box->mPos.y;
	f32 absY = colliAbs(dy);
	if (absX >= sizeX + radius || absY >= sizeY + radius || absZ >= sizeZ + radius) {
		return 0;
	}

	f32 dist2 = 0.0f;
	u8 mask   = 0;
	int count = 0;
	f32 outX  = absX - sizeX;
	if (outX > 0.0f) {
		dist2 = outX * outX + dist2;
		mask |= 1;
		count++;
	}
	f32 outY = absY - sizeY;
	f32 most = outX;
	int axis = 1;
	if (outY > 0.0f) {
		dist2 = outY * outY + dist2;
		mask |= 2;
		count++;
	}
	if (outY > most) {
		most = outY;
		axis = 2;
	}
	f32 outZ = absZ - sizeZ;
	if (outZ > 0.0f) {
		dist2 = outZ * outZ + dist2;
		mask |= 4;
		count++;
	}
	if (outZ > most) {
		most = outZ;
		axis = 4;
	}
	if (count >= 2 && dist2 > radius * radius) {
		return 0;
	}

	if (test == 0) {
		if (count <= 1) {
			f32 k;
			switch (axis) {
			case 1:
				k = radius - most;
				if (lx > 0.0f) {
					sColliPush.x = cs * k;
					sColliPush.y = 0.0f;
					sColliPush.z = -sn * k;
				} else {
					sColliPush.x = -cs * k;
					sColliPush.y = 0.0f;
					sColliPush.z = sn * k;
				}
				break;
			case 4:
				k = radius - most;
				if (lz > 0.0f) {
					sColliPush.z = cs * k;
					sColliPush.x = sn * k;
					sColliPush.y = 0.0f;
				} else {
					sColliPush.y = 0.0f;
					sColliPush.z = -cs * k;
					sColliPush.x = -sn * k;
				}
				break;
			case 2:
				sColliPush.y = radius - most;
				if (dy <= 0.0f) {
					sColliPush.y = -sColliPush.y;
				}
				sColliPush.x = 0.0f;
				sColliPush.z = 0.0f;
				break;
			default:
				sColliPush.x = 0.0f;
				sColliPush.y = 0.0f;
				sColliPush.z = 0.0f;
				break;
			}
		} else {
			sColliPush.x = 0.0f;
			sColliPush.y = 0.0f;
			sColliPush.z = 0.0f;
			if (mask & 1) {
				if (lx <= 0.0f) {
					sColliPush.z += sn * outX;
					sColliPush.x += -cs * outX;
				} else {
					sColliPush.x += cs * outX;
					sColliPush.z += -sn * outX;
				}
			}
			if (mask & 2) {
				sColliPush.y = outY;
				if (dy <= 0.0f) {
					sColliPush.y = -outY;
				}
			}
			if (mask & 4) {
				if (lz <= 0.0f) {
					sColliPush.z += -cs * outZ;
					sColliPush.x += -sn * outZ;
				} else {
					sColliPush.z += cs * outZ;
					sColliPush.x += sn * outZ;
				}
			}
			f32 k = radius - normalizeVector(&sColliPush);
			sColliPush.y *= k;
			sColliPush.x *= k;
			sColliPush.z *= k;
		}
	}
	return 1;
}

// 0x80009AAC
int colliCylinderCylinder(ColliShape* a, ColliShape* b, int test)
{
	f32 over;
	f32 dy = a->mPos.y - b->mPos.y;
	f64 absY = __fabs(dy);
	over   = absY;
	over -= a->mSizeY + b->mSizeY;
	if (over >= 0.0f) {
		return 0;
	}
	Vec3f d;
	d.x       = a->mPos.x - b->mPos.x;
	d.y       = 0.0f;
	d.z       = a->mPos.z - b->mPos.z;
	f32 dist2 = njInnerProduct(&d, &d);
	f32 reach = a->mSizeX + b->mSizeX;
	if (dist2 - reach * reach >= 0.0f) {
		return 0;
	}
	if (test == 0) {
		f32 k = normalizeVector(&d) - reach;
		if (over > k) {
			if (dy > 0.0f) {
				sColliPush.x = 0.0f;
				sColliPush.y = -over;
				sColliPush.z = 0.0f;
			} else {
				sColliPush.x = 0.0f;
				sColliPush.y = over;
				sColliPush.z = 0.0f;
			}
		} else {
			sColliPush.set(-d.x * k, 0.0f, -d.z * k);
		}
	}
	return 1;
}

// 0x80009C28
int colliCylinderBox(ColliShape* cyl, ColliShape* box, int test)
{
	f32 dy;
	f32 outY;
	dy = cyl->mPos.y - box->mPos.y;
	outY   = colliAbs(dy);
	outY -= cyl->mSizeY + box->mSizeY;
	if (outY >= 0.0f) {
		return 0;
	}
	{
		f32 sn;
		f32 cs;
		f32 lx;
		f32 lz;
		f32 absX;
		f32 absZ;
		if (box->mRotY != 0) {
			f32 dx = cyl->mPos.x - box->mPos.x;
			f32 dz = cyl->mPos.z - box->mPos.z;
			sn     = njSin(box->mRotY);
			cs     = njCos(box->mRotY);
			lz     = cs * dz + sn * dx;
			lx     = -sn * dz + cs * dx;
			absZ   = colliAbs(lz);
			absX   = colliAbs(lx);
		} else {
			lz   = cyl->mPos.z - box->mPos.z;
			cs   = 1.0f;
			lx   = cyl->mPos.x - box->mPos.x;
			sn   = 0.0f;
			absZ = colliAbs(lz);
			absX = colliAbs(lx);
		}
		f32 radius = cyl->mSizeX;
		f32 outX   = absX - box->mSizeX;
		if (outX >= radius) {
			return 0;
		}
		{
			f32 outZ = absZ - box->mSizeZ;
			if (outZ >= radius) {
				return 0;
			}
			if (outX > 0.0f && outZ > 0.0f && outX * outX + outZ * outZ >= radius * radius) {
				return 0;
			}
			if (test == 0) {
				outY += radius;
				sColliPush.y = 0.0f;
				if (outY > outX && outY > outZ) {
					// Over or under the box
					sColliPush.x = 0.0f;
					outY -= radius;
					sColliPush.z = 0.0f;
					if (dy > 0.0f) {
						sColliPush.y = -outY;
					} else {
						sColliPush.y = outY;
					}
				} else if (outX > 0.0f && outZ > 0.0f) {
					// Past a corner
					Vec3f e;
					e.x = lx > 0.0f ? outX : -outX;
					e.y = 0.0f;
					if (lz > 0.0f) {
						e.z = outZ;
					} else {
						e.z = -outZ;
					}
					radius -= normalizeVector(&e);
					f32 ex       = e.x * radius;
					sColliPush.y = 0.0f;
					f32 ez       = e.z * radius;
					sColliPush.z = cs * ez - sn * ex;
					sColliPush.x = sn * ez + cs * ex;
				} else if (outX > outZ) {
					f32 k = radius - outX;
					if (lx > 0.0f) {
						sColliPush.x = cs * k;
						sColliPush.z = -sn * k;
					} else {
						sColliPush.z = sn * k;
						sColliPush.x = -cs * k;
					}
				} else {
					f32 k = radius - outZ;
					if (lz > 0.0f) {
						sColliPush.z = cs * k;
						sColliPush.x = sn * k;
					} else {
						sColliPush.z = -cs * k;
						sColliPush.x = -sn * k;
					}
				}
			}
			return 1;
		}
	}
}

// 0x80009F7C
void colliFree(CharaColli* colli) { colliRetire(colli); }

// 0x80009F9C
void colliClearContacts(CharaColli* colli)
{
	colli->mContactCount = 0;
	colli->mContacts     = 0;
	colli->mLastContact  = 0;
	for (int i = 0; i < colli->mShapeCount; i++) {
		colli->mShapes[i].mFlags &= ~2;
	}
}

// 0x80009FE0
void colliListClearContacts(CharaColli* list)
{
	for (CharaColli* colli = list->mNext; colli != 0; colli = colli->mNext) {
		colliClearContacts(colli);
	}
}

// 0x8000A020
// Tests every shape of a against every shape of b and records a contact on both for each touch
int colliTestPair(CharaColli* a, CharaColli* b)
{
	int i;
	int countB;
	ColliShape* shapeA;
	ColliShape* shapeB;
	ColliShape* boundsA;
	int near;
	int result;
	int mode;
	int groupB;
	int j;
	if (a->mIgnoreMask & b->mGroup) {
		return 0;
	}
	if (b->mIgnoreMask & a->mGroup) {
		return 0;
	}
	near = 1;
	if (colliHasBounds(a) != 0 || colliHasBounds(b) != 0) {
		boundsA = colliGetBounds(a);
		ColliShape* boundsB = colliGetBounds(b);
		near                = colliSphereSphere(boundsA, boundsB, 1);
	}
	result = 0;
	if (near != 0) {
		countB     = colliGetShapeCount(b);
		groupB = colliGetGroup(b);
		if (colliIsSensor(b, a->mGroup) != 0 || colliIsSensor(a, groupB) != 0) {
			mode = 0;
		} else {
			int firmA = colliIsFirm(a, groupB);
			int firmB = colliIsFirm(b, a->mGroup);
			if (firmA != 0) {
				if (firmB != 0) {
					mode = 0;
				} else {
					mode = 2;
				}
			} else if (firmB != 0) {
				mode = 1;
			} else {
				mode = 3;
			}
		}
		sColliPushA.set(0.0f, 0.0f, 0.0f);
		sColliPushB.set(0.0f, 0.0f, 0.0f);
		shapeA = a->mShapes;
		for (i = 0; i < a->mShapeCount; i++) {
			shapeB = colliGetShapes(b);
			for (j = 0; j < countB; j++) {
				if (colliTestShapes(shapeA, shapeB, mode) != 0) {
					result = 1;
					colliAddContact(a, b, j, i, &sColliPushA, &shapeB->mPos);
					colliAddContact(b, a, i, j, &sColliPushB, &shapeA->mPos);
				}
				shapeB++;
			}
			shapeA++;
		}
	}
	return result;
}

// 0x8000A238
// Tests the colliders of one list against each other
void colliTestList(CharaColli* list)
{
	for (CharaColli* a = list->mNext; a != 0; a = a->mNext) {
		if (colliIsDisabled(a) == 0) {
			for (CharaColli* b = a->mNext; b != 0; b = b->mNext) {
				if (colliIsDisabled(b) == 0) {
					colliTestPair(a, b);
				}
			}
		}
	}
}

// 0x8000A2B0
// Tests the colliders of one list against those of another
void colliTestLists(CharaColli* listA, CharaColli* listB)
{
	for (CharaColli* a = listA->mNext; a != 0; a = a->mNext) {
		if (colliIsDisabled(a) == 0) {
			for (CharaColli* b = listB->mNext; b != 0; b = b->mNext) {
				if (colliIsDisabled(b) == 0) {
					colliTestPair(a, b);
				}
			}
		}
	}
}

// 0x8000A32C
void colliAddContact(CharaColli* colli, CharaColli* other, int otherShape, int shape, Vec3f* push,
                     Vec3f* pos)
{
	ColliContact* contact = colliAllocContact();
	if (contact != 0) {
		contact->mOther      = other;
		contact->mOtherShape = otherShape;
		contact->mPos.x      = pos->x;
		contact->mPos.y      = pos->y;
		contact->mPos.z      = pos->z;
		contact->mShape      = shape;
		contact->mPush.x     = push->x;
		contact->mPush.y     = push->y;
		contact->mPush.z     = push->z;
		colli->mContactCount++;
		if (colli->mContacts == 0) {
			colli->mContacts = contact;
		} else {
			colli->mLastContact->mNext = contact;
		}
		colli->mLastContact = contact;
	}
}

// 0x8000A3DC
void colliSetList(CharaColli* colli, int list)
{
	if (list >= 0 && list < 4) {
		colliLink(colli, sColliLists[list]);
	}
}

// 0x8000A41C
// Unlinks colli and when list is set puts it first after list
void colliLink(CharaColli* colli, CharaColli* list)
{
	if (colli->mPrev != 0) {
		colli->mPrev->mNext = colli->mNext;
	}
	if (colli->mNext != 0) {
		colli->mNext->mPrev = colli->mPrev;
	}
	if (list != 0) {
		colli->mNext = list->mNext;
		if (colli->mNext != 0) {
			colli->mNext->mPrev = colli;
		}
		colli->mPrev = list;
		list->mNext  = colli;
		return;
	}
	colli->mPrev = 0;
	colli->mNext = 0;
}

// 0x8000A480
CharaColli* colliNext(CharaColli* colli) { return colli->mNext; }

// 0x8000A488
// Draws each shape with colliShapeDebugDraw
void colliDrawShapes(CharaColli* colli)
{
	ColliColor color;
	int i;

	for (i = 0; i < colli->mShapeCount; i++) {
		if (colliIsDisabled(colli) != 0) {
			color.r = sColliColorGrey.r;
			color.g = sColliColorGrey.g;
			color.b = sColliColorGrey.b;
			color.a = sColliColorGrey.a;
			colliShapeDebugDraw((&colli->mShapes[i]), &color);
		} else if (((&colli->mShapes[i])->mFlags & 2) != 0) {
			color.r = sColliColorYellow.r;
			color.g = sColliColorYellow.g;
			color.b = sColliColorYellow.b;
			color.a = sColliColorYellow.a;
			colliShapeDebugDraw((&colli->mShapes[i]), &color);
		} else {
			color.r = sColliColorGreen.r;
			color.g = sColliColorGreen.g;
			color.b = sColliColorGreen.b;
			color.a = sColliColorGreen.a;
			colliShapeDebugDraw((&colli->mShapes[i]), &color);
		}
	}
}

// 0x8000A590
void initColli()
{
	int i;
	colliInitPush();
	gColliFree    = 0;
	gColliRetired = 0;
	for (i = 0; i < 4; i++) {
		sColliLists[i] = 0;
	}
	CharaColli* pool = new CharaColli[0x180];
	CharaColli* list = pool + 1;
	gColliRetired    = list;
	gColliPool       = pool;
	gColliFree       = pool;
	list++;
	for (i = 0; i < 4; i++) {
		sColliLists[i] = list++;
	}
	for (i = 6; i < 0x180; i++) {
		colliLink(&gColliPool[i], gColliFree);
	}
	gColliContactCount = 0;
	gColliContacts     = new ColliContact[0x100];
	for (i = 0; i < 0x100; i++) {
		colliContactInit(&gColliContacts[i]);
	}
	gColliDebug = 4;
	colliClearDebug();
}

// 0x8000A6CC
TCharaColliCtrl* createTCharaColliCtrl() { return new TCharaColliCtrl(gTL_01); }

// 0x8000A70C
void exitColli()
{
	if (gColliContacts != 0) {
		delete[] gColliContacts;
		gColliContacts = 0;
	}
	gColliContactCount = 0;
	if (gColliPool != 0) {
		delete[] gColliPool;
		gColliPool = 0;
	}
	gColliFree    = 0;
	gColliRetired = 0;
	for (int i = 0; i < 4; i++) {
		sColliLists[i] = 0;
	}
	colliExitPush();
}

// 0x8000A7A0
// Takes a collider with count shapes from the free list and puts it in list
CharaColli* colliCreate(int count, int list)
{
	CharaColli* colli = colliNext(gColliFree);
	if (colli != 0) {
		colliAllocShapes(colli, count);
		switch (list) {
		case 0:
			colliLink(colli, sColliLists[0]);
			break;
		case 1:
		default:
			colliLink(colli, sColliLists[1]);
			break;
		case 2:
			colliLink(colli, sColliLists[2]);
			break;
		case 3:
			colliLink(colli, sColliLists[3]);
			break;
		}
	}
	return colli;
}

// 0x8000A868
void colliRetire(CharaColli* colli)
{
	if (colli != 0) {
		colliLink(colli, gColliRetired);
	}
}

// 0x8000A894
// Once a frame clears last frame's contacts then tests list 0 against itself and the rest and list 1 against itself and
// list 2
void colliUpdate()
{
	CharaColli* list0;
	CharaColli* list1;
	CharaColli* list2;
	CharaColli* list3;
	gColliContactCount = 0;
	for (int i = 0; i < 4; i++) {
		colliListClearContacts(sColliLists[i]);
	}
	list0 = sColliLists[0];
	list1 = sColliLists[1];
	list2 = sColliLists[2];
	list3 = sColliLists[3];
	colliTestList(list0);
	colliTestLists(list0, list1);
	colliTestLists(list0, list2);
	colliTestLists(list0, list3);
	colliTestList(list1);
	colliTestLists(list1, list2);
}

// 0x8000A93C
ColliContact* colliAllocContact()
{
	if (gColliContactCount >= 0x100) {
		return 0;
	}
	ColliContact* contact = &gColliContacts[gColliContactCount++];
	colliContactInit(contact);
	return contact;
}

// 0x8000A994
void colliClearDebug() { gColliDebug &= ~1; }
}
