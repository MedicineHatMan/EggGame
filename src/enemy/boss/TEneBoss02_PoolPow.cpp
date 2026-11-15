#include "enemy/boss/EneBoss.h"
#include "enemy/boss/TEneBoss02.h"
#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "enemy/boss/TEneBoss02_Pool.h"
#include "Dolphin/OS/OSCache.h"
#include "stage/Stage.h"
#include "engine/Colli.h"
#include "ui/AeArc.h"
#include "PowerPC_EABI_Support/Runtime/runtime.h"
#include "enemy/boss/TEneBoss02_PoolPow.h"
#include "core/TMainTask.h"
struct ColliShape;
struct CharaColli;
struct ColliContact;
static inline f32 rippleSqrt(f32 x)
{
	return __fres((f32)__frsqrte(x));
}

extern "C" {
extern const ColliShape sBoss02_PoolPowShape[] = {
	{ { 0.0f, 614.2f, 0.0f }, 0x0, 0, 0x0, 245.0f, 4.0f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 },
};
}

typedef char TEneBoss02_PoolPow_size_check[(sizeof(TEneBoss02_PoolPow) == 0x40) ? 1 : -1];

/**
 * @size 0xEC
 */
class BossPoolWatcher : public TObjAction {
public:
	BossPoolWatcher(TObject* parent);
	virtual ~BossPoolWatcher();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual int getKind(); // _2C 0x8007A028

	int mCount;         // _30
	u16 mIds[10];       // _34
	HitVec mLast[10];   // _48
	int _C0[10];        // _C0
	CharaColli* mColli; // _E8
};

typedef char BossPoolWatcher_size_check[(sizeof(BossPoolWatcher) == 0xEC) ? 1 : -1];

/**
 * @size 0x30
 */
class TEneBoss02_PoolPowGen_Local : public TObject {
public:
	TEneBoss02_PoolPowGen_Local(TObject* parent, const Vec3f* pos, int time);
	virtual ~TEneBoss02_PoolPowGen_Local();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	Vec3f mPos; // _20
	int mTime;  // _2C
};

typedef char TEneBoss02_PoolPowGen_Local_size_check[(sizeof(TEneBoss02_PoolPowGen_Local) == 0x30) ? 1 : -1];

// 0x80078CA0
extern "C" void createPoolPow(const Vec3f* pos, f32 speed, f32 maxRadius, f32 height)
{
	new TEneBoss02_PoolPow(gTL_05, pos, speed, maxRadius, height);
}

const char* gTEneBoss02_PoolPowName = "TEneBoss02_PoolPow";

// 0x80078D10
TEneBoss02_PoolPow::TEneBoss02_PoolPow(TObject* parent, const Vec3f* pos, f32 speed, f32 maxRadius, f32 height)
    : TObject(parent)
{
	_00        = (u32)gTEneBoss02_PoolPowName;
	mSize      = 0x40;
	mPos       = *pos;
	mPrev      = 0.0f;
	mRadius    = 0.0f;
	mSpeed     = speed;
	mMaxRadius = maxRadius;
	mHeight    = height;
}

// 0x80078DCC
TEneBoss02_PoolPow::~TEneBoss02_PoolPow()
{
}

// 0x80078E28
void TEneBoss02_PoolPow::draw()
{
	mPrev = mRadius;
	mRadius += mSpeed;
	mHeight *= 0.99f;
	if (mRadius > mMaxRadius) {
		mFlags |= 1;
	}
	rippleBossGates(&mPos, mPrev, mRadius, mHeight);
}

// 0x80078EA0
extern "C" void createPoolWatcher()
{
	new BossPoolWatcher(gTL_05);
}

// 0x80078ED8
BossPoolWatcher::BossPoolWatcher(TObject* parent)
    : TObjAction(parent)
{
	setObjId(this, 0xFFFF);
	mCount = 0;
	for (int i = 0; i < 10; i++) {
		mIds[i]    = 0xFFFF;
		_C0[i]     = 0;
		mLast[i].x = 0.0f;
		mLast[i].y = 0.0f;
		mLast[i].z = 0.0f;
	}
	mColli = colliCreate(1, 1);
	colliCopyShapes(mColli, (const ColliShape*)sBoss02_PoolPowShape);
	colliSetOwner(mColli, (u16)mObjId);
	colliSetMasks(mColli, 4, 0xFF, 0xFF, 6);
	colliShapeSetBit10(mColli, 0);
	colliEnable(mColli);
}

