#include "core/types.h"
#include "io/Resource.h"
#include "object/ObjArchive.h"
#include "ui/AeArc.h"
#include "object/ObjCommon.h"

// 0x80040B00
void loadObjCommon(const char* stgName, const char* geName)
{
	loadArc("stgobj_common.arc", &sCommonArc);
	gCommonModels = (StageModelList*)sCommonArc->mData;
	if (stgName != NULL) {
		loadArc(stgName, &sStageObjArc);
		gStageModels = (StageModelList*)sStageObjArc->mData;
	} else {
		sStageObjArc = NULL;
		gStageModels = NULL;
	}

	loadArc("geobj_common.arc", &gStgObjCommonFile);
	gStgObjCommon = (ObjCommonData*)gStgObjCommonFile->mData;
	if (geName != NULL) {
		loadArc(geName, &gGeObjCommonFile);
		gGeObjCommon = (ObjCommonData*)gGeObjCommonFile->mData;
	} else {
		gGeObjCommonFile = NULL;
		gGeObjCommon     = NULL;
	}

	void* textures = gStgObjCommon->mTextures;
	int i;
	for (i = 0; i < gStgObjCommon->mSet.mNbTexList; i++) {
		bindGvmTextures((u8*)textures, gStgObjCommon->mSet.mTexLists[i]);
	}
	for (i = 0; i < gStgObjCommon->mFix.mNbTexList; i++) {
		bindGvmTextures((u8*)textures, gStgObjCommon->mFix.mTexLists[i]);
	}
	if (gGeObjCommon != NULL) {
		textures = gGeObjCommon->mTextures;
		for (i = 0; i < gGeObjCommon->mSet.mNbTexList; i++) {
			bindGvmTextures((u8*)textures, gGeObjCommon->mSet.mTexLists[i]);
		}
		for (i = 0; i < gGeObjCommon->mFix.mNbTexList; i++) {
			bindGvmTextures((u8*)textures, gGeObjCommon->mFix.mTexLists[i]);
		}
	}
}

// 0x80040CB0
void releaseObjCommon()
{
	int i;
	if (gGeObjCommon != NULL) {
		for (i = gGeObjCommon->mFix.mNbTexList - 1; i >= 0; i--) {
			releaseTextures(gGeObjCommon->mFix.mTexLists[i]);
		}
		for (i = gGeObjCommon->mSet.mNbTexList - 1; i >= 0; i--) {
			releaseTextures(gGeObjCommon->mSet.mTexLists[i]);
		}
	}
	for (i = gStgObjCommon->mFix.mNbTexList - 1; i >= 0; i--) {
		releaseTextures(gStgObjCommon->mFix.mTexLists[i]);
	}
	for (i = gStgObjCommon->mSet.mNbTexList - 1; i >= 0; i--) {
		releaseTextures(gStgObjCommon->mSet.mTexLists[i]);
	}
	releaseResource(gGeObjCommonFile);
	releaseResource(gStgObjCommonFile);
	releaseResource(sStageObjArc);
	releaseResource(sCommonArc);
}

// 0x80040DCC
void* findStgObjData(const char* name)
{
	if (name != NULL && gStgObjCommonFile != NULL) {
		return findArcExport(gStgObjCommonFile, name);
	}
	return NULL;
}

// 0x80040E08
void* findGeObjData(const char* name)
{
	if (name != NULL && gGeObjCommonFile != NULL) {
		return findArcExport(gGeObjCommonFile, name);
	}
	return NULL;
}

// 0x80040E44
NJS_OBJECT** getStgObjModels()
{
	if (gStgObjCommon != NULL) {
		return gStgObjCommon->mSet.mModels;
	}
	return NULL;
}

// 0x80040E60
NJS_OBJECT** getGeObjModels()
{
	if (gGeObjCommon != NULL) {
		return gGeObjCommon->mSet.mModels;
	}
	return NULL;
}

// 0x80040E7C
NJS_TEXLIST** getStgObjTextures()
{
	if (gStgObjCommon != NULL) {
		return gStgObjCommon->mSet.mTexLists;
	}
	return NULL;
}

// 0x80040E98
NJS_TEXLIST** getGeObjTextures()
{
	if (gGeObjCommon != NULL) {
		return gGeObjCommon->mSet.mTexLists;
	}
	return NULL;
}
