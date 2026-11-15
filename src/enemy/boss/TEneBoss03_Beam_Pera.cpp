#include "object/blue/TObjBlueCaptain.h"
#include "engine/DrawTask.h"
#include "camera/EventCamArc.h"

#include "egg/ObjEgg.h"
#include "object/ObjArchive.h"
#include "object/TObjLauncher.h"
#include "stage/TEventCtrl.h"
#include "core/MslMath.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "sound/Sound3DPlay.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "object/TObjKey00.h"
#include "sound/SoundCtrl.h"
#include "enemy/boss/EneBoss.h"
#include "engine/ParticleBank.h"
#include "enemy/ObjEnemy.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/TParticle.h"
#include "stl/math.h"
#include "engine/Colli.h"
#include "camera/TCamEditor.h"
#include "enemy/boss/TEneBoss03_Beam_Pera.h"

static inline f32 beamRandomRange(f32 lo, f32 hi)
{
	return lo + (hi - lo) * (3.0517578125e-05f * (f32)rand());
}

// Unused so the linker strips it
int beamRandomAngle()
{
	return (int)(65536.0f * beamRandomRange(0.0f, 1.0f));
}

// 0x800BD9EC
void initCaptainRing()
{
	CaptainRing desc;
	desc.mA     = 0.0f;
	desc.mB     = 0.0f;
	desc.mX     = 200.0f;
	desc.mY     = 180.0f;
	desc.mZ     = 190.0f;
	desc.mFlag0 = 1;
	desc.mFlag1 = 1;
	desc.mKind  = 0x4C;
	desc.mId    = 0x4B;
	buildCaptainRing(&desc);
}

// 0x800BDA50
// Walks a circle out from radius 12 until it passes the length and stores one point per step
static inline void ringPoint(Vec3f* p, const f32* c, const f32* s, f32 one, f32 zero)
{
	p->x = one * *c;
	p->y = one * *s;
	p->z = zero * *c;
}

void buildCaptainRing(const CaptainRing* desc)
{
	gBeamPeraGround                = (BeamArena*)new CaptainRing;
	int count                      = 0;
	*(CaptainRing*)gBeamPeraGround = *desc;
	CaptainRing* ring              = (CaptainRing*)gBeamPeraGround;

	f32 radius;
	f32 x       = (15.0f + ring->mX) - ring->mX;
	f32 z       = 25.0f + ring->mZ;
	radius      = 12.0f;
	f32 len     = __fres((f32)__frsqrte(x * x + z * z));
	sCaptainLen = len;

	while (radius < len) {
		count++;
		radius += 24.0f * (24.0f / (6.283184f * radius));
	}
	gHitPointCount = count;

	int angle;
	HitVec* pts = new HitVec[count];
	gHitPoints  = pts;

	radius = 12.0f;
	angle  = 0;
	for (int i = 0; i < gHitPointCount; i++) {
		f32 sin;
		f32 cos;
		njSinCos(angle, &sin, &cos);
		sin *= radius;
		cos *= radius;
		ringPoint(pts, &cos, &sin, 1.0f, 0.0f);
		f32 t    = 24.0f / (6.283184f * radius);
		int step = (int)(65536.0f * t);
		radius += 24.0f * t;
		pts++;
		angle += step;
	}
}

// 0x800BDC60
void freeCaptainRing()
{
	if (gHitPoints != 0) {
		delete[] (HitVec*)gHitPoints;
		gHitPoints = 0;
	}
	gHitPointCount = 0;
	sCaptainLen    = 0.0f;
	if (gBeamPeraGround != 0) {
		delete (CaptainRing*)gBeamPeraGround;
		gBeamPeraGround = 0;
	}
}

struct ColliContact;
struct CharaColli;
struct ColliShape;

typedef char BeamSprite_size_check[(sizeof(BeamSprite) == 0x38) ? 1 : -1];

typedef char TEneBoss03_Beam_Pera_size_check[(sizeof(TEneBoss03_Beam_Pera) == 0xA8) ? 1 : -1];

// 0x800BDCC4
extern "C" TObject* createTEneBoss03_Beam_Pera(TObject* parent, const Vec3f* pos)
{
	return new TEneBoss03_Beam_Pera(parent, pos);
}

const char* gTEneBoss03_Beam_PeraName = "TEneBoss03_Beam_Pera";