// 0x80078FF8
BossPoolWatcher::~BossPoolWatcher()
{
	colliRetire(mColli);
	mColli = 0;
	releaseObjId(this);
}

// 0x80079070
void BossPoolWatcher::draw()
{
	HitVec now[10];
	u16 ids[10];
	int count         = 0;
	ColliContact* hit = mColli->mContacts;
	while (hit != 0) {
		if ((colliContactShape(hit)->mFlags & 0x10) == 0) {
			u16 id    = colliContactOwner(hit);
			int fresh = 1;
			for (int i = 0; i < count; i++) {
				if (id == ids[i]) {
					fresh = 0;
					break;
				}
			}
			if (fresh != 0) {
				TObjAction* obj = findObjById(id);
				if (obj != 0) {
					Vec3f pos;
					pos       = *obj->getPos();
					int isNew = 1;
					int pow   = 1;
					for (int i = 0; i < mCount; i++) {
						if (mIds[i] == id) {
							isNew      = 0;
							ids[count] = id;
							f32 dx     = mLast[i].x - pos.x;
							f32 dz     = mLast[i].z - pos.z;
							if (dx * dx + dz * dz > 64.0f) {
								now[count].x = pos.x;
								now[count].y = pos.y;
								now[count].z = pos.z;
								pow          = 1;
							} else {
								now[count].x = mLast[i].x;
								now[count].y = mLast[i].y;
								now[count].z = mLast[i].z;
								pow          = 0;
							}
							break;
						}
					}
					if (isNew != 0) {
						now[count].x = pos.x;
						now[count].y = pos.y;
						ids[count]   = id;
						now[count].z = pos.z;
						pow          = 1;
					}
					if (pow != 0) {
						createPoolPow(&pos, 4.0f, 25.0f, 5.0f);
					}
					count++;
					if (count >= 10) {
						break;
					}
				}
			}
		}
		hit = (ColliContact*)(*(void**)hit);
	}
	for (int i = 0; i < count; i++) {
		mIds[i]    = ids[i];
		mLast[i].x = now[i].x;
		mLast[i].y = now[i].y;
		mLast[i].z = now[i].z;
	}
	mCount = count;
}

// 0x800792F4
extern "C" void* createPoolPowGen(TObject* parent, const Vec3f* pos, int time)
{
	return new TEneBoss02_PoolPowGen_Local(parent, pos, time);
}

const char* gTEneBoss02_PoolPowGen_LocalName = "TEneBoss02_PoolPowGen_Local";

// 0x80079350
TEneBoss02_PoolPowGen_Local::TEneBoss02_PoolPowGen_Local(TObject* parent, const Vec3f* pos, int time)
    : TObject(parent)
{
	_00   = (u32)gTEneBoss02_PoolPowGen_LocalName;
	mSize = 0x30;
	mPos  = *pos;
	mTime = time;
}

// 0x800793C0
TEneBoss02_PoolPowGen_Local::~TEneBoss02_PoolPowGen_Local()
{
}

// 0x8007941C
void TEneBoss02_PoolPowGen_Local::draw()
{
	mTime--;
	if (mTime <= 0) {
		mFlags |= 1;
	}
	int angle = (int)(65536.0f * (3.0517578125e-05f * (f32)rand()));
	f32 dist  = 55.0f * (3.0517578125e-05f * (f32)rand());
	Vec3f at;
	njSinCos(angle, &at.x, &at.z);
	at.x *= dist;
	at.y = 0.0f;
	at.z *= dist;
	njAddVector(&at, &mPos);
	createPoolPow(&at, 4.0f, 25.0f, 18.0f);
}

HitVec* sRippleGrid; // 0x802C5008 unit directions over the disc 10 by 10

// 0x80079534
// Spreads a 10 by 10 square over the unit disc
extern "C" void bossRippleInit()
{
	f32 u;
	f32 v;
	f32 au;
	f32 av;
	f32 r;
	f32 s;
	HitVec* dir = sRippleGrid = new HitVec[100];
	for (int i = 0; i < 10; i++) {
		u        = 0.22222222f * i - 1.0f;
		f64 absu = __fabs(u);
		au       = absu;
		for (int j = 0; j < 10; j++) {
			v  = 0.22222222f * j - 1.0f;
			av = __fabs(v);
			if (au > av) {
				r = av / au;
			} else {
				r = au / av;
			}
			s      = (f32)__frsqrte(r * r + 1.0f);
			dir->x = u * s;
			dir->y = 0.0f;
			dir->z = v * s;
			dir++;
		}
	}
}

