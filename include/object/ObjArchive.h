#ifndef _OBJARCHIVE_H
#define _OBJARCHIVE_H

#include "core/types.h"

struct NJS_OBJECT;
struct NJS_TEXLIST;

/**
 * One bank of an archive: the models, motions, texture lists and collision meshes, each indexed by number.
 * The archive's table has two, "set" and "fix". The loader binds both banks' texture lists to mTextures.
 *
 * @size 0x18
 */
struct ObjBank {
	u8 _00[4];
	u16 mNbTexList; // _04
	u8 _06[2];
	NJS_OBJECT** mModels;    // _08
	void** mMotions;         // _0C
	NJS_TEXLIST** mTexLists; // _10
	void** mMeshes;          // _14 collision meshes for meshColliSetData
};

/**
 * The table at the head of each common archive. getStgObjModels and the other getters read the set bank.
 *
 * @size 0x34
 */
struct ObjCommonData {
	ObjBank mSet;    // _00
	ObjBank mFix;    // _18
	void* mTextures; // _30 the texture file the lists bind to
};

extern "C" {
extern ObjCommonData* gStgObjCommon;
extern ObjCommonData* gGeObjCommon;

void* findStgObjData(const char* name); // 0x80040DCC any export of stgobj_common.arc by name
void* findGeObjData(const char* name);  // 0x80040E08 any export of geobj_common.arc by name
NJS_OBJECT** getStgObjModels();         // 0x80040E44
NJS_OBJECT** getGeObjModels();          // 0x80040E60
NJS_TEXLIST** getStgObjTextures();      // 0x80040E7C
NJS_TEXLIST** getGeObjTextures();       // 0x80040E98
}

// Named exports that are models
inline NJS_OBJECT* findStgObjModel(const char* name)
{
	return (NJS_OBJECT*)findStgObjData(name);
}
inline NJS_OBJECT* findGeObjModel(const char* name)
{
	return (NJS_OBJECT*)findGeObjData(name);
}

#endif
