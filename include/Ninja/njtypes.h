#ifndef _NINJA_NJTYPES_H
#define _NINJA_NJTYPES_H

#include "core/types.h"

typedef struct NJS_TEXNAME {
	void* filename;           // _00
	u32 attr;                 // _04 the low half is an index into the bank
	struct GjTexMem* texaddr; // _08
} NJS_TEXNAME;

typedef struct NJS_TEXLIST {
	NJS_TEXNAME* textures; // _00
	u32 nbTexture;         // _04
} NJS_TEXLIST;

#endif
