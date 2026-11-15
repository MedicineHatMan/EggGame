#include "ui/StaffRoll.h"
#include "engine/DrawTask.h"
#include "io/ResManager.h"
#include "ui/Font.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "ui/AeArc.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "io/NinjaFile.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;

struct GvmLoader;
struct GvmList;

extern "C" {

// ResManager.h brings in dolphin.h whose GXColor clashes with Ninja/ninja.h so the GX calls are declared here
}

#define GX_FIFO_F32 (*(volatile f32*)0xCC008000)
#define GX_FIFO_U32 (*(volatile u32*)0xCC008000)

/** A screen rectangle and its texture coordinates */
struct StaffQuad {
	f32 mX0, mY0, mX1, mY1; // _00
	f32 mU0, mV0, mU1, mV1; // _10
};

/** One line of the staff roll on the screen */
// The members come before the virtuals CodeWarrior puts the vtable pointer where the first virtual is declared and here
// it is at _18 after them
class StaffRollItem {
public:
	StaffRollItem(StaffRollItem* head); // 0x801C653C links in after head

	bool scroll(f32 pos);
	int clip(f32 x0, f32 y0, f32 x1, f32 y1, f32 u0, f32 v0, f32 u1, f32 v1, StaffQuad* out);
	void beginDraw();
	void drawQuad(f32 z, StaffQuad* quad, u32 color);
	void endDraw();

	f32 mX;               // _00
	f32 mY;               // _04
	f32 mHeight;          // _08
	f32 mStart;           // _0C the scroll position at which it is at the bottom of the screen
	StaffRollItem* mPrev; // _10
	StaffRollItem* mNext; // _14

	// _18 = vtable
	virtual ~StaffRollItem(); // 0x801C659C
	virtual void draw() { }   // 0x801C75F8
};

/** A line of text drawn twice a dark copy a little down and right then the text */
class StaffRollText : public StaffRollItem {
public:
	StaffRollText(f32 x, f32 start, f32 height, StaffRollItem* head, const u8* text, u32 color); // 0x801C692C
	virtual ~StaffRollText();                                                                    // 0x801C6A84
	virtual void draw();                                                                         // 0x801C6B24

	int mCount;         // _1C
	FontGlyph* mGlyphs; // _20
	u32 mColor;         // _24
};

/** A logo one rectangle of a texture from staff_logo.arc */
class StaffRollLogo : public StaffRollItem {
public:
	StaffRollLogo(f32 x, f32 start, f32 size, StaffRollItem* head, int tex, const f32* uv); // 0x801C6DC8
	virtual ~StaffRollLogo();                                                               // 0x801C6EBC
	virtual void draw();                                                                    // 0x801C6F14

	int mTex;   // _1C
	f32 mWidth; // _20
	f32 mU0;    // _24
	f32 mV0;    // _28
	f32 mU1;    // _2C
	f32 mV1;    // _30
};

/** One line of staff_list.arc */
struct StaffLine {
	s16 mKind;       // _00
	s16 mIndent;     // _02 in steps of 8 pixels
	const u8* mText; // _04
};

/**
 * @size 0x40
 */
class TStaffRoll : public TObject {
public:
	TStaffRoll(TObject* parent); // 0x801C70D4
	virtual ~TStaffRoll();       // 0x801C722C
	virtual void draw();         // 0x801C7370 steps the roll
	virtual void render();       // 0x801C75A0

	f32 getScroll();

	DrawTask* mDrawTask;  // _20
	int mCount;           // _24
	StaffLine* mLines;    // _28
	int mNext;            // _2C the next line to bring on
	f32* mStarts;         // _30 where each line comes on
	f32 mLength;          // _34 the scroll position at the end
	StaffRollItem* mHead; // _38
	int mFrame;           // _3C
};

TArc* sStaffList;           // 0x802C5C38
TArc* sStaffLogo;           // 0x802C5C3C
NJS_TEXLIST* sStaffLogoTex; // 0x802C5C40

