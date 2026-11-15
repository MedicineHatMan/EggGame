#ifndef _OBJECT_YELLOW_TOBJYELLOWSAND_H_
#define _OBJECT_YELLOW_TOBJYELLOWSAND_H_
struct BinFile;
struct SetDesc;
struct ResBank;
struct NJS_OBJECT;
struct NJS_TEXLIST;

#include "core/types.h"

class TObject;
struct Vec3f;

extern "C" {
void createSandStorm(TObject*, const Vec3f*, const Vec3f*);
void initYellowSand();
void termYellowSand();
void updateSandBoardTimer(int*, f32);
}

extern "C" {
TObject* createTObjYellowSandGen(TObject*, SetDesc*);
TObject* createTObjYellowSandMdl(TObject*, SetDesc*);
}

extern "C" {
extern u32 sYellowSandClearColor;
extern ResBank sYellowSandBank;
extern NJS_OBJECT** sYellowSandModels;
extern NJS_TEXLIST** sYellowSandTextures;
extern BinFile* sYellowSandGvm;
}

#endif
