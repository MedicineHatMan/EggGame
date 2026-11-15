#include "camera/TCamera.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "enemy/EnemyFx.h"
#include "camera/CamCurve.h"
#include "camera/CamChase.h"
#include "Ninja/ninja.h"
#include "stl/math.h"
#include "egg/EggCreature.h"
#include "animal/RacerMover.h"

struct CamHeadState {
	Vec3f mPos; // _00
	int mRot;   // _0C the way it last went
	u16 mId;    // _10
	u8 _12[2];
	int mHidden;     // _14 the object said not to follow it
	Vec3f mLastPos;  // _18
	Vec3f mDir;      // _24
	f32 mSpeed;      // _30
	f32 mRise;       // _34
	int mStill;      // _38 frames it has not moved
	CamTrail mTrail; // _3C
};

#define HEAD(h) ((CamHeadState*)h)

extern "C" {

// 0x80098F24
void camTrailClear(CamTrail* trail)
{
	trail->mIndex = 0;
	for (int i = 0; i < 15; i++) {
		trail->mPoints[i].x = 0.0f;
		trail->mPoints[i].y = 0.0f;
		trail->mPoints[i].z = 0.0f;
	}
}

// 0x80098F54
// Adds a place once it is more than step from the newest
f32 camTrailPush(f32 step, CamTrail* trail, const Vec3f* pos)
{
	Vec3f d;
	Vec3f* p = &trail->mPoints[trail->mIndex];
	d.x      = p->x;
	d.y      = p->y;
	d.z      = p->z;
	njSubVector(&d, pos);
	f32 dist = njScalor(&d);
	if (dist > step) {
		trail->mIndex++;
		if (trail->mIndex >= 15) {
			trail->mIndex = 0;
		}
		trail->mPoints[trail->mIndex] = *pos;
	}
	return dist;
}

// 0x8009901C
// A place counting back from the newest
void camTrailGet(const CamTrail* trail, int back, Vec3f* out)
{
	int i  = camTrailIndex(trail, back);
	out->x = trail->mPoints[i].x;
	out->y = trail->mPoints[i].y;
	out->z = trail->mPoints[i].z;
}

// 0x8009906C
int camTrailIndex(const CamTrail* trail, int back)
{
	int i = back + trail->mIndex;
	i %= 15;
	if (i < 0) {
		i += 15;
	}
	return i;
}

// 0x80099090
void camHeadStart(CamHead* head, u16 id)
{
	CamHeadState* h = HEAD(head);
	h->mId          = id;
	TObjAction* obj = camFindTarget(h->mId, &h->mPos);
	if (obj != 0) {
		h->mRot = obj->getRotY();
	} else {
		h->mPos.x = 0.0f;
		h->mPos.y = 0.0f;
		h->mPos.z = 0.0f;
		h->mRot   = 0;
	}
	h->mHidden  = 0;
	h->mLastPos = h->mPos;
	h->mDir.x   = 0.0f;
	h->mDir.y   = 0.0f;
	h->mDir.z   = 0.0f;
	h->mSpeed   = 0.0f;
	h->mRise    = 0.0f;
	h->mStill   = 1;
	camTrailClear(&h->mTrail);
}

// 0x80099148
// Follows the object part way each frame at most 10 unless it asks not to be followed
void camHeadUpdate(CamHead* head, u16 id)
{
	TObjAction* obj;
	CamHeadState* h = HEAD(head);
	h->mLastPos     = h->mPos;
	h->mId          = id;
	Vec3f at;
	obj = camFindTarget(h->mId, &at);
	if (obj == 0) {
		return;
	}
	ObjMsg msg;
	msg.mType = 0x3C;
	if ((obj->sendToHandle(&msg) & 0x80) != 0) {
		h->mHidden = 1;
	} else {
		h->mHidden = 0;
		h->mRot    = obj->getRotY();
		Vec3f d;
		d = h->mLastPos;
		njSubVector(&d, &at);
		f32 len = njUnitVector(&d);
		if (len < gUnitVectorMin) {
			d = gVecZero;
		}
		len *= 0.6f;
		if (len > 10.0f) {
			len = 10.0f;
		}
		gjMulAddVector(len, &h->mPos, &at, &d);
	}
	h->mDir = h->mPos;
	njSubVector(&h->mDir, &h->mLastPos);
	h->mRise  = h->mDir.y;
	h->mDir.y = 0.0f;
	f32 speed = njUnitVector(&h->mDir);
	if (speed < gUnitVectorMin) {
		h->mDir = gVecZero;
	}
	h->mSpeed = speed;
	if (h->mSpeed < 0.2f) {
		h->mStill++;
	} else {
		h->mStill = 0;
		h->mRot   = (int)(10430.38043493439 * (f32)atan2(h->mDir.x, h->mDir.z));
	}
	camTrailPush(10.0f, &h->mTrail, &h->mPos);
}

// 0x80099334
void camHeadGetPos(const CamHead* head, Vec3f* out)
{
	*out = HEAD(head)->mPos;
}

// 0x80099350
void camHeadGetDir(const CamHead* head, Vec3f* out)
{
	*out = HEAD(head)->mDir;
}

// 0x8009936C
f32 camHeadGetRise(CamHead* head)
{
	return HEAD(head)->mRise;
}

// 0x80099374
u16 camHeadGetId(CamHead* head)
{
	return HEAD(head)->mId;
}

// 0x8009937C
int camHeadGetRot(CamHead* head)
{
	return HEAD(head)->mRot;
}

// 0x80099384
int camHeadGetStill(CamHead* head)
{
	return HEAD(head)->mStill;
}

// 0x8009938C
int camHeadIsHidden(CamHead* head)
{
	return HEAD(head)->mHidden;
}

// 0x80099394
// Which way the trail turns from a facing 0 ahead 1 back 2 and 3 to the sides 4 up 5 down
int camHeadTurn(CamHead* head, const Vec3f* facing)
{
	CamHeadState* h = HEAD(head);
	Vec3f p0;
	Vec3f p1;
	Vec3f p2;
	camTrailGet(&h->mTrail, 0, &p0);
	camTrailGet(&h->mTrail, -1, &p1);
	camTrailGet(&h->mTrail, -2, &p2);
	Vec3f d;
	d = p0;
	d.x -= p1.x;
	d.y -= p2.y;
	d.z -= p1.z;
	if (njUnitVector(&d) < gUnitVectorMin) {
		d = gVecZero;
	}
	if ((f32)fabs(d.y) > 0.4f) {
		if (d.y > 0.0f) {
			return 4;
		}
		return 5;
	}
	f32 a    = (f32)atan2(facing->x, facing->z);
	int diff = angleDiff((int)(10430.38043493439 * a), (int)(10430.38043493439 * (f32)atan2(d.x, d.z)));
	int result;
	if (diff < -0x6800) {
		result = 1;
	} else if (diff < -0x1800) {
		result = 3;
	} else if (diff < 0x1800) {
		result = 0;
	} else if (diff < 0x6800) {
		result = 2;
	} else {
		result = 1;
	}
	return result;
}
}
