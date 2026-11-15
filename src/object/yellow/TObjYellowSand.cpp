#include "stage/SetManager.h"
struct DrawTask;
#include "io/ResFile.h"
#include "io/Resource.h"
#include "Ninja/ninja.h"
#include "engine/ScrollModel.h"
#include "engine/CharaColli.h"
#include "object/ObjAction.h"
#include "egg/ObjEgg.h"
#include "engine/DrawTask.h"
#include "sound/SeHolder.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "io/NinjaFile.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/FrameTex.h"
#include "object/yellow/TObjYellowDoubleFog.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "engine/Colli.h"
#include "egg/EggCreature.h"
#include "engine/GroundQuery.h"
#include "io/ResBank.h"
#include "enemy/EnemyTexFx.h"
#include "core/TMainTask.h"
#include "object/yellow/TObjYellowSand.h"
struct ColliShape;
struct CharaColli;
struct ResBank;
struct Fx;

extern "C" {

// Effects and the looping sound

const char* gTObjYellowSandGenName   = "TObjYellowSandGen";
const char* gTObjYellowSandBoardName = "TObjYellowSandBoard";
const char* gTObjYellowSandMdlName   = "TObjYellowSandMdl";
const char* gTObjYellowSandStormName = "TObjYellowSandStorm";
}

#define GX_FIFO_F32 (*(volatile f32*)0xCC008000)
#define GX_FIFO_U32 (*(volatile u32*)0xCC008000)

static inline f32 randUnit()
{
	return 3.0517578e-05f * (f32)rand();
}

/**
 * The trigger that asks TObjYellowDoubleFog for sand storms. It has a cylinder or box from its record.
 *
 * @size 0xA0
 */
class TObjYellowSandGen : public TSetObj {
public:
	TObjYellowSandGen(TObject* parent, SetDesc* desc);
	virtual ~TObjYellowSandGen();
	virtual void draw();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void* arg);

	void setCollision(f32 x, f32 y, f32 z, int type);

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	u8 _90[0x10]; // _90 unused
};

typedef char TObjYellowSandGen_size_check[(sizeof(TObjYellowSandGen) == 0xA0) ? 1 : -1];

/**
 * A strip of drifting sand on the ground. It fades in, lives 100 frames, then fades out.
 *
 * @size 0x5C
 */
class TObjYellowSandBoard : public TObject {
public:
	TObjYellowSandBoard(TObject* parent, const Vec3f* pos, int rot, const Vec3f* vel);
	virtual ~TObjYellowSandBoard();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	Vec3f mPos;          // _20
	Vec3f mVel;          // _2C
	int mRot;            // _38
	int mLife;           // _3C
	int mAlpha;          // _40
	f32 mLenFront;       // _44
	f32 mLenMiddle;      // _48
	f32 mLenBack;        // _4C
	f32 mWidthSide;      // _50
	f32 mWidthMid;       // _54
	DrawTask* mDrawTask; // _58
};

typedef char TObjYellowSandBoard_size_check[(sizeof(TObjYellowSandBoard) == 0x5C) ? 1 : -1];

/**
 * A moving sand whirl: grows, runs along the ground until the ground ends or 480 frames pass, then slows, shrinks and sinks.
 *
 * @size 0xC4
 */
class TObjYellowSandStorm : public TObjAction {
public:
	TObjYellowSandStorm(TObject* parent, const Vec3f* pos, const Vec3f* dir);
	virtual ~TObjYellowSandStorm();
	virtual void draw();
	virtual Vec3f* getPos();
	virtual int getKind();

	static int isOffGround(const Vec3f* pos);

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	Vec3f mPos;      // _30
	Vec3f mDir;      // _3C
	f32 mSpeed;      // _48
	f32 mMaxSpeed;   // _4C
	int mTimer;      // _50
	f32 mScale;      // _54
	int mState;      // _58 0 = grow 1 = run 2 = die
	void* mEffect;   // _5C
	f32 mMatrix[12]; // _60
	SeHolder mSound; // _90
};

typedef char TObjYellowSandStorm_size_check[(sizeof(TObjYellowSandStorm) == 0xC4) ? 1 : -1];

/**
 * A placed model with the sand's scrolling texture.
 *
 * @size 0xBC
 */
class TObjYellowSandMdl : public TSetObj {
public:
	TObjYellowSandMdl(TObject* parent, SetDesc* desc);
	virtual ~TObjYellowSandMdl();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void* arg);

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	ScrollModel mSurface; // _90
	NJS_OBJECT* mModel;   // _B0
	f32 mCullRadius;      // _B4
	DrawTask* mDrawTask;  // _B8
};

