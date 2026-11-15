#include "egg/EggCreature.h"
#include "stage/Stage.h"
#include "engine/DrawTask.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "object/TObjLauncher.h"
#include "camera/TCamPath.h"
#include "engine/DrawState.h"
#include "ui/AeArc.h"
#include "object/ObjShadow.h"
#include "egg/TEggCreatureTypeE.h"
#include "sound/Sound3DPlay.h"
#include "engine/GroundQuery.h"
#include "stl/math.h"
#include "object/TObjEggRing.h"
#include "engine/TParticle.h"
#include "camera/TCameraCtrl.h"
#include "engine/Color.h"
#include "animal/TAniKijiSai.h"
#include "animal/TAniSpiralFire.h"
#include "object/ObjClassName.h"

const ColliShape sSpiralFireShape = { 0 };

// One ring x y z u v and colour
const SpiralVtx sSpiralFireRing[8] = {
	{ 1.0f, 0.0f, 0.0f, 0.01f, 0.01f, 0xFFFFFFFF },
	{ 0.7071f, 0.0f, 0.7071f, 0.24f, 0.01f, 0xFFFFFFFF },
	{ 0.0f, 1.0f, 1.0f, 0.01f, 0.01f, 0xFFFFFFFF },
	{ -0.7071f, 1.0f, 0.7071f, 0.24f, 0.01f, 0xFFFFFFFF },
	{ -1.0f, 1.0f, 0.0f, 0.01f, 0.01f, 0xFFFFFFFF },
	{ -0.7071f, 1.0f, -0.7071f, 0.24f, 0.01f, 0xFFFFFFFF },
	{ 0.0f, 1.0f, -1.0f, 0.01f, 0.01f, 0xFFFFFFFF },
	{ 0.7071f, 1.0f, -0.7071f, 0.24f, 0.01f, 0xFFFFFFFF },
};

ObjClassName gTAniSpiralFireName = { "TAniSpiralFire", 0 };

// 0x80133420
TAniSpiralFire::TAniSpiralFire(TEggCreatureTypeE* owner, const Vec3f* pos, u16 ownerId)
    : TObject(owner)
    , mPos(*pos)
{
	_00       = (u32)gTAniSpiralFireName.mName;
	mSize     = 0x98;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mTexList  = owner->mTexList;
	mRadius   = sSpiralFireShape.mSizeX;
	mMid      = 0.0f;
	mInner    = 0.0f;
	_3C       = sSpiralFireShape.mSizeY;
	_44       = 6;
	_48       = 0;
	_4C       = 0;
	mFrame    = 0.0f;
	mGrowth   = 0.0f;
	mColor    = 0xFFFFFFFF;
	_58       = 0;
	_5C       = 0;
	mColli    = colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, (u16)ownerId);
		colliSetMasks((CharaColli*)mColli, 0x20, 0xFF, 0xFF, 0x10);
		colliCopyShapes((CharaColli*)mColli, &sSpiralFireShape);
		colliPlaceShapes((CharaColli*)mColli, &mPos, &sSpiralFireShape);
		colliShapeSetBit8((CharaColli*)mColli, 0);
	}
}

// 0x80133570
TAniSpiralFire::~TAniSpiralFire()
{
	colliFree((CharaColli*)mColli);
	removeDrawTask(mDrawTask);
}

// 0x801335E0
void TAniSpiralFire::draw()
{
	if (mFrame < 3.75f) {
		mFrame += 0.25f;
	} else {
		mFrame = 0.0f;
	}
	mGrowth += 0.15f;
	mRadius += mGrowth;
	mInner = 0.2f * mGrowth + mInner;
	mMid   = 0.5f * (mRadius + mInner);

	f32 r = mRadius;
	f32 size = r < 0.0f ? 0.0f : (r > 80.0f ? 80.0f : r);
	colliPlaceShapes(mColli, &mPos, &sSpiralFireShape);
	colliGetShapes(mColli)->mSizeX = size;

	if (mRadius > 60.0f) {
		if (mColor == 0) {
			mFlags |= 1;
		} else {
			mColor -= 0x11111111;
		}
	}
}

// 0x8013370C
void TAniSpiralFire::vfn_10()
{
	Vec3f view;
	if (cameraIsVisible(1000.0f, 50.0f, getPos(), &view) != 0) {
		mDrawTask->mSortKey = drawTaskDepthKey(view.z);
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80133790
void TAniSpiralFire::render()
{
	u32 state;
	SpiralVtx verts[36];
	f32 uShift = mFrame - (f32)(s16)(int)mFrame;
	f32 vShift = 0.25f * (f32)(s16)(int)mFrame;
	s16 i;
	s16 ring = 0;
	for (i = 0; i < 0x24; i += 2, ring++) {
		if (ring == 9) {
			ring = 0;
		}
		s16 entry = ring % 8;
		if (i < 0x12) {
			verts[i] = verts[i + 1] = sSpiralFireRing[entry];
			verts[i].x *= mRadius;
			verts[i].z *= mRadius;
			verts[i + 1].x *= mMid;
			verts[i + 1].z *= mMid;
			verts[i + 1].v = 0.24f;
		} else {
			verts[i] = verts[i + 1] = sSpiralFireRing[entry];
			verts[i].x *= mMid;
			verts[i].z *= mMid;
			verts[i].v = 0.24f;
			verts[i + 1].x *= mInner;
			verts[i + 1].z *= mInner;
		}
		verts[i].u += uShift;
		verts[i + 1].u += uShift;
		verts[i].v += vShift;
		verts[i + 1].v += vShift;
		verts[i].color     = mColor;
		verts[i + 1].color = mColor;
	}

	getCullMode((int*)&state);
	setCullMode(0);
	blendModeAdd();
	njSetTexture(mTexList);
	njSetTextureNum(0);
	njPushMatrixEx();
	njTranslateEx(&mPos);
	njDrawTexture3DEx(verts, 0x24, 1);
	gjRotateX(0x4000);
	njDrawTexture3DEx(verts, 0x24, 1);
	gjRotateZ(0x4000);
	njDrawTexture3DEx(verts, 0x24, 1);
	njPopMatrixEx();
	blendModeAlpha();
	setCullMode(state);
}

// 0x80133AF0
Vec3f* TAniSpiralFire::getPos()
{
	return &mPos;
}

// 0x80133AF8
void TAniSpiralFire::setPos(const Vec3f* pos)
{
	mPos = *pos;
}

// 0x80133B14
int TAniSpiralFire::vfn_24()
{
	return 4;
}
