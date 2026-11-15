#include "player/TPlyComb.h"
#include "engine/SceneMeshes.h"
#include "core/VecMath.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
#include "core/Mem.h"
#include "engine/DrawState.h"
#include "core/MathUtil.h"
#include "object/TObjEggRing.h"
#include "ui/AeArc.h"
#include "stl/math.h"
#include "egg/EggCreature.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "engine/ActorMotion.h"
#include "player/Player.h"
#include "animal/RacerMover.h"
struct Vec3f;
struct GjActor;
struct DrawState;
struct DrawStateSlot;

extern "C" {

f32 sCombLinkRadii[] = {
	0.5f, 0.5f, 0.8f, 1.0f, 1.0f,
};
f32 sCombLinkLimits[] = {
	0.5f, 0.4f, 0.3f, 0.22f, 0.1f,
};
}

// A node of the model the strands follow
struct ActorNode {
	u32 mFlags;       // _00
	u32 mModel;       // _04
	f32 mTrans[3];    // _08
	f32 mRot[3];      // _14
	f32 mScale[3];    // _20
	ActorNode* mChild; // _2C
	ActorNode* mNext;  // _30
	f32 mQuat;        // _34
};

struct JointCursor {
	int mIndex;
	TPlyComb* mComb;
};

typedef char TPlyComb_size_check[(sizeof(TPlyComb) == 0x11C) ? 1 : -1];
typedef char CombLink_size_check[(sizeof(CombLink) == 0xDC) ? 1 : -1];
typedef char CombVec_size_check[(sizeof(CombVec<HitVec>) == 0xC) ? 1 : -1];

static int linkIndex(int index);
static int copyNodes(ActorNode* node, int index, f32* mtx);
static int skinNodes(ActorNode* node, int index, CombLink* link, Vec3f* hit);
static void captureJoint(void*, JointCursor* cursor);

// The vector instances come last CombList<CombLink>'s and CombVec<HitVec>'s then CombVec<u8>'s constructor then
// CombList's reserve

const char* gTPlyCombName = "TPlyComb";

// 0x800450C0
TPlyComb::TPlyComb(TObject* parent)
    : TObject(parent)
    , mStore(0x2580)
    , mHits(0x15)
    , mRoot(0)
    , mLinks(0xF)
{
	_00         = (u32)gTPlyCombName;
	mSize       = 0x11C;
	mOn         = 1;
	mBits       = 0;
	mBits |= 0x40;
	mLight.mFlags = 2;
}

// 0x80045164
TPlyComb::~TPlyComb()
{
}

// 0x80045210
void TPlyComb::build(void* model)
{
	f32 nodeMtx[21 * 12];
	f32 inv[12];
	CombLink* link;
	int i;
	void* actor = mStore.mData;
	gjCreateActorEx((NJS_OBJECT*)model, (u8*)actor, 1);
	mtxIdentity(mMtx);
	njPushMatrixEx();
	njUnitMatrix(0);
	copyNodes((ActorNode*)model, 0, nodeMtx);
	njPopMatrixEx();
	link = mLinks.mData;
	for (i = 4; i <= 8; i++, link++) {
		mtxInverse(inv, nodeMtx + (i - 1) * 12);
		mtxConcat(link->mLocal, inv, nodeMtx + i * 12);
	}
	for (i = 10; i <= 0xE; i++, link++) {
		mtxInverse(inv, nodeMtx + (i - 1) * 12);
		mtxConcat(link->mLocal, inv, nodeMtx + i * 12);
	}
	for (i = 0x10; i <= 0x14; i++, link++) {
		mtxInverse(inv, nodeMtx + (i - 1) * 12);
		mtxConcat(link->mLocal, inv, nodeMtx + i * 12);
	}
	mGroup[0].mLink  = mLinks.mData;
	mGroup[0].mCount = 5;
	mGroup[0].fill(nodeMtx + 4 * 12);
	mGroup[1].mLink  = mLinks.mData + 5;
	mGroup[1].mCount = 5;
	mGroup[1].fill(nodeMtx + 10 * 12);
	mGroup[2].mLink  = mLinks.mData + 10;
	mGroup[2].mCount = 5;
	mGroup[2].fill(nodeMtx + 16 * 12);
	njPushMatrixEx();
	njUnitMatrix(0);
	gjBeginCalcActor((GjActor*)actor);
	skinNodes((ActorNode*)model, 0, mLinks.mData, mHits.mData);
	gjEndCalcActor((GjActor*)actor);
	njPopMatrixEx();
	mBits |= 0x40;
}

