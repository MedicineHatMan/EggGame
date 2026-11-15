#ifndef AEARC_H
#define AEARC_H
struct E3Palette;
struct NJS_TEXLIST;

#include "core/types.h"
#include "io/LangFile.h"
#include "io/ResManager.h"

// The loaders in front of each TAeObj* class
// the ae file its texture list the gvm the list binds to and a list of palettes for the banks
// The loader keeps the arc the ae and the texture list in three .sbss words

/** @size 0x10 */
struct AeArcBlock {
	void* mAe;             // _00
	NJS_TEXLIST* mTexList; // _04
	void* mGvm;            // _08
	u32* mPalettes;        // _0C may be null
};

extern "C" {
void releaseTextures(NJS_TEXLIST*);
void bindGvmTextures(u8*, NJS_TEXLIST*);
void loadE3Palette(u32 enable, E3Palette* pal, u32 slot);
void njSetTexture(void*);
void njSetPaletteBankNum(int bank, int palette);
void GXInvalidateTexAll();
}

inline AeArcBlock* aeArcBlock(TArc* arc)
{
	return (AeArcBlock*)arc->mData;
}

#endif
