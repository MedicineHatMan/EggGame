#include "stage/SetManager.h"
#include "object/orange/TObjOrangePanelDeOpen.h"
#include "engine/DrawTask.h"
#include "object/ObjScript.h"
#include "engine/CharaColli.h"
#include "Ninja/ninja.h"
#include "object/ObjArchive.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "camera/TCamPath.h"
#include "core/MathUtil.h"
#include "engine/TParticle.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "object/TObjKey00.h"
#include "stage/SetSlot.h"
#include "core/VecMath.h"
#include "sound/SoundCtrl.h"
struct SetLink;
struct MeshColli;
struct MeshData;
struct MeshObject;
struct ColliContact;
struct EnemyLight;
struct DrawState;

static inline Vec3f colliNeg(const Vec3f& v)
{
	Vec3f r;
	r.x = -v.x;
	r.y = -v.y;
	r.z = -v.z;
	return r;
}

const f32 gOrangePanelDeOpenSetRange = 1200.0f;
const char* sPanelAnswerNames[4]     = { "SPADE", "HEART", "DIAMOND", "CLOVER" };

extern "C" {

// 0x802C42D0
ObjClassName gTObjOrangePanelDeOpenName = { "TObjOrangePanelDeOpen", 0 };
}

struct TileVert {
	f32 x, y, z, u, v;
	u32 mColor;

	TileVert& operator=(const TileVert& o)
	{
		x      = o.x;
		y      = o.y;
		z      = o.z;
		u      = o.u;
		v      = o.v;
		mColor = o.mColor;
		return *this;
	}
};

struct TileQuad {
	TileVert mV[4];
};

struct TileContact {
	TileContact* mNext; // _00
	u8 _04[0x14];
	int mIndex; // _18
};

static void addTilePos(f32* dst, const f32* src);

typedef char TObjOrangePanelDeOpen_size_check[(sizeof(TObjOrangePanelDeOpen) == 0x2DC) ? 1 : -1];
typedef char PanelTile_size_check[(sizeof(PanelTile) == 0x10) ? 1 : -1];

// 0x8011F780
extern "C" TObject* createTObjOrangePanelDeOpen(TObject* parent, SetDesc* desc)
{
	return new TObjOrangePanelDeOpen(parent, desc);
}

// 0x8011F7D4
TObjOrangePanelDeOpen::TObjOrangePanelDeOpen(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00        = (u32)gTObjOrangePanelDeOpenName.mName;
	mSize      = 0x2DC;
	mModelData = findGeObjModel("MODEL_PANEL_FLOOR");
	mTexFloor  = findGeObjData("TLS_MODEL_PANEL_FLOOR");
	mTexTiles  = findGeObjData("TLS_MODEL_PANEL");
	mColliData = (MeshData*)findGeObjData("COL_MODEL_PANEL_FLOOR");
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	readTiles();
	makeShapes();
	initColli();
	initFromDesc(desc);
	vfn_74();
}

// 0x8011F8F4
PanelTile::PanelTile()
{
}

// 0x8011F8F8
TObjOrangePanelDeOpen::~TObjOrangePanelDeOpen()
{
	freeColli();
	freeShapes();
	removeDrawTask(mDrawTask);
}

// 0x8011F984
void TObjOrangePanelDeOpen::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mSwitch = (s16)desc->getInt(0);

	mAnswer = (u8)CLAMP_RANGE(desc->getInt(1), 0, 3);

	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);

	mCenter = *(Vec3f*)&mModelData->model->center;
	mtxMultVec(mMatrix, &mCenter, &mCenter);
	mRadius = mModelData->model->r;
	updateShapes();
	updateColli();
}

