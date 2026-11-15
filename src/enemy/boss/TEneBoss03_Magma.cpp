#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "stl/math.h"
#include "Dolphin/OS/OSCache.h"
#include "Dolphin/PPCArch.h"
#include "camera/TCamPath.h"
#include "engine/GenTex.h"
#include "enemy/boss/TEneBoss03_Magma.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"
#include "core/TSceneManager.h"
#include "core/TMainTask.h"
struct SceneRecord;

// 0x160 the same as the left edge so they never pass the moving flag to the column before

/**
 * @size 0x40
 */
class TEneBoss03_MagmaPow : public TObject {
public:
	TEneBoss03_MagmaPow(TObject* parent, const Vec3f* pos, f32 speed, f32 maxRadius, f32 push);
	virtual ~TEneBoss03_MagmaPow();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	Vec3f mPos;     // _20
	f32 mPrev;      // _2C
	f32 mRadius;    // _30
	f32 mSpeed;     // _34
	f32 mPush;      // _38 falls by 1.5% a frame
	f32 mMaxRadius; // _3C
};

typedef char TEneBoss03_MagmaPow_size_check[(sizeof(TEneBoss03_MagmaPow) == 0x40) ? 1 : -1];

typedef char MagmaGrid_size_check[(sizeof(MagmaGrid) == 0x38) ? 1 : -1];

typedef char TEneBoss03_Magma_size_check[(sizeof(TEneBoss03_Magma) == 0x48) ? 1 : -1];

// 0x800864C4
// A pow on the magma under the action layer
extern "C" void createMagmaPow(const Vec3f* pos, f32 speed, f32 maxRadius, f32 push)
{
	new TEneBoss03_MagmaPow(gTL_05, pos, speed, maxRadius, push);
}

const char* gTEneBoss03_MagmaPowName = "TEneBoss03_MagmaPow";

// 0x80086534
TEneBoss03_MagmaPow::TEneBoss03_MagmaPow(TObject* parent, const Vec3f* pos, f32 speed, f32 maxRadius, f32 push)
    : TObject(parent)
{
	_00        = (u32)gTEneBoss03_MagmaPowName;
	mSize      = 0x40;
	mPos       = *pos;
	mPrev      = 0.0f;
	mRadius    = 0.0f;
	mSpeed     = speed;
	mMaxRadius = maxRadius;
	mPush      = push;
}

// 0x800865F0
TEneBoss03_MagmaPow::~TEneBoss03_MagmaPow()
{
}

// 0x8008664C
void TEneBoss03_MagmaPow::draw()
{
	mPrev = mRadius;
	mRadius += mSpeed;
	mPush *= 0.985f;
	if (mRadius > mMaxRadius) {
		mFlags |= 1;
	}
	rippleMagma(&mPos, mPrev, mRadius, mPush);
}

// 0x800866C4
MagmaGrid::MagmaGrid(int cols, int rows, const Vec3f* pos, f32 width, f32 depth)
{
	mFlags    = 0;
	mDepth    = depth;
	mWidth    = width;
	mRows     = rows;
	mCols     = cols;
	mPos      = *pos;
	int count = mRows * mCols;
	mVerts    = new HitVec[count];
	mNormals  = new HitVec[count];
	mSpeeds   = new f32[count];
	mCells    = new u32[count];
	mScrollA  = 0;
	mScrollB  = 0;
	for (int i = 0; i < count; i++) {
		HitVec* normal = &mNormals[i];
		normal->x      = 0.0f;
		normal->y      = 1.0f;
		normal->z      = 0.0f;
		mSpeeds[i]     = 0.0f;
	}
	int lastCol                       = mCols - 1;
	int lastRow                       = mRows - 1;
	mCells[0]                         = 0x140;
	mCells[lastCol]                   = 0xC0;
	mCells[lastRow * mCols]           = 0x120;
	mCells[lastCol + lastRow * mCols] = 0xA0;
	int top                           = 1;
	int bottom                        = lastRow * mCols + 1;
	for (int i = 1; i < mCols - 1; i++) {
		mCells[top]    = 0x1C0;
		mCells[bottom] = 0x1A0;
		top++;
		bottom++;
	}
	int right = mCols + lastCol;
	int left  = mCols;
	for (int i = 1; i < mRows - 1; i++) {
		mCells[left]  = 0x160;
		mCells[right] = 0xE0;
		left += mCols;
		right += mCols;
	}
	// Fills one cell to the right of each inner cell so the last column's 0xE0 becomes 0x160
	for (int r = 1; r < mRows - 1; r++) {
		int cell = r * mCols + 2;
		for (int c = 1; c < mCols - 1; c++) {
			mCells[cell++] = 0x160;
		}
	}
	HitVec* vert = mVerts;
	f32 rowStep  = 1.0f / (f32)(mRows - 1);
	f32 colStep  = 1.0f / (f32)(mCols - 1);
	for (int r = 0; r < mRows; r++) {
		f32 z = ((f32)r * rowStep - 0.5f) * mDepth;
		for (int c = 0; c < mCols; c++) {
			vert->x = ((f32)c * colStep - 0.5f) * mWidth;
			vert->y = 0.0f;
			vert->z = z;
			vert++;
		}
	}
	flush();
}

