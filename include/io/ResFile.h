#ifndef _RESFILE_H
#define _RESFILE_H

#include "io/Resource.h"

struct NinjaFile;

/**
 * A whole file read into memory as it is. loadSizedResource makes these. When the file is in a pack the data
 * points into the pack instead, and is not freed. Vtable 0x80255A0C.
 *
 * @size 0x38
 */
class BinFile : public TResource {
public:
	BinFile();                        // 0x8004CCFC
	BinFile(void* data, u32 size);    // 0x8004CD48 for a file in a pack
	virtual ~BinFile();               // 0x8004CD9C
	virtual int load(TFile* file);    // 0x8004CDFC
	virtual int getState();           // 0x8004CEBC 0 = loading 1 = done 2 = failed

	void free();                      // 0x8004CEF0

	inline u32 getSize() { return mSize; }

	u8 _28[4];
	void* mData; // _2C
	u32 mSize;   // _30
	int mState;  // _34 0 = new 1 = reading 2 = done 3 = points into memory it does not own
};

/**
 * A Ninja chunk file with its POF0 table (see ninjaFileFromMemory). loadOwnedResource makes these, with the chunk
 * magic it expects. Vtable 0x802559F8.
 *
 * @size 0x40
 */
class ChunkFile : public TResource {
public:
	ChunkFile(u32 magic);             // 0x8004CF40
	ChunkFile(u32 magic, void* mem);  // 0x8004CFCC for a file in a pack
	virtual ~ChunkFile();             // 0x8004D07C
	virtual int load(TFile* file);    // 0x8004D108
	virtual int getState();           // 0x8004D19C

	u8 _28[4];
	int mState;        // _2C 0 = new 1 = reading 2 = done 3 = points into memory it does not own
	NinjaFile* mFile;  // _30
	void* mData;       // _34
	u32 mKinds[2];     // _38 the magic to accept then 0 for NinjaFile::mKinds
};

extern "C" int loadOwnedResource(const char* name, u32 magic, ChunkFile** out); // finds or reads a chunk file
extern "C" int loadSizedResource(const char* name, BinFile** out); // 0x80032CF8 shares a cached file or reads it

#endif
