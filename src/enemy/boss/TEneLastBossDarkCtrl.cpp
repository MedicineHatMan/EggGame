#include "enemy/boss/TEneLastBossDarkCtrl.h"
#include "engine/CharaColli.h"
#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "io/ResBank.h"
#include "enemy/boss/TEneLastBoss.h"
#include "enemy/ObjEnemy.h"
#include "Dolphin/OS/OSCache.h"
#include "Dolphin/PPCArch.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "stl/math.h"
#include "sound/SeHolder.h"
#include "object/ObjAction.h"
#include "engine/Colli.h"
#include "engine/DrawTask.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "enemy/EnemyFx.h"
#include "object/last/TObjLastEggLight.h"
#include "enemy/boss/EneBoss.h"
#include "camera/TCamera.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyAppear.h"
#include "enemy/EnemySight.h"
#include "enemy/EnemyLight.h"
#include "engine/MotionPose.h"
#include "enemy/boss/BossParts.h"
#include "egg/ObjEgg.h"
#include "object/ObjCommon.h"
#include "enemy/TEneAm01Parts.h"
#include "engine/FxHolder.h"
#include "egg/EggCreature.h"
#include "enemy/EnemyHit.h"
#include "camera/TCamPath.h"
#include "engine/FxSystem.h"
#include "object/ObjShadow.h"
#include "enemy/EnemyTexFx.h"
#include "core/dolphin.h"
#include "enemy/boss/LastBossRes.h"
#include "enemy/boss/TEneLastBossFeather_Home.h"
#include "object/last/TObjLastBossInd.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "engine/TParticle.h"
#include "enemy/boss/TEneLastBossDemo_Darkness.h"
#include "core/TMainTask.h"
#include "enemy/TEneAm01.h"

const char* gTEneLastBossDarkCtrlName = "TEneLastBossDarkCtrl";

// 0x8015AC30
DarkPixelMap::DarkPixelMap(f32 radius, DarkPixelMap* prev, const Vec3f* pos, int id, int hold)
{
	int side;

	mPrev = prev;
	mNext = 0;
	if (mPrev != 0) {
		mNext        = mPrev->mNext;
		mPrev->mNext = this;
		if (mNext != 0) {
			mNext->mPrev = this;
		}
	}
	mCount    = 0;
	mCursor   = 0;
	mPerFrame = 1;
	mLast     = 0;
	mSeen     = 0;
	mPos      = *pos;
	mRadius   = radius;
	mId       = id;
	mHold     = hold;
	mState    = 0;
	mTimer    = 0;
	radius *= 2.0f;
	side = (int)(f32)floor(radius);
	side *= side;
	if (side > 0) {
		mPixels = new int[side];
	} else {
		mPixels = 0;
	}
}

// 0x8015AD1C
DarkPixelMap::~DarkPixelMap()
{
	if (mPrev != 0) {
		mPrev->mNext = mNext;
	}
	if (mNext != 0) {
		mNext->mPrev = mPrev;
	}
	if (mPixels != 0) {
		delete[] mPixels;
		mPixels = 0;
	}
}

// 0x8015ADA0
int* DarkPixelMap::getPixels()
{
	return mPixels;
}

// 0x8015ADA8
void DarkPixelMap::setCount(int count)
{
	mCount    = count;
	mLast     = count;
	mSeen     = 0;
	mPerFrame = mCount / 0x28;
}

// 0x8015ADCC
void DarkPixelMap::shuffle()
{
	if (mPixels != 0) {
		int count = mCount;

		if (count >= 2) {
			int last;
			int i;

			for (i = 0; i < count; i++) {
				last = mCount - 1;
				int pick = (int)((f32)last * (3.0517578E-5f * (f32)rand()));
				int temp;

				if (pick >= last) {
					pick = last - 1;
				}
				temp          = mPixels[i];
				mPixels[i]    = mPixels[pick];
				mPixels[pick] = temp;
			}
		}
	}
}

// 0x8015AEC4
int DarkPixelMap::getLastCount()
{
	return mLast;
}

// 0x8015AECC
void DarkPixelMap::fill(u8* pixels, int value)
{
	int i;

	if (mCount <= 0 || mPixels == 0) {
		return;
	}
	for (i = 0; i < mCount; i++) {
		pixels[mPixels[i]] = value;
	}
}

// 0x8015AF18
void DarkPixelMap::clear(u8* pixels)
{
	fill(pixels, 0);
	mLast   = 0;
	mSeen   = 0;
	mCursor = 0;
}

