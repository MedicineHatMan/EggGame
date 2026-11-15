#include "object/last/TObjLastBossInd.h"
struct NJS_TEXLIST;
struct NJS_OBJECT;
#include "io/ResBank.h"

static const int sLastExBossIndModels[1] = { 2 }; // 0x802CB0E8

static ResBank sLastBossBank;           // 0x80286220
static NJS_OBJECT** sLastBossModels;    // 0x802C5838
static NJS_TEXLIST** sLastBossTexLists; // 0x802C583C

int* gLastBossIndModels;
int gLastBossIndCount;
int gLastBossIndTex;
int gLastBossIndShown;

static void lastBossIndSetup();
static void lastExBossIndSetup();

// 0x8016C734
static void lastBossBankLoad(const char* arc, const char* gvm)
{
	resBankLoad(&sLastBossBank, arc);
	resBankAddTextures(&sLastBossBank, 0, gvm);
	sLastBossModels   = resBankGetModels(&sLastBossBank);
	sLastBossTexLists = resBankGetTextures(&sLastBossBank);
}

// 0x8016C7A4
static void lastBossBankFree()
{
	resBankRemoveTextures(&sLastBossBank, 0);
	resBankRelease(&sLastBossBank);
	sLastBossModels   = NULL;
	sLastBossTexLists = NULL;
}

// 0x8016C7E8
void lastBossLoad()
{
	lastBossBankLoad("ar_obj_last_boss.arc", "obj_last_boss.gvm");
	lastBossIndSetup();
	lastBossIndInit();
}

// 0x8016C820
void lastBossFree()
{
	lastBossIndExit();
	lastBossBankFree();
}

// 0x8016C844
void lastExBossLoad()
{
	lastBossBankLoad("ar_obj_last_ex_boss.arc", "obj_last_ex_boss.gvm");
	lastExBossIndSetup();
}

// 0x8016C878
void lastExBossFree()
{
	lastBossBankFree();
}

// 0x8016C898
NJS_OBJECT* lastBossGetModel(int no)
{
	return sLastBossModels[no];
}

// 0x8016C8A8
NJS_TEXLIST* lastBossGetTexList(int no)
{
	return sLastBossTexLists[no];
}

static const int sLastBossIndModels[3] __attribute__((aligned(8))) = { 0, 1, 2 }; // 0x8024A590

// 0x8016C8B8
static void lastBossIndSetup()
{
	gLastBossIndModels = (int*)sLastBossIndModels;
	gLastBossIndCount  = 3;
	gLastBossIndTex    = 0;
	lastBossIndShow();
}

// 0x8016C8F4
static void lastExBossIndSetup()
{
	gLastBossIndModels = (int*)sLastExBossIndModels;
	gLastBossIndCount  = 1;
	gLastBossIndTex    = 0;
	lastBossIndShow();
}

// 0x8016C92C
TObject* createTObjLastBossInd(TObject* parent, SetDesc* desc)
{
	return new TObjLastBossInd(parent, desc);
}

// 0x8016C980
void lastBossIndShow()
{
	gLastBossIndShown = 1;
}

// 0x8016C98C
void lastBossIndHide()
{
	gLastBossIndShown = 0;
}

extern const f32 gLastBossIndMargin = 200.0f; // 0x802CB0EC

const char* gTObjLastBossIndName = "TObjLastBossInd";
