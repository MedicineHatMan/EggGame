#include "ui/TSpeakCursor.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "ui/AeArc.h"
#include "object/ObjAction.h"
#include "object/TObjLauncher.h"
#include "object/TObjEggRing.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "stage/TEventCtrl.h"
#include "camera/TCamLockOnCursor.h"
#include "io/NinjaFile.h"
struct NJS_TEXLIST;

typedef char TSpeakCursor_size_check[(sizeof(TSpeakCursor) == 0x40) ? 1 : -1];

const char* gTSpeakCursorName = "TSpeakCursor";

// 0x8010F198
TSpeakCursor::TSpeakCursor(TObject* parent)
    : TObject(parent)
{
	_00   = (u32)gTSpeakCursorName;
	mSize = 0x40;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x32, 0);
	_34   = 0;
	mAnim = 0.0f;
	mFlags |= 0x10;
	mTex = 0;
	if (gIcon3dArc != 0) {
		void** lists = (void**)findArcExport(gIcon3dArc, "texlists");
		if (lists != 0) {
			mTex = *lists;
		}
	}
}

// 0x8010F248
TSpeakCursor::~TSpeakCursor()
{
	removeDrawTask(mDraw);
}

// 0x8010F2B0
void TSpeakCursor::hide()
{
	mFlags &= ~0x10;
}

// 0x8010F2C0
void TSpeakCursor::show()
{
	mFlags |= 0x10;
}

// 0x8010F2D0
void TSpeakCursor::place(const Vec3f* pos, f32 lift)
{
	mPos  = *pos;
	mLift = lift;
}

// 0x8010F2F0
// Ghidra had no function here
void TSpeakCursor::draw()
{
	mAnim += 0.03f;
	if (mAnim >= 1.0f) {
		mAnim -= 1.0f;
	}
}

// 0x8010F320
// Copies the matrix the cursor just built into the global the quad uses
extern "C" void copySpeakMatrix(const f32* mtx)
{
	mtxCopy(sSpeakCursorMtx, mtx);
}

// 0x8010F34C
// opaque == 0 uses the opaque vertex state
extern "C" void beginSpeakTex(int opaque)
{
	GXLoadPosMtxImm((const float*)gjIdentityMatrix, GX_PNMTX0);
	if (opaque != 0) {
		gjStartVertexTexTrans();
	} else {
		gjStartVertexTexOpaque();
	}
	gjSetTexture();
}

// 0x8010F39C
extern "C" void endSpeakTex()
{
	gjEndVertex();
}

// 0x8010F3BC
// One quad per record A record is 12 words xyz then the values written after each position indices 3 4 5 6 and 10 and
// the half extents at 7 and 8
extern "C" void drawSpeakQuad(f32* quad, int count)
{
	while (count > 0) {
		Vec3f at;
		at.x = quad[0];
		at.y = quad[1];
		at.z = quad[2];
		mtxMultVec(sSpeakCursorMtx, &at, &at);
		f32 x0 = at.x - quad[7];
		f32 x1 = at.x + quad[7];
		f32 y1 = at.y + quad[8];
		f32 y0 = at.y - quad[8];
		f32 z  = at.z;
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT4, 4);
		GXPosition3f32(x0, y0, z);
		GXColor1u32(*(u32*)&quad[10]);
		GXTexCoord2f32(quad[3], quad[6]);
		GXPosition3f32(x0, y1, z);
		GXColor1u32(*(u32*)&quad[10]);
		GXTexCoord2f32(quad[3], quad[4]);
		GXPosition3f32(x1, y0, z);
		GXColor1u32(*(u32*)&quad[10]);
		GXTexCoord2f32(quad[5], quad[6]);
		GXPosition3f32(x1, y1, z);
		GXColor1u32(*(u32*)&quad[10]);
		GXTexCoord2f32(quad[5], quad[4]);
		quad += 12;
		count -= 1;
	}
}

// One quad of the speak icon as drawSpeakQuad reads it position uvs half size a colour word
struct SpeakQuad {
	f32 mPos[3];  // _00
	f32 mUV[4];   // _0C
	f32 mSize[2]; // _1C
	f32 _24;      // _24
	u32 mColor;   // _28
	f32 _2C;      // _2C
};

static const SpeakQuad sSpeakQuadA
    = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.296875f, 0.28125f }, { 3.8f, 3.6000001f }, 0.0f, 0xFFFFFFFF, 0.0f };
static const SpeakQuad sSpeakQuadB
    = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.296875f, 0.296875f, 0.46875f }, { 3.8f, 2.2f }, 0.0f, 0xFFFFFFFF, 0.0f };

// 0x8010F544
// Two quads copied from the tables at 0x80247F34 and 0x80247F64
void TSpeakCursor::render()
{
	if (eventIsBusy() != 0) {
		return;
	}
	f32 mtx[12];
	mtxTranslate((float*)mtx, (const float*)gjGetMatrix(), mPos.x, mPos.y + mLift, mPos.z);
	copySpeakMatrix(mtx);
	njZBufferMode(0);
	pushCullMode(0);
	njSetTexture(mTex);
	njSetTextureNum(0);
	beginSpeakTex(1);

	SpeakQuad quadA = sSpeakQuadA;
	SpeakQuad quadB = sSpeakQuadB;

	f32 anim = mAnim;
	anim     = anim < 0.5f ? anim : 1.0f - anim;
	anim *= 2.0f;
	anim = 0.25f * anim + 1.0f;
	quadA.mPos[1] += 5.0f;
	quadA.mSize[0] *= anim;
	quadA.mSize[1] *= anim;
	drawSpeakQuad((f32*)&quadA, 1);
	quadB.mPos[1] -= 0.5f;
	drawSpeakQuad((f32*)&quadB, 1);

	endSpeakTex();
	popCullMode();
	njZBufferMode(1);
}