// 0x8015AF58
void DarkPixelMap::startFade(u8*)
{
	mState  = 3;
	mTimer  = 0;
	mLast   = mCount;
	mSeen   = 0;
	mCursor = 0;
}

// 0x8015AF7C
int DarkPixelMap::update(u8* pixels)
{
	int finished;
	int count;
	int i;
	int result = 0;

	if (mState == 0) {
		f32 frames = lastBossMagicGrowFrames();

		mTimer++;
		darkCtrlPaint((mRadius * (f32)mTimer) / frames, &mPos, 0xFF);
		result = 1;
		if (mTimer >= (int)frames) {
			mState  = 1;
			mTimer  = 0;
			mSeen   = 0;
			mLast   = mCount;
			mCursor = 0;
		}
	} else {
		if (mLast <= 1000) {
			count    = mCount;
			mCursor  = 0;
			mSeen    = 0;
			finished = 1;
		} else {
			count    = mPerFrame;
			finished = 0;
			if (mCursor + count > mCount) {
				finished = 1;
				count    = mCount - mCursor;
			}
		}
		switch (mState) {
		case 3:
			for (i = 0; i < count; i++) {
				result = 1;
				pixels[mPixels[mCursor]] = 0xFF;
				mSeen++;
				mCursor++;
			}
			break;
		case 1:
		case 2:
			mTimer--;
			for (i = 0; i < count; i++) {
				if (pixels[mPixels[mCursor]] == 0xFF) {
					mSeen++;
				}
				mCursor++;
			}
			break;
		}
		if (finished) {
			switch (mState) {
			case 3:
				mState = 1;
				break;
			case 1:
				if (mSeen < mLast) {
					mTimer = mHold;
					mState = 2;
				}
				break;
			case 2:
				if (mSeen < mLast) {
					mTimer = mHold;
				} else if (mTimer <= 0 && mHold > 0) {
					mState = 3;
				}
				break;
			}
			mLast   = mSeen;
			mSeen   = 0;
			mCursor = 0;
		}
	}
	return result;
}

// 0x8015B1F8
void DarkFlashSlot::reset()
{
	mValue = 0.0f;
	mId    = 0xFFFF;
	mAge   = 0;
}

// 0x8015B218
void DarkFlashSlot::age()
{
	if (mAge == 2) {
		reset();
	} else if (mAge == 1) {
		mAge = 2;
	}
}

// 0x8015B258
int DarkFlashSlot::check(f32 value, u16 id, int* same)
{
	if (mAge == 0) {
		mValue = value;
		mId    = id;
		mAge   = 1;
		*same  = 0;
		return 1;
	}
	if (mId == id) {
		f32 old = mValue;

		mValue = value;
		*same  = (f32)__fabs(old - value) < 0.1f;
		return 1;
	}
	return 0;
}

// 0x8015B2CC
extern "C" void darkFlashesReset(DarkFlashSlot* slots)
{
	int i;

	for (i = 0; i < 8; i++) {
		slots[i].reset();
	}
}

// 0x8015B314
extern "C" void darkFlashesAge(DarkFlashSlot* slots)
{
	
	int free;
	int i;
	
	for (int i = 0; i < 8; i++) {
		slots[i].age();
	}
	free = -1;
	for (int i = 0; i < 8; i++) {
		if (slots[i].mAge == 0) {
			free = i;
			break;
		}
	}
	if (free >= 0) {
		for (i = free + 1; i < 8; i++) {
			if (slots[i].mAge != 0) {
				slots[free].mValue = slots[i].mValue;
				slots[free].mId    = slots[i].mId;
				slots[free].mAge   = slots[i].mAge;
				slots[i].reset();
				free = i;
			}
		}
	}
}

// 0x8015B3F0
extern "C" int darkFlashesCheck(f32 value, DarkFlashSlot* slots, u16 id)
{
	int same = 0;
	int i;

	for (i = 0; i < 8; i++) {
		if (slots[i].check(value, id, &same)) {
			break;
		}
	}
	return same;
}

// 0x8015B464
extern "C" void initLastBossDark()
{
	u32 size;

	resBankLoad(&gLastBossDarkBank, "ar_ene_last_dark.arc");
	resBankAddTextures(&gLastBossDarkBank, 0, "ene_last_dark.gvm");
	size         = GXGetTexBufferSize(0x100, 0x100, 2, GX_FALSE, 0);
	gDarkTexture = new u8[size];
	gDarkTextureSize = size;
	syMemSet4(gDarkTexture, 0, (int)size / 4);
	DCStoreRange(gDarkTexture, size);
	gDarkCtrl = 0;
}