typedef char TObjYellowSandMdl_size_check[(sizeof(TObjYellowSandMdl) == 0xBC) ? 1 : -1];

// 0x80140A60
extern "C" void initYellowSand()
{
	resBankLoad(&sYellowSandBank, (const char*)("ar_obj_yellow_sand.arc"));
	GvmLoader* tex = new GvmLoader;
	loadSizedResource("obj_yellow_sand.gvm", &sYellowSandGvm);
	gvmLoaderSetList(tex, sYellowSandBank.mData->mTextures[0]);
	gvmLoaderFromMemory(tex, (unsigned char*)sYellowSandGvm->mData);
	gvmLoaderTakeList(tex);
	delete tex;
	sYellowSandModels   = resBankGetModels(&sYellowSandBank);
	sYellowSandTextures = resBankGetTextures(&sYellowSandBank);
}

// 0x80140B20
extern "C" void termYellowSand()
{
	resBankRemoveTextures(&sYellowSandBank, 0);
	resBankRelease(&sYellowSandBank);
	sYellowSandModels   = 0;
	sYellowSandTextures = 0;
	releaseResource(sYellowSandGvm);
}

// 0x80140B6C
extern "C" TObject* createTObjYellowSandGen(TObject* parent, SetDesc* desc)
{
	return new TObjYellowSandGen(parent, desc);
}

// 0x80140BC0
extern "C" NJS_OBJECT* getYellowSandModel(int index)
{
	return sYellowSandModels[index];
}

// 0x80140BD0
extern "C" NJS_TEXLIST* getYellowSandTexList()
{
	return sYellowSandTextures[0];
}

// 0x80140BDC
TObjYellowSandGen::TObjYellowSandGen(TObject* parent, SetDesc* desc)
    : TSetObj(parent, desc)
{
	_00   = (u32)gTObjYellowSandGenName;
	mSize = 0xA0;
	_80   = 0;
	_80   = (int)colliCreate(1, 1);
	colliSetOwner((CharaColli*)_80, (u16)mObjId);
	colliSetMasks((CharaColli*)_80, 4, 255, 255, 252);
	colliShapeSetBit10((CharaColli*)_80, 0);
	initFromDesc(desc);
}

// 0x80140C8C
TObjYellowSandGen::~TObjYellowSandGen()
{
	if ((void*)_80 != 0) {
		colliRetire((CharaColli*)_80);
		_80 = 0;
	}
}

// 0x80140D04
// The first int is the shape and the first three floats are its radius or size
void TObjYellowSandGen::initFromDesc(SetDesc* desc)
{
	readDesc(desc);
	int type = desc->getInt(0);
	f32 x    = desc->getFloat(0);
	f32 y    = desc->getFloat(1);
	f32 z    = desc->getFloat(2);
	setCollision(x, y, z, type);
}

// 0x80140DDC
// No side is ever under 10
void TObjYellowSandGen::setCollision(f32 x, f32 y, f32 z, int type)
{
	if ((void*)_80 == 0) {
		return;
	}
	x = (x < 10.0f) ? 10.0f : x;
	y = (y < 10.0f) ? 10.0f : y;
	z = (z < 10.0f) ? 10.0f : z;

	ColliShape* shape = colliGetShape((CharaColli*)_80, 0);
	if (shape == 0) {
		return;
	}
	switch (type) {
	case 0:
		colliShapeMakeCylinder(shape, &mTrans.mPos, 1, x, y);
		break;
	default:
	case 1:
		colliShapeMakeBox(shape, &mTrans.mPos, mTrans.mRot.y, x, y, z, 1);
		break;
	}
	colliRefreshBounds((CharaColli*)_80);
}

// Half of a width
static inline f32 halfOf(const f32& width)
{
	return width * 0.5f;
}

