#include "egg/ObjEgg.h"
#include "io/Resource.h"
#include "egg/TEggShockWave.h"
#include "engine/TMobileObject.h"
#include "camera/TCamera.h"
#include "egg/EggLevel.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "animal/AniMount.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "engine/Colli.h"
#include "core/VecMath.h"
#include "core/TMainTask.h"
struct CharaColli;
struct ColliContact;
struct ColliShape;
struct Fx;

// What egglevel.bin says about each type of egg and the small helpers the egg embeds

// What an object's onHit fills in a HitInfo with room after it
struct EggDamage : public HitInfo {
	EggDamage() { }
	u8 _18[8]; // _18
};

TArc* sEggLevelArc;
EggTable* sEggLevelTable;

extern "C" void loadEggLevels()
{
	loadArc("egglevel.bin", &sEggLevelArc);
	if (sEggLevelArc != 0) {
		sEggLevelTable = (EggTable*)sEggLevelArc->mData;
	}
}

extern "C" void unloadEggLevels()
{
	releaseResource(sEggLevelArc);
	sEggLevelTable = 0;
	sEggLevelArc = 0;
}

int EggLevel::pickRandomType(f32 random)
{
	if (sEggLevelTable != 0) {
		return sEggLevelTable->mRandom[(int)(512.0f * random)];
	}
	return 0;
}

EggLevel::EggLevel()
{
	mType = 0;
}

// 0x800C48A8
extern "C" EggType* findEggType(EggTable* table, int id)
{
	int offset = 0;
	for (u32 i = 0; i < table->mTypeCount; i++) {
		EggType* type = (EggType*)((u8*)table->mTypes + offset);
		if (id == type->mId) {
			return type;
		}
		offset += sizeof(EggType);
	}
	return 0;
}

void EggLevel::setType(int id)
{
	EggType* type;
	if (sEggLevelTable == 0) {
		type = 0;
	} else {
		type = findEggType(sEggLevelTable, id);
		if (type == 0) {
			type = findEggType(sEggLevelTable, 0);
		}
	}
	if (type != 0) {
		mSize = type->mMinSize;
	} else {
		mSize = 0;
	}
	mPoints = 0;
	mType   = type;
}

int EggLevel::getType()
{
	if (mType != 0) {
		return mType->mId;
	}
	return 0;
}

int EggLevel::getMaxSize()
{
	if (mType != 0) {
		return mType->mMaxSize;
	}
	return 1;
}

int EggLevel::getMinSize()
{
	if (mType != 0) {
		return mType->mMinSize;
	}
	return 1;
}

// 0x800C49BC
f32 EggLevel::getRadius() const
{
	if (mType != 0) {
		return 0.05f * (f32)sEggLevelTable->mSizes[mSize - 1].mRadius;
	}
	return 10.0f;
}

int EggLevel::getPower()
{
	return sEggLevelTable->mSizes[mSize - 1].mPower;
}

int EggLevel::sizeForPoints(int points)
{
	EggType* type = mType;
	if (type != 0) {
		int size = type->mMinSize;
		for (int i = size; i <= type->mMaxSize; i++) {
			u32 need = type->mMask[i];
			if (need != 0 && (int)need <= points) {
				size = i;
			}
		}
		return size;
	}
	return 1;
}

int EggLevel::feed(int food, int amount)
{
	EggType* type = mType;
	if (type != 0) {
		if (type->mMask[0] & (1 << food)) {
			amount <<= 1;
		}
		int points = mPoints + amount;
		if (points > type->mMaxPoints) {
			points = type->mMaxPoints;
		}
		if (points != mPoints) {
			mPoints = points;
			mSize   = sizeForPoints(points);
		}
	}
	return mPoints;
}

void EggLevel::setPoints(int points)
{
	if (mType != 0 && points >= 0 && points <= mType->mMaxPoints && points != mPoints) {
		mPoints = points;
		mSize   = sizeForPoints(points);
	}
}

EggGrow::EggGrow()
{
	mTimer = 0;
}

int EggGrow::isActive()
{
	return mTimer;
}

void EggGrow::start(f32 from, f32 to)
{
	mTimer = 60;
	mFrom  = from;
	mTo    = to;
}

void EggGrow::tick()
{
	if (mTimer != 0) {
		mTimer--;
	}
}