// 0x8015B500
extern "C" void shutdownLastBossDark()
{
	if (gDarkCtrl != 0) {
		gDarkCtrl->mFlags |= 1;
		gDarkCtrl = 0;
	}
	if (gDarkTexture != 0) {
		delete[] gDarkTexture;
		gDarkTexture = 0;
	}
	resBankRemoveTextures(&gLastBossDarkBank, 0);
	resBankRelease(&gLastBossDarkBank);
}

// 0x8015B570
extern "C" void createLastBossDark()
{
	if (gDarkCtrl == 0) {
		gDarkCtrl = new TEneLastBossDarkCtrl(gTL_04);
	}
}

#define DARK_RAND() ((1.0f / 32768.0f) * rand())

static inline f32 darkSqrt(f32 x) { return __fres((f32)__frsqrte(x)); }

// A random value in [base base + range)
static inline f32 darkRandRange(f32 range, f32 base) { return range * DARK_RAND() + base; }

// The texel of row col in the 4 by 8 tiles of an I8 texture 256 wide
#define DARK_ROW(row) ((((row) >> 2) << 10) + (((row) & 3) << 3))
#define DARK_COL(col) ((((col) >> 3) << 5) + ((col) & 7))

typedef char TEneLastBossDarkCtrl_size_check[(sizeof(TEneLastBossDarkCtrl) == 0x2E5C) ? 1 : -1];

// 0x8015B5BC
TEneLastBossDarkCtrl::TEneLastBossDarkCtrl(TObject* parent)
    : TObjAction(parent)
{
	Vec3f origin;
	CharaColli* colli;
	HitVec* rest;
	HitVec* wave;
	f32 z;
	f32 x;
	f32 row;
	f32 col;
	int i;

	_00    = (u32)gTEneLastBossDarkCtrlName;
	mSize  = 0x2E5C;
	setObjId(this, 0xFFFF);
	mDraw       = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mPos.x      = 0.0f;
	mPos.y      = 20.0f;
	mPos.z      = 0.0f;
	mNextId     = 0x1E;
	mDarkFlags  = 0;
	mAlpha      = 0xFF;
	mPhase      = 0.0f;
	mPhaseRate  = 1.0f;
	mHitPos.x   = 0.0f;
	mHitPos.y   = 0.0f;
	mHitPos.z   = 0.0f;
	mHitTimer   = 0;
	seHolderStop(&mSe);
	mFxKind = -1;
	mFxHit  = 0;
	mFxGone = 0;
	mSeId   = -1;
	darkFlashesReset(mFlashes);
	origin.x = 0.0f;
	origin.y = 0.0f;
	origin.z = 0.0f;
	mMaps    = new DarkPixelMap(0.0f, 0, &origin, 0, 0);
	mShape   = (CharaColli*)colliCreate(1, 1);
	colli    = mShape;
	colliSetOwner(colli, (u16)mObjId);
	colliSetMasks(colli, 2, 255, 255, 0x36);
	colliShapeMakeCylinder(colliGetShapes(colli), &mPos, 1, 250.0f, 10.0f);
	colliEnable(colli);
	colliShapeSetBit8(colli, 0);
	colliShapeSetBit10(colli, 0);
	rest = mPoint;
	wave = mPoint2;
	for (row = 0.0f; row <= 19.0f; row += 1.0f) {
		z = 250.0f - (500.0f * row) / 19.0f;
		for (col = 0.0f; col <= 19.0f; col += 1.0f) {
			x = 250.0f - (500.0f * col) / 19.0f;
			rest->x = x;
			rest->y = 0.0f;
			rest->z = z;
			rest++;
			wave->x = x;
			wave->y = 0.0f;
			wave->z = z;
			wave++;
		}
	}
	DCStoreRangeNoSync(mPoint, sizeof(mPoint));
	DCStoreRangeNoSync(mPoint2, sizeof(mPoint2));
	PPCSync();
	for (i = 0; i < 400; i++) {
		mLive[i] = 0;
		mVel[i]  = 0.0f;
	}
	mRippleTimer = 0;
	mTexAngleA   = 0;
	mTexAngleB   = 0;
	mScaleX      = 3.0f;
	mScaleZ      = 3.0f;
	mPulseX      = 0;
	mPulseZ      = 0;
}

// 0x8015B890
TEneLastBossDarkCtrl::~TEneLastBossDarkCtrl()
{
	seHolderStop(&mSe);
	colliRetire(mShape);
	mShape = 0;
	deleteMaps();
	if (mMaps != 0) {
		delete mMaps;
		mMaps = 0;
	}
	removeDrawTask(mDraw);
	if (gDarkCtrl == this) {
		gDarkCtrl = 0;
	}
	releaseObjId(this);
}