// 0x80086A1C
MagmaGrid::~MagmaGrid()
{
	if (mCells != 0) {
		delete[] mCells;
		mCells = 0;
	}
	if (mSpeeds != 0) {
		delete[] mSpeeds;
		mSpeeds = 0;
	}
	if (mNormals != 0) {
		delete[] mNormals;
		mNormals = 0;
	}
	if (mVerts != 0) {
		delete[] mVerts;
		mVerts = 0;
	}
}

// 0x80086AD0
void MagmaGrid::setWarp()
{
	mFlags |= 1;
}

static inline int clampCount(int value, int limit)
{
	if (value < 0) {
		return 0;
	}
	if (value > limit) {
		return limit;
	}
	return value;
}

// 0x80086AE0
void MagmaGrid::push(const Vec3f* pos, f32 inner, f32 outer, f32 amount)
{
	f32 z     = pos->z - (mPos.z - 0.5f * mDepth);
	f32 zHigh = z + outer;
	f32 x     = pos->x - (mPos.x - 0.5f * mWidth);
	f32 xHigh = x + outer;
	if (zHigh < 0.0f || xHigh < 0.0f) {
		return;
	}
	f32 zLow = z - outer;
	f32 xLow = x - outer;
	if (zLow > mDepth || xLow > mWidth) {
		return;
	}
	f32 toRow   = (f32)(mRows - 1) / mDepth;
	f32 toCol   = (f32)(mCols - 1) / mWidth;
	int rowFrom = (int)ceil(zLow * toRow);
	int rowTo   = (int)ceil(zHigh * toRow);
	int colFrom = (int)ceil(xLow * toCol);
	int colTo   = (int)ceil(xHigh * toCol);
	// The square inside the circle which the ring cannot reach unless inner is small
	f32 side      = 0.5f * (inner * 1.4142135f);
	int inRowFrom = (int)ceil((z - side) * toRow);
	int inRowTo   = (int)ceil((z + side) * toRow);
	int inColFrom = (int)ceil((x - side) * toCol);
	int inColTo   = (int)ceil((x + side) * toCol);
	f32 inner2    = inner * inner;
	f32 cx        = pos->z - mPos.z;
	f32 cz        = pos->x - mPos.x;
	f32 outer2    = outer * outer;
	rowFrom       = clampCount(rowFrom, mRows);
	rowTo         = clampCount(rowTo, mRows);
	colFrom       = clampCount(colFrom, mCols);
	colTo         = clampCount(colTo, mCols);
	inColFrom     = clampCount(inColFrom, mCols);
	inColTo       = clampCount(inColTo, mCols);
	inRowFrom     = clampCount(inRowFrom, mRows);
	inRowTo       = clampCount(inRowTo, mRows);
#define MAGMA_BAND(rowA, rowB, colA, colB)     \
	for (int r = rowA; r < rowB; r++) {        \
		for (int c = colA; c < colB; c++) {    \
			int i  = r * mCols + c;            \
			f32 dx = mVerts[i].x - cz;         \
			f32 dz = mVerts[i].z - cx;         \
			f32 d2 = dx * dx + dz * dz;        \
			if (d2 >= inner2 && d2 < outer2) { \
				markMoving(i);                 \
				mSpeeds[i] += amount;          \
			}                                  \
		}                                      \
	}
	MAGMA_BAND(rowFrom, inRowFrom, colFrom, colTo);
	MAGMA_BAND(inRowFrom, inRowTo, colFrom, inColFrom);
	MAGMA_BAND(inRowFrom, inRowTo, inColTo, colTo);
	MAGMA_BAND(inRowTo, rowTo, colFrom, colTo);
#undef MAGMA_BAND
}

