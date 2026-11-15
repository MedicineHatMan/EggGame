#include "object/blue/TObjBlueFlagScene.h"
#include "engine/DrawTask.h"
#include "engine/FlagCloth.h"
#include "object/ObjScript.h"
#include "object/ObjArchive.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "enemy/ObjEnemy.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "ui/AeArc.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "core/VecMath.h"
#include "core/Mem.h"
#include "engine/LightSet.h"
struct DrawStateSlot;
struct DrawState;
struct NJS_TEXLIST;

extern "C" {

// 0x802C4130
ObjClassName gTObjBlueFlagSceneName = { "TObjBlueFlagScene", 0 };
}

typedef char TObjBlueFlagScene_size_check[(sizeof(TObjBlueFlagScene) == 0x524) ? 1 : -1];

// 0x800EC660
TObjBlueFlagScene::TObjBlueFlagScene(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjBlueFlagSceneName.mName;
	mSize = 0x524;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	TSetActor::initFromDesc(desc);

	ClothMatrix matrix;
	mtxSetTransV(matrix.m, &mPos);
	mtxRotateZYX(matrix.m, matrix.m, &mRot.x);
	mCloth   = new FlagCloth(matrix, 10, 8, 100.0f, 80.0f, 0.3f, 0.99f, 0.0f);
	mTexList = getGeObjTextures()[15];
	mAnchor  = -1;
	initFromDesc(desc);
	vfn_74();
}

// 0x800EC7AC
TObjBlueFlagScene::~TObjBlueFlagScene()
{
	delete mCloth;
	removeDrawTask(mDraw);
}

// 0x800EC834
void TObjBlueFlagScene::initFromDesc(SetDesc* desc)
{
	s16 fluct;
	TSetActor::initFromDesc(desc);
	if (mAnchor != desc->getInt(0)) {
		mAnchor = desc->getInt(0);
		ClothMatrix matrix;
		mtxSetTransV(matrix.m, &mPos);
		mtxRotateZYX(matrix.m, matrix.m, &mRot.x);
		mCloth->rebuild(&matrix);

		mCloth->mPinCount = 0;
		f32 cols          = mCloth->mCols;
		f32 rows          = mCloth->mRows;
		switch (mAnchor) {
		case 0:
			mCloth->pin(0, 0);
			mCloth->pin(cols - 1.0f, 0);
			mCloth->pin(cols - 1.0f, rows - 1.0f);
			mCloth->pin(0, rows - 1.0f);
			break;
		case 1:
			for (u16 x = 0; x < cols; x += 2) {
				mCloth->pin(x, rows - 1.0f);
			}
			break;
		case 2:
			for (u16 y = 0; y < rows; y += 2) {
				mCloth->pin(0, y);
			}
			break;
		case 3:
			for (u16 x = 0; x < cols; x += 2) {
				mCloth->pin(x, rows - 1.0f);
				mCloth->pin(x, 0);
			}
			break;
		case 4:
			for (u16 x = 0; x < cols; x += 2) {
				mCloth->pin(x, rows - 1.0f);
			}
			for (u16 y = 0; y < rows; y += 2) {
				mCloth->pin(0, y);
			}
			break;
		case 5:
			mCloth->pin(0, 0);
			mCloth->pin(0, rows - 1.0f);
			break;
		case 6:
		default:
			mCloth->pin(0, 0);
			mCloth->pin(0, rows - 1.0f);
			mCloth->pin(cols - 1.0f, rows - 1.0f);
			break;
		}
	}

	mTex = desc->getInt(1);
	mTex = (mTex < 0) ? 0 : (((u32)mTex > mTexList->nbTexture) ? mTexList->nbTexture : mTex);

	Vec3f wind;
	wind.set(0.0f, 0.0f, 1.0f);
	int yaw = (int)(182.04445f * (f32)desc->getInt(2));
	vecRotateY(&wind, &wind, yaw);
	mRef  = 0.5f + desc->getFloat(0);
	fluct = (s16)desc->getInt(3);
	mCloth->setWind(&wind, fluct, desc->getFloat(1));
	mCloth->mSpring = 0.3f + desc->getFloat(2);
	mCloth->mLoss   = 0.99f + desc->getFloat(3);
}