// By style the space after a line the space before the first line of a new style and how far in it starts
static const f32 sStyleAfter[9]  = { 22.0f, 24.0f, 28.0f, 12.0f, 176.0f, 128.0f, 128.0f, 52.0f, 80.0f };
static const f32 sStyleBefore[9] = { 8.0f, 38.0f, 72.0f, 16.0f, 4.0f, 4.0f, 4.0f, 72.0f, 72.0f };
static const f32 sStyleX[9]      = { 28.0f, 10.0f, 0.0f, 0.0f, 20.0f, 20.0f, 20.0f, 6.0f, 6.0f };
// Each logo's rectangle in its texture as u0 v0 u1 v1
static const f32 sLogoUV[16][4] = {
	{ 0.0f, 0.0f, 1.0f, 0.6875f },
	{ 0.0f, 0.125f, 1.0f, 0.5f },
	{ 0.0f, 0.09375f, 1.0f, 0.5f },
	{ 0.0f, 0.0f, 0.24023438f, 0.1015625f },
	{ 0.25195312f, 0.0f, 0.80078125f, 0.1015625f },
	{ 0.0f, 0.1015625f, 0.578125f, 0.203125f },
	{ 0.609375f, 0.1015625f, 1.0f, 0.203125f },
	{ 0.0f, 0.203125f, 0.47265625f, 0.3046875f },
	{ 0.609375f, 0.203125f, 1.0f, 0.3046875f },
	{ 0.0f, 0.3046875f, 0.32421875f, 0.40625f },
	{ 0.33984375f, 0.3046875f, 1.0f, 0.4609375f },
	{ 0.0f, 0.5078125f, 0.5546875f, 0.609375f },
	{ 0.640625f, 0.5078125f, 1.0f, 0.609375f },
	{ 0.0f, 0.609375f, 0.6796875f, 0.7109375f },
	{ 0.0f, 0.7109375f, 0.57421875f, 0.8125f },
	{ 0.0f, 0.8125f, 0.47949219f, 0.9140625f },
};

// 0x801C641C
static int staffStyle(int kind)
{
	int style = 0;
	switch (kind) {
	default:
		if (kind >= 0x10 && kind <= 0x1F) {
			if (kind == 0x10) {
				style = 4;
			} else if (kind == 0x11) {
				style = 5;
			} else if (kind == 0x12) {
				style = 6;
			} else if (kind == 0x1A) {
				style = 8;
			} else {
				style = 7;
			}
		}
		break;
	case 0:
		style = 0;
		break;
	case 1:
		style = 1;
		break;
	case 2:
		style = 2;
		break;
	case 8:
		style = 3;
		break;
	}
	return style;
}

// 0x801C64C0
static u32 staffColor(int kind)
{
	if (kind == 1) {
		return 0xFF8200FF;
	}
	return 0xFFFFFFFF;
}

// 0x801C64D8
static f32 staffLogoSize(int kind)
{
	if (kind >= 0x13 || kind < 0x10) {
		return 512.0f;
	}
	return 256.0f;
}

// 0x801C64F8
static int staffLogoTex(int kind)
{
	switch (kind) {
	default:
		return 3;
	case 0x10:
		return 4;
	case 0x11:
		return 2;
	case 0x12:
		return 1;
	}
}

// 0x801C653C
StaffRollItem::StaffRollItem(StaffRollItem* head)
{
	if (head != NULL) {
		mNext = head->mNext;
		if (mNext != NULL) {
			mNext->mPrev = this;
		}
		head->mNext = this;
		mPrev       = head;
	} else {
		mPrev = NULL;
		mNext = NULL;
	}
	mX      = 0.0f;
	mY      = 0.0f;
	mHeight = 22.0f;
	mStart  = 0.0f;
}

// 0x801C659C
StaffRollItem::~StaffRollItem()
{
	if (mPrev != NULL) {
		mPrev->mNext = mNext;
	}
	if (mNext != NULL) {
		mNext->mPrev = mPrev;
	}
	mNext = NULL;
	mPrev = NULL;
}

// 0x801C661C
// Moves the item to pos and returns zero once it is above the top of the screen
bool StaffRollItem::scroll(f32 pos)
{
	f32 up = pos - mStart;
	mY     = 460.0f - up;
	up -= mHeight;
	return !(up >= 420.0f);
}

// 0x801C6658
// Cuts a rectangle down to the screen from 10 40 to 630 460 moving its texture coordinates to match
int StaffRollItem::clip(f32 x0, f32 y0, f32 x1, f32 y1, f32 u0, f32 v0, f32 u1, f32 v1, StaffQuad* out)
{
	if (x0 >= 630.0f || x1 <= 10.0f || y0 >= 460.0f || y1 <= 40.0f) {
		return 0;
	}

	f32 du;
	f32 w;
	f32 cut = 10.0f - x0;
	w       = x1 - x0;
	du      = u1 - u0;
	if (cut > 0.0f) {
		x0 = 10.0f;
		u0 = du * (cut / w) + u0;
	}
	f32 right = 630.0f - x1;
	if (right < 0.0f) {
		x1 = 630.0f;
		u1 = du * (right / w) + u1;
	}
	out->mX0 = x0;
	out->mX1 = x1;
	out->mU0 = u0;
	out->mU1 = u1;

	f32 h   = y1 - y0;
	f32 dv  = v1 - v0;
	f32 top = 40.0f - y0;
	if (top > 0.0f) {
		y0 = 40.0f;
		v0 = dv * (top / h) + v0;
	}
	f32 bottom = 460.0f - y1;
	if (bottom < 0.0f) {
		y1 = 460.0f;
		v1 = dv * (bottom / h) + v1;
	}
	out->mY0 = y0;
	out->mY1 = y1;
	out->mV0 = v0;
	out->mV1 = v1;
	return 1;
}