// 0x8011FAA8
void TObjOrangePanelDeOpen::debugPrint(void*)
{
	njPrintC(0xA0013, ":SW ID");
	njPrint(0xA0014, ":ANS %s", sPanelAnswerNames[mAnswer]);
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x8011FB68
void TObjOrangePanelDeOpen::vfn_74()
{
	mModel.setModel(mModelData);
	mModel.mTexList = (NJS_TEXLIST*)mTexFloor;
	mModel.mState   = &mState;
	setLinkFree((SetLink*)&mLink);
	*(u32*)((u8*)&mLight + 0x3C) = 0;

	if (keyBitIsSet(mSwitch) == 1) {
		for (int i = 0; i < 25; i++) {
			mTiles[i].mSuit = mAnswer;
		}
		colliDisable(mShape);
		colliDisable(mTileColli);
		mAct = 1;
	} else {
		mAct = 0;
	}
}

// 0x8011FC10
// 0.5f is 0.5 3.0f is 3 3.0517578e-05f is 1/32768
void TObjOrangePanelDeOpen::readTiles()
{
	int count = 0;
	for (NJS_OBJECT* node = mModelData->child; node != 0; node = node->sibling) {
		mTiles[count].mPos.set(node->pos[0], 0.5f - node->pos[1], node->pos[2]);
		mTiles[count].mSuit    = (u8)(int)(3.0f * (3.0517578e-05f * (f32)rand()) + 0.5f);
		mTiles[count].mPressed = 0;
		count++;
		if (count == 25) {
			break;
		}
	}
}

// 0x8011FD14
void TObjOrangePanelDeOpen::makeShapes()
{
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xF7, 0xF7);
	mTileColli = colliCreate(25, 1);
	colliSetOwner(mTileColli, (u16)mObjId);
	colliSetMasks(mTileColli, 4, 0xFF, 0xF7, 0xF7);
}

// 0x8011FDA4
// The body box is 168.5f by 15.0f by 168.5f lifted by 5.0f
// Each tile is 25.0f by 10.0f by 25.0f
void TObjOrangePanelDeOpen::updateShapes()
{
	Vec3f pos = mPos;
	pos.y += 5.0f;

	ColliShape body;
	colliShapeMakeBox(&body, &pos, mRot.y, 168.5f, 15.0f, 168.5f, 0);
	body.mFlags |= 0x10;
	colliCopyShapes(mShape, &body);

	ColliShape tiles[25];
	for (int i = 0; i < 25; i++) {
		Vec3f at;
		vecRotateY(&mTiles[i].mPos, &at, mRot.y);
		vecAdd(&at, &at, &mPos);
		colliShapeMakeBox(&tiles[i], &at, mRot.y, 25.0f, 10.0f, 25.0f, 0);
		tiles[i].mFlags |= 0x10;
	}
	colliCopyShapes(mTileColli, tiles);
}

// 0x8011FEB0
void TObjOrangePanelDeOpen::freeShapes()
{
	colliRetire(mTileColli);
	colliRetire(mShape);
	mShape = 0;
}

// 0x8011FEF0
// 1.0f is the unit half-extent scaled by the model radius
void TObjOrangePanelDeOpen::setColliBox(void* colli, const Vec3f* center, f32 radius)
{
	Vec3f size;
	size.x = 1.0f;
	size.y = 1.0f;
	size.z = 1.0f;
	gjScaleVector(radius, &size, &size);

	Vec3f high;
	high.x = size.x;
	high.y = size.y;
	high.z = size.z;
	Vec3f low;
	Vec3f t = colliNeg(size);
	low.x   = t.x;
	low.y   = t.y;
	low.z   = t.z;

	gjPushUnitMatrix();
	njTranslateEx(center);
	njCalcPointEx(&high, &high);
	njCalcPointEx(&low, &low);
	njPopMatrixEx();

	f32 box[6];
	boxAroundLine(box, &low, &high);
	meshColliSetBox((MeshObject*)colli, (const f32*)box);
}

// 0x8011FFDC
void TObjOrangePanelDeOpen::initColli()
{
	mMesh = (MeshColli*)meshColliCreate();
	meshColliSetData((MeshObject*)mMesh, mColliData);
}

// 0x80120018
void TObjOrangePanelDeOpen::updateColli()
{
	setColliBox(mMesh, &mCenter, mRadius);
	meshColliSetMatrix(mMesh, (const f32*)mMatrix);
}

// 0x8012005C
void TObjOrangePanelDeOpen::freeColli()
{
	meshColliFree(mMesh);
}

// 0x80120080
int TObjOrangePanelDeOpen::suitsMatch()
{
	for (u16 i = 0; i < 25; i++) {
		if (mTiles[i].mSuit != mAnswer) {
			return 0;
		}
	}
	return 1;
}

// 0x801200C0
// 1250.0f is how far the panel stays alive
void TObjOrangePanelDeOpen::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (!isInRange(this, &mPos, 1250.0f)) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
		statePlay();
		break;
	case 1:
		stateDone();
		break;
	}
}

