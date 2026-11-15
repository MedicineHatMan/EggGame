#include "object/blue/TObjBlueFlag.h"
#include "sdk/stl/math.h"
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
#include "object/ObjAction.h"
#include "core/MathUtil.h"
#include "sound/Sound3DPlay.h"
#include "Ninja/ninja.h"
#include "ui/AeArc.h"
#include "core/Pad.h"
#include "engine/Colli.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "core/VecMath.h"
#include "core/Mem.h"
#include "engine/LightSet.h"
struct ColliContact;
struct ColliShape;
struct DrawStateSlot;
struct DrawState;
struct CharaColli;
struct NJS_TEXLIST;

// 0x800EB13C
extern "C" void initBlueFlag()
{
}

// 0x800EB140
extern "C" void termBlueFlag()
{
}

// 0x800EB144
extern "C" void initBlueFlagSet()
{
}

// 0x800EB148
extern "C" void termBlueFlagSet()
{
}

// 0x800EB14C
extern "C" TObject* createTObjBlueFlag(TObject* parent, SetDesc* desc)
{
	return new TObjBlueFlag(parent, desc);
}

// 0x800EB1A0
extern "C" TObject* createTObjBlueFlagScene(TObject* parent, SetDesc* desc)
{
	return new TObjBlueFlagScene(parent, desc);
}

extern "C" {

// 0x802C4128
ObjClassName gTObjBlueFlagName = { "TObjBlueFlag", 0 };
}

typedef char TObjBlueFlag_size_check[(sizeof(TObjBlueFlag) == 0xA5C) ? 1 : -1];

// 0x800EB1F4
TObjBlueFlag::TObjBlueFlag(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00      = (u32)gTObjBlueFlagName.mName;
	mSize    = 0xA5C;
	mTexList = getGeObjTextures()[15];
	mDraw    = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	makeShape();
	TSetActor::initFromDesc(desc);

	ClothMatrix matrix;
	mtxSetTransV(matrix.m, &mPos);
	mtxRotateZYX(matrix.m, matrix.m, &mRot.x);
	mCloth  = new FlagCloth(matrix, 15, 12, 100.0f, 80.0f, 0.3f, 0.99f, 0.0f);
	mAnchor = -1;
	initFromDesc(desc);
	vfn_74();
}

// 0x800EB364
TObjBlueFlag::~TObjBlueFlag()
{
	delete mCloth;
	freeShape();
	removeDrawTask(mDraw);
}

// Pins the cloth again only when the anchor kind changes
// 0x800EB3F4
void TObjBlueFlag::initFromDesc(SetDesc* desc)
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
	placeShape();
}

// 0x800EB914
void TObjBlueFlag::debugPrint(void*)
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

// 0x800EB9E8
void TObjBlueFlag::vfn_74()
{
	mTouch[0].set(0.0f, 0.0f, 0.0f);
	mTouch[1].set(0.0f, 0.0f, 0.0f);
	mWasClear = 0;
}

// Out of range the object asks to be deleted
// 0x800EBA10
void TObjBlueFlag::draw()
{
	if (isInRange((void*)this, (const Vec3f*)&mPos, 1100.0f) == 0) {
		mFlags |= 1;
		return;
	}

	FlagCloth* cloth  = mCloth;
	cloth->mHit       = 0;
	ColliContact* hit = (ColliContact*)mShape->mContacts;
	TObjAction* other = 0;
	if (hit == 0) {
		mWasClear = 1;
	} else {
		for (; hit != 0; hit = *(ColliContact**)hit) {
			ColliShape* shape = colliContactShape(hit);
			if ((shape->mFlags & 0x10) != 0) {
				continue;
			}
			other = findObjById((u16)colliContactOwner(hit));
			if (other == 0) {
				continue;
			}
			f32 radius = shape->mSizeX;
			if (radius < 15.0f) {
				radius = 15.0f;
			}
			Vec3f* pos  = other->getPos();
			mTouch[0].x = pos->x;
			mTouch[0].y = pos->y;
			mTouch[0].z = pos->z;
			Vec3f delta;
			vecSub(&delta, &mTouch[0], &mTouch[1]);
			mTouch[1] = mTouch[0];
			cloth->setSphere(pos, &delta, radius);
			cloth->mHit = 1;
			break;
		}
	}

	cloth->step();
	if (other != 0 && cloth->mReact != 0) {
		struct VelMsg {
			u16 mType;
			u16 _02;
			Vec3f mVel;
		};
		VelMsg got;
		got.mType = 0x43;
		other->sendToHandle((ObjMsg*)&got);

		VelMsg push;
		push.mType  = 1;
		f32 scale   = 60.0f * mRef;
		push.mVel.x = -got.mVel.x * scale;
		push.mVel.z = -got.mVel.z * scale;
		other->sendToHandle((ObjMsg*)&push);
		if (mWasClear != 0) {
			sePlay3D(0x209, other->getPos());
			padVibStart(0, (int)this, 0xFF, 0, 0x22);
			mWasClear = 0;
		}
	}
	buildNormals();
}