// 0x8015B95C
void darkCtrlSetFx(int kind, int hit, int gone)
{
	if (gDarkCtrl != 0) {
		gDarkCtrl->mFxKind = kind;
		gDarkCtrl->mFxHit  = hit;
		gDarkCtrl->mFxGone = gone;
	}
}

// 0x8015B980
void darkCtrlSetSe(int id)
{
	if (gDarkCtrl != 0) {
		gDarkCtrl->mSeId = id;
	}
}

// 0x8015B994
void darkCtrlSetPos(const Vec3f* pos)
{
	if (gDarkCtrl != 0) {
		gDarkCtrl->setPos(pos);
	}
}

// 0x8015B9C8
void TEneLastBossDarkCtrl::setPos(const Vec3f* pos)
{
	ColliShape* shape;

	mPos.x   = pos->x;
	mPos.y   = pos->y;
	mPos.z   = pos->z;
	shape    = colliGetShapes(mShape);
	shape->mPos.x = pos->x;
	shape->mPos.y = pos->y;
	shape->mPos.z = pos->z;
	colliRefreshBounds(mShape);
}

// 0x8015BA34
void TEneLastBossDarkCtrl::deleteMaps()
{
	while (mMaps->mNext != 0) {
		delete mMaps->mNext;
	}
}

// 0x8015BA78
void darkCtrlClearAll()
{
	if (gDarkCtrl != 0) {
		gDarkCtrl->clearAll();
	}
}

// 0x8015BAA4
// The store takes a quarter of the size as the fill does
void TEneLastBossDarkCtrl::clearAll()
{
	deleteMaps();
	syMemSet4(gDarkTexture, 0, (int)gDarkTextureSize / 4);
	DCStoreRange(gDarkTexture, (int)gDarkTextureSize / 4);
}

// 0x8015BAF0
void TEneLastBossDarkCtrl::show()
{
	mDarkFlags &= ~1;
	mAlpha = 0xFF;
}

// 0x8015BB08
void TEneLastBossDarkCtrl::hide()
{
	mDarkFlags |= 1;
	mAlpha = 0;
}

// 0x8015BB20
void TEneLastBossDarkCtrl::fadeOut() { mDarkFlags |= 8; }

// 0x8015BB30
void darkCtrlFadeOut()
{
	if (gDarkCtrl != 0) {
		gDarkCtrl->fadeOut();
	}
}

// 0x8015BB5C
void TEneLastBossDarkCtrl::setNoContact() { mDarkFlags |= 4; }

// 0x8015BB6C
void darkCtrlSetNoContact()
{
	if (gDarkCtrl != 0) {
		gDarkCtrl->setNoContact();
	}
}

// 0x8015BB98
void TEneLastBossDarkCtrl::clearNoContact() { mDarkFlags &= ~4; }

// 0x8015BBA8
void darkCtrlClearNoContact()
{
	if (gDarkCtrl != 0) {
		gDarkCtrl->clearNoContact();
	}
}

// 0x8015BBD4
void darkCtrlSetPhaseRate(f32 rate)
{
	if (gDarkCtrl != 0) {
		gDarkCtrl->mPhaseRate = rate;
	}
}

// 0x8015BBE8
// A world point to texels
void TEneLastBossDarkCtrl::toTexel(f32 radius, const Vec3f* pos, f32* x, f32* z, f32* r)
{
	f32 dx = pos->x - mPos.x;
	f32 dz = pos->z - mPos.z;

	*r = 0.512f;
	dx -= 250.0f;
	*x = dx * -*r;
	dz -= 250.0f;
	*z = dz * -*r;
	*r = *r * radius;
}

// 0x8015BC44
// The texels a disc covers and whether any are on the texture
int TEneLastBossDarkCtrl::getBounds(f32 x, f32 z, f32 r, f32* x0, f32* x1, f32* z0, f32* z1)
{
	int below;

	*x0 = ceil(x - r);
	*x1 = floor(x + r);
	*z0 = ceil(z - r);
	*z1 = floor(z + r);
	if (*x0 > 255.0f) {
		return 0;
	}
	if (*x1 < 0.0f) {
		return 0;
	}
	if (*z0 > 255.0f) {
		return 0;
	}
	below = *z1 < 0.0f;
	return !below ? 1 : 0;
}

// 0x8015BD50
int darkCtrlAddMap(f32 radius, const Vec3f* pos, int hold)
{
	if (gDarkCtrl != 0) {
		return gDarkCtrl->addMap(radius, pos, hold);
	}
	return 0;
}

