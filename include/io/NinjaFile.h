#ifndef _IO_NINJAFILE_H_
#define _IO_NINJAFILE_H_
#include "Ninja/ninja.h"
struct GvrHeader;
#include "Ninja/njtypes.h"
struct GvmHeader;
struct GvmLoader;
struct NinjaFile;
struct TFile;
struct TResource;

#include "core/types.h"

struct NJS_TEXLIST;
struct ResName;

extern "C" {
int debugEditActive();
void debugEditDraw();
void debugEditExec();
void gvmCopy16(u8*, u8*);
void gvmCopy32(u8*, u8*);
u8* gvmLoadFromMemory(void*, NJS_TEXLIST*);
int gvmRecordSize(u16);
void ninjaApplyPof(void*, void*, u32);
int resNameCompare(const ResName*, const ResName*);
ResName* resNameCopy(ResName*, const ResName*);
u32 resNameHash(const ResName*);
ResName* resNameMake(ResName*, const char*);
}

extern "C" {
int debugEditInit();
int resLoadAll(TResource*, TFile*);
void ninjaSwap32(u32*);
NinjaFile* ninjaFileInit(NinjaFile*);
NinjaFile* ninjaFileFree(NinjaFile*, s16);
int ninjaFileRead(NinjaFile*, TFile*);
void* ninjaFileTake(NinjaFile*, u32*);
void ninjaFileExpect(NinjaFile*, int, void*, u32);
void* ninjaFileAlloc(NinjaFile*, u32, u32, u32*);
u8* ninjaFileFromMemory(NinjaFile*, u8*);
void gvmLoaderReset(GvmLoader*);
void gvmLoaderFreeInfo(GvmLoader*);
void gvmLoaderRelease(GvmLoader*);
int gvmLoaderRead(GvmLoader*, TFile*);
int gvmLoaderStep(GvmLoader*, TFile*, int);
int gvmLoaderNeedsImage(GvmLoader*, int);
int gvmLoadTextures(NJS_TEXLIST*, GvmHeader*, u8*);
void gvmLoaderExpect(GvmLoader*, int, void*, u32);
void gvmSwap32(u32*);
void gvmRecordInfo(GvmHeader*, u8*, u16*, u16*, u16*);
void gvmReadRecords(NJS_TEXLIST*, GvmHeader*, u8*);
}

extern "C" {
ResName* resNameSet(ResName*, const char*);
void gvmLoaderSetList(GvmLoader*, NJS_TEXLIST*);
NJS_TEXLIST* gvmLoaderTakeList(GvmLoader*);
int gvmLoaderReadAll(GvmLoader*, TFile*);
}

extern "C" {
int debugInfoActive();
void gvmLoaderFromMemory(GvmLoader*, u8*);
}

/** @size 0x28 */
struct NinjaFile {
	void* mData;  // _00
	void* mPof;   // _04
	int mState;   // _08
	int mStep;    // _0C
	u32 mMagic;   // _10
	u32 mSize;    // _14
	u32 mKind;    // _18 '****' until known
	void* mDst;   // _1C
	u32 mDstSize; // _20
	u32* mKinds;  // _24 more magics to accept ending with 0
};

/** @size 0xC */
struct GvmHeader {
	u32 mMagic; // _00 GVMH
	u32 mSize;  // _04 from _08 to the first GVR
	u16 mFlags; // _08 GvmHeaderFlags
	s16 mCount; // _0A
};

/** @size 0x10 */
struct GvrHeader {
	u32 mMagic;      // _00 GVRT
	u32 mSize;       // _04 from _08 to the next GVR
	GjTexDesc mDesc; // _08
};

/** Loads a GVM a step at a time. @size 0x40 */
struct GvmLoader {
	GvmLoader();
	~GvmLoader();

	NJS_TEXLIST* mList; // _00
	int mState;         // _04 a GvmLoaderState
	int mStep;          // _08 a GvmStep
	void* mDst;         // _0C
	u32 mDstSize;       // _10
	GvrHeader mTexture; // _14
	GvmHeader mHeader;  // _24
	u32 mNext;          // _30 where the next GVR starts
	u8* mInfo;          // _34 the records their first two bytes become 0 for a texture to read
	int mInfoSize;      // _38 of a record
	int mIndex;         // _3C the texture being read -1 before
};

extern "C" {
extern u8 gjIdentityMatrix[];
extern NJS_TEXERRSTAT gTexErrStat;
extern u16 sGvmSizes[12];
}

#endif