// 0x801C6754
void StaffRollItem::beginDraw()
{
	njDrawTexture2DExStart(1);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);                                // position direct
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);                               // colour 0 direct
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);                               // texture coordinate 0 direct
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);     // xyz f32
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0); // rgba8
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);     // st f32
	GXSetAlphaCompare(GX_GREATER, 0x20, GX_AOP_AND, GX_LEQUAL, 0xFF);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
	GXSetZCompLoc(GX_FALSE);
	GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
	GXSetCullMode(GX_CULL_NONE);
}

// 0x801C6834
void StaffRollItem::drawQuad(f32 z, StaffQuad* quad, u32 color)
{
	GXBegin(GX_QUADS, GX_VTXFMT0, 4);
	f32 x0 = quad->mX0 * z;
	f32 x1 = quad->mX1 * z;
	f32 y0 = quad->mY0 * z;
	f32 y1 = quad->mY1 * z;

	GXPosition3f32(x0, y0, z);
	GXColor1u32(color);
	GXTexCoord2f32(quad->mU0, quad->mV0);

	GXPosition3f32(x1, y0, z);
	GXColor1u32(color);
	GXTexCoord2f32(quad->mU1, quad->mV0);

	GXPosition3f32(x1, y1, z);
	GXColor1u32(color);
	GXTexCoord2f32(quad->mU1, quad->mV1);

	GXPosition3f32(x0, y1, z);
	GXColor1u32(color);
	GXTexCoord2f32(quad->mU0, quad->mV1);
}

// 0x801C6928
void StaffRollItem::endDraw()
{
}

// 0x801C692C
StaffRollText::StaffRollText(f32 x, f32 start, f32 height, StaffRollItem* head, const u8* text, u32 color)
    : StaffRollItem(head)
{
	mX      = x;
	mY      = 0.0f;
	mHeight = height;
	mStart  = start;
	mColor  = color;

	mCount      = 0;
	const u8* p = text;
	for (int i = 0; i < 0x40; i++) {
		if (*p++ == 0) {
			break;
		}
		if (fontIsLead(p[-1])) {
			p++;
		}
		mCount++;
	}

	mGlyphs = new FontGlyph[mCount];
	for (int i = 0; i < mCount; i++) {
		u16 code = *text++;
		if (fontIsLead(code)) {
			code = (code << 8) | *text++;
		}
		fontGetGlyph(code, &mGlyphs[i]);
	}
}

// 0x801C6A84
StaffRollText::~StaffRollText()
{
	for (int i = mCount - 1; i >= 0; i--) {
		fontReleaseGlyph(&mGlyphs[i]);
	}
	if (mGlyphs != NULL) {
		delete[] mGlyphs;
		mGlyphs = NULL;
	}
}

// 0x801C6B24
void StaffRollText::draw()
{
	fontSetTexture();
	njSetTextureNum(0);
	njSetPaletteBankNum(0, 0);
	beginDraw();

	f32 x  = mX;
	f32 y0 = mY;
	f32 y1 = mY + mHeight;
	for (int i = 0; i < mCount; i++) {
		FontGlyph* glyph = &mGlyphs[i];
		f32 x1;
		f32 u0;
		f32 u1;
		f32 v0;
		f32 v1;
		x1 = glyph->mWidth * (mHeight / glyph->mHeight) + x;
		u0 = glyph->mU0 / 256.0f;
		v0 = glyph->mV0 / 256.0f;
		u1 = glyph->mU1 / 256.0f;
		v1 = glyph->mV1 / 256.0f;
		StaffQuad quad;
		if (clip(x, y0, x1, y1, u0, v0, u1, v1, &quad)) {
			drawQuad(-10.0f, &quad, mColor);
		}
		if (clip(4.0f + x, 4.0f + y0, 4.0f + x1, 4.0f + y1, u0, v0, u1, v1, &quad)) {
			drawQuad(-10.1f, &quad, 0x40404060);
		}
		x = 2.0f + x1;
	}

	endDraw();
}

// 0x801C6DC8
StaffRollLogo::StaffRollLogo(f32 x, f32 start, f32 size, StaffRollItem* head, int tex, const f32* uv)
    : StaffRollItem(head)
{
	mX      = x;
	mStart  = start;
	mTex    = tex < 0 ? 0 : (tex > 4 ? 4 : tex);
	mU0     = uv[0];
	mV0     = uv[1];
	mU1     = uv[2];
	mV1     = uv[3];
	mWidth  = size * (mU1 - mU0);
	mHeight = size * (mV1 - mV0);
}