// 0x8015BD94
// A new patch for the disc its pixels in a shuffled order
int TEneLastBossDarkCtrl::addMap(f32 radius, const Vec3f* pos, int hold)
{
	f32 x;
	f32 z;
	f32 r;
	f32 x0;
	f32 x1;
	f32 z0;
	f32 z1;
	DarkPixelMap* map;
	int* pixels;
	int row;
	int count;
	int i;
	int col;
	int base;
	int last;
	f32 r2;
	f32 rz;
	f32 half;

	toTexel(radius, pos, &x, &z, &r);
	if (!getBounds(x, z, r, &x0, &x1, &z0, &z1)) {
		return 0;
	}
	mDarkFlags |= 2;
	if (z0 < 0.0f) {
		z0 = 0.0f;
	}
	if (z1 > 255.0f) {
		z1 = 255.0f;
	}
	map    = new DarkPixelMap(radius, mMaps, pos, mNextId, hold);
	count  = 0;
	i      = 0;
	pixels = map->getPixels();
	rz     = z0;
	r2     = r * r;
	for (row = (int)z0; rz <= z1; rz += 1.0f, row++) {
		base = (row >> 2) << 10;
		base += (row & 3) << 3;
		half = darkSqrt(r2 - (rz - z) * (rz - z));
		x0   = ceil(x - half);
		if (x0 < 0.0f) {
			x0 = 0.0f;
		}
		x1 = floor(x + half);
		if (x1 > 255.0f) {
			x1 = 255.0f;
		}
		last = x1;
		for (col = x0; col <= last; col++) {
			count++;
			int at = base;
			at += (col >> 3) << 5;
			at += col & 7;
			pixels[i++] = at;
		}
	}
	map->setCount(count);
	map->shuffle();
	return mNextId++;
}

// 0x8015C00C
int darkCtrlPaint(f32 radius, const Vec3f* pos, u8 value)
{
	if (gDarkCtrl != 0) {
		return gDarkCtrl->paint(radius, pos, value);
	}
	return 0;
}

// 0x8015C050
// Sets the disc's pixels to value
int TEneLastBossDarkCtrl::paint(f32 radius, const Vec3f* pos, u8 value)
{
	f32 x;
	f32 z;
	f32 r;
	f32 x0;
	f32 x1;
	f32 z0;
	f32 z1;
	int count = 0;
	int row;
	int col;
	int base;
	int last;
	f32 r2;
	f32 rz;
	f32 half;

	toTexel(radius, pos, &x, &z, &r);
	if (!getBounds(x, z, r, &x0, &x1, &z0, &z1)) {
		return 0;
	}
	if (z0 < 0.0f) {
		z0 = 0.0f;
	}
	if (z1 > 255.0f) {
		z1 = 255.0f;
	}
	rz = z0;
	r2 = r * r;
	for (row = (int)z0; rz <= z1; rz += 1.0f, row++) {
		base = (row >> 2) << 10;
		base += (row & 3) << 3;
		half = darkSqrt(r2 - (rz - z) * (rz - z));
		x0   = ceil(x - half);
		if (x0 < 0.0f) {
			x0 = 0.0f;
		}
		x1 = floor(x + half);
		if (x1 > 255.0f) {
			x1 = 255.0f;
		}
		last = x1;
		for (col = x0; col <= last; col++) {
			int at = base;
			at += (col >> 3) << 5;
			at += col & 7;
			u8 old = gDarkTexture[at];
			if (old != value) {
				gDarkTexture[at] = value;
				count++;
			}
		}
	}
	return count;
}

// 0x8015C26C
void darkCtrlClearMap(int id)
{
	if (gDarkCtrl != 0) {
		gDarkCtrl->clearMap(id);
	}
}

// 0x8015C2A0
void TEneLastBossDarkCtrl::clearMap(int id)
{
	DarkPixelMap* map;

	for (map = mMaps->mNext; map != 0; map = map->mNext) {
		if (map->mId == id) {
			map->clear(gDarkTexture);
			return;
		}
	}
}

// 0x8015C2EC
void darkCtrlFadeMaps()
{
	if (gDarkCtrl != 0) {
		gDarkCtrl->fadeMaps();
	}
}

// 0x8015C318
void TEneLastBossDarkCtrl::fadeMaps()
{
	DarkPixelMap* map;

	for (map = mMaps->mNext; map != 0; map = map->mNext) {
		map->startFade(gDarkTexture);
	}
	mDarkFlags |= 2;
}

// 0x8015C370
int darkCtrlCountMaps()
{
	if (gDarkCtrl != 0) {
		return gDarkCtrl->countMaps();
	}
	return 0;
}