// 0x800453F0
void CombGroup::fill(f32* mtx)
{
	Vec3f prev;
	CombLink* links = mLink;
	int count = mCount;
	f32* m;
	CombLink* link;
	for (int i = 0; i < count; i++) {
		m = mtx + i * 12;
		link = &links[i];
		mtxGetPos(m, &link->mPos);
		mtxCopy(link->mMtx, m);
		link->mPrev  = link->mPos;
		link->mForce.x = 0.0f;
		link->mForce.y = 0.0f;
		link->mForce.z = 0.0f;
		if (i != 0) {
			Vec3f gap;
			vecSub(&gap, &link->mPos, &prev);
			link->mLen = njScalor(&gap);
			vecNormalize(&link->mDir, &gap);
			if (i == 1) {
				links[0].mDir = link->mDir;
			}
		} else {
			link->mLen = 0.0f;
		}
		link->mRadius = sCombLinkRadii[i];
		link->mLimit  = sCombLinkLimits[i];
		link->mRest   = 0.5f;
		prev          = link->mPos;
	}
	links[0].mBack = 0;
	for (int i = 1; i < count; i++) {
		links[i].mBack = &links[i - 1];
	}
	for (int i = 0; i < count - 1; i++) {
		links[i].mNext = &links[i + 1];
	}
	links[count - 1].mNext = 0;
}

// 0x800455D0
void TPlyComb::getJoint(int index, f32** out)
{
	if (index >= 4 && index <= 8) {
		*out = mLinks.mData[index - 4].mWorld;
		return;
	}
	if (index >= 10 && index <= 0xE) {
		*out = mLinks.mData[index - 5].mWorld;
		return;
	}
	if (index >= 0x10 && index <= 0x14) {
		*out = mLinks.mData[index - 6].mWorld;
		return;
	}
	if (index == 0) {
		*out = mMtx;
		return;
	}
	*out = 0;
}

// 0x80045674
void TPlyComb::drawModel(void* state)
{
	f32 mtx[12];
	DrawStateSlot* slot;
	njGetMatrix(mtx);
	mtxConcat(mtx, mtx, mMtx);
	slot = getDrawStateSlot();
	njPushMatrix(mtx);
	njSetTexture(mTexture);
	beginDrawState(slot, (const DrawState*)state);
	gjDrawActor((GjActor*)mStore.mData);
	endDrawState(slot);
	njPopMatrixEx();
}

// 0x800456F4
void TPlyComb::setRoot(f32* mtx)
{
	mRoot = mtx;
	mtxCopy(mMtx, mtx);
}

// 0x8004571C
void TPlyComb::draw()
{
	f32 rootInv[12];
	f32 inv[12];
	int i;
	int n;
	GjActor* actor;
	if ((s8)mOn != 0 && mRoot != 0) {
		if (mBits & 0x40) {
			mBits &= ~0x40;
		} else {
			mtxInverse(rootInv, mRoot);
			mtxConcat(rootInv, rootInv, mMtx);
			for (i = 0; i < 3; i++) {
				mGroup[i].drag(rootInv);
			}
		}
	}
	if (mRoot != 0) {
		mtxCopy(mMtx, mRoot);
	}
	if ((s8)mOn != 0) {
		mtxInverse(inv, mMtx);
		for (i = 0; i < 3; i++) {
			mGroup[i].push(inv);
		}
		for (n = 0; n < 3; n++) {
			for (i = 0; i < 3; i++) {
				mGroup[i].limit();
			}
			for (i = 0; i < 2; i++) {
				mGroup[i].separate(&mGroup[i + 1]);
			}
		}
		for (i = 0; i < 3; i++) {
			mGroup[i].solve(mMtx);
		}
		actor = (GjActor*)mStore.mData;
		njPushMatrixEx();
		njUnitMatrix(0);
		gjBeginCalcActor(actor);
		skinNodes(*(ActorNode**)actor, 0, mLinks.mData, mHits.mData);
		gjEndCalcActor(actor);
		njPopMatrixEx();
	} else {
		for (int j = 0; j < mLinks.mCount; j++) {
			mtxConcat(mLinks.mData[j].mWorld, mMtx, mLinks.mData[j].mMtx);
		}
	}
}

// 0x8004590C
static void captureJoint(void*, JointCursor* cursor)
{
	int which = linkIndex(cursor->mIndex);
	if (which >= 0) {
		CombLink* link = (CombLink*)cursor->mComb->mLinks.mData;
		njGetMatrix(link[which].mMtx);
	}
	cursor->mIndex++;
}

