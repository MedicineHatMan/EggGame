#include "animal/AniMount.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"

struct AniZeroVec : public Vec3f {
	inline AniZeroVec()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
};

AniZeroVec sHitOffset; // 0x80286D48

// 0x801CC3EC
AniHitPart::AniHitPart() : mHits(NULL), mKind(0), mOwnerId(0xFFFF), mOffset(sHitOffset), mRadius(10.0f)
{
	mLink.mPart = this;
}

// 0x801CC44C
void AniHitPart::getPos(Vec3f* out) const
{
	AniBody* body   = mHits->mBody;
	AniModel* model = mHits->mModel;
	switch (mKind) {
	case 0:
		*out = body->mPos;
		vecAdd(out, &mOffset, out);
		break;
	case 1: {
		f32 mtx[12];
		model->getMatrix(mNode, mtx);
		mtxGetPos(mtx, out);
		break;
	}
	}
}

// 0x801CC4E4
AniHitList::AniHitList()
{
	mBody  = NULL;
	mModel = NULL;
	mLife  = NULL;
}

// 0x801CC524
void AniHitList::add(AniHitPart* part)
{
	if (part->mLink.mList == NULL) {
		mParts.add(&part->mLink);
		part->mHits = this;
		part->attach();
	}
}

// 0x801CC57C
void AniHitList::addAt(AniHitPart* part)
{
	if (part->mLink.mList == NULL) {
		mParts.add(&part->mLink);
		part->mHits = this;
		part->attach();
		part->update(mBody);
	}
}

// 0x801CC5EC
void AniHitList::remove(AniHitPart* part)
{
	if (part->mLink.mList != NULL) {
		mParts.remove(&part->mLink);
		part->detach();
		part->mHits = NULL;
	}
}

// 0x801CC644
void AniHitList::hitAll()
{
	for (AniHitLink* link = mParts.mHead; link != NULL; link = link->mNext) {
		link->mPart->hit(mLife);
	}
}

// 0x801CC698
void AniHitList::updateAll()
{
	for (AniHitLink* link = mParts.mHead; link != NULL; link = link->mNext) {
		link->mPart->update(mBody);
	}
}

// 0x801CC6EC
AniHitLinks::AniHitLinks()
{
	mHead  = NULL;
	mTail  = NULL;
	mCount = 0;
}

// 0x801CC700
AniHitList::~AniHitList()
{
	mParts.clear();
}

// 0x801CC748
void AniHitLinks::add(AniHitLink* link)
{
	if (mHead == NULL) {
		mTail = link;
		mHead = link;
	} else {
		link->mPrev  = mTail;
		mTail->mNext = link;
		mTail        = link;
	}
	link->mList = this;
	mCount++;
}

// 0x801CC788
void AniHitLinks::remove(AniHitLink* link)
{
	if (link->mPrev != NULL) {
		link->mPrev->mNext = link->mNext;
	} else {
		mHead = link->mNext;
	}
	if (link->mNext != NULL) {
		link->mNext->mPrev = link->mPrev;
	} else {
		mTail = link->mPrev;
	}
	link->mNext = NULL;
	link->mPrev = NULL;
	link->mList = NULL;
	mCount--;
}

// 0x801CC7E8
void AniHitLinks::clear()
{
	AniHitLink* link = mHead;
	while (link != NULL) {
		AniHitLink* next = link->mNext;
		link->mList      = NULL;
		link->mNext      = NULL;
		link->mPrev      = NULL;
		link             = next;
	}
}

// 0x801CC814
inline void AniHitPart::attach() { }

// 0x801CC818
inline void AniHitPart::update(AniBody* body) { }

// 0x801CC81C
inline void AniHitPart::detach() { }

// 0x801CC820
inline void AniHitPart::hit(AniLife* life) { }