// 0x8015C3A4
int TEneLastBossDarkCtrl::countMaps()
{
	DarkPixelMap* map;
	int count = 0;

	for (map = mMaps->mNext; map != 0; map = map->mNext) {
		count++;
	}
	return count;
}

// 0x8015C3C8
int darkCtrlGetMap(int index, Vec3f* pos, f32* radius)
{
	if (gDarkCtrl != 0) {
		return gDarkCtrl->getMap(index, pos, radius);
	}
	return 0;
}

// 0x8015C414
int TEneLastBossDarkCtrl::getMap(int index, Vec3f* pos, f32* radius)
{
	DarkPixelMap* map;
	int i = 0;

	for (map = mMaps->mNext; map != 0; map = map->mNext, i++) {
		if (i == index) {
			pos->x  = map->mPos.x;
			pos->y  = map->mPos.y;
			pos->z  = map->mPos.z;
			*radius = map->mRadius;
			return 1;
		}
	}
	return 0;
}

// 0x8015C46C
int darkCtrlHasMap(int id)
{
	if (gDarkCtrl != 0) {
		return gDarkCtrl->hasMap(id);
	}
	return 0;
}

// 0x8015C4A8
int TEneLastBossDarkCtrl::hasMap(int id)
{
	DarkPixelMap* map;

	for (map = mMaps->mNext; map != 0; map = map->mNext) {
		if (map->mId == id) {
			return 1;
		}
	}
	return 0;
}

// 0x8015C4DC
// Whether obj stands on a fully dark pixel
int TEneLastBossDarkCtrl::isInDark(const TObjAction* obj)
{
	Vec3f pos;
	f32 r;
	f32 x;
	f32 z;
	Vec3f* at;
	int col;
	int row;

	at    = ((TObjAction*)obj)->getPos();
	pos.x = at->x;
	pos.y = at->y;
	pos.z = at->z;
	if (!darkFlashesCheck(pos.y, mFlashes, obj->mObjId)) {
		return 0;
	}
	toTexel(5.0f, &pos, &x, &z, &r);
	if (x <= 0.0f || z <= 0.0f || x >= 255.0f || z >= 255.0f) {
		return 0;
	}
	col = (f32)ceil(x);
	row = (f32)ceil(z);
	int i = ((row >> 2) << 10) + ((col >> 3) << 5);
	i += (row & 3) << 3;
	i += col & 7;
	return gDarkTexture[i] == 0xFF;
}

// 0x8015C618
int TEneLastBossDarkCtrl::onHit(const TObjAction* other, int arg, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	int ret = 0;

	if (isPlayerOrKind4(other)) {
		if (isInDark(other)) {
			hit->mDamage = 10;
			ret = 1;
			hit->mFlags.clear();
		}
	} else if (isEggObj(other)) {
		if (isInDark(other)) {
			hit->mDamage = 1;
			ret = 1;
			hit->mFlags.clear();
			hit->mFlags.setBit(0xD);
		}
	}
	return ret;
}

// 0x8015C6D0
// Hits what touches the darkness and clears the dark round each one it hurt
int TEneLastBossDarkCtrl::touchContacts()
{
	int touched = 0;
	ColliContact* contact;
	TObjAction* obj;

	if (mDarkFlags & 4) {
		return 0;
	}
	for (contact = mShape->mContacts; contact != 0; contact = contact->mNext) {
		if (colliContactHasBit8(contact)) {
			HitInfo hit;
			obj = findObjById(colliContactOwner(contact));
			if (obj != 0 && obj->onHit(this, colliContactTag(contact), (int*)&hit) && hit.mDamage > 0
			    && paint(10.0f + obj->getRadiusV(), &contact->mPos, 0) > 0) {
				Vec3f* at = obj->getPos();
				touched   = 1;
				mHitPos.x = at->x;
				mHitPos.y = at->y;
				mHitPos.z = at->z;
				mHitPos.y = mPos.y;
			}
		}
	}
	return touched;
}

// 0x8015C804
// A scale that rests at 5 and now and then dips on a timer of 292 to 892 frames
void darkPulse(f32* scale, int* timer)
{
	(*timer)--;
	if (*timer < 0) {
		*timer = (int)(600.0f * DARK_RAND()) + 0x124;
	}
	*scale = 5.0f;
	if (*timer < 0x70) {
		*scale += 4.5f * (0.5f * (njCos(((0x70 - *timer) << 16) / 0x70) - 1.0f));
	}
}