// 0x801C6EBC
StaffRollLogo::~StaffRollLogo()
{
}

// 0x801C6F14
void StaffRollLogo::draw()
{
	njSetTexture(sStaffLogoTex);
	njSetTextureNum(mTex);
	beginDraw();
	StaffQuad quad;
	f32 x = mX;
	f32 y = mY;
	if (clip(x, y, x + mWidth, y + mHeight, mU0, mV0, mU1, mV1, &quad)) {
		drawQuad(-10.0f, &quad, 0xFFFFFFFF);
	}
	endDraw();
}

// 0x801C6FAC
void staffRollLoad()
{
	GvmLoader* loader;
	loadArc("staff_list.arc", &sStaffList);
	loadArc("staff_logo.arc", &sStaffLogo);
	void** data   = (void**)sStaffLogo->mData;
	sStaffLogoTex = (NJS_TEXLIST*)data[0];

	loader = new GvmLoader;
	gvmLoaderSetList(loader, (NJS_TEXLIST*)sStaffLogoTex);
	gvmLoaderFromMemory(loader, (u8*)data[1]);
	gvmLoaderTakeList(loader);
	delete loader;
}

// 0x801C7044
void staffRollFree()
{
	njReleaseTexture(sStaffLogoTex);
	if (sStaffLogo != NULL) {
		releaseResource(sStaffLogo);
	}
	sStaffLogo    = NULL;
	sStaffLogoTex = NULL;
	if (sStaffList != NULL) {
		releaseResource(sStaffList);
	}
	sStaffList = NULL;
}

// 0x801C709C
void staffRollCreate()
{
	new TStaffRoll(gTL_02);
}

// 0x801C70D4
TStaffRoll::TStaffRoll(TObject* parent)
    : TObject(parent)
{
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);

	// The file is a count then the lines
	int* list = (int*)sStaffList->mData;
	mCount    = list[0];
	mLines    = (StaffLine*)list[1];
	mNext     = 0;
	mStarts   = new f32[mCount];

	StaffLine* line;
	f32* start = mStarts;
	line       = mLines;
	f32 pos    = 0.0f;
	int prev   = 0;
	for (int i = 0; i < mCount; i++) {
		int style = staffStyle(line->mKind);
		if (style != prev) {
			pos += sStyleBefore[style];
		} else {
			pos += 6.0f;
		}
		*start = pos;
		prev   = style;
		pos += sStyleAfter[style];
		line++;
		start++;
	}
	pos += 420.0f;
	mLength = pos;

	mHead  = new StaffRollItem(NULL);
	mFrame = 0;
}

// 0x801C722C
TStaffRoll::~TStaffRoll()
{
	StaffRollItem* item = mHead;
	StaffRollItem* next = item->mNext;
	if (next != NULL) {
		item = next;
		while ((next = item->mNext) != NULL) {
			item = next;
		}
		while (item != NULL) {
			StaffRollItem* prev = item->mPrev;
			if (item != NULL) {
				delete item;
			}
			item = prev;
		}
	} else if (item != NULL) {
		delete item;
		mHead = NULL;
	}

	if (mStarts != NULL) {
		delete[] mStarts;
		mStarts = NULL;
	}
	removeDrawTask(mDrawTask);
}

// 0x801C7334
f32 TStaffRoll::getScroll()
{
	return mLength * (mFrame / 18000.0f);
}

// 0x801C7370
void TStaffRoll::draw()
{
	mFrame++;
	f32 pos = getScroll();

	// Bring on every line that has reached the bottom
	while (mNext <= mCount) {
		f32 start = mStarts[mNext];
		if (!(start <= pos)) {
			break;
		}
		int kind  = mLines[mNext].mKind;
		int style = staffStyle(kind);
		if (kind >= 0 && kind <= 2) {
			f32 x = 8.0f * mLines[mNext].mIndent + (10.0f + sStyleX[style]);
			new StaffRollText(x, start, sStyleAfter[style], mHead, mLines[mNext].mText, staffColor(kind));
		} else if (kind >= 0x10 && kind <= 0x1F) {
			new StaffRollLogo(10.0f + sStyleX[style], start, staffLogoSize(kind), mHead, staffLogoTex(kind), sLogoUV[kind - 0x10]);
		}
		mNext++;
	}

	// Move the rest and drop the ones that have gone off the top
	StaffRollItem* item = mHead->mNext;
	while (item != NULL) {
		StaffRollItem* next = item->mNext;
		if (!item->scroll(pos)) {
			delete item;
		}
		item = next;
	}
}

// 0x801C75A0
void TStaffRoll::render()
{
	getScroll();
	for (StaffRollItem* item = mHead->mNext; item != NULL; item = item->mNext) {
		item->draw();
	}
}