// 0x80079624
extern "C" void bossRippleExit()
{
	if (sRippleGrid != 0) {
		delete[] sRippleGrid;
		sRippleGrid = 0;
	}
}

// 0x80079660
extern "C" BossRipple* createBossRipple()
{
	return newBossRipple();
}

// 0x80079680
extern "C" BossRipple* newBossRipple()
{
	return new BossRipple;
}

// 0x800796B8
BossRippleBase::BossRippleBase()
{
	mRadius = mPos.z = mPos.y = mPos.x = 0.0f;
	mFlags                             = 0;
}

// 0x800796E4
BossRippleBase::~BossRippleBase()
{
}

// 0x8007972C
// A little above the point a random bit higher still so that surfaces don't fight
extern "C" void bossRippleSetPos(BossRippleBase* surface, const Vec3f* pos)
{
	surface->mPos = *pos;
	f32 r         = 2.0f * (3.0517578125e-05f * (f32)rand() - 0.5f);
	surface->mPos.y += 0.01f * r + 0.1f;
	surface->mFlags |= 1;
}

// 0x800797C4
extern "C" void bossRippleSetRadius(BossRippleBase* surface, f32 radius)
{
	surface->mRadius = radius;
	surface->mFlags |= 1;
}

// 0x800797D8
BossRipple::BossRipple()
{
	int i;
	for (i = 0; i < 100; i++) {
		mVerts[i].z = mVerts[i].y = mVerts[i].x = 0.0f;
	}
	for (i = 0; i < 100; i++) {
		mRest[i][1] = mRest[i][0] = 0.0f;
	}
	for (i = 0; i < 100; i++) {
		mVel[i][1] = mVel[i][0] = 0.0f;
	}
	for (i = 0; i < 100; i++) {
		mMoving[i] = 0;
	}
	flush();
}

// 0x800798D8
BossRipple::~BossRipple()
{
}

// 0x80079930
// Pushes the points in the ring between inner and outer away from pos
void BossRipple::ripple(f32 inner, f32 outer, f32 strength, const Vec3f* pos)
{
	f32 pz = pos->z;
	f32 px = pos->x;
	f32 dz = mPos.z - pz;
	f32 dx = mPos.x - px;
	if (rippleSqrt(dx * dx + dz * dz) > mRadius + outer) {
		return;
	}
	f32 inner2 = inner * inner;
	f32 outer2 = outer * outer;
	f32 push   = strength * 0.1f;
	f32 two    = 2.0f;
	f32 half   = 0.5f * (inner * rippleSqrt(two));
	for (int i = 0; i < 100; i++) {
		f32 rx = mRest[i][0];
		if (rx < px - outer || px + outer < rx) {
			continue;
		}
		f32 rz = mRest[i][1];
		if (rz < pz - outer || pz + outer < rz) {
			continue;
		}
		if (px - half <= rx && rx <= px + half && pz - half <= rz && rz <= pz + half) {
			continue;
		}
		f32 oz = rz - pz;
		f32 ox = rx - px;
		f32 d2 = ox * ox + oz * oz;
		if (d2 > outer2 || d2 <= inner2) {
			continue;
		}
		mMoving[i] = 1;
		f32 k      = push * (f32)__frsqrte(d2);
		ox *= k;
		oz *= k;
		mVel[i][0] += ox;
		mVel[i][1] += oz;
	}
}

// 0x80079AC8
void BossRipple::flush()
{
	DCStoreRangeNoSync(mVerts, sizeof(mVerts));
	DCStoreRange(mRest, sizeof(mRest));
}

// 0x80079B08
// Lays the grid over the disc again after it moved or changed size keeping each point's offset
int BossRipple::place()
{
	if ((mFlags & 1) == 0) {
		return 0;
	}
	mFlags &= ~1;
	f32 radius = mRadius;
	gjPushUnitMatrix();
	njTranslateEx(&mPos);
	njScale(0, radius, 1.0f, radius);
	Vec3f p;
	int i;
	for (i = 0; i < 100; i++) {
		njCalcPointEx(&sRippleGrid[i], &p);
		f32 ox      = mVerts[i].x - mRest[i][0];
		f32 oz      = mVerts[i].z - mRest[i][1];
		mRest[i][0] = p.x;
		mRest[i][1] = p.z;
		mVerts[i].x = p.x + ox;
		mVerts[i].z = p.z + oz;
	}
	for (i = 0; i < 100; i++) {
		mVerts[i].y = p.y;
	}
	njPopMatrixEx();
	return 1;
}

