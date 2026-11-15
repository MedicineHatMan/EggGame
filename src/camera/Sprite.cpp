#include "camera/TCamera.h"
#include "core/Pad.h"
#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCameraCtrl.h"
#include "engine/TFogCtrl.h"
#include "object/ObjAction.h"
#include "camera/TCamEditor.h"
#include "ui/AeArc.h"
#include "egg/EggCreature.h"
#include "core/TMainTask.h"
#include "animal/RacerMover.h"
#include "core/SdkCalls.h"

enum {
	SPRITE_SLOTS       = 18,
	SPRITE_FIXED_SLOTS = 10, // set by number the rest are handed out by spriteAddTexList
	SPRITE_MAX         = 0x600,
};

// The sprite renderer's state Separate statics the DOL reaches them all from one base register
static NJS_TEXLIST* gSpriteTexList[SPRITE_SLOTS]; // 0x80279A48
static Sprite gSpriteHead;                        // 0x80279A90 mNext is the first queued
static Sprite* gSpriteSorted[SPRITE_MAX];         // 0x80279AC8 the ones in front of the eye this frame
static Sprite* gSpriteSpare[SPRITE_MAX];          // 0x8027B2C8

#define GX_FIFO_U8 (*(volatile u8*)0xCC008000)

extern "C" {
// 0x802407B0
// One quad's texture coordinates
const f32 gSpriteCoords[8] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };

// 0x802407D0
// Sixteen quads a 4 by 4 grid of a texture
const f32 gSpriteCoordGrid[0x80] = {
	0.0f, 0.0f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f, 0.25f,
	0.25f, 0.0f, 0.5f, 0.0f, 0.5f, 0.25f, 0.25f, 0.25f,
	0.5f, 0.0f, 0.75f, 0.0f, 0.75f, 0.25f, 0.5f, 0.25f,
	0.75f, 0.0f, 1.0f, 0.0f, 1.0f, 0.25f, 0.75f, 0.25f,
	0.0f, 0.25f, 0.25f, 0.25f, 0.25f, 0.5f, 0.0f, 0.5f,
	0.25f, 0.25f, 0.5f, 0.25f, 0.5f, 0.5f, 0.25f, 0.5f,
	0.5f, 0.25f, 0.75f, 0.25f, 0.75f, 0.5f, 0.5f, 0.5f,
	0.75f, 0.25f, 1.0f, 0.25f, 1.0f, 0.5f, 0.75f, 0.5f,
	0.0f, 0.5f, 0.25f, 0.5f, 0.25f, 0.75f, 0.0f, 0.75f,
	0.25f, 0.5f, 0.5f, 0.5f, 0.5f, 0.75f, 0.25f, 0.75f,
	0.5f, 0.5f, 0.75f, 0.5f, 0.75f, 0.75f, 0.5f, 0.75f,
	0.75f, 0.5f, 1.0f, 0.5f, 1.0f, 0.75f, 0.75f, 0.75f,
	0.0f, 0.75f, 0.25f, 0.75f, 0.25f, 1.0f, 0.0f, 1.0f,
	0.25f, 0.75f, 0.5f, 0.75f, 0.5f, 1.0f, 0.25f, 1.0f,
	0.5f, 0.75f, 0.75f, 0.75f, 0.75f, 1.0f, 0.5f, 1.0f,
	0.75f, 0.75f, 1.0f, 0.75f, 1.0f, 1.0f, 0.75f, 1.0f,
};

int gSpriteCount;    // 0x802C4C70
Sprite* gSpriteTail; // 0x802C4C74
int gSpriteRoll;     // 0x802C4C78
}

// 0x80015C90
extern "C" void spriteInit(Sprite* sprite)
{
	sprite->mPos.x        = 0.0f;
	sprite->mPos.y        = 0.0f;
	sprite->mPos.z        = 0.0f;
	sprite->mWidth        = 0.0f;
	sprite->mHeight       = 0.0f;
	sprite->mColor        = 0;
	sprite->mPush         = 0.0f;
	sprite->mKey.mWord   = 0;
	sprite->mRoll         = 0;
	sprite->mCoord        = 0;
	sprite->mRotX         = 0;
	sprite->mRotY         = 0;
}