// 0x8012014C
void TObjOrangePanelDeOpen::statePlay()
{
	if (mShape->mContacts == 0) {
		colliDisable(mTileColli);
		return;
	}

	colliEnable(mTileColli);
	u8 down[25];
	syMemSet(down, 0, 25);

	for (TileContact* hit = (TileContact*)mTileColli->mContacts; hit != 0; hit = hit->mNext) {
		if (mTiles[hit->mIndex].mSuit != mAnswer) {
			if (colliContactShape((ColliContact*)hit)->mFlags & 0x400) {
				if (findObjById(colliContactOwner((ColliContact*)hit)) != 0) {
					if (mTiles[hit->mIndex].mPressed != 0) {
						down[hit->mIndex] = 1;
					} else {
						mTiles[hit->mIndex].mSuit++;
						mTiles[hit->mIndex].mSuit &= 3;
						mTiles[hit->mIndex].mPressed = 1;
						ColliShape* sh               = colliGetShape(mTileColli, hit->mIndex);
						Vec3f pos;
						pos.set(sh->mPos.x, sh->mPos.y, sh->mPos.z);
						particleOnceById(this, &pos, 0, 0xFD);
						sh = colliGetShape(mTileColli, hit->mIndex);
						Vec3f pos2;
						pos2.set(sh->mPos.x, sh->mPos.y, sh->mPos.z);
						particleOnceById(this, &pos2, 0, 0xF9);
						sePlay(0x24D);
						down[hit->mIndex] = 1;
					}
				}
			}
		}
	}

	for (u16 i = 0; i < 25; i++) {
		mTiles[i].mPressed = down[i];
	}

	if (suitsMatch() != 0) {
		colliDisable(mShape);
		colliDisable(mTileColli);
		setKeyBit(mSwitch);
		mAct = 1;
	}
}

// 0x80120350
void TObjOrangePanelDeOpen::stateDone()
{
}

// 0x80120354
// 1100.0f is the light's far distance
void TObjOrangePanelDeOpen::vfn_10()
{
	int visible = mLight.isVisible(mDrawTask, 1100.0f, &mCenter, mRadius, 0);
	if (visible != 0) {
		setDrawAmbient(&mLight, &mState);
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// The four corners of a tile, white
struct TileQuadData {
	f32 x, y, z, u, v;
	u32 mColor;
};
extern "C" const TileQuadData sPanelTileQuad[4] = {
	{ -25.0f, 0.0f, 25.0f, 0.0f, 0.0f, 0xFFFFFFFF },
	{ -25.0f, 0.0f, -25.0f, 0.0f, 1.0f, 0xFFFFFFFF },
	{ 25.0f, 0.0f, 25.0f, 1.0f, 0.0f, 0xFFFFFFFF },
	{ 25.0f, 0.0f, -25.0f, 1.0f, 1.0f, 0xFFFFFFFF },
};

// 0x801203D0
static void buildTileQuad(TileQuad* quad, const f32* pos)
{
	*(TileQuad*)quad = *(TileQuad*)&sPanelTileQuad;
	addTilePos(&quad->mV[0].x, pos);
	addTilePos(&quad->mV[1].x, pos);
	addTilePos(&quad->mV[2].x, pos);
	addTilePos(&quad->mV[3].x, pos);
}

// 0x801204E8
void TObjOrangePanelDeOpen::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel.draw();
	njSetTexture(mTexTiles);

	u32 color;
	if ((int)mState.mFlags == 0x200) {
		color = (mState.mAmbient << 24) | 0xFFFFFF;
		njDrawTexture3DExStart(1);
	} else {
		color = 0xFFFFFFFF;
		njDrawTexture3DExStart(0);
	}

	for (int i = 0; i < 25; i++) {
		TileQuad quad;
		buildTileQuad(&quad, (const f32*)&mTiles[i].mPos);
		quad.mV[0].mColor = color;
		quad.mV[1].mColor = color;
		quad.mV[2].mColor = color;
		quad.mV[3].mColor = color;
		gjLoadTextureNum(mTiles[i].mSuit, 0, 0, 0);
		njDrawTexture3DExSetData((const NJS_TEXTURE_VTX*)&quad, 4);
	}
	njDrawTexture3DExEnd();
	njPopMatrixEx();

	if (objDebugEnabled(this) != 0) {
		colliDrawShapes(mTileColli);
	}
}

// 0x801205DC
static void addTilePos(f32* dst, const f32* src)
{
	dst[0] += src[0];
	dst[1] += src[1];
	dst[2] += src[2];
}
