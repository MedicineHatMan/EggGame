#include "object/purple/TObjPurpleVFSnow.h"
struct DrawTask;
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "stage/StageObjUtil.h"
#include "object/ObjAction.h"
#include "object/TObjSnowman.h"
#include "Dolphin/OS/OSCache.h"
#include "camera/TCamera.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "stage/SetSlot.h"
#include "core/VecMath.h"
struct CharaColli;
struct DrawState;
struct SetLink;
struct DrawStateSlot;
struct Fx;

// A contact on mShape+0x80 colliGetShape takes the owner and the shape id
struct SnowTouch {
	SnowTouch* next; // _00
	void* owner;     // _04
	int shapeId;     // _08
};

#define sCells sSnowCells
#define sBase  sSnowBase

#define GX_FIFO_U16 (*(volatile u16*)0xCC008000)
#define GX_FIFO_U8  (*(volatile u8*)0xCC008000)
#define GX_FIFO_F32 (*(volatile f32*)0xCC008000)

static const int SNOW_STRIDE = 0x81;

typedef char TObjPurpleVFSnow_size_check[(sizeof(TObjPurpleVFSnow) == 0x80) ? 1 : -1];

static inline int clampSnow(int value, int limit)
{
	if (value < 1) {
		return 1;
	}
	if ((u32)value > (u32)limit) {
		return limit;
	}
	return value;
}

static inline f32 randUnit()
{
	return 3.0517578e-05f * (f32)rand();
}

// 0x800F9554
static SnowCell* snowCell(u32 x, u32 z)
{
	return &sCells[(x & 0xFFFF) + (z & 0xFFFF) * SNOW_STRIDE];
}

// 0x800F95C4
// resetSnowField writes the resting height through this
void setSnowBase(u32 x, u32 z, f32 height)
{
	sBase[(x & 0xFFFF) + (z & 0xFFFF) * SNOW_STRIDE] = height;
}

// 0x800F95E4
static f32 snowBase(u32 x, u32 z)
{
	return sBase[(x & 0xFFFF) + (z & 0xFFFF) * SNOW_STRIDE];
}

// 0x800F9574
static void updateSnowColor(SnowCell* cell)
{
	int color = (int)(35.0f * (cell->y / 6.2f) + 220.0f);
	if (color > 0xFF) {
		color = 0xFF;
	} else if (color < 0) {
		color = 0;
	}
	cell->color = (u8)color;
}

static inline void raiseSnow(u32 x, u32 z)
{
	SnowCell* cell = &sCells[(x & 0xFFFF) + (z & 0xFFFF) * SNOW_STRIDE];
	if (cell->y < snowBase(x & 0xFFFF, (u16)z)) {
		cell->y += 2.0f;
		updateSnowColor(cell);
	}
}

// 0x800F8CBC
// At stage load the cells and the base heights
extern "C" void initSnowField()
{
	sCells = new SnowCell[0x81 * 0x56];
	sBase = new f32[0x81 * 0x56];
	resetSnowField();
}

// 0x800F8D18
// Lays the cells out 6.2 apart with a little random depth the edge rows and columns bare and pushed slightly out
extern "C" void resetSnowField()
{
	u32 x;
	u32 z;
	for (z = 0; z < 0x56; z++) {
		f32 zf = 6.2f * (f32)(int)z;
		f32 v = zf / 52.7f;
		for (x = 0; x < 0x81; x++) {
			f32 height = 6.2f * (0.2f * randUnit() + 0.8f);
			f32 xf = 6.2f * (f32)(int)x;
			SnowCell* cell = snowCell(x, z);
			cell->x = xf;
			cell->y = height;
			cell->z = zf;
			cell->u = xf / 79.36f;
			cell->v = v;
			cell->color = 0;
			updateSnowColor(snowCell(x, z));
			setSnowBase(x, z, height);
		}
	}
	for (x = 0; x < 0x81; x++) {
		snowCell(x, 0)->y = 0.0f;
		snowCell(x, 0)->color = 0xDC;
		SnowCell* cell = snowCell(x, 0);
		cell->z = 3.1f * randUnit() + cell->z;
		snowCell(x, 0x55)->y = 0.0f;
		snowCell(x, 0x55)->color = 0xDC;
		cell = snowCell(x, 0x55);
		cell->z = cell->z - 3.1f * randUnit();
	}
	for (z = 0; z < 0x56; z++) {
		snowCell(0, z)->y = 0.0f;
		snowCell(0, z)->color = 0xDC;
		SnowCell* cell = snowCell(0, z);
		cell->x = 3.1f * randUnit() + cell->x;
		snowCell(0x80, z)->y = 0.0f;
		snowCell(0x80, z)->color = 0xDC;
		cell = snowCell(0x80, z);
		cell->x = cell->x - 3.1f * randUnit();
	}
}