// 0x800ECD4C
void TObjBlueFlagScene::debugPrint(void*)
{
	njPrint(0xA0013, ":ANCHOR TYPE");
	njPrint(0xA0014, ":TEX TYPE");
	njPrint(0xA0015, ":WIND DIR");
	njPrint(0xA0016, ":WIND FLUCT");
	njPrint(0xA0017, "+0.5:REF COEF");
	njPrint(0xA0018, ":WIND SPD");
	njPrint(0xA0019, "+%f:K", 0.3f);
	njPrint(0xA001A, "+%f:E LOSS", 0.99f);
}

// 0x800ECE20
void TObjBlueFlagScene::vfn_74()
{
	FlagCloth* cloth = mCloth;
	cloth->mWind.set(0.0f, -0.01f, 0.0f);
}

// 0x800ECE3C
void TObjBlueFlagScene::draw()
{
	if (isInRange((void*)this, (const Vec3f*)&mPos, 1100.0f) == 0) {
		mFlags |= 1;
		return;
	}
	FlagCloth* cloth = mCloth;
	cloth->step();
	buildNormals();
}

// 0x800ECEA4
void TObjBlueFlagScene::vfn_10()
{
	if (cameraIsVisible(1000.0f, 1000.0f, &mPos, 0) != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// Same two-triangle quads as the flag on a 10 by 8 grid
// 0x800ECF0C
void TObjBlueFlagScene::buildNormals()
{
	Vec3f sum;
	Vec3f cur;
	Vec3f diag;
	Vec3f other;
	Vec3f toBelow;
	Vec3f toDiag;
	Vec3f toDiag2;
	Vec3f toRight;
	SpringPoint* pts = mCloth->mPoints;
	u16 cols         = mCloth->mCols;
	u16 rows         = mCloth->mRows;
	if (sFlagSceneHitVecsGuard == 0) {
		__construct_array(sFlagSceneHitVecs, (void*)__ct__6HitVecFv, 0, 0xC, 8 * 10 * 2);
		sFlagSceneHitVecsGuard = 1;
	}

	for (u16 y = 0; y < 8; y++) {
		int rowStart = y * cols;
		HitVec* row  = &sFlagSceneHitVecs[y * 10 * 2];
		for (u16 x = 0; x < 10; x++) {
			u16 face = x * 2;
			u16 i    = x + rowStart;
			cur.x    = pts[i].mPos.x;
			cur.y    = pts[i].mPos.y;
			cur.z    = pts[i].mPos.z;
			diag.x   = pts[i + cols + 1].mPos.x;
			diag.y   = pts[i + cols + 1].mPos.y;
			diag.z   = pts[i + cols + 1].mPos.z;
			other.x  = pts[i + cols].mPos.x;
			other.y  = pts[i + cols].mPos.y;
			other.z  = pts[i + cols].mPos.z;
			vecSub(&toDiag, &diag, &cur);
			vecSub(&toBelow, &other, &cur);
			njOuterProduct(&toBelow, &toDiag, &row[face]);
			other.x = pts[i + 1].mPos.x;
			other.y = pts[i + 1].mPos.y;
			other.z = pts[i + 1].mPos.z;
			vecSub(&toRight, &other, &cur);
			vecSub(&toDiag2, &diag, &cur);
			njOuterProduct(&toDiag2, &toRight, &row[face + 1]);
		}
	}

	int colsLast = cols - 1;
	int rowsLast = rows - 1;
	for (u16 y = 0; y < rows; y++) {
		HitVec* here = &sFlagSceneHitVecs[y * 10 * 2];
		HitVec* prev = &sFlagSceneHitVecs[(y - 1) * 10 * 2];
		for (u16 x = 0; x < cols; x++) {
			u16 face = x * 2;
			sum.set(0.0f, 0.0f, 0.0f);
			if (x < colsLast && y < rowsLast) {
				vecAdd(&sum, &sum, &here[face]);
				vecAdd(&sum, &sum, &here[face + 1]);
			}
			if (x != 0 && y != 0) {
				vecAdd(&sum, &sum, &prev[face - 1]);
				vecAdd(&sum, &sum, &prev[face - 2]);
			}
			if (x < colsLast && y != 0) {
				vecScaleAdd(2.0f, &sum, &prev[face], &sum);
			}
			if (x != 0 && y < rowsLast) {
				vecScaleAdd(2.0f, &sum, &here[face - 1], &sum);
			}
			vecNormalize(&sum, &sum);
			mNormal[y][x].set(sum.x, sum.y, sum.z);
		}
	}
}

// Loads an identity texture matrix which the plain flag does not
// 0x800ED24C
void TObjBlueFlagScene::render()
{
	u16 cols;
	SpringPoint* pts;
	u16 rows;
	pts  = mCloth->mPoints;
	cols = mCloth->mCols;
	rows = mCloth->mRows;
	lightSetSelect(0);
	beginDrawState(getDrawStateSlot(), getDefaultDrawState());
	GXSetZCompLoc(GX_FALSE);
	gjSetZBufferMode(1);
	gjGXSetBlendMode(0);
	GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0xFF);
	njPushMatrixEx();
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	if (mTex != 0) {
		njSetTexture(mTexList);
		gjLoadTextureNum(mTex - 1, 0, 0, 0);
	}
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	f32 texMtx[12];
	PSMTXIdentity(texMtx);
	GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX2x4);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	GXSetNumTexGens(1);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_ALPHA0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXColor white = { 0xFF, 0xFF, 0xFF, 0xFF };
	GXSetChanMatColor(GX_COLOR0, white);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	if (mTex != 0) {
		GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	} else {
		GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	}

	// The front then the back with the normals turned around
	GXSetCullMode(GX_CULL_BACK);
	int rowsLast = rows - 1;
	f32 du       = 1.0f / (f32)(cols - 1);
	f32 dv       = 1.0f / (f32)rowsLast;
	for (u16 y = 0; y < rowsLast; y++) {
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, cols * 2);
		f32 vTop = 1.0f - dv * (f32)y;
		f32 vBot = vTop - dv;
		for (u16 x = 0; x < cols; x++) {
			int hi = x + y * cols;
			int lo = hi + cols;
			f32 u  = du * (f32)x;
			GXPosition3f32(pts[lo].mPos.x, pts[lo].mPos.y, pts[lo].mPos.z);
			GXNormal3f32(mNormal[y + 1][x].x, mNormal[y + 1][x].y, mNormal[y + 1][x].z);
			GXTexCoord2f32(u, vBot);
			GXPosition3f32(pts[hi].mPos.x, pts[hi].mPos.y, pts[hi].mPos.z);
			GXNormal3f32(mNormal[y][x].x, mNormal[y][x].y, mNormal[y][x].z);
			GXTexCoord2f32(u, vTop);
		}
	}

	GXSetCullMode(GX_CULL_FRONT);
	for (u16 y = 0; y < rowsLast; y++) {
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, cols * 2);
		f32 vTop = 1.0f - dv * (f32)y;
		f32 vBot = vTop - dv;
		for (u16 x = 0; x < cols; x++) {
			int hi = x + y * cols;
			int lo = hi + cols;
			f32 u  = du * (f32)x;
			GXPosition3f32(pts[lo].mPos.x, pts[lo].mPos.y, pts[lo].mPos.z);
			GXNormal3f32(-mNormal[y + 1][x].x, -mNormal[y + 1][x].y, -mNormal[y + 1][x].z);
			GXTexCoord2f32(u, vBot);
			GXPosition3f32(pts[hi].mPos.x, pts[hi].mPos.y, pts[hi].mPos.z);
			GXNormal3f32(-mNormal[y][x].x, -mNormal[y][x].y, -mNormal[y][x].z);
			GXTexCoord2f32(u, vTop);
		}
	}
	njPopMatrixEx();
	GXSetZCompLoc(GX_TRUE);
	endDrawState(getDrawStateSlot());
}