// 0x800870F4
void MagmaGrid::flush()
{
	u32 bytes = mCols * mRows * 0xC;
	DCStoreRangeNoSync(mVerts, bytes);
	DCStoreRangeNoSync(mNormals, bytes);
}

// 0x80087144
void MagmaGrid::markMoving(int index)
{
	u32 cell      = mCells[index] | 1;
	int cols      = mCols;
	mCells[index] = cell;
	if ((cell & 0x20) != 0) {
		mCells[index - cols] |= 4;
	}
	if ((cell & 0x40) != 0) {
		mCells[index + cols] |= 2;
	}
	if ((cell & 0x80) != 0) {
		mCells[index - 1] |= 0x10;
	}
	if ((cell & 0x100) != 0) {
		mCells[index + 1] |= 8;
	}
}

// 0x800871D8
void MagmaGrid::markMoving2(int index)
{
	u32 cell      = mCells[index] | 1;
	int cols      = mCols;
	mCells[index] = cell;
	if ((cell & 0x20) != 0) {
		mCells[index - cols] |= 4;
	}
	if ((cell & 0x40) != 0) {
		mCells[index + cols] |= 2;
	}
	if ((cell & 0x80) != 0) {
		mCells[index - 1] |= 0x10;
	}
	if ((cell & 0x100) != 0) {
		mCells[index + 1] |= 8;
	}
}

// 0x8008726C
int MagmaGrid::update()
{
	int moved = 0;
	mScrollA -= 2;
	mScrollB += 0x280;
	int count = mRows * mCols;
	for (int i = 0; i < count; i++) {
		if ((mCells[i] & 1) != 0) {
			f32 height;
			f32 speed = mSpeeds[i];
			height    = mVerts[i].y;
			speed += -0.001f * height;
			speed *= 0.995f;
			height += speed;
			if ((f32)fabs(speed) <= 0.01f && (f32)fabs(height) <= 0.01f) {
				markMoving2(i);
				speed  = 0.0f;
				height = 0.0f;
			}
			moved       = 1;
			mSpeeds[i]  = speed;
			mVerts[i].y = height;
		}
	}
	for (int i = 0; i < count; i++) {
		u32 cell = mCells[i];
		if ((cell & 0x1E) == 0) {
			if (1.0f != mNormals[i].y) {
				mNormals[i].x = 0.0f;
				mNormals[i].y = 1.0f;
				mNormals[i].z = 0.0f;
			}
		} else {
			f32 y;
			f32 nx;
			f32 ny;
			f32 nz;
			nx = 0.0f;
			y  = mVerts[i].y;
			ny = 0.3f;
			nz = 0.0f;
			if ((cell & 2) != 0) {
				ny += 3.0f;
				nz += mVerts[i - mCols].y - y;
			}
			if ((cell & 4) != 0) {
				ny += 3.0f;
				nz -= mVerts[i + mCols].y - y;
			}
			if ((cell & 8) != 0) {
				nx += mVerts[i - 1].y - y;
				ny += 3.0f;
			}
			if ((cell & 0x10) != 0) {
				nx -= mVerts[i + 1].y - y;
				ny += 3.0f;
			}
			mNormals[i].x = nx;
			mNormals[i].y = ny;
			mNormals[i].z = nz;
		}
	}
	if (moved != 0) {
		flush();
	}
	return moved;
}

// 0x800874C0
void MagmaGrid::cull()
{
	f32 z;
	f32 xHigh;
	f32 halfW;
	f32 xLow;
	f32 halfD;
	f32 x0;
	f32 y;
	f32 z0;
	int behind = 0;
	x0         = mPos.x;
	halfW      = mWidth * 0.5f;
	halfD      = 0.5f * mDepth;
	xHigh      = x0 + halfW;
	z0         = mPos.z;
	y          = mPos.y;
	Vec3f at;
	z    = z0 + halfD;
	at.y = y;
	at.x = xHigh;
	at.z = z;
	njCalcPointEx(&at, &at);
	if (at.z >= 0.0f) {
		behind = 1;
	}
	xLow = x0 - halfW;
	at.x = xLow;
	at.y = y;
	at.z = z;
	njCalcPointEx(&at, &at);
	if (at.z >= 0.0f) {
		behind++;
	}
	at.x = xHigh;
	at.y = y;
	z    = z0 - halfD;
	at.z = z;
	njCalcPointEx(&at, &at);
	if (at.z >= 0.0f) {
		behind++;
	}
	at.x = xLow;
	at.y = y;
	at.z = z;
	njCalcPointEx(&at, &at);
	if (at.z >= 0.0f) {
		behind += 1;
	}
	if (4 <= behind) {
		mFlags = mFlags | 2;
	} else {
		mFlags &= ~2;
	}
}

