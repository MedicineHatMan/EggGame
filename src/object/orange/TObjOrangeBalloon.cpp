#include "object/orange/TObjOrangeBalloon.h"
#include "engine/DrawTask.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "egg/EggCreature.h"
#include "core/Mem.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "core/TSceneManager.h"
#include "core/TMainTask.h"
#include "io/Resource.h"
#include "object/ObjAction.h"
#include "object/purple/TObjPurpleIceTrap.h"
struct SceneRecord;

typedef char TObjOrangeBalloon_size_check[(sizeof(TObjOrangeBalloon) == 0x40) ? 1 : -1];
typedef char BalloonPuff_size_check[(sizeof(BalloonPuff) == 0x14) ? 1 : -1];

static void onBalloonCell(u16 cell, void* self);

// 0x802C9FA8
// First in this file's pool
__declspec(section ".sdata2") f32 gBalloonRandOffset = 0.5f;

// 0x80126B98
extern "C" void loadBalloonArc()
{
	loadArc("balloon.arc", &sBalloonArc);
	if (sBalloonArc != 0) {
		gBalloonModels   = (NJS_OBJECT**)findArcExport(sBalloonArc, "models");
		gBalloonTexLists = (NJS_TEXLIST**)findArcExport(sBalloonArc, "texlists");
		void** textures  = (void**)findArcExport(sBalloonArc, "textures");
		for (int i = 0; i < 8; i++) {
			bindGvmTextures((u8*)textures[0], gBalloonTexLists[i]);
		}
		gBalloonPosExport = findArcExport(sBalloonArc, "pos");
	}
	sBalloonObj = 0;
}

// 0x80126C48
extern "C" void releaseBalloonArc()
{
	if (gBalloonTexLists != 0) {
		for (int i = 0; i < 8; i++) {
			releaseTextures(gBalloonTexLists[i]);
		}
	}
	releaseResource(sBalloonArc);
	sBalloonArc       = 0;
	gBalloonModels    = 0;
	gBalloonTexLists  = 0;
	gBalloonPosExport = 0;
}

// 0x80126CB8
extern "C" void makeOrangeBalloon()
{
	sBalloonObj = new TObjOrangeBalloon(gTL_06);
}

const char* gTObjOrangeBalloonName = "TObjOrangeBalloon";

// 0x80126CF8
TObjOrangeBalloon::TObjOrangeBalloon(TObject* parent)
    : TObject(parent)
{
	_00       = (u32)gTObjOrangeBalloonName;
	mSize     = 0x40;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mPool     = new BalloonPuff[400];
	for (int i = 0; i < 399; i++) {
		mPool[i].mNext = &mPool[i + 1];
	}
	mPool[399].mNext = 0;
	mFree            = mPool;
	mActive          = 0;
	mCount           = 0;
	if (gBalloonPosExport == 0) {
		mGrid = 0;
	} else {
		mGrid = (u8*)memAlloc(*(u32*)((u8*)gBalloonPosExport + 4));
		syMemSet(mGrid, 0, *(u32*)((u8*)gBalloonPosExport + 4));
	}
	mTimer = 1;
}

// 0x80126E0C
BalloonPuff::BalloonPuff()
{
}

// 0x80126E10
TObjOrangeBalloon::~TObjOrangeBalloon()
{
	if (mGrid != 0) {
		memFree(mGrid);
	}
	delete[] mPool;
	removeDrawTask(mDrawTask);
}

// 0x80126E9C
static void onBalloonCell(u16 cell, void* self)
{
	((TObjOrangeBalloon*)self)->spawn(cell);
}

// 0x80126EC8
// 1000.0f pads the camera box
void TObjOrangeBalloon::draw()
{
	Vec3f cam;
	camGetPos(0, &cam);
	f32 pad = 1000.0f;
	f32 box[6];
	box[0] = cam.x - pad;
	box[1] = cam.y - pad;
	box[2] = cam.z - pad;
	box[3] = cam.x + pad;
	box[4] = cam.y + 100.0f;
	box[5] = cam.z + pad;

	if (gBalloonPosExport != 0 && mTimer != 0) {
		if (--mTimer == 0) {
			mTimer = 0x3C;
			queryTrapNodes((TrapTree*)gBalloonPosExport, (TrapBox*)box, (TrapHitFn)onBalloonCell, this);
		}
	}

	f32 margin = 50.0f;
	box[0] -= margin;
	box[1] -= margin;
	box[2] -= margin;
	box[3] += margin;
	box[4] += margin;
	box[5] += margin;
	BalloonPuff* puff;
	BalloonPuff* next;
	BalloonPuff* prev;
	prev = 0;
	puff = mActive;
	while (puff != 0) {
		next = puff->mNext;
		puff->mY += 0.8333334f;
		if (puff->mX < box[0] || puff->mY < box[1] || puff->mZ < box[2] || puff->mX > box[3] || puff->mY > box[4] || puff->mZ > box[5]) {
			if (prev != 0) {
				prev->mNext = next;
			} else {
				mActive = next;
			}
			mCount--;
			puff->mNext = mFree;
			mFree       = puff;
			mGrid[puff->mCell]--;
		} else {
			prev = puff;
		}
		puff = next;
	}
}

// 0x80127098
void TObjOrangeBalloon::bindTex(int kind)
{
	if (sBalloonLastKind != kind) {
		void** lists = (void**)gBalloonTexLists;
		njSetTexture(lists[kind]);
		gjLoadTextureNum(0, 1, 1, 0);
		sBalloonLastKind = kind;
	}
}

