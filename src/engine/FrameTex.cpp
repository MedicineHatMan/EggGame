#include "engine/FrameTex.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "egg/EggCreature.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"
#include "core/SdkCalls.h"

static FrameTex* sFrameTex[8];
static int sFrameTexOn;
static u16 sFrameTexCur;

static void frameTexCheck(u16 id);

// 0x8002B774
void frameTexBoot()
{
}

// 0x8002B778
// Slot 0 copies the whole screen
void frameTexInit()
{
	frameTexCreate(0, 640, 480, 5);
	sFrameTexOn  = 1;
	sFrameTexCur = 0;
}

// 0x8002B7B8
void frameTexExit()
{
	for (int i = 0; i < 8; i++) {
		if (sFrameTex[i] != NULL) {
			delete sFrameTex[i];
			sFrameTex[i] = NULL;
		}
	}
	sFrameTexOn = 0;
}

// 0x8002B820
int frameTexIsOn()
{
	return sFrameTexOn;
}

// 0x8002B828
void frameTexSelect(u16 id)
{
	sFrameTexCur = id;
}

// 0x8002B830
FrameTex::FrameTex(u16 width, u16 height, u32 texId, s32 format)
{
	NJS_TEXINFO info;
	mTexList.textures  = &mTexName;
	mTexList.nbTexture = 1;
	mFlags             = 0;
	mX                 = 0;
	mY                 = 0;
	mWidth             = width;
	mHeight            = height;
	mClear             = 0;
	mFormat            = format;
	njSetTextureInfo(&info, NULL, format, width, height);
	njSetTextureName(&mTexName, (void*)&info, (void*)texId, (u32)0x08800000);
	njLoadTexture(&mTexList);
	mDest = mTexList.textures->texaddr->hdr->info.image;
}

// 0x8002B8DC
FrameTex::~FrameTex()
{
	njReleaseTexture(&mTexList);
}

// 0x8002B928
void FrameTex::request()
{
	mFlags |= 1;
}

// 0x8002B938
void FrameTex::update()
{
	if (mFlags & 1) {
		mFlags &= ~1;
		copy();
	}
}

// 0x8002B96C
void FrameTex::copy()
{
	GXSetTexCopySrc(mX, mY, mWidth, mHeight);
	GXSetTexCopyDst(mWidth, mHeight, mFormat, GX_FALSE);
	GXCopyTex(mDest, mClear);
}

// 0x8002B9CC
void FrameTex::set()
{
	njSetTexture(&mTexList);
}

// 0x8002B9F0
void FrameTex::setRect(u16 x, u16 y, u16 width, u16 height)
{
	mX      = x;
	mY      = y;
	mWidth  = width;
	mHeight = height;
}

// 0x8002BA04
// The rectangle at x y the size of the texture
void FrameTex::setOrigin(u16 x, u16 y)
{
	GjTexHeader* hdr = mTexList.textures->texaddr->hdr;
	setRect(x, y, hdr->info.width, hdr->info.height);
}

// 0x8002BA40
// Points the copy at texel x y of the texture in its 4x4 tiles of 16-bit texels
void FrameTex::setDest(u16 x, u16 y)
{
	GjTexHeader* hdr = mTexList.textures->texaddr->hdr;
	int off = y * hdr->info.width * 2;
	off += (x >> 2) * 32;
	u8* dest = (u8*)hdr->info.image;
	dest += off;
	mDest = dest;
}

// 0x8002BA74
void FrameTex::setClear(int clear)
{
	mClear = clear;
}

// 0x8002BA7C
void FrameTex::setFormat(int format)
{
	mFormat = format;
}

// 0x8002BA84
// Global texture 0x100C + id
void frameTexCreate(u16 id, u16 width, u16 height, int format)
{
	frameTexCheck(id);
	if (sFrameTex[id] == NULL) {
		sFrameTex[id] = new FrameTex(width, height, id + 0x100C, format);
	}
}

// 0x8002BB04
void frameTexDelete(u16 id)
{
	frameTexCheck(id);
	if (sFrameTex[id] != NULL) {
		delete sFrameTex[id];
		sFrameTex[id] = NULL;
	}
}

// 0x8002BB58
void frameTexRequest(u16 id)
{
	frameTexCheck(id);
	if (sFrameTex[id] != NULL) {
		sFrameTex[id]->request();
	}
}

// 0x8002BBA0
void frameTexRequestCur()
{
	frameTexRequest(sFrameTexCur);
}

// 0x8002BBC4
void frameTexUpdate(u16 id)
{
	frameTexCheck(id);
	if (sFrameTex[id] != NULL) {
		sFrameTex[id]->update();
	}
}

// 0x8002BC0C
void frameTexUpdateCur()
{
	frameTexUpdate(sFrameTexCur);
}

// 0x8002BC30
void frameTexSet(u16 id)
{
	frameTexCheck(id);
	if (sFrameTex[id] != NULL) {
		sFrameTex[id]->set();
	}
}

// 0x8002BC78
void frameTexSetCur()
{
	frameTexSet(sFrameTexCur);
}

// 0x8002BC9C
void frameTexSetOrigin(u16 id, u16 x, u16 y)
{
	frameTexCheck(id);
	if (sFrameTex[id] != NULL) {
		sFrameTex[id]->setOrigin(x, y);
	}
}

// 0x8002BCF4
void frameTexSetRect(u16 id, u16 x, u16 y, u16 width, u16 height)
{
	frameTexCheck(id);
	if (sFrameTex[id] != NULL) {
		sFrameTex[id]->setRect(x, y, width, height);
	}
}

// 0x8002BD5C
void frameTexSetDest(u16 id, u16 x, u16 y)
{
	frameTexCheck(id);
	if (sFrameTex[id] != NULL) {
		sFrameTex[id]->setDest(x, y);
	}
}

// 0x8002BDB4
void frameTexSetClear(u16 id, int clear)
{
	frameTexCheck(id);
	if (sFrameTex[id] != NULL) {
		sFrameTex[id]->setClear(clear);
	}
}

// 0x8002BE04
void frameTexSetFormat(u16 id, int format)
{
	frameTexCheck(id);
	if (sFrameTex[id] != NULL) {
		sFrameTex[id]->setFormat(format);
	}
}

// 0x8002BE54
// An id check compiled out
static void frameTexCheck(u16 id)
{
}