// 0x80087670
void MagmaGrid::draw()
{
	if ((mFlags & 2) != 0) {
		return;
	}
	njSetTexture(redMagmaTexList());
	gjLoadTextureNum(0, 1, 1, 0);
	genTexLoad(2, 1);
	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	njPopMatrixEx();
	f32 mtx[12];
	PSMTXIdentity(mtx);
	mtx[0]  = 0.005f;
	mtx[1]  = 0.0f;
	mtx[2]  = 0.0f;
	mtx[3]  = 0.0f;
	mtx[4]  = 0.0f;
	mtx[5]  = 0.0f;
	mtx[6]  = 0.005f;
	mtx[7]  = 0.0f;
	mtx[8]  = 0.0f;
	mtx[9]  = 0.0f;
	mtx[10] = 0.0f;
	mtx[11] = 1.0f;
	GXLoadTexMtxImm(mtx, GX_TEXMTX0, GX_MTX3x4);
	f32 sa, ca, sb, cb;
	njSinCos(mScrollA, &sa, &ca);
	njSinCos(mScrollB, &sb, &cb);
	sa *= 0.001f;
	ca *= 0.001f;
	sb *= 0.002f;
	cb *= 0.002f;
	mtx[0]  = ca;
	mtx[1]  = sb;
	mtx[2]  = -sa;
	mtx[3]  = 0.0f;
	mtx[4]  = sa;
	mtx[5]  = cb;
	mtx[6]  = ca;
	mtx[7]  = 0.0f;
	mtx[8]  = 0.0f;
	mtx[9]  = 0.0f;
	mtx[10] = 0.0f;
	mtx[11] = 1.0f;
	GXLoadTexMtxImm(mtx, GX_TEXMTX1, GX_MTX3x4);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
	GXSetVtxDesc(GX_VA_NRM, GX_INDEX16);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	GXSetArray(GX_VA_POS, mVerts, 0xC);
	GXSetArray(GX_VA_NRM, mNormals, 0xC);
	void* light = *(void**)((u8*)getSceneRecord() + 0xC);
	lightGroupLoad((LightGroup*)light);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG, *(u32*)((u8*)light + 0xC), GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_ALPHA0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	u32 white = 0xFFFFFFFF;
	GXSetChanMatColor(GX_COLOR0A0, *(GXColor*)&white);
	GXSetNumTexGens(2);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
	GXSetNumTevStages(1);
	GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	if ((mFlags & 1) != 0) {
		f32 warp[6];
		warp[1] = 0.0f;
		warp[0] = 0.5f;
		warp[2] = 0.0f;
		warp[3] = 0.0f;
		warp[4] = 0.5f;
		warp[5] = 0.0f;
		GXSetIndTexMtx(GX_ITM_0, warp, -2);
		GXSetNumIndStages(1);
		GXSetIndTexOrder(GX_IND_TEX_STAGE_0, GX_TEXCOORD1, GX_TEXMAP1);
		GXSetIndTexCoordScale(GX_IND_TEX_STAGE_0, GX_ITS_1, GX_ITS_1);
		GXSetTevIndWarp(GX_TEVSTAGE0, GX_IND_TEX_STAGE_0, GX_TRUE, GX_FALSE, GX_ITM_0);
	} else {
		GXSetNumIndStages(0);
	}
	GXSetZCompLoc(GX_TRUE);
	GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
	GXSetZMode(GX_ENABLE, GX_LESS, GX_ENABLE);
	GXSetCullMode(GX_CULL_BACK);
	volatile u16* fifo;
	int bottom;
	int top;
	fifo   = (volatile u16*)0xCC008000;
	top    = 0;
	bottom = (s16)mCols;
	for (int r = 0; r < mRows - 1; r++) {
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, (u16)(mCols * 2));
		for (int c = 0; c < mCols; c++) {
			*fifo = bottom;
			*fifo = bottom;
			bottom++;
			*fifo = top;
			*fifo = top;
			top++;
		}
	}
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetNumIndStages(0);
}