// 0x80045968
void TPlyComb::readPose(void* motion, f32 frame)
{
	JointCursor cursor;
	cursor.mIndex = 0;
	cursor.mComb  = this;
	njPushMatrixEx();
	njUnitMatrix(0);
	calcActorMotion((GjActor*)mStore.mData, motion, (NodeCallback)captureJoint, frame, (void*)&cursor);
	njPopMatrixEx();
}
// 0x800459D8
void CombGroup::drag(f32* mtx)
{
	Vec3f moved;
	f32 scale;
	mtxMultVecSR(mtx, &gAxisZ, &moved);
	if (njInnerProduct(&gAxisZ, &moved) < 0.0f) {
		scale = 0.0f;
	} else {
		scale = 0.5f;
	}
	for (int i = 0; i < mCount; i++) {
		mtxMultVec(mtx, &mLink[i].mPos, &moved);
		vecSub(&moved, &moved, &mLink[i].mPos);
		vecScaleAdd(scale, &mLink[i].mPos, &moved, &mLink[i].mPos);
	}
}

// 0x80045AB0
void CombGroup::push(f32* mtx)
{
	Vec3f wind;
	wind.x = 0.0f;
	wind.y = -0.33333334f;
	wind.z = 0.0f;
	mtxMultVecSR(mtx, &wind, &wind);
	for (int i = 1; i < mCount; i++) {
		njAddVector(&mLink[i].mPos, &mLink[i].mForce);
		njAddVector(&mLink[i].mPos, &wind);
	}
}

// 0x80045B40
void CombGroup::solve(f32* root)
{
	for (int i = 1; i < mCount; i++) {
		vecSub(&mLink[i].mForce, &mLink[i].mPos, &mLink[i].mPrev);
		vecScale(0.8f, &mLink[i].mForce, &mLink[i].mForce);
		mLink[i].mPrev = mLink[i].mPos;
	}
	for (int i = 1; i < mCount - 1; i++) {
		f32 basis[12];
		f32 inv[12];
		f32 rot[12];
		Vec3f delta;
		mtxInverse(inv, mLink[i - 1].mMtx);
		vecSub(&delta, &mLink[i + 1].mPos, &mLink[i].mPos);
		mtxMultVecSR(inv, &delta, &delta);
		mtxIdentity(rot);
		mtxRotateY(rot, rot, -(int)(10430.38043493439 * (f32)atan2(delta.z, delta.x)));
		mtxRotateZ(rot, rot, (int)(10430.38043493439 * (f32)atan2(delta.y, delta.x)));
		mtxConcat(basis, mLink[i].mLocal, rot);
		mtxScale(vecDistance(&mLink[i].mPos, &mLink[i + 1].mPos) / mLink[i + 1].mLen, 1.0f, 1.0f, basis, basis);
		mtxConcat(mLink[i].mMtx, mLink[i - 1].mMtx, basis);
		mtxConcat(mLink[i].mWorld, root, mLink[i].mMtx);
	}
	mtxTranslate((float*)mLink[mCount - 1].mMtx, (const float*)mLink[mCount - 2].mMtx, mLink[mCount - 1].mLen, 0.0f, 0.0f);
	mtxConcat(mLink[mCount - 1].mWorld, root, mLink[mCount - 1].mMtx);
}

static inline Vec3f combScaled(f32 s, const Vec3f* v)
{
	Vec3f r;
	gjScaleVector(s, &r, v);
	return r;
}

static inline void combSubScaled(CombGroup* g, int i, f32 s, const Vec3f* v)
{
	Vec3f scaled = combScaled(s, v);
	njSubVector(&g->mLink[i].mPos, &scaled);
}

// 0x80045D98
void CombGroup::limit()
{
	for (int i = 0; i < mCount; i++) {
		Vec3f dir;
		if (mLink[i].mBack != 0) {
			f32 len;
			vecSub(&dir, &mLink[i].mPos, &mLink[i].mBack->mPos);
			len = njUnitVector(&dir);
			if (len < gUnitVectorMin) {
				dir = gVecZero;
			}
			combSubScaled(this, i, 0.8f * (len - mLink[i].mLen), &dir);
		} else {
			dir = mLink[i].mDir;
		}
		if (mLink[i].mNext != 0) {
			Vec3f tmp;
			Vec3f other;
			f32 len;
			vecSub(&other, &mLink[i].mNext->mPos, &mLink[i].mPos);
			len = njUnitVector(&other);
			if (len < gUnitVectorMin) {
				other = gVecZero;
			}
			vecScale(njScalor(&other), &tmp, &dir);
			vecSub(&tmp, &tmp, &other);
			if (njInnerProduct(&dir, &other) < mLink[i].mNext->mRest) {
				vecScale(1.0f, &tmp, &tmp);
			} else {
				vecScale(mLink[i].mLimit, &tmp, &tmp);
			}
			njAddVector(&mLink[i].mNext->mPos, &tmp);
		}
	}
	mLink[0].mPos = mLink[0].mPrev;
}