// 0x800BDD18
TEneBoss03_Beam_Pera::TEneBoss03_Beam_Pera(TObject* parent, const Vec3f* pos)
    : TObjAction(parent)
{
	setObjId(this, 0xFFFF);
	_00    = (u32)gTEneBoss03_Beam_PeraName;
	mSize  = 0xA8;
	mPos.x = pos->x;
	mPos.y = pos->y;
	mPos.z = pos->z;
	Vec3f flat;
	flat.x = gBeamPeraGround->mCentreX - pos->x;
	flat.y = 0.0f;
	flat.z = gBeamPeraGround->mCentreZ - pos->z;
	if (njUnitVector(&flat) < gUnitVectorMin) {
		flat = gVecZero;
	}
	mYaw    = (int)(10430.38043493439 * (f32)atan2(flat.x, flat.z));
	mDir.x  = gBeamPeraGround->mCentreX - pos->x;
	mDir.y  = 0.0f;
	mDir.z  = gBeamPeraGround->mCentreZ - pos->z;
	f32 len = njUnitVector(&mDir);
	if (len < gUnitVectorMin) {
		mDir = gVecZero;
	}
	f32 time    = (0.3f * len) / 16.0f;
	mLife       = (f32)(int)((50.0f + (len + gBeamPeraGround->mRadius)) / 16.0f);
	mDir.y      = ((15.0f + gBeamPeraGround->mFloorY) - pos->y) / time;
	mPoints     = new HitVec[gHitPointCount];
	mPointFlags = new u32[gHitPointCount];
	mSprites    = new BeamSprite[gHitPointCount * 3];
	u32 s;
	u32 t;
	if (spriteFindTexture(0x1339EA, (int*)&s, (int*)&t) == 0) {
		spriteFindTexture(0x6071A, (int*)&s, (int*)&t);
	}
	BeamSprite proto;
	spriteInit((Sprite*)&proto);
	proto._24 = -0x60;
	setSpriteMode(&proto.mKey.mField.mFlags, 1, 2, 0);
	proto.mKey.mField.mSlot = s;
	proto.mKey.mField.mTex  = t;
	for (int i = 0; i < gHitPointCount; i++) {
		mPointFlags[i] = 0;
	}
	for (int i = 0; i < gHitPointCount * 3; i++) {
		mSprites[i]        = proto;
		mSprites[i].mFrame = i & 0xF;
	}
	int roll = (int)(65536.0f * (3.0517578125e-05f * (f32)rand()));
	gjPushUnitMatrix();
	gjRotateY(mYaw);
	gjRotateZ(roll);
	njCalcVectors(0, gHitPoints, mPoints, gHitPointCount);
	njPopMatrixEx();
	mColli = colliCreate(1, 1);
	setupCollider(mColli, -0xA, 0);
	mGrow = 0.0f;
	placeSprites();
	for (int i = 0; i < 32; i++) {
		mHitIds[i] = 0xFFFF;
	}
	clearHitList();
}

// 0x800BE164
TEneBoss03_Beam_Pera::~TEneBoss03_Beam_Pera()
{
	colliRetire((CharaColli*)mColli);
	if (mSprites != 0) {
		delete[] mSprites;
		mSprites = 0;
	}
	if (mPointFlags != 0) {
		delete[] mPointFlags;
		mPointFlags = 0;
	}
	if (mPoints != 0) {
		delete[] mPoints;
		mPoints = 0;
	}
	releaseObjId(this);
}

// 0x800BE22C
int TEneBoss03_Beam_Pera::onHit(const TObjAction* other, int arg, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	int took     = 0;
	if (inHitList(other->mObjId) != 0) {
		hit->mFlags.clear();
		hit->mPos.x = 0.0f;
		hit->mPos.y = 0.0f;
		hit->mPos.z = 0.0f;
		if (isPlayerObj(other) != 0) {
			took         = 1;
			hit->mDamage = 0xA;
		} else {
			isEggObj(other);
		}
	}
	return took;
}

// 0x800BE2B0
f32 TEneBoss03_Beam_Pera::getScale()
{
	return 1.6f * (12.0f * mGrow);
}