// 0x80087A8C
extern "C" void rippleMagma(const Vec3f* pos, f32 inner, f32 outer, f32 push)
{
	if (sBoss03MagmaObj != 0) {
		((TEneBoss03_Magma*)sBoss03MagmaObj)->push(pos, inner, outer, push);
	}
}

const char* gTEneBoss03_MagmaName = "TEneBoss03_Magma";

// 0x80087AC0
TEneBoss03_Magma::TEneBoss03_Magma(TObject* parent)
    : TObject(parent)
{
	_00   = (u32)gTEneBoss03_MagmaName;
	mSize = 0x48;
	if (sBoss03MagmaObj == 0) {
		sBoss03MagmaObj = this;
	}
	for (int i = 0; i < 9; i++) {
		mGrids[i] = 0;
	}
	Vec3f at;
	at.x      = 0.0f;
	at.y      = 180.0f;
	at.z      = 0.0f;
	mGrids[0] = new MagmaGrid(0x1A, 0x1A, &at, 1000.0f, 1000.0f);
	mGrids[0]->setWarp();
	at.z = 0.0f;
	at.x = 0.0f;
	at.y = 180.0f;
	at.z -= 750.0f;
	mGrids[1] = new MagmaGrid(0x1A, 6, &at, 1000.0f, 500.0f);
	at.z      = 0.0f;
	at.x      = 0.0f;
	at.y      = 180.0f;
	at.z += 750.0f;
	mGrids[2] = new MagmaGrid(0x1A, 6, &at, 1000.0f, 500.0f);
	at.x      = 0.0f;
	at.y      = 180.0f;
	at.z      = 0.0f;
	at.x -= 750.0f;
	mGrids[3] = new MagmaGrid(6, 0x1A, &at, 500.0f, 1000.0f);
	at.x      = 0.0f;
	at.y      = 180.0f;
	at.z      = 0.0f;
	at.x += 750.0f;
	mGrids[4] = new MagmaGrid(6, 0x1A, &at, 500.0f, 1000.0f);
	at.x      = 0.0f;
	at.z      = 0.0f;
	at.y      = 180.0f;
	at.x += 750.0f;
	at.z += 750.0f;
	mGrids[5] = new MagmaGrid(6, 6, &at, 500.0f, 500.0f);
	at.x      = 0.0f;
	at.z      = 0.0f;
	at.y      = 180.0f;
	at.x -= 750.0f;
	at.z += 750.0f;
	mGrids[6] = new MagmaGrid(6, 6, &at, 500.0f, 500.0f);
	at.x      = 0.0f;
	at.z      = 0.0f;
	at.y      = 180.0f;
	at.x += 750.0f;
	at.z -= 750.0f;
	mGrids[7] = new MagmaGrid(6, 6, &at, 500.0f, 500.0f);
	at.x      = 0.0f;
	at.z      = 0.0f;
	at.y      = 180.0f;
	at.x -= 750.0f;
	at.z -= 750.0f;
	mGrids[8] = new MagmaGrid(6, 6, &at, 500.0f, 500.0f);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
}

// 0x80087E3C
TEneBoss03_Magma::~TEneBoss03_Magma()
{
	removeDrawTask(mDrawTask);
	for (int i = 8; i >= 0; i--) {
		if (mGrids[i] != 0) {
			delete mGrids[i];
			mGrids[i] = 0;
		}
	}
	if (sBoss03MagmaObj == this) {
		sBoss03MagmaObj = 0;
	}
}

// 0x80087EEC
void TEneBoss03_Magma::push(const Vec3f* pos, f32 inner, f32 outer, f32 amount)
{
	for (int i = 0; i < 9; i++) {
		if (mGrids[i] != 0) {
			mGrids[i]->push(pos, inner, outer, amount);
		}
	}
}

// 0x80087F70
void TEneBoss03_Magma::draw()
{
	u8 now;
	int moved = 0;
	for (int i = 0; i < 9; i++) {
		if (mGrids[i] != 0) {
			now = 0;
			if (mGrids[i]->update() != 0 || moved != 0) {
				now = 1;
			}
			moved = now;
		}
	}
	if (moved != 0) {
		PPCSync();
	}
}

// 0x80087FF0
void TEneBoss03_Magma::vfn_10()
{
	for (int i = 0; i < 9; i++) {
		if (mGrids[i] != 0) {
			mGrids[i]->cull();
		}
	}
}

// 0x80088044
void TEneBoss03_Magma::render()
{
	for (int i = 0; i < 9; i++) {
		if (mGrids[i] != 0) {
			mGrids[i]->draw();
		}
	}
}