f32 EggGrow::getTarget() const
{
	return mTo;
}

// 0x800C4BC8
void EggGrow::getScale(void* mtx) const
{
	// Both sides read mTo so this never flickers
	f32 scale;
	if (mTimer & 8) {
		scale = getTarget();
	} else {
		scale = mTo;
	}
	mtxSetScale((float*)mtx, scale, scale, scale);
}

EggGlow::EggGlow()
{
	mFx    = 0;
	mTimer = 0;
}

EggGlow::~EggGlow()
{
	if (mFx != 0) {
		fxStop((Fx*)mFx);
	}
}

void EggGlow::stop()
{
	if (mFx != 0) {
		fxStop(mFx);
		mFx = 0;
	}
}

void EggGlow::startGrow(void* mtx)
{
	stop();
	mFx    = fxSpawnFollow(1, 7, 0x2D, (f32*)mtx);
	mTimer = 12;
}

void EggGlow::startFull(void* mtx)
{
	stop();
	mFx    = fxSpawnFollow(1, 7, 0x31, (f32*)mtx);
	mTimer = 0;
}

void EggGlow::update()
{
	if (mTimer != 0 && --mTimer == 0) {
		fxStop(mFx);
		mFx = 0;
	}
}

EggTilt::EggTilt()
{
	mState = -1;
	setState(1);
}

void EggTilt::wobble()
{
	setState(0);
}

void EggTilt::update()
{
	runState(mState, 2);
	runState(mState, 3);
}

void EggTilt::setState(int state)
{
	if (mState != -1) {
		runState(mState, 1);
	}
	if (state != -1) {
		runState(state, 0);
	}
	mState = state;
}

void EggTilt::runState(int state, int phase)
{
	switch (state) {
	case 0:
		stateWobble(phase);
		break;
	}
}

// 0x800C4EEC
void EggTilt::stateWobble(int phase)
{
	if (phase == 0) {
		f32 two = 2.0f;
		mY      = 0.7f;
		f32 zero = 0.0f;
		mXZ     = two - mY;
		mVel    = zero;
		mTimer  = 60;
	} else if (phase == 2) {
		if (mTimer != 0 && --mTimer == 0) {
			setState(1);
		}
	} else if (phase == 3) {
		f32 two = 2.0f;
		mVel += 0.08f * -mVel - 0.08f * (mY - 1.0f);
		mY += mVel;
		mXZ = two - mY;
	}
}

// 0x800C4FBC
int EggTilt::getScale(void* mtx)
{
	if (mState == 0) {
		mtxSetScale((float*)mtx, mXZ, mY, mXZ);
		return 1;
	}
	return 0;
}

// 0x800C5008
extern "C" int eggAnyTouch(void* coll, EggContact* it, EggContact* end)
{
	EggContact* cur;
	for (cur = it; cur != end; cur = cur->mNext) {
		if ((*(u32*)((u8*)colliContactShape((ColliContact*)cur) + 0x24) & 0x40) != 0 && (*(u32*)((u8*)colliGetShape((CharaColli*)coll, cur->mShape) + 0x24) & 0x400) != 0) {
			return 1;
		}
	}
	return 0;
}

extern "C" int eggFindHatchTouch(EggColl* coll, u16* who)
{
	EggContact* it = coll->mContacts;
	for (; it != 0; it = it->mNext) {
		if ((*(u32*)((u8*)colliContactShape((ColliContact*)it) + 0x24) & 0x40) != 0 && (*(u32*)((u8*)colliGetShape((CharaColli*)coll, it->mShape) + 0x24) & 0x400) != 0) {
			if (who != 0) {
				*who = colliContactOwner((ColliContact*)it);
			}
			return 1;
		}
	}
	return 0;
}

// 0x800C510C
extern "C" int eggFirstDamage(void* coll, EggContact* first, EggContact* end, void* me)
{
	EggDamage damage;

	for (EggContact* it = first; it != end; it = it->mNext) {
		if ((*(u32*)((u8*)colliContactShape((ColliContact*)it) + 0x24) & 8) != 0 && (*(u32*)((u8*)colliGetShape((CharaColli*)coll, it->mShape) + 0x24) & 0x400) != 0) {
			TObjAction* other = findObjById(colliContactOwner((ColliContact*)it));
			if (other != 0) {
				if (((TObjEgg*)other)->onHit((TObjEgg*)me, colliContactTag((ColliContact*)it), (int*)&damage) != 0 && damage.mFlags.testBit(0xD) != 0) {
					return damage.mDamage;
				}
			}
		}
	}
	return 0;
}