// 0x800BE2C8
void TEneBoss03_Beam_Pera::placeSprites()
{
	HitVec* point;
	BeamSprite* sprite;
	f32 scale = getScale();
	sprite    = mSprites;
	point     = mPoints;
	for (int i = 0; i < gHitPointCount; i++) {
		gjMulAddVector(mGrow, sprite->pos(), &mPos, point);
		sprite->mScaleX = scale;
		sprite->mScaleY = scale;
		f32 y           = sprite->mY;
		if (y > 150.0f + gBeamPeraGround->mFloorY) {
			mPointFlags[i] &= ~1;
		} else {
			mPointFlags[i] &= ~2;
			if (y > gBeamPeraGround->mCeilingY - 5.0f) {
				mPointFlags[i] &= ~1;
			} else {
				mPointFlags[i] |= 1;
			}
		}
		point++;
		sprite++;
	}
}

// 0x800BE3E0
// Never below 15 over the floor
void TEneBoss03_Beam_Pera::moveOn(Vec3f* pos)
{
	pos->x = mPos.x;
	pos->y = mPos.y;
	pos->z = mPos.z;
	pos->x = 16.0f * mDir.x + pos->x;
	pos->y = pos->y + mDir.y;
	pos->z = 16.0f * mDir.z + pos->z;
	if (pos->y < 15.0f + gBeamPeraGround->mFloorY) {
		pos->y = 15.0f + gBeamPeraGround->mFloorY;
	}
}

// 0x800BE450
void TEneBoss03_Beam_Pera::setupCollider(void* colli, int mask, int group)
{
	ColliShape* shape = (ColliShape*)colliGetShape((CharaColli*)colli, 0);
	if (shape != 0) {
		BeamArena* arena = gBeamPeraGround;
		colliShapeMakeBox(shape, &mPos, mYaw, 25.0f + arena->mRadius, 0.5f * ((150.0f + arena->mFloorY) - (arena->mCeilingY - 5.0f)), 16.0f,
		                  0);
		shape->mTag = group;
	}
	colliSetOwner((CharaColli*)colli, (u16)mObjId);
	colliSetMasks((CharaColli*)colli, 2, 0xFF, 0, mask & 0xFF);
	colliDisable((CharaColli*)colli);
	colliShapeSetBit8((CharaColli*)colli, 0);
	colliShapeSetBit10((CharaColli*)colli, 0);
}

// 0x800BE528
// Whether at in front of the beam is within radius of a point still alive
int TEneBoss03_Beam_Pera::testRing(const Vec3f* at, int mark, f32 radius)
{
	int found = 0;
	f32 y;
	f32 dirX;
	f32 px;
	f32 dirZ;
	f32 pz;
	f32 reach;
	f32 dx;
	f32 dz;
	f32 ahead;
	reach = radius;
	reach += 12.0f;
	dz    = at->z - (pz = mPos.z);
	dirZ  = mDir.z;
	dx    = at->x - (px = mPos.x);
	dirX  = mDir.x;
	y     = at->y;
	ahead = dx * dirX + dz * dirZ;
	if (ahead > 0.0f) {
		f32 x = (px + dx) - dirX * ahead;
		f32 z = (pz + dz) - dirZ * ahead;
		for (int i = 0; i < gHitPointCount; i++) {
			if ((mPointFlags[i] & 7) == 0) {
				const Vec3f* pos = mSprites[i].pos();
				if ((f32)__fabs(pos->y - y) < reach) {
					f32 sx;
					f32 sz;
					sx = pos->x - x;
					sz = pos->z - z;
					if (__fres((f32)__frsqrte(sx * sx + sz * sz)) < reach) {
						found = 1;
						if (mark == 0) {
							break;
						}
						mPointFlags[i] |= 4;
					}
				}
			}
		}
	}
	return found;
}