// 0x8015C8D4
void TEneLastBossDarkCtrl::draw()
{
	DarkPixelMap* map;
	DarkPixelMap* done;
	u8* live;
	HitVec* point;
	f32* vel;
	int changed = 0;
	int i;
	f32 rest = 0.0f;

	if (mDarkFlags & 8) {
		mAlpha -= 4;
		if (mAlpha <= 0) {
			mAlpha = 0;
			mDarkFlags &= ~8;
			clearAll();
		}
	}
	if (mDarkFlags & 1) {
		if (darkCtrlCountMaps() > 0) {
			show();
		}
		if (!(mDarkFlags & 0x10)) {
			mDarkFlags |= 0x10;
			colliDisable(mShape);
			seHolderStop(&mSe);
		}
		return;
	}
	if (darkCtrlCountMaps() <= 0) {
		hide();
	}
	if (mDarkFlags & 0x10) {
		mDarkFlags &= ~0x10;
		colliEnable(mShape);
		darkFlashesReset(mFlashes);
	}
	mPhase += 0.01f * mPhaseRate;
	if (mPhase >= 1.0f) {
		mPhase -= 1.0f;
	}
	darkFlashesAge(mFlashes);
	mHitTimer--;
	if (touchContacts()) {
		changed |= 1;
		if (mHitTimer <= 0) {
			mHitTimer = 6;
			if (mFxKind >= 0) {
				fxSpawnFlat(mFxKind, (void*)mFxHit, (const Vec3f*)&mHitPos);
			}
		}
		if (mSeId >= 0) {
			seHolderPlay(&mSe, mSeId, &mHitPos);
		}
	} else {
		seHolderStop(&mSe);
	}
	for (map = mMaps->mNext; map != 0; map = map->mNext) {
		changed |= map->update(gDarkTexture);
	}
	for (map = mMaps->mNext; map != 0;) {
		done = map;
		map  = map->mNext;
		if (done->getLastCount() <= 0) {
			if (mFxKind >= 0) {
				Vec3f at;
				f32 scale;
				at.x  = done->mPos.x;
				at.y  = done->mPos.y;
				at.z  = done->mPos.z;
				at.y += 2.0f;
				scale = done->mRadius / 100.0f;
				fxSpawnScaled(scale, scale, 1.0f, mFxKind, (void*)mFxGone, (const Vec3f*)&at, -0x4000, 0);
			}
			delete done;
		}
	}
	if (changed || (mDarkFlags & 2)) {
		mDarkFlags &= ~2;
		DCStoreRangeNoSync(gDarkTexture, gDarkTextureSize);
		GXInvalidateTexAll();
	}
	live  = mLive;
	point = mPoint2;
	vel   = mVel;
	for (i = 0; i < 400; i++, live++, point++, vel++) {
		if (*live != 0) {
			f32 spring = -0.001f;
			f64 ad;
			f32 py;
			f32 v;
			f32 d;
			py = point->y;
			d  = py - rest;
			ad = __fabs(d);
			v = *vel;
			v += d * spring;
			v *= 0.995f;
			f32 y = py + v;
			if ((f32)ad <= 0.1f && (f32)__fabs(v) <= 0.1f) {
				*live    = 0;
				*vel     = rest;
				point->y = rest;
			} else {
				*vel     = v;
				point->y = y;
			}
		}
	}
	mRippleTimer--;
	if (mRippleTimer <= 0) {
		mRippleTimer = 2;
		// The flag word is reused as the index of the point to kick
		changed = 400.0f * DARK_RAND();
		if (changed >= 400) {
			changed = 399;
		}
		mLive[changed] = 1;
		mVel[changed]  = 0.5f + darkRandRange(1.0f, 0.0f);
	}
	mTexAngleA -= 0x400;
	mTexAngleB += 0x30;
	darkPulse(&mScaleX, &mPulseX);
	darkPulse(&mScaleZ, &mPulseZ);
	DCStoreRangeNoSync(mPoint2, sizeof(mPoint2));
	PPCSync();
}

