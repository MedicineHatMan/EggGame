#ifndef _OBJSCRIPT_H
#define _OBJSCRIPT_H

#include "core/types.h"
#include "object/ObjClassName.h"

struct ObjScriptStep {
	u8 mCmd; // _00
	u8 _01;  // _01
	u16 mId; // _02
	s16 _04; // _04 always -1
	u16 _06; // _06
	u32 _08; // _08
	f32 _0C; // _0C 30 or 10
	f32 _10; // _10 always 1
	u32 _14; // _14
	u32 _18; // _18
};

static const ObjScriptStep sObjScriptA[6] = {
	{ 0, 0, 0, -1, 0, 0, 30.0f, 1.0f, 0, 0 }, { 0, 0, 1, -1, 0, 0, 10.0f, 1.0f, 0, 0 }, { 0, 0, 2, -1, 0, 0, 10.0f, 1.0f, 0, 0 },
	{ 0, 0, 3, -1, 0, 0, 30.0f, 1.0f, 0, 0 }, { 0, 0, 4, -1, 0, 0, 30.0f, 1.0f, 0, 0 }, { 0xFF, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0, 0 },
};

static const ObjScriptStep sObjScriptB[3] = {
	{ 0, 0, 0, -1, 0, 0, 30.0f, 1.0f, 0, 0 },
	{ 0, 0, 1, -1, 0, 0, 10.0f, 1.0f, 0, 0 },
	{ 2, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0, 0 },
};

#endif