// 0x80140EE8
// Puts a sand board ahead of the player at a fixed slant at height y
extern "C" void spawnSandBoard(f32 y)
{
	TObjAction* player = findObjById(0);
	if (player == 0) {
		return;
	}
	Vec3f start;
	Vec3f dir;
	Vec3f pos;
	f32 sn;
	f32 cs;
	pos   = *player->getPos();
	pos.y = y;

	njSinCos(0x1800, &dir.x, &dir.z);
	dir.y     = 0.0f;
	f32 reach = halfOf(800.0f);
	gjMulAddVector(-reach, &pos, &pos, &dir);

	njSinCos((int)(65536.0f * randUnit()), &sn, &cs);

	start     = pos;
	f32 ahead = reach * randUnit();
	start.x += ahead * sn;
	start.z += ahead * cs;

	f32 side = 500.0f * randUnit();
	dir.y    = 0.0f;
	start.x += side * cs;
	start.z -= side * sn;

	gjScaleVector(4.0f + 8.0f * randUnit(), &dir, &dir);

	new TObjYellowSandBoard(gTL_05, &start, 0x1800, &dir);
}

// 0x801410F8
extern "C" void updateSandBoardTimer(int* timer, f32 y)
{
	(*timer)--;
	if (*timer <= 0) {
		spawnSandBoard(y);
		*timer = 15;
	}
}

// 0x80141144
void TObjYellowSandGen::draw()
{
	if (debugEditActive() != 0 && ((int (*)(void*))debugInfoActive)(this) != 0) {
		return;
	}
	if (!isInRange(this, &mTrans.mPos, 1000020.0f)) {
		mFlags |= 1;
	} else if ((void*)_80 != 0 && colliGetContactCount((CharaColli*)_80) > 0) {
		triggerYellowDoubleFog();
	}
}

// 0x801411CC
void TObjYellowSandGen::debugPrint(void*)
{
	ColliShape* shape = 0;
	if ((void*)_80 != 0) {
		shape = (ColliShape*)colliGetShapes((CharaColli*)_80);
	}
	if (shape != 0) {
		switch (shape->mType) {
		case COLLISHAPE_CYLINDER:
			njPrint(0xA0013, ":CYLINDER");
			break;
		case COLLISHAPE_BOX:
			njPrint(0xA0013, ":CUBE");
			break;
		default:
			njPrint(0xA0013, ":UNKNOWN");
			break;
		}
	} else {
		njPrint(0xA0013, ":COLLI TYPE");
	}
	if (shape != 0) {
		switch (shape->mType) {
		case COLLISHAPE_CYLINDER:
			njPrint(0xA0017, ":COLLI RAD");
			njPrint(0xA0018, ":COLLI HEIGHT");
			break;
		case COLLISHAPE_BOX:
			njPrint(0xA0017, ":COLLI WIDTH");
			njPrint(0xA0018, ":COLLI HEIGHT");
			njPrint(0xA0019, ":COLLI DEPTH");
			break;
		}
	}
}

// 0x8014131C
TObjYellowSandBoard::TObjYellowSandBoard(TObject* parent, const Vec3f* pos, int rot, const Vec3f* vel)
    : TObject(parent)
{
	_00        = (u32)gTObjYellowSandBoardName;
	mSize      = 0x5C;
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 42, 0);
	mPos       = *pos;
	mRot       = rot;
	mVel       = *vel;
	mLife      = 100;
	mAlpha     = 0;
	mLenFront  = 10.0f;
	mLenMiddle = 33.0f;
	mLenBack   = 70.0f;
	mWidthSide = 40.0f;
	mWidthMid  = 140.0f;
}

// 0x801413FC
TObjYellowSandBoard::~TObjYellowSandBoard()
{
	removeDrawTask(mDrawTask);
	mDrawTask = 0;
}

// 0x8014146C
void TObjYellowSandBoard::draw()
{
	njAddVector(&mPos, &mVel);
	mLife--;
	if (mLife > 0) {
		mAlpha += 0x10;
		if (mAlpha > 0xFF) {
			mAlpha = 0xFF;
		}
	} else {
		mAlpha -= 0x10;
		if (mAlpha <= 0) {
			mAlpha = 0;
			mFlags |= 1;
		}
	}
}

// 0x80141508
void TObjYellowSandBoard::vfn_10()
{
	if (cameraIsVisible(2000.0f, 200.0f, &mPos, 0) != 0) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x80141568
// One band of the board four rows of two vertices where the first and last rows are clear
static void drawSandBand(f32 x0, f32 x1, f32 z0, f32 z1, f32 z2, f32 z3, u32 color0, u32 color1)
{
	f32 y = 0.05f;
	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 8);

	GX_FIFO_F32 = x0;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z0;
	GX_FIFO_U32 = 0xFFFFFF00;

	GX_FIFO_F32 = x1;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z0;
	GX_FIFO_U32 = 0xFFFFFF00;

	GX_FIFO_F32 = x0;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z1;
	GX_FIFO_U32 = color0;

	GX_FIFO_F32 = x1;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z1;
	GX_FIFO_U32 = color1;

	GX_FIFO_F32 = x0;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z2;
	GX_FIFO_U32 = color0;

	GX_FIFO_F32 = x1;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z2;
	GX_FIFO_U32 = color1;

	GX_FIFO_F32 = x0;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z3;
	GX_FIFO_U32 = 0xFFFFFF00;

	GX_FIFO_F32 = x1;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z3;
	GX_FIFO_U32 = 0xFFFFFF00;
}

