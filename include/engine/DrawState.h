#ifndef _DRAWSTATE_H
#define _DRAWSTATE_H

#include "core/types.h"
#include "object/ObjModel.h"
#include "io/ResManager.h"

struct DrawStateExtra {
	u8 mTexGens;                // _00
	u8 mTevStages;              // _01
	void (*mTexGen)(int first); // _04
	void (*mTev)(int first);    // _08
};

/**
 * The one slot getDrawStateSlot gives out. It keeps the flags of the state begun last, and a 32x32 noise texture
 * for the 0x400 pass.
 *
 * @size 0x18
 */
struct DrawStateSlot {
	DrawStateSlot();  // 0x80043B48
	~DrawStateSlot(); // 0x80043BCC

	u32 mFlags;           // _00
	NJS_TEXNAME mTexName; // _04
	NJS_TEXLIST mTexList; // _10 just mTexName
};

extern "C" {
void drawStateInit();  // 0x80043B40
void drawStateExit();  // 0x80043B44

void beginDrawState(DrawStateSlot* slot, const DrawState* state); // 0x80043C18
void endDrawState(DrawStateSlot* slot);                            // 0x800442D8

// Light a draw with a fixed ambient colour in place of the scene's
void beginDrawAmbient(DrawStateSlot* slot, const u32* color);
void endDrawAmbient(DrawStateSlot* slot);

// Fill an IA8 texture with noise for the 0x400 pass
void makeDrawNoise(u8* image);

void drawObjectDefault(void* object);
void drawModelDefault(void* model);
void drawActorDefault(void* actor);
void drawMotionDefault(void* object, void* motion, f32 frame);

void saveDrawBlend();
void setDrawBlendAmbient();
void restoreDrawBlend();
}

extern "C" {
extern s32 gDrawAlphaRef;
}

#endif