// 0x800F9088
extern "C" void termSnowField()
{
	delete[] sBase;
	delete[] sCells;
}

// 0x800F90C0
extern "C" TObject* createTObjPurpleVFSnow(TObject* parent, SetDesc* desc)
{
	return new TObjPurpleVFSnow(parent, desc);
}

const char* gTObjPurpleVFSnowName = "TObjPurpleVFSnow";

// 0x800F9114
TObjPurpleVFSnow::TObjPurpleVFSnow(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjPurpleVFSnowName;
	mSize = 0x80;
	resetSnowField();

	mPlate = new MeshPlate(0x80, 793.6f, 527.0f);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	makeShape();
	initFromDesc(desc);
	vfn_74();
}

// 0x800F91E0
TObjPurpleVFSnow::~TObjPurpleVFSnow()
{
	freeShape();
	removeDrawTask(mDrawTask);
	delete mPlate;
}

// The length of a vector passed by value
static inline f32 vecLen(Vec3f v)
{
	return njScalor(&v);
}

// a - b returned by value
static inline Vec3f vecDiff(const Vec3f* a, const Vec3f* b)
{
	Vec3f d;
	gjSubVector(&d, a, b);
	return d;
}

// 0x800F926C
void TObjPurpleVFSnow::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mCenter = mPos;
	mCenter.x += 396.8f;
	mCenter.y += 3.1f;
	mCenter.z += 263.5f;

	mRadius = vecLen(vecDiff(&mPos, &mCenter));

	Vec3f at = mCenter;
	at.y += 0.05f;
	f32 matrix[12];
	mtxSetTransV(matrix, &at);
	mPlate->setMatrix(matrix);
	placeShape();
}

// 0x800F935C
void TObjPurpleVFSnow::debugPrint(void*)
{
	njPrintC(0xA0013, "");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x800F93F8
void TObjPurpleVFSnow::vfn_74()
{
	mSoundWait = 0;
	setLinkFree((SetLink*)&mLink);
}

// 0x800F9424
void TObjPurpleVFSnow::makeShape()
{
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xFF, 0xF7);
}

// 0x800F9480
void TObjPurpleVFSnow::placeShape()
{
	Vec3f pos = mPos;
	pos.x += 396.8f;
	pos.y += 6.2f;
	pos.z += 263.5f;

	ColliShape shape;
	colliShapeMakeBox(&shape, &pos, 0, 396.8f, 6.2f, 263.5f, 0);
	shape.mFlags |= 0x10;
	colliCopyShapes(mShape, &shape);
}

// 0x800F951C
void TObjPurpleVFSnow::freeShape()
{
	colliRetire(mShape);
	mShape = 0;
}

