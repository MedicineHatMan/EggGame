#ifndef _RESOURCE_H
#define _RESOURCE_H

#include "core/types.h"

struct ResName {
	char mText[0x20];
};

/** A file that can be read a piece at a time */
class TFile {
public:
	TFile() { }
#ifdef TFILE_DTOR_OUT_OF_LINE
	virtual ~TFile();
#else
	virtual ~TFile() { }
#endif                                          // _08
	virtual int seek(int offset, int from) = 0; // _0C
	virtual int tell()                     = 0; // _10
	virtual void cancel()                  = 0; // _14
	virtual int getSize()                  = 0; // _18
	virtual int getStatus()                = 0; // _1C
	virtual int read(void* dest, int size) = 0; // _20
};

/**
 * A file with a reference count, that knows how to load itself from a TFile.
 * @size 0x28
 */
class TResource {
public:
	TResource();

	int mRefCount; // _00

	// _04 = vtable
	virtual ~TResource();              // _08
	virtual int load(TFile* file) = 0; // _0C 1 = done 0 = keep calling -1 = failed
	virtual int getState()        = 0; // _10

	ResName mName; // _08
};

/**
 * A .arc file: prelinked data, relocations and a table of named exports.
 * @size 0x70
 */
class TArc : public TResource {
public:
	TArc();
	TArc(void* memory);
	virtual ~TArc();
	virtual int load(TFile* file);
	virtual int getState();

	void link(const char* name, void* value);
	void linkFrom(TArc* other);
	const char* getImportName(int index);
	int loadStep(TFile* file);

	u8 _28[4];
	int mFileSize;    // _2C the 0x20 byte header starts here
	u32 mDataSize;    // _30
	u32 mRelocSize;   // _34
	int mExportCount; // _38
	int mImportCount; // _3C
	u8 _40[0xC];
	u8* mData;        // _4C
	u8* mRelocs;      // _50
	int* mExports;    // _54 pairs of data offset and name offset
	int* mImports;    // _58 pairs of a chain into the data and name offset
	char* mNames;     // _5C
	u8 mState;        // _60 0 = new 1 = loading 2 = loaded 3 = points into memory it does not own
	u8 mStep;         // _61
	u32 mExportsSize; // _64
	u32 mImportsSize; // _68
	u32 mNamesSize;   // _6C
};

extern "C" {
int loadArc(const char* name, TArc** out);        // 0x80032E5C shares a cached arc or loads it
int releaseResource(TResource* res);              // 0x80032F04 deletes it with its last reference
void* findArcExport(TArc* arc, const char* name); // 0x800C2E34 in Arc.cpp 0 when the arc has no such export
}

#endif