// 0x80045F84
void CombGroup::separate(CombGroup* other)
{
	for (int i = 0; i < mCount; i++) {
		for (int j = 0; j < other->mCount; j++) {
			Vec3f gap;
			f32 dist;
			f32 reach;
			vecSub(&gap, &mLink[i].mPos, &other->mLink[j].mPos);
			dist = njUnitVector(&gap);
			if (dist < gUnitVectorMin) {
				gap = gVecZero;
			}
			reach = mLink[i].mRadius + other->mLink[j].mRadius;
			if (dist < reach) {
				vecScale(0.5f * (reach - dist), &gap, &gap);
				njAddVector(&mLink[i].mPos, &gap);
				njSubVector(&other->mLink[j].mPos, &gap);
			}
		}
	}
}

// 0x800460AC
static int copyNodes(ActorNode* node, int index, f32* mtx)
{
	do {
		u32 flags = node->mFlags;
		if ((flags & 7) == 7) {
			njGetMatrix(mtx + index * 12);
			index++;
			if ((flags & 0x10) == 0) {
				index = copyNodes(node->mChild, index, mtx);
			}
		} else {
			njPushMatrixEx();
			if ((flags & 1) == 0) {
				njTranslateEx((const Vec3f*)node->mTrans);
			}
			if ((flags & 2) == 0) {
				if (flags & 0x400) {
					njQuaternionEx2(node->mQuat, (const Vec3f*)node->mRot);
				} else {
					njRotateEx((const s32*)node->mRot, (s32)flags & 0x20);
				}
			}
			if ((flags & 4) == 0) {
				njScaleEx((const Vec3f*)node->mScale);
			}
			njGetMatrix(mtx + index * 12);
			index++;
			if ((flags & 0x10) == 0) {
				index = copyNodes(node->mChild, index, mtx);
			}
			njPopMatrixEx();
		}
		node = node->mNext;
	} while (node != 0);
	return index;
}

// 0x800461AC
static int linkIndex(int index)
{
	if (index >= 4 && index <= 8) {
		return index - 4;
	}
	if (index >= 0xa && index <= 0xe) {
		return index - 5;
	}
	if (index >= 0x10 && index <= 0x14) {
		return index - 6;
	}
	return -1;
}

// 0x800461FC
static int skinNodes(ActorNode* node, int index, CombLink* link, Vec3f* hit)
{
	do {
		u32 flags = node->mFlags;
		int which;
		if ((flags & 7) == 7) {
			which = linkIndex(index);
			if (which >= 0) {
				njSetMatrix(0, link[which].mMtx);
			}
			njGetTranslation(0, &hit[index]);
			if ((flags & 8) == 0) {
				gjCalcActor((GjModel*)node->mModel);
			}
			index++;
			if ((flags & 0x10) == 0) {
				index = skinNodes(node->mChild, index, link, hit);
			}
		} else {
			njPushMatrixEx();
			which = linkIndex(index);
			if (which >= 0) {
				njSetMatrix(0, link[which].mMtx);
			} else {
				if ((flags & 1) == 0) {
					njTranslateEx((const Vec3f*)node->mTrans);
				}
				if ((flags & 2) == 0) {
					if (flags & 0x400) {
						njQuaternionEx2(node->mQuat, (const Vec3f*)node->mRot);
					} else {
						njRotateEx((const s32*)node->mRot, (s32)flags & 0x20);
					}
				}
				if ((flags & 4) == 0) {
					njScaleEx((const Vec3f*)node->mScale);
				}
			}
			njGetTranslation(0, &hit[index]);
			if ((flags & 8) == 0) {
				gjCalcActor((GjModel*)node->mModel);
			}
			index++;
			if ((flags & 0x10) == 0) {
				index = skinNodes(node->mChild, index, link, hit);
			}
			njPopMatrixEx();
		}
		node = node->mNext;
	} while (node != 0);
	return index;
}