// 0x80015CCC
extern "C" void spritesInit()
{
	int i;

	for (i = 0; i < SPRITE_SLOTS; i++) {
		gSpriteTexList[i] = 0;
	}
	gSpriteRoll = 0;
	spriteClear();
	for (int k = 0; k < SPRITE_MAX; k++) {
		gSpriteSorted[k] = 0;
		gSpriteSpare[k]  = 0;
	}
}

// 0x80015D4C
extern "C" void spritesTerm()
{
}

// 0x80015D50
BOOL spriteIsFixedSlot(int slot)
{
	u8 ok = FALSE;

	if (slot >= 0 && slot < SPRITE_FIXED_SLOTS) {
		ok = TRUE;
	}
	return ok;
}

// 0x80015D70
BOOL spriteIsFreeSlot(int slot)
{
	u8 ok = FALSE;

	if (slot >= SPRITE_FIXED_SLOTS && slot < SPRITE_SLOTS) {
		ok = TRUE;
	}
	return ok;
}

// 0x80015D90
// Empties the queue
extern "C" void spriteClear()
{
	gSpriteHead.mNext = 0;
	gSpriteCount         = 0;
	gSpriteTail          = &gSpriteHead;
}

// 0x80015DA8
extern "C" void spriteAdd(Sprite* sprite)
{
	gSpriteCount++;
	gSpriteTail->mNext = sprite;
	gSpriteTail        = sprite;
	sprite->mNext      = 0;
}

// 0x80015DCC
// Which list and texture a loaded texture is
extern "C" int spriteFindTexture(u32 tex, int* slot, int* index)
{
	NJS_TEXLIST* list;
	int i;
	int j;

	for (i = 0; i < SPRITE_SLOTS; i++) {
		list = gSpriteTexList[i];
		if (list != 0) {
			for (j = 0; j < (int)list->nbTexture; j++) {
				if (tex == (u32)list->textures[j].texaddr->hdr->texaddr) {
					*slot  = i;
					*index = j;
					return TRUE;
				}
			}
		}
	}
	*slot  = 0;
	*index = 0;
	for (int k = 0; k < SPRITE_SLOTS; k++) {
		if (gSpriteTexList[k] != 0) {
			*slot  = k;
			*index = 0;
			return FALSE;
		}
	}
	return FALSE;
}

// 0x80015EA0
extern "C" void spriteSetTexList(int slot, NJS_TEXLIST* list)
{
	if (spriteIsFixedSlot(slot)) {
		gSpriteTexList[slot] = list;
	}
}

// 0x80015EE8
// Returns the slot it went in or -1 when they are all taken
extern "C" int spriteAddTexList(NJS_TEXLIST* list)
{
	int slot = -1;
	int i;

	for (i = SPRITE_FIXED_SLOTS; i < SPRITE_SLOTS; i++) {
		if (gSpriteTexList[i] == 0) {
			gSpriteTexList[i] = list;
			slot                 = i;
			break;
		}
	}
	return slot;
}

// 0x80015F30
extern "C" void spriteRemoveTexList(int slot)
{
	if (spriteIsFreeSlot(slot)) {
		gSpriteTexList[slot] = 0;
	}
}