// 0x800EBC68
void TObjBlueFlag::makeShape()
{
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xFF, 0xF7);
}

// 0x800EBCC4
void TObjBlueFlag::placeShape()
{
	// The root of 16400 is worked out and thrown away
	dolsqrtf(16400.0f);
	colliShapeMakeCylinder(colliGetShapes(mShape), &mPos, 1, 120.00001f, 48.0f);
}

// 0x800EBD54
void TObjBlueFlag::freeShape()
{
	colliRetire(mShape);
	mShape = 0;
}

// 0x800EBD8C
void TObjBlueFlag::vfn_10()
{
	ColliShape* shape = colliGetShape(mShape, 0);
	f32 radius        = shape->mSizeX;
	if (cameraIsVisible(1000.0f, radius, &mPos, 0) != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// Two triangles per quad into the scratch at sFlagHitVecs then one normal per point
// 15 and 12 are the quad counts
// 0x800EBE00
void TObjBlueFlag::buildNormals()
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
	if (sFlagHitVecsGuard == 0) {
		__construct_array(sFlagHitVecs, (void*)__ct__6HitVecFv, 0, 0xC, 12 * 15 * 2);
		sFlagHitVecsGuard = 1;
	}

	for (u16 y = 0; y < 12; y++) {
		int rowStart = y * cols;
		HitVec* row  = &sFlagHitVecs[y * 15 * 2];
		for (u16 x = 0; x < 15; x++) {
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
		HitVec* here = &sFlagHitVecs[y * 15 * 2];
		HitVec* prev = &sFlagHitVecs[(y - 1) * 15 * 2];
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

// The front is culled one way and the back the other with the normals flipped
// 0x800EC140
void TObjBlueFlag::render()
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
	GXSetNumTexGens(1);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
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
		for (u16 x = 0; x < cols; x++) {
			int hi = x + y * cols;
			int lo = hi + cols;
			f32 u  = du * (f32)x;
			GXPosition3f32(pts[lo].mPos.x, pts[lo].mPos.y, pts[lo].mPos.z);
			GXNormal3f32(mNormal[y + 1][x].x, mNormal[y + 1][x].y, mNormal[y + 1][x].z);
			GXTexCoord2f32(u, vTop - dv);
			GXPosition3f32(pts[hi].mPos.x, pts[hi].mPos.y, pts[hi].mPos.z);
			GXNormal3f32(mNormal[y][x].x, mNormal[y][x].y, mNormal[y][x].z);
			GXTexCoord2f32(u, vTop);
		}
	}

	GXSetCullMode(GX_CULL_FRONT);
	for (u16 y = 0; y < rowsLast; y++) {
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, cols * 2);
		f32 vTop = 1.0f - dv * (f32)y;
		for (u16 x = 0; x < cols; x++) {
			int hi = x + y * cols;
			int lo = hi + cols;
			f32 u  = du * (f32)x;
			GXPosition3f32(pts[lo].mPos.x, pts[lo].mPos.y, pts[lo].mPos.z);
			GXNormal3f32(-mNormal[y + 1][x].x, -mNormal[y + 1][x].y, -mNormal[y + 1][x].z);
			GXTexCoord2f32(u, vTop - dv);
			GXPosition3f32(pts[hi].mPos.x, pts[hi].mPos.y, pts[hi].mPos.z);
			GXNormal3f32(-mNormal[y][x].x, -mNormal[y][x].y, -mNormal[y][x].z);
			GXTexCoord2f32(u, vTop);
		}
	}
	njPopMatrixEx();
	GXSetZCompLoc(GX_TRUE);
	endDrawState(getDrawStateSlot());
}
