#ifndef _UI_CONSTPALETTE_H_
#define _UI_CONSTPALETTE_H_
struct ConstPaletteFile;
struct ConstPalette;

#include "core/types.h"

extern "C" {
void constPaletteInit();
}

extern "C" {
void setConstPalette(ConstPalette*);
}

/** What ge_constplt.bin starts with */
struct ConstPaletteFile {
	u16 mCount; // _00
	u8 _02[2];
	u32 mEntries;   // _04 offset from the start
	u32 mTables;    // _08 offset from the start
	u32 mTableSize; // _0C
};

/** One table out of ge_constplt.bin. @size 0xC */
class ConstPalette {
public:
	ConstPalette();
	void init(ConstPaletteFile* file, int index);

	u16 mCount; // _00
	u8 _02[2];
	u8* mEntries; // _04
	u8* mTable;   // _08
};

extern "C" {
extern ConstPalette* sConstPalette1;
}

#endif