// 0x80079C20
static inline f32 square(f32 x)
{
	x *= x;
	return x;
}

int BossRipple::relax()
{
	int moved = 0;
	for (int i = 0; i < 100; i++) {
		if ((u32)mMoving[i] != 0) {
			f32 dx = mVerts[i].x - mRest[i][0];
			f32 dz = mVerts[i].z - mRest[i][1];
			if (dz * dz + dx * dx < 0.010000001f && (f32)square(mVel[i][0]) + square(mVel[i][1]) < 0.0025000002f) {
				mVerts[i].x = mRest[i][0];
				mVerts[i].z = mRest[i][1];
				mVel[i][0]  = 0.0f;
				mVel[i][1]  = 0.0f;
				mMoving[i]  = 0;
			} else {
				moved = 1;
				dx *= -0.05f;
				dz *= -0.05f;
				dx += 0.95f * mVel[i][0];
				dz += 0.95f * mVel[i][1];
				mVerts[i].x += dx;
				mVerts[i].z += dz;
				mVel[i][0] = dx;
				mVel[i][1] = dz;
			}
		}
	}
	return moved;
}

// 0x80079D1C
void BossRipple::update()
{
	int placed = place();
	u8 dirty   = relax() != 0 || placed != 0;
	if (dirty) {
		flush();
	}
}

// 0x80079D7C
// Nine strips of twenty white at the given alpha the texture placed by the rest positions
void BossRipple::draw(f32 alpha)
{
	f32 mtx[12];
	njSetTexture(shapeTEneBoss02()[1]);
	gjLoadTextureNum(0, 1, 1, 0);
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	PSMTXIdentity(mtx);
	mtx[0]  = 0.01f;
	mtx[1]  = 0.0f;
	mtx[2]  = 0.0f;
	mtx[3]  = 0.0f;
	mtx[4]  = 0.0f;
	mtx[5]  = 0.01f;
	mtx[6]  = 0.0f;
	mtx[7]  = 0.0f;
	mtx[8]  = 0.0f;
	mtx[9]  = 0.0f;
	mtx[10] = 0.0f;
	mtx[11] = 1.0f;
	GXLoadTexMtxImm(mtx, GX_TEXMTX0, GX_MTX2x4);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_INDEX16);  // position 16-bit index
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);  // colour direct
	GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16); // texture 0 16-bit index
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GXSetArray(GX_VA_POS, mVerts, 12);
	GXSetArray(GX_VA_TEX0, mRest, 8);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetNumTexGens(1);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	GXSetNumTevStages(1);
	GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_RASA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
	GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0xFF);
	u32 color;
	s16 top    = 0;
	s16 bottom = 10;
	int row    = 0;
	color      = (__cvt_fp2unsigned(255.0f * alpha) & 0xFF) | 0xFFFFFF00;
	for (; row < 9; row++) {
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 20);
		for (int col = 0; col < 10; col++) {
			GX_FIFO_S16 = top;
			GX_FIFO_U32 = color;
			GX_FIFO_S16 = top;
			top++;
			GX_FIFO_S16 = bottom;
			GX_FIFO_U32 = color;
			GX_FIFO_S16 = bottom;
			bottom++;
		}
	}
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetNumIndStages(0);
}

// 0x8007A028
int BossPoolWatcher::getKind()
{
	return 3;
}

/** Theme "blueboss" TEneBoss02's pool */
class TStageBlueBoss : public TStage {
public:
	TStageBlueBoss();
	virtual ~TStageBlueBoss();
	virtual void loadAssets();
	virtual void freeAssets();
	virtual void vfn_14();
};

typedef char TStageBlueBoss_size_check[(sizeof(TStageBlueBoss) == 0x30) ? 1 : -1];

// 0x8007A030
extern "C" TStage* createBlueBossStage()
{
	return new TStageBlueBoss;
}

// 0x8007A068
TStageBlueBoss::TStageBlueBoss()
{
}

// 0x8007A0A4
void TStageBlueBoss::loadAssets()
{
	stageLoadBossConsole(this);
}

// 0x8007A0C4
void TStageBlueBoss::freeAssets()
{
	stageFreeBossConsole();
}

// 0x8007A0E4
void TStageBlueBoss::vfn_14()
{
	stageCamLockAll();
	boss02CreatePoolWatcher();
}

// 0x8007A108
TStageBlueBoss::~TStageBlueBoss()
{
}
