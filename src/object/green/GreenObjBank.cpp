#include "object/green/GreenObjBank.h"
struct NJS_TEXLIST;
struct NJS_OBJECT;
#include "io/ResBank.h"

static ResBank sGreenObjBank;           // 0x80286638
static NJS_OBJECT** sGreenObjModels;    // 0x802C5988
static NJS_TEXLIST** sGreenObjTexLists; // 0x802C598C

// 0x801901D0
void greenObjLoad()
{
	resBankLoad(&sGreenObjBank, "ar_obj_green_boss.arc");
	resBankAddTextures(&sGreenObjBank, 0, "obj_green_boss.GVM");
	sGreenObjModels   = resBankGetModels(&sGreenObjBank);
	sGreenObjTexLists = resBankGetTextures(&sGreenObjBank);
}

// 0x80190238
void greenObjFree()
{
	resBankRemoveTextures(&sGreenObjBank, 0);
	resBankRelease(&sGreenObjBank);
	sGreenObjModels   = NULL;
	sGreenObjTexLists = NULL;
}

// 0x8019027C
NJS_OBJECT** getGreenObjModels()
{
	return sGreenObjModels;
}

// 0x80190284
NJS_TEXLIST** getGreenObjTextures()
{
	return sGreenObjTexLists;
}

// 0x8019028C
// The second green set pair does nothing
void greenObjInit()
{
}

// 0x80190290
void greenObjExit()
{
}
