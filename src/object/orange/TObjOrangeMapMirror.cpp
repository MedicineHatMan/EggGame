#include "object/orange/TObjOrangeMapMirror.h"
#include "ui/TextVec.h"
#include "engine/DrawTask.h"
#include "engine/CharaColli.h"
#include "object/ObjArchive.h"
#include "stage/TStageOrange.h"
#include "engine/DrawState.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "camera/TCameraCtrl.h"
#include "core/VecMath.h"

const char* gTObjOrangeMapMirrorName = "TObjOrangeMapMirror";

static TextVec mirrorPlanePos[3] = {
	textVec(1588.5f, 0.0f, 0.0f),
	textVec(1980.0f, 0.0f, 0.0f),
	textVec(1858.0f, 0.0f, 0.0f),
};
static TextVec mirrorPlaneNormal[3] = {
	textVec(1.0f, 0.0f, 0.0f),
	textVec(1.0f, 0.0f, 0.0f),
	textVec(1.0f, 0.0f, 0.0f),
};
struct CharaColli;
struct CamView;

extern "C" {

extern const int sMapMirrorModels[] = {
	0,
	1,
	2,
};
extern const int sMapMirrorTextures[] = {
	0,
	1,
	2,
	0,
};
}

typedef char TObjOrangeMapMirror_size_check[(sizeof(TObjOrangeMapMirror) == 0x28) ? 1 : -1];
typedef char MirrorPiece_size_check[(sizeof(MirrorPiece) == 0x20) ? 1 : -1];

// 0x80123D54
extern "C" TObjOrangeMapMirror* createTObjOrangeMapMirror(TObject* parent)
{
	TObjOrangeMapMirror* obj;
	if (gMapMirrorObj == 0) {
		obj           = new TObjOrangeMapMirror(parent);
		gMapMirrorObj = obj;
		return obj;
	}
	return gMapMirrorObj;
}

// 0x80123DB8
// Ours Drops one mirror's sphere
void mapMirrorDropSphere(int index)
{
	if (gMapMirrorObj == 0 || index < 0 || index >= 3) {
		return;
	}
	if (gMapMirrorObj->mPieces[index].mShape != 0) {
		colliDisable(gMapMirrorObj->mPieces[index].mShape);
	}
}

// 0x80123E10
TObjOrangeMapMirror::TObjOrangeMapMirror(TObject* parent)
    : TObject(parent)
{
	_00     = (u32)gTObjOrangeMapMirrorName;
	mSize   = 0x28;
	mPieces = new MirrorPiece[3];
	for (int i = 0; i < 3; i++) {
		mPieces[i].mModel  = getGeObjModels()[sMapMirrorModels[i]];
		mPieces[i].mTex    = getGeObjTextures()[sMapMirrorTextures[i]];
		mPieces[i].mCenter = *(Vec3f*)&mPieces[i].mModel->model->center;
		mPieces[i].mRadius = mPieces[i].mModel->model->r;
	}
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 10, 0);
	for (int i = 0; i < 3; i++) {
		mPieces[i].mShape = colliCreate(1, 1);
		colliSetOwner(mPieces[i].mShape, 0xFFFF);
		colliSetMasks(mPieces[i].mShape, 4, 0xFF, 0xFE, 0xFE);

		ColliShape shape;
		colliShapeMakeSphere(&shape, &mPieces[i].mCenter, 0, mPieces[i].mRadius);
		shape.mFlags |= 0x10;
		colliCopyShapes(mPieces[i].mShape, &shape);
	}
}

// 0x80123FE0
MirrorPiece::MirrorPiece()
{
}

// 0x80123FE4
// Only pieces 1 and 0 have their collision retired
TObjOrangeMapMirror::~TObjOrangeMapMirror()
{
	int i = 1;
	do {
		colliRetire(mPieces[i].mShape);
	} while (i-- != 0);
	removeDrawTask(mDrawTask);
	delete[] mPieces;
	if (this == gMapMirrorObj) {
		gMapMirrorObj = 0;
	}
}

// 0x8012409C
void TObjOrangeMapMirror::draw()
{
	for (int i = 0; i < 3; i++) {
		if (mPieces[i].mShape->mContacts != 0) {
			mirrorSetPlane(&mirrorPlanePos[i], &mirrorPlaneNormal[i]);
			mirrorRequestCopy();
			return;
		}
	}
}

// 0x80124114
void TObjOrangeMapMirror::vfn_10()
{
	Vec3f* from = (Vec3f*)camGetDrawView();
	f32 margin  = 200.0f;
	for (int i = 0; i < 3; i++) {
		Vec3f gap;
		vecSub(&gap, &mPieces[i].mCenter, from);
		f32 dist = njScalor(&gap);
		if (dist < margin + mPieces[i].mRadius) {
			mPieces[i].mShow = 1;
		} else {
			mPieces[i].mShow = 0;
		}
	}
}

// 0x801241BC
void TObjOrangeMapMirror::render()
{
	for (int i = 0; i < 3; i++) {
		if (mPieces[i].mShow != 0) {
			njSetTexture(mPieces[i].mTex);
			drawObjectDefault(mPieces[i].mModel);
		}
	}
}

static inline Vec3f makeVec(f32 x, f32 y, f32 z)
{
	Vec3f v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

// Where each of the three mirrors sits and which way it faces as draw hands them to mirrorSetPlane