// 0x800C51F4
extern "C" int eggFindHit(EggColl* coll, TObjEgg* me, Vec3f* away)
{
	Vec3f dir;
	EggDamage damage;

	for (EggContact* it = coll->mContacts; it != 0; it = it->mNext) {
		if ((*(u32*)((u8*)colliContactShape((ColliContact*)it) + 0x24) & 8) != 0 && (*(u32*)((u8*)colliGetShape((CharaColli*)coll, it->mShape) + 0x24) & 0x400) != 0) {
			TObjAction* other = findObjById(colliContactOwner((ColliContact*)it));
			if (other != 0) {
				if (((TObjEgg*)other)->onHit(me, colliContactTag((ColliContact*)it), (int*)&damage) != 0 && damage.mFlags.testBit(0xD) != 0) {
					if (away != 0) {
						dir = *me->getPos();
						vecSub(&dir, &dir, &it->mPos);
						vecFromAngleXZ(70.0f, &dir, vecAngleXZ(&dir));
						f32 up  = 150.0f;
						away->x = dir.x;
						away->y = up;
						away->z = dir.z;
					}
					return damage.mDamage;
				}
			}
		}
	}
	return 0;
}

// 0x800C534C
extern "C" void eggPushFromContacts(f32 radius, void* coll, EggContact* first, EggContact* end, Vec3f* pos, Vec3f* vel, Vec3f* out)
{
	Vec3f dir;

	f32 zero = 0.0f;
	out->x   = zero;
	out->y   = zero;
	out->z   = zero;
	for (EggContact* it = first; it != end; it = it->mNext) {
		u32 mine = *(u32*)((u8*)colliContactShape((ColliContact*)it) + 0x24);
		if ((*(u32*)((u8*)colliGetShape((CharaColli*)coll, it->mShape) + 0x24) & 0x400) != 0 && (*(u32*)((u8*)colliContactShape((ColliContact*)it) + 0x24) & 0x10) == 0) {
			if (mine & 0x400) {
				vecSub(&dir, pos, &it->mPos);
				f32 dist = vecNormalize(&dir, &dir);
				ColliShape* other = (ColliShape*)(colliContactShape((ColliContact*)it));
				f32 push = radius + other->mSizeX - dist;
				push *= 360.0f;
				vecScaleAdd(push, out, &dir, out);
			} else if ((mine & 0x10) == 0 && (mine & 0x800) == 0) {
				f32 depth = vecNormalize(&dir, &it->mNormal);
				if (njInnerProduct(vel, &dir) < 0.0f) {
					f32 push = depth;
					push *= 1800.0f;
					vecScaleAdd(push, out, &dir, out);
				}
				if (depth > 0.0f && vecLenXZ(&it->mNormal) < 0.01f) {
					out->z += 180.0f;
				}
			}
		}
	}
}

// 0x800C54DC
extern "C" void eggPushHeld(void* coll, EggContact* first, EggContact* end, TMobileObject* holder)
{
	Vec3f pos;
	Vec3f vel;
	Vec3f out;

	pos = holder->mBody.mPos;
	vel = holder->mBody.mVel;
	eggPushFromContacts(holder->mBody.mRadius, coll, first, end, &pos, &vel, &out);
	njAddVector(&holder->mBody.mAccel, &out);
}

// 0x800C5554
extern "C" void eggSpawnShockWave(f32 radius, u16 obj, u16 holder, Vec3f* pos, int power, void* tag)
{
	TEggShockWave* wave = new TEggShockWave(3.0f * radius, gTL_06, obj, tag);
	if (wave != 0) {
		wave->mHolderId = holder;
		((TSetObj*)wave)->setPos(pos);
		wave->mPower = power;
	}
}

void EggLevel::getProgress(void* msg)
{
	f32 progress;
	int minSize = getMinSize();
	int maxSize = getMaxSize();
	if (minSize == maxSize) {
		progress = 1.0f;
	} else {
		progress = (f32)(mSize - minSize) / (f32)(maxSize - minSize);
	}
	((EggLevelMsg*)msg)->mProgress = progress;
}