// 0x801416A8
void TObjYellowSandBoard::render()
{
	njSetTexture(sYellowSandTextures[0]);
	gjLoadTextureNum(1, 1, 1, 0);
	frameTexSetCur();
	gjLoadTextureNum(0, 0, 0, 1);

	f32 viewMtx[12];
	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjRotateY(mRot);
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	texFxScreenMtx(viewMtx);
	GXLoadTexMtxImm(viewMtx, GX_TEXMTX1, GX_MTX3x4);
	njPopMatrixEx();

	// The texture follows world position so the sand slides under the board
	f32 scroll[12];
	mtxIdentity(scroll);
	scroll[0]  = 0.0f;
	scroll[1]  = 0.0f;
	scroll[2]  = 0.019f;
	scroll[3]  = 0.0f;
	scroll[4]  = -0.019f;
	scroll[5]  = 0.0f;
	scroll[6]  = 0.0f;
	scroll[7]  = 0.0f;
	scroll[8]  = 0.0f;
	scroll[9]  = 0.0f;
	scroll[10] = 0.0f;
	scroll[11] = 1.0f;

	f32 texMtx[12];
	mtxSetTransV(texMtx, &mPos);
	mtxRotateY(texMtx, texMtx, mRot);
	mtxConcat(texMtx, scroll, texMtx);
	GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX2x4);

	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	GXSetNumTexGens(2);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
	GXSetNumTevStages(2);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

	GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K0);
	GXSetTevKColor(GX_KCOLOR0, *(GXColor*)&sYellowSandClearColor);
	GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_CPREV, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
	GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_COMP_R8_GT, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
	GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_COPY);
	GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 255);
	GXSetZCompLoc(GX_FALSE);
	GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_DISABLE);

	u32 alpha = 0xFFFFFF00 | (mAlpha & 0xFF);

	f32 z3;
	f32 z0;
	f32 z1;
	f32 z2;
	z0 = 0.5f * (mLenBack + (mLenFront + mLenMiddle));
	z1 = z0 - mLenFront;
	z2 = z1 - mLenMiddle;
	z3 = z2 - mLenBack;

	f32 halfWidth = 0.5f * (mWidthMid + mWidthSide);
	f32 x0        = halfWidth - mWidthSide;
	f32 x1        = x0 - mWidthMid;

	drawSandBand(halfWidth, x0, z0, z1, z2, z3, 0xFFFFFF00, alpha);
	drawSandBand(x0, x1, z0, z1, z2, z3, alpha, alpha);
	drawSandBand(x1, x1 - mWidthSide, z0, z1, z2, z3, alpha, 0xFFFFFF00);

	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetNumIndStages(0);
}

// 0x80141AEC
extern "C" void createSandStorm(TObject* parent, const Vec3f* pos, const Vec3f* dir)
{
	if (TObjYellowSandStorm::isOffGround(pos) == 0) {
		new TObjYellowSandStorm(parent, pos, dir);
	}
}

// 0x80141B50
TObjYellowSandStorm::TObjYellowSandStorm(TObject* parent, const Vec3f* pos, const Vec3f* dir)
    : TObjAction(parent)
{
	_00     = (u32)gTObjYellowSandStormName;
	mSize   = 0xC4;
	mPos    = *pos;
	mDir    = *dir;
	f32 len = njUnitVector(&mDir);
	if (len < 1e-06f) {
		mDir = gVecZero;
	}
	mMaxSpeed = len;
	mSpeed    = 0.0f;
	mTimer    = 480;
	mScale    = 0.0f;
	mState    = 0;
	mEffect   = 0;
	mtxIdentity(mMatrix);
	seHolderStop(&mSound);
	mSound.setRange(400.0f);
}

// 0x80141C4C
TObjYellowSandStorm::~TObjYellowSandStorm()
{
	seHolderStop(&mSound);
	if (mEffect != 0) {
		fxStop((Fx*)mEffect);
		mEffect = 0;
	}
}