// 0x800F9604
void TObjPurpleVFSnow::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (!isInRange(this, &mPos, 2100.0f)) {
		mFlags |= 1;
		return;
	}

	for (SnowTouch* hit = (SnowTouch*)mShape->mContacts; hit != 0; hit = hit->next) {
		ColliShape* shape = colliGetShape((CharaColli*)hit->owner, hit->shapeId);
		if ((shape->mFlags & 0x10) != 0) {
			continue;
		}
		TObjAction* other = findObjById((u16)colliContactOwner((ColliContact*)hit));
		if (other == 0 || other->_00 != (u32)gTObjSnowmanHeadName) {
			continue;
		}

		Vec3f head;
		head = *other->getPos();
		f32 radius = other->getRadiusV();
		vecSub(&head, &head, &mPos);

		int reach = (int)(radius / 6.2f) + 1;
		int ix    = (int)(head.x / 6.2f);
		int iz    = (int)(head.z / 6.2f);
		int x0    = clampSnow(ix - reach, 0x7F);
		int x1    = clampSnow(ix + reach, 0x7F);
		int z0    = clampSnow(iz - reach, 0x54);
		int z1    = clampSnow(iz + reach, 0x54);
		int deep  = 0;
		for (int z = z0; z <= z1; z++) {
			for (int x = x0; x <= x1; x++) {
				SnowCell* cell = snowCell(x, z);
				Vec3f gap;
				vecSub(&gap, (Vec3f*)cell, &head);
				if (njScalor(&gap) >= radius) {
					continue;
				}
				if (cell->y > 1.55f) {
					feed__15TObjSnowmanHeadFv(other);
				}
				cell->y -= 1.55f;
				if (cell->y < 1.55f) {
					cell->y = -3.1f;
				}
				updateSnowColor(cell);
				if (cell->y > 3.1f) {
					deep = 1;
				}
			}
		}
		if (deep != 0 && mSoundWait == 0) {
			Vec3f at;
			at.x = head.x + mPos.x;
			at.y = mPos.y;
			at.z = head.z + mPos.z;
			f32 matrix[12];
			mtxSetTransV(matrix, &at);
			fxSpawnAt(1, 8, 0x21, matrix);
			mSoundWait = 10;
		}
	}

	settle();
	mSoundWait -= 1;
	if (mSoundWait < 0) {
		mSoundWait = 0;
	}
	DCFlushRange(sCells, 4);
}

// 0x800F9944
void TObjPurpleVFSnow::settle()
{
	int x = (int)(126.0f * randUnit() + 0.5f) + 1;
	int z = (int)(83.0f * randUnit() + 0.5f) + 1;
	raiseSnow(x, z);
	if (x > 1) {
		raiseSnow(x - 1, z);
	}
	if ((u32)x < 0x7F) {
		raiseSnow(x + 1, z);
	}
	if (z > 1) {
		raiseSnow(x, z - 1);
	}
	if ((u32)z < 0x54) {
		raiseSnow(x, z + 1);
	}
}

// 0x800F9B5C
void TObjPurpleVFSnow::vfn_10()
{
	if (cameraIsVisible(2000.0f, mRadius, &mCenter, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// One strip vertex index grey colour then the texture coordinates
static inline void snowVertex(u16 index, const SnowCell* cell)
{
	GX_FIFO_U16 = index;
	u8 color = cell->color;
	GX_FIFO_U8  = color;
	GX_FIFO_U8  = color;
	GX_FIFO_U8  = color;
	f32 u = cell->u;
	f32 v = cell->v;
	GX_FIFO_F32 = u;
	GX_FIFO_F32 = v;
}

// 0x800F9BC4
void TObjPurpleVFSnow::render()
{
	DrawState* state = getDefaultDrawState();
	DrawStateSlot* slot  = getDrawStateSlot();
	beginDrawState(slot, state);
	gjGXSetBlendMode(0);
	GXSetZCompLoc(GX_FALSE);
	gjSetZBufferMode(1);
	njPushMatrixEx();
	njTranslate(0, mPos.x, mPos.y + 0.05f, mPos.z);
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetChanCtrl(GX_ALPHA0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetNumTexGens(0);
	f32 texMtx[12];
	PSMTXIdentity(texMtx);
	GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX2x4);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB8, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GXSetArray(GX_VA_POS, sCells, 0x18);
	pushGXCullMode(2);

	int index = SNOW_STRIDE;
	for (u32 row = 1; row < 0x56; row++, index += SNOW_STRIDE) {
		SnowCell* line = &sCells[row * SNOW_STRIDE];
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 0x102);
		for (int col = 0; col < SNOW_STRIDE; col++) {
			SnowCell* cell = &line[col];
			u16 v = index + col;
			snowVertex(v, cell);
			snowVertex(v - SNOW_STRIDE, cell - SNOW_STRIDE);
		}
	}

	popGXCullMode();
	njPopMatrixEx();
	GXSetZCompLoc(GX_TRUE);
	endDrawState(getDrawStateSlot());

	if (objDebugEnabled(this) != 0) {
		colliDrawShapes(mShape);
		mPlate->debugDraw();
	}
}
