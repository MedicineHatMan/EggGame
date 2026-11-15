#include "io/ResManager.h"
#include "core/dolphin.h"

// 0x800686F4
extern "C" int resTocFind(const ResToc* toc, const char* name)
{
	u32 count;
	char* names;
	ResTocEntry* entry;
	entry = toc->mEntries;
	names = toc->mNames;
	count = toc->mCount;
	for (u32 i = 0; i < count; i++, entry++) {
		if (strcmp(name, names + entry->mName) == 0) {
			return i;
		}
	}
	return -1;
}

// 0x80068764
extern "C" u32 resTocOffset(ResToc* toc, int index)
{
	return toc->mEntries[index].mOffset;
}

// 0x80068778
extern "C" u32 resTocSize(ResToc* toc, int index)
{
	return toc->mEntries[index].mSize;
}
