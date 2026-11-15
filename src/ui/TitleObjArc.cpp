#include "core/types.h"
#include "io/Resource.h"
#include "io/ResManager.h"
#include "Ninja/ninja.h"

struct NJS_TEXLIST;

/**
 * The table at arc+0x4C.
 * @size 0x10
 */
struct TitleObjData {
	NJS_OBJECT** mModels;     // _00
	NJS_MOTION*** mMotions;   // _04 a motion list per entry
	NJS_TEXLIST** mTexLists;  // _08 the first is the one obj_title.GVM binds to
	void** mScene;            // _0C TModeTitle's own model
};

extern "C" {
TArc* sTitleObjArc;          // 0x802C5A70
TitleObjData* sTitleObjData; // 0x802C5A74
}

// 0x801AC758
extern "C" void initTitleObjArc()
{
	loadArc("ar_obj_title.arc", &sTitleObjArc);
	sTitleObjData = (TitleObjData*)sTitleObjArc->mData;
	numberTextures(*sTitleObjData->mTexLists);
	loadChunkFile((const char*)("obj_title.GVM"), (int)*sTitleObjData->mTexLists);
}

// 0x801AC7B4
extern "C" void shutdownTitleObjArc()
{
	njReleaseTexture(*sTitleObjData->mTexLists);
	if (sTitleObjArc != 0) {
		releaseResource(sTitleObjArc);
	}
	sTitleObjArc  = 0;
	sTitleObjData = 0;
}

// 0x801AC7FC
extern "C" NJS_OBJECT** getTitleObjModels()
{
	return sTitleObjData->mModels;
}

// 0x801AC808
extern "C" NJS_MOTION** getTitleObjMotion(int i)
{
	return sTitleObjData->mMotions[i];
}

// 0x801AC81C
extern "C" NJS_TEXLIST** getTitleObjTexLists()
{
	return sTitleObjData->mTexLists;
}

// 0x801AC828
extern "C" void** getTitleObjScene()
{
	return sTitleObjData->mScene;
}

// 0x801AC834
// An empty init pair in the title set file
extern "C" void initTitleObjNone()
{
}

// 0x801AC838
extern "C" void shutdownTitleObjNone()
{
}