// 0x801270F4
void TObjOrangeBalloon::bindShape(int lod)
{
	void** models = (void**)gBalloonModels;
	u8* model     = (u8*)models[lod];
	int* node     = *(int**)(model + 4);
	if (sBalloonLastLod != lod) {
		u32 bits = *(u32*)(*(int*)node[2] + 0x14);
		GXClearVtxDesc();
		GXSetVtxDesc(9, (bits >> 2) & 3);
		GXSetVtxDesc(10, (bits >> 4) & 3);
		GXSetVtxDesc(0xD, (bits >> 10) & 3);
		GXSetArray(9, *(void**)(*node + 8), *(u8*)(*node + 1));
		GXSetArray(10, *(void**)(*node + 0x18), *(u8*)(*node + 0x11));
		GXSetArray(0xD, *(void**)(*node + 0x28), *(u8*)(*node + 0x21));
		sBalloonLastLod = lod;
	}
	GXCallDisplayList(*(void**)(node[2] + 8), (unsigned long)(*(void**)(node[2] + 0xC)));
}

// 0x802C9FC8
// white
const u32 sOrangeBalloonClearColor = 0xFFFFFFFF;

// 0x801271BC
void TObjOrangeBalloon::render()
{
	f32 view[12];
	njGetMatrix(view);
	GXSetVtxAttrFmt(0, 9, 1, 4, 0);
	GXSetVtxAttrFmt(0, 10, 0, 4, 0);
	GXSetVtxAttrFmt(0, 0xB, 1, 5, 0);
	GXSetVtxAttrFmt(0, 0xD, 1, 3, 8);
	GXSetNumTevStages(1);
	GXSetTevOp(0, 0);
	GXSetTevOrder(0, 0, 0, 4);
	GXSetNumTexGens(1);
	GXSetTexCoordGen2(0, 0, 4, 0x3C, 0, 0x7D);
	GXSetBlendMode(0, 0, 0, 0);

	u8* light = (u8*)getSceneRecord();
	int chan  = *(int*)(light + 0xC);
	GXSetNumChans(1);
	GXSetChanMatColor(4, *(GXColor*)&sOrangeBalloonClearColor);
	GXSetChanCtrl(0, 1, 0, 0, *(int*)(chan + 0xC), 2, 1);
	GXSetChanCtrl(2, 0, 0, 0, 0, 2, 2);
	GXSetZCompLoc(1);
	gjSetZBufferMode(1);
	GXSetCullMode(2);

	sBalloonLastLod  = -1;
	sBalloonLastKind = -1;

	u32 batch[10][2];
	BalloonPuff* puff;
	int count;
	u32 verts;
	count = 0;
	verts = 0;
	for (puff = mActive; puff != 0; puff = puff->mNext) {
		f32 mtx[12];
		mtxTranslateV(mtx, view, (Vec3f*)&puff->mX);
		f32 depth = mtx[11];
		if (depth > -10.0f) {
			continue;
		}
		u32 lod;
		if (depth < -700.0f) {
			lod = 0;
		} else if (depth > -100.0f) {
			lod = 6;
		} else {
			lod = 6 - (int)(0.01f * (-100.0f - depth));
		}
		GXLoadPosMtxImm(mtx, verts);
		GXLoadNrmMtxImm(mtx, verts);
		batch[count][0] = lod;
		batch[count][1] = puff->mKind;
		count++;
		verts += 3;
		if (count == 10) {
			u32* entry = batch[0];
			for (u32 id = 0; id != verts; id += 3, entry += 2) {
				GXSetCurrentMtx(id);
				bindTex((int)entry[1]);
				bindShape((int)entry[0]);
			}
			count = 0;
			verts = 0;
		}
	}
	if (count != 0) {
		u32* entry = batch[0];
		for (u32 id = 0; id != verts; id += 3, entry += 2) {
			GXSetCurrentMtx(id);
			bindTex((int)entry[1]);
			bindShape((int)entry[0]);
		}
	}
	GXSetCurrentMtx(0);
}

// 0x80127484
// A cell refuses an 8th balloon
void TObjOrangeBalloon::spawn(u16 cell)
{
	if (mGrid[cell] == 8) {
		return;
	}
	BalloonPuff* puff = mFree;
	if (puff == 0) {
		return;
	}
	mFree       = puff->mNext;
	puff->mKind = (u8)(int)(8.0f * (3.0517578e-05f * (f32)rand()));
	puff->mCell = cell;

	puff->mX = 100.0f * (2.0f * (3.0517578e-05f * (f32)rand() - gBalloonRandOffset)) + (*(Vec3f**)gBalloonPosExport)[cell].x;
	puff->mY = 10.0f * (2.0f * (3.0517578e-05f * (f32)rand() - gBalloonRandOffset)) + (*(Vec3f**)gBalloonPosExport)[cell].y;
	puff->mZ = 100.0f * (2.0f * (3.0517578e-05f * (f32)rand() - gBalloonRandOffset)) + (*(Vec3f**)gBalloonPosExport)[cell].z;

	puff->mNext = mActive;
	mActive     = puff;
	mCount++;
	mGrid[cell]++;
	if (mGrid[cell] <= 2) {
		puff->mY += 200.0f * (f32)(2 - mGrid[cell]);
	}
}