// 0x800BE620
void TEneBoss03_Beam_Pera::checkHits()
{
	Vec3f at;
	clearHitList();
	for (ColliContact* it = mColli->mContacts; it != 0; it = it->mNext) {
		if (colliContactInGroup8(it) != 0 && colliContactNoBit10(it) != 0) {
			TObjAction* obj = findObjById(colliContactOwner(it));
			if (obj != 0) {
				at.x       = it->mPos.x;
				at.y       = it->mPos.y;
				at.z       = it->mPos.z;
				f32 radius = obj->getRadiusV();
				radius += 5.0f;
				Vec3f test = at;
				if (testRing(&test, 1, radius) != 0) {
					addHit(obj->mObjId);
					if ((int)gBeamPeraGround->mHitFx != 0) {
						f32 s;
						f32 c;
						njSinCos(mYaw, &s, &c);
						at.x -= s * radius;
						at.z -= c * radius;
						particleGenTimedByIdRotDefault((const Vec3f*)&at, (int)((int)gBeamPeraGround->mFxSet),
						                               (int)((int)gBeamPeraGround->mFxFlat), 0, mYaw, 0, 0xA, 0);
						TParticleGenerator* fx = particleGenTimedByIdRotDefault((const Vec3f*)&at, (int)gBeamPeraGround->mFxSet,
						                                                        (int)gBeamPeraGround->mFxRound, 0x4000, mYaw, 0, 5, 0);
						if (fx != 0) {
							fx->makeOwnDesc();
							ParticleDesc* desc = fx->getOwnDesc();
							desc->mSpeed       = (4.0f * radius) / 10.0f;
						}
					}
				}
			}
		}
	}
	for (ColliContact* it = mColli->mContacts; it != 0; it = it->mNext) {
		if (colliContactInGroup1(it) != 0 && colliContactNoBit10(it) != 0) {
			TObjAction* obj = findObjById(colliContactOwner(it));
			if (obj != 0) {
				at.x       = it->mPos.x;
				at.y       = it->mPos.y;
				at.z       = it->mPos.z;
				Vec3f test = at;
				if (testRing(&test, 1, 5.0f) != 0) {
					addHit(obj->mObjId);
				}
			}
		}
	}
}

// The squared length of x z
static inline f32 distSq(f32 x, f32 z)
{
	z *= z;
	return x * x + z;
}

// 0x800BE87C
// Only collides while over the arena
void TEneBoss03_Beam_Pera::updateCollider()
{
	BeamArena* arena = gBeamPeraGround;
	f32 dz           = mPos.z - arena->mCentreZ;
	f32 dx           = mPos.x - arena->mCentreX;
	f32 range        = 25.0f + arena->mRadius;
	if (distSq(dx, dz) > range * range) {
		colliDisable(mColli);
	} else {
		colliEnable(mColli);
		Vec3f at;
		at.x = mPos.x;
		at.y = mPos.y;
		at.z = mPos.z;
		at.y = 0.5f * ((150.0f + gBeamPeraGround->mFloorY) + (gBeamPeraGround->mCeilingY - 5.0f));
		colliShapeSetPos(colliGetShapes(mColli), &at);
		colliRefreshBounds(mColli);
	}
}

// 0x800BE950
void TEneBoss03_Beam_Pera::clearHitList()
{
	mHitCount = 0;
}

// 0x800BE95C
void TEneBoss03_Beam_Pera::addHit(u16 id)
{
	if (mHitCount < 0x20) {
		mHitIds[mHitCount] = id;
		mHitCount++;
	}
}

// 0x800BE984
int TEneBoss03_Beam_Pera::inHitList(u16 id)
{
	for (int i = 0; i < mHitCount; i++) {
		if (mHitIds[i] == id) {
			return 1;
		}
	}
	return 0;
}

// 0x800BE9C4
void TEneBoss03_Beam_Pera::draw()
{
	mLife -= 1.0f;
	if (mLife <= 0.0f) {
		mFlags |= 1;
	}
	checkHits();
	moveOn(&mPos);
	mGrow += 0.06f;
	if (mGrow > 1.0f) {
		mGrow = 1.0f;
	}
	placeSprites();
	updateCollider();
	for (int i = 0; i < gHitPointCount; i++) {
		if ((mPointFlags[i] & 5) == 0) {
			spriteAdd((Sprite*)&mSprites[i]);
			int f = mSprites[i].mFrame;
			f += 3;
			mSprites[i].mFrame = f & 0xF;
		}
	}
	BeamSprite* lead;
	f32 backZ;
	f32 backX;
	BeamSprite* trail = &mSprites[gHitPointCount];
	f32 scale         = getScale();
	for (int layer = 0; layer < 2; layer++) {
		f32 back = 30.0f * (f32)(layer + 1);
		scale *= 0.8f;
		backX = mDir.x * back;
		backZ = mDir.z * back;
		lead  = mSprites;
		for (int i = 0; i < gHitPointCount; i++) {
			if ((mPointFlags[i] & 5) == 0) {
				f32 x = lead->mX;
				f32 z = lead->mZ;
				trail->pos()->set(x - backX, lead->mY, z - backZ);
				trail->mScaleX = scale;
				trail->mScaleY = scale;
				spriteAdd((Sprite*)trail);
				trail->mFrame = (trail->mFrame + 3) & 0xF;
			}
			trail++;
			lead++;
		}
	}
}

// 0x800BEC04
int TEneBoss03_Beam_Pera::getKind()
{
	return 2;
}
