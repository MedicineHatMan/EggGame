#include "animal/AniMount.h"
#include "object/ObjAction.h"
#include "enemy/boss/EneBoss.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"
#include "engine/ActorMotion.h"

extern "C" {
}

// What AniBodyHit::hit looks for the first object that says it was hit
struct AniHitSearch {
	TObjAction* mSelf;  // _00 the animal
	TObjAction* mFound; // _04
	int mDamage;        // _08 what it does back
	Vec3f mPos;         // _0C where they touched
};

// Message 0x2A telling an object the animal ran into it
struct AniHitMsg : public ObjMsg {
	u8 _02[2];
	u16 mFrom; // _04
};

// 0x801CC884
AniBodyHit::AniBodyHit()
{
	mColli = NULL;
}

// 0x801CC8C8
AniBodyHit::~AniBodyHit()
{
	if (mColli != NULL) {
		colliFree(mColli);
	}
}

// 0x801CC93C
void AniBodyHit::attach()
{
	if (mColli == NULL) {
		mColli = (CharaColli*)colliCreate(1, 1);
	}
	colliSetOwner(mColli, (u16)mOwnerId);
	colliSetMasks(mColli, 1, 1, 0, 0);
	colliShapeSetSphere(colliGetShape(mColli, 0));
	colliGetShape(mColli, 0)->mSizeX = mRadius;
	colliGetShape(mColli, 0)->mFlags |= 0x1000;
}

// 0x801CC9F0
void AniBodyHit::detach()
{
	if (mColli != NULL) {
		colliFree(mColli);
		mColli = NULL;
	}
}

// 0x801CCA30
extern "C" void aniBodyHitSearch(ColliContact* contact, AniHitSearch* search)
{
	if ((colliContactShape(contact)->mFlags & 8) != 0) {
		TObjAction* obj = findObjById(colliContactOwner(contact));
		if (obj != NULL) {
			HitInfo hit;
			if (obj->onHit(search->mSelf, colliContactTag(contact), (int*)&hit) != 0) {
				search->mPos    = contact->mPos;
				search->mFound  = obj;
				search->mDamage = hit.mDamage;
			}
		}
	}
}

// 0x801CCAE0
void AniBodyHit::hit(AniLife* life)
{
	AniHitMsg msg;
	AniHitSearch search;
	search.mSelf  = findObjById(mOwnerId);
	search.mFound = NULL;
	forEachContact((CharaColli*)mColli, (ContactCallback)aniBodyHitSearch, &search);
	if (search.mFound != NULL) {
		msg.mType = 0x2A;
		msg.mFrom = mOwnerId;
		search.mFound->sendToHandle(&msg);
		life->damage(search.mDamage);
	}
}

// 0x801CCB70
void AniBodyHit::update(AniBody* body)
{
	Vec3f pos;
	pushOut(body);
	getPos(&pos);
	colliShapeSetPos(colliGetShape(mColli, 0), &pos);
}

// 0x801CCBBC
void AniBodyHit::pushOut(AniBody* body)
{
	CharaColli* colli = mColli;
	if (colli->mContactCount != 0) {
		Vec3f push;
		push.x = 0.0f;
		push.y = 0.0f;
		push.z = 0.0f;
		for (ColliContact* contact = colli->mContacts; contact != NULL; contact = contact->mNext) {
			if ((colliContactShape(contact)->mFlags & 0x10) == 0) {
				vecScaleAdd(0.5f, &push, &contact->mPush, &push);
				if (vecLength(&contact->mPush) > 0.0f && vecLenXZ(&contact->mPush) < 0.001f) {
					push.z += 1.0f;
				}
			}
		}
		vecAdd(&body->mPushOut, &push, &body->mPushOut);
	}
}