// 0x8015CCE4
void TEneLastBossDarkCtrl::vfn_10()
{
	if (mDarkFlags & 1) {
		mFlags |= 0x10;
	} else if (cameraIsVisible(3000.0f, 500.0f, &mPos, 0)) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x8015CD60
// The sheet in 19 strips with the dark texture through two moving texture matrices
void TEneLastBossDarkCtrl::render()
{
	f32 base[3][4];
	f32 out[3][4];
	f32 move[3][4];
	GXTexObj tex;
	f32 sin;
	f32 cos;
	GXColor k;
	f32 t;
	s16 lo;
	s16 hi;
	int row;

	njSetTexture(*resBankGetTextures(&gLastBossDarkBank));
	gjLoadTextureNum(0, 1, 1, 1);
	GXInitTexObj(&tex, gDarkTexture, 256, 256, GX_TF_IA4, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GXLoadTexObj(&tex, GX_TEXMAP0);
	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjTranslate(0.0f, 0.1f, 0.0f);
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	njPopMatrixEx();
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
	GXSetVtxDesc(GX_VA_NRM, GX_INDEX16);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GXSetArray(GX_VA_POS, mPoint, sizeof(HitVec));
	GXSetArray(GX_VA_NRM, mPoint2, sizeof(HitVec));
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);

	PSMTXIdentity(base[0]);
	base[0][0] = -0.002f;
	base[0][1] = 0.0f;
	base[0][2] = 0.0f;
	base[0][3] = 0.5f;
	base[1][0] = 0.0f;
	base[1][1] = 0.0f;
	base[1][2] = -0.002f;
	base[1][3] = 0.5f;
	base[2][0] = 0.0f;
	base[2][1] = 0.0f;
	base[2][2] = 0.0f;
	base[2][3] = 1.0f;
	GXLoadTexMtxImm(base[0], GX_TEXMTX0, GX_MTX2x4);

	njSinCos(mTexAngleB, &sin, &cos);
	sin *= 100.0f;
	cos *= 100.0f;
	mtxIdentity(move[0]);
	mtxRotateY(move[0], move[0], mTexAngleB);
	mtxScale(mScaleX, 5.0f, mScaleZ, move[0], move[0]);
	mtxTranslate((float*)move[0], (const float*)move[0], sin, 0.0f, cos);
	njSinCos(mTexAngleA, &sin, &cos);
	base[0][0] = -0.002f;
	base[0][1] = 0.5f * (-0.002f * sin);
	base[0][2] = 0.0f;
	base[0][3] = 0.5f;
	base[1][0] = 0.0f;
	base[1][1] = 0.5f * (-0.002f * cos);
	base[1][2] = -0.002f;
	base[1][3] = 0.5f;
	base[2][0] = 0.0f;
	base[2][1] = 0.0f;
	base[2][2] = 0.0f;
	base[2][3] = 1.0f;
	PSMTXConcat(base[0], move[0], out[0]);
	GXLoadTexMtxImm(out[0], GX_TEXMTX1, GX_MTX2x4);

	GXSetNumTexGens(2);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2X4, GX_TG_NRM, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
	GXSetNumTevStages(2);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
	if (mPhase < 0.5f) {
		t = 2.0f * mPhase;
	} else {
		t = 2.0f * (1.0f - mPhase);
	}
	k.r = 50.0f * t;
	k.g = 8.0f * t;
	k.b = 20.0f * t;
	k.a = mAlpha;
	GXSetTevKColor(GX_KCOLOR0, k);
	GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K0);
	GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
	GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_KONST);
	GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_KONST, GX_CA_ZERO);
	GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetAlphaCompare(GX_GREATER, mAlpha < 20L ? mAlpha : 20L, GX_AOP_AND, GX_LEQUAL, 0xFF);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
	GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_DISABLE);
	GXSetZCompLoc(GX_FALSE);
	GXSetCullMode(GX_CULL_BACK);
	for (row = 0; row < 19; row++) {
		int col;
		lo = row * 20;
		hi = lo + 20;
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 40);
		for (col = 0; col < 20; col++) {
			GX_FIFO_S16 = hi;
			GX_FIFO_S16 = hi;
			hi++;
			GX_FIFO_U32 = 0xFFFFFFFF;
			GX_FIFO_S16 = lo;
			GX_FIFO_S16 = lo;
			lo++;
			GX_FIFO_U32 = 0xFFFFFFFF;
		}
	}
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetNumIndStages(0);
}

// 0x8015D2A8
int TEneLastBossDarkCtrl::getKind() { return 2; }

// 0x8015D2B0
Vec3f* TEneLastBossDarkCtrl::getPos() { return &mPos; }

// 0x8015D2B8
// Loads the feather resources TEneLastBossFeather's at the end of this unit
void lastBossFeatherInit()
{
	gLastBossFeatherObject = modelTEneLastBoss(1);
	gLastBossFeatherTexList = textureTEneLastBoss();
	gLastBossFeatherMotion = motionTEneLastBoss(3);
}

// 0x8015D2F4
void lastBossFeatherTerm()
{
	gLastBossFeatherObject = 0;
	gLastBossFeatherTexList = 0;
	gLastBossFeatherMotion = 0;
}

// 0x8015D308
u8* lastBossFeatherTable() { return (u8*)gLastBossFeatherNodes; }