// 0x80141CD8
// 1 when pos has no ground under it that the storm can stand on
int TObjYellowSandStorm::isOffGround(const Vec3f* pos)
{
	EggHits hits;
	Vec3f from;
	from = *pos;
	from.y += 80.0f;

	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&from, 9) != 0) {
		u32 mask = hits.mMask;
		if ((mask & 1) && hits.mHits[0].mPos.y - pos->y > 3.0f) {
			return 1;
		}
		if ((mask & 4) && hits.mHits[2].mPos.y - pos->y >= -5.0f) {
			return 0;
		}
		return 1;
	}
	return 1;
}

// 0x80141DBC
void TObjYellowSandStorm::draw()
{
	if (mEffect == 0) {
		mEffect = fxSpawnFollow(1, 8, 9, mMatrix);
	}
	mtxSetTransV(mMatrix, &mPos);

	switch (mState) {
	case 0:
		mScale += 0.02f;
		if (mScale >= 1.0f) {
			mScale = 1.0f;
			mState = 1;
		}
		mtxScale(mScale, 1.0f, mScale, mMatrix, mMatrix);
		break;
	case 1:
		mSpeed += 0.01f;
		if (mSpeed >= mMaxSpeed) {
			mSpeed = mMaxSpeed;
		}
		gjMulAddVector(mSpeed, &mPos, &mPos, &mDir);
		if (isOffGround(&mPos) != 0) {
			mTimer = 0;
		}
		mTimer--;
		if (mTimer <= 0) {
			mTimer = 120;
			mState = 2;
		}
		break;
	case 2:
		mSpeed -= 0.01f;
		if (mSpeed <= 0.0f) {
			mSpeed = 0.0f;
		}
		gjMulAddVector(mSpeed, &mPos, &mPos, &mDir);
		mScale -= 0.02f;
		if (mScale <= 0.0f) {
			mScale = 0.0f;
			mTimer--;
			mPos.y -= 1.2f;
			if (mTimer <= 0) {
				mFlags |= 1;
			}
		}
		mtxScale(mScale, 1.0f, mScale, mMatrix, mMatrix);
		break;
	}

	if (mScale >= 0.2f) {
		seHolderPlay(&mSound, 0x261, &mPos);
	} else {
		seHolderStop(&mSound);
	}
}

// 0x80141FE8
extern "C" TObject* createTObjYellowSandMdl(TObject* parent, SetDesc* desc)
{
	return new TObjYellowSandMdl(parent, desc);
}

// 0x8014203C
TObjYellowSandMdl::TObjYellowSandMdl(TObject* parent, SetDesc* desc)
    : TSetObj(parent, desc)
{
	_00    = (u32)gTObjYellowSandMdlName;
	mSize  = 0xBC;
	mModel = getYellowSandModel(0);
	GjModel* model;
	if (mModel != 0 && (model = mModel->model) != 0) {
		mCullRadius = 200.0f + model->r;
	} else {
		mCullRadius = 200.0f;
	}
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 34, 0);
	mSurface.reset();
	initFromDesc(desc);
}

// 0x80142110
TObjYellowSandMdl::~TObjYellowSandMdl()
{
	removeDrawTask(mDrawTask);
	mDrawTask = 0;
}

// 0x8014218C
void TObjYellowSandMdl::initFromDesc(SetDesc* desc)
{
	readDesc(desc);
	mSurface.readDesc(desc);
	mSurface.bindModel(mModel->model, &mTrans);
}

// 0x801421E8
void TObjYellowSandMdl::draw()
{
	if (!isInRange(this, &mTrans.mPos, 1000020.0f)) {
		mFlags |= 1;
	} else {
		mSurface.update();
	}
}

// 0x8014223C
void TObjYellowSandMdl::debugPrint(void*)
{
	mSurface.debugPrint();
	mSurface.debugPrintTrans(&mTrans);
}

// 0x80142278
void TObjYellowSandMdl::vfn_10()
{
	if (cameraIsVisible(1000000.0f, mCullRadius, &mTrans.mPos, 0) != 0) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x801422D8
void TObjYellowSandMdl::render()
{
	mSurface.draw(&mTrans, mModel, getYellowSandTexList());
}

// 0x80142318
int TObjYellowSandStorm::getKind()
{
	return 3;
}

// 0x80142320
Vec3f* TObjYellowSandStorm::getPos()
{
	return &mPos;
}
