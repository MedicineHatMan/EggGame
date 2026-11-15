#include "stage/Stage.h"
#include "io/ResFile.h"
#include "ui/ConstPalette.h"

BinFile* sConstPaletteFile;   // 0x802C4FC0
ConstPalette* sConstPalette0; // 0x802C4FC4
ConstPalette* sConstPalette1; // 0x802C4FC8 battle mode's
ConstPalette* sConstPalette;  // 0x802C4FCC the one in use

// 0x8006C470
ConstPalette::ConstPalette()
{
	mCount = 0;
}

// 0x8006C47C
void ConstPalette::init(ConstPaletteFile* file, int index)
{
	mEntries = (u8*)file + file->mEntries;
	mCount   = file->mCount;
	mTable   = (u8*)file + file->mTables + file->mTableSize * index;
}

// 0x8006C4AC
extern "C" void constPaletteInit()
{
	loadSizedResource("ge_constplt.bin", &sConstPaletteFile);
	sConstPalette0 = new ConstPalette;
	sConstPalette0->init((ConstPaletteFile*)sConstPaletteFile->mData, 0);
	sConstPalette1 = new ConstPalette;
	sConstPalette1->init((ConstPaletteFile*)sConstPaletteFile->mData, 1);
	setConstPalette(sConstPalette0);
}

// 0x8006C540
extern "C" void setConstPalette(ConstPalette* palette)
{
	sConstPalette = palette;
}