// 0x80015F78
// Unlit coloured and textured positions already in view space
void spriteSetupGX()
{
	f32 mtx[12];

	GXSetCullMode(GX_CULL_NONE);
	PSMTXIdentity(mtx);
	GXLoadPosMtxImm(mtx, GX_PNMTX0);
	GXLoadNrmMtxImm(mtx, GX_PNMTX0);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetChanCtrl(GX_ALPHA0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GXSetNumTexGens(1);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
	GXSetNumTevStages(1);
	GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GXSetAlphaCompare(GX_GREATER, 8, GX_AOP_AND, GX_ALWAYS, 0xFF);
	GXSetZCompLoc(GX_FALSE);
	GXSetZMode(GX_ENABLE, GX_LESS, GX_DISABLE);
	fogOff();
	GXEnableTexOffsets(GX_TEXCOORD0, GX_ENABLE, GX_ENABLE);
}

// 0x80016124
void spriteSetBlend(int blend)
{
	switch (blend) {
	case 0:
	default:
		blendModeAlpha();
		break;
	case 1:
		blendModeAdd();
		break;
	case 2:
		blendModeInvertAlpha();
		break;
	case 3:
		blendModeInvert();
		break;
	}
}

// 0x80016184
void spriteSetCoords(int coords)
{
	switch (coords) {
	case 0:
	default:
		GXSetArray(GX_VA_TEX0, gSpriteCoords, 8);
		GXSetPointSize(0xFF, GX_TO_ONE);
		break;
	case 4:
	case 8:
		GXSetArray(GX_VA_TEX0, gSpriteCoordGrid, 8);
		GXSetPointSize(0xFF, GX_TO_FOURTH);
		break;
	}
}

// 0x80016204
// Changes only what differs
void spriteApplyKey(SpriteKey old, SpriteKey key)
{
	if (old.mField.mSlot != key.mField.mSlot) {
		njSetTexture(gSpriteTexList[key.mField.mSlot]);
		gjLoadTextureNum(key.mField.mTex, 0, 0, 0);
	} else if (old.mField.mTex != key.mField.mTex) {
		gjLoadTextureNum(key.mField.mTex, 0, 0, 0);
	}
	u32 blend = key.mField.mFlags & 3;
	if ((u32)(old.mField.mFlags & 3) != blend) {
		spriteSetBlend(blend);
	}
	u32 coords = key.mField.mFlags & 0xC;
	if ((u32)(old.mField.mFlags & 0xC) != coords) {
		spriteSetCoords(coords);
	}
}

/** Draws a run of points */
static inline void spriteDrawPoints(Sprite** list, int count)
{
	int i;

	gjGXSetBlendMode(1);
	GXBegin(GX_POINTS, GX_VTXFMT0, count);
	for (i = 0; i < count; i++) {
		Sprite* sprite = *list++;
		GXPosition3f32(sprite->mView.x, sprite->mView.y, sprite->mView.z);
		GX_FIFO_U32 = sprite->mColor;
		GX_FIFO_U8  = (u8)((sprite->mCoord & 0x3F) << 2);
	}
}

// 0x800162C4
// Draws every queued sprite in front of the eye a run at a time for each key
extern "C" void spriteDrawAll()
{
	Sprite* sprite;
	SpriteKey last;
	SpriteKey key;
	int count;
	int first;
	int i;
	int run;
	int j;

	if (gSpriteCount <= 0) {
		return;
	}
	sprite = gSpriteHead.mNext;
	count  = 0;
	for (i = 0; i < SPRITE_MAX && sprite != 0; i++) {
		njCalcPointEx(&sprite->mPos, &sprite->mView);
		if (sprite->mPush != 0.0f) {
			f32 push = -sprite->mPush;
			Vec3f dir;
			dir.set(sprite->mView.x, sprite->mView.y, sprite->mView.z);
			if (njUnitVector(&dir) < gUnitVectorMin) {
				dir = gVecZero;
			}
			sprite->mView.x = dir.x * push + sprite->mView.x;
			sprite->mView.y = dir.y * push + sprite->mView.y;
			sprite->mView.z = dir.z * push + sprite->mView.z;
		}
		if (sprite->mView.z < -1.5f) {
			gSpriteSorted[count++] = sprite;
		}
		sprite = sprite->mNext;
	}
	if (count <= 0) {
		return;
	}

	spriteSetupGX();
	gSpriteRoll = camGetDrawRoll();
	first = 0;
	run   = 1;
	last.mWord  = gSpriteSorted[0]->mKey.mWord;
	key.mWord   = last.mWord;
	last.mField.mSlot++;
	last.mField.mTex++;
	spriteSetBlend(last.mField.mFlags & 3);
	spriteSetCoords(last.mField.mFlags & 0xC);
	for (j = 1; j < count; j++) {
		if (key.mWord != gSpriteSorted[j]->mKey.mWord) {
			spriteApplyKey(last, key);
			if ((key.mField.mFlags & 0x10) == 0x10) {
				spriteDrawPoints(&gSpriteSorted[first], run);
			} else {
				spriteDrawQuads(&gSpriteSorted[first], run);
			}
			first = j;
			run   = 1;
			last.mWord = key.mWord;
			key.mWord  = gSpriteSorted[j]->mKey.mWord;
		} else {
			run++;
		}
	}
	if (run >= 1) {
		key.mWord = gSpriteSorted[first]->mKey.mWord;
		spriteApplyKey(last, key);
		if ((key.mField.mFlags & 0x10) == 0x10) {
			spriteDrawPoints(&gSpriteSorted[first], run);
		} else {
			spriteDrawQuads(&gSpriteSorted[first], run);
		}
	}
	blendModeAlpha();
	gjGXSetBlendMode(0);
	fogApply();
}

// 0x80016654
// Quads face the eye turned by their roll and the view's
void spriteDrawQuads(Sprite** list, int count)
{
	int i;

	gjGXSetBlendMode(1);
	GXBegin(GX_QUADS, GX_VTXFMT0, count * 4);
	for (i = 0; i < count; i++) {
		Sprite* sprite;
		u8 coord;
		int roll;
		int rotX;
		f32 x;
		f32 y;
		f32 z;
		f32 w;
		f32 h;
		sprite = *list++;
		rotX = sprite->mRotX;
		coord = sprite->mCoord;
		roll = sprite->mRoll + gSpriteRoll;
		x              = sprite->mView.x;
		y              = sprite->mView.y;
		z              = sprite->mView.z;
		w              = sprite->mWidth;
		h              = sprite->mHeight;
		u32 color      = sprite->mColor;
		int rotY       = sprite->mRotY;

		if (rotX != 0 || rotY != 0) {
			Vec3f corner;
			Vec3f out;
			f32 nw = -w;
			f32 nh = -h;
			u8 base;

			corner.z = 0.0f;
			njPushMatrixEx();
			njTranslateEx(&sprite->mPos);
			gjRotateY(rotY);
			gjRotateX(rotX);
			gjRotateZ(roll);
			corner.x = nw;
			corner.y = h;
			njCalcPointEx(&corner, &out);
			GX_FIFO_F32 = out.x;
			GX_FIFO_F32 = out.y;
			GX_FIFO_F32 = out.z;
			GX_FIFO_U32 = color;
			GX_FIFO_U8  = (u8)(coord << 2);
			corner.x    = w;
			corner.y    = h;
			njCalcPointEx(&corner, &out);
			GX_FIFO_F32 = out.x;
			GX_FIFO_F32 = out.y;
			GX_FIFO_F32 = out.z;
			GX_FIFO_U32 = color;
			GX_FIFO_U8  = (u8)(coord << 2) + 1;
			corner.x    = w;
			corner.y    = nh;
			njCalcPointEx(&corner, &out);
			GX_FIFO_F32 = out.x;
			GX_FIFO_F32 = out.y;
			GX_FIFO_F32 = out.z;
			GX_FIFO_U32 = color;
			GX_FIFO_U8  = (u8)(coord << 2) + 2;
			corner.x    = nw;
			corner.y    = nh;
			njCalcPointEx(&corner, &out);
			GX_FIFO_F32 = out.x;
			GX_FIFO_F32 = out.y;
			GX_FIFO_F32 = out.z;
			GX_FIFO_U32 = color;
			GX_FIFO_U8  = (u8)(coord << 2) + 3;
			njPopMatrixEx();
		} else {
			f32 ax;
			f32 ay;
			u8 base;

			// w and h become the second corner's offsets
			if (roll == 0) {
				ax = w;
				ay = -h;
			} else {
				f32 s;
				f32 c;
				njSinCos(roll, &s, &c);
				ax = h * s;
				ay = h * c;
				s *= w;
				c *= w;
				h  = s + ay;
				ay = s - ay;
				w  = c - ax;
				ax = c + ax;
			}
			base = (coord & 0x3F) << 2;
			GXPosition3f32(x - ax, y - ay, z);
			GX_FIFO_U32 = color;
			GX_FIFO_U8  = base;
			GXPosition3f32(x + w, y + h, z);
			GX_FIFO_U32 = sprite->mColor;
			GX_FIFO_U8  = base + 1;
			GXPosition3f32(x + ax, y + ay, z);
			GX_FIFO_U32 = color;
			GX_FIFO_U8  = base + 2;
			GXPosition3f32(x - w, y - h, z);
			GX_FIFO_U32 = color;
			GX_FIFO_U8  = base + 3;
		}
	}
}
