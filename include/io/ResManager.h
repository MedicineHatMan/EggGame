#ifndef _RESMANAGER_H
#define _RESMANAGER_H

#include "core/types.h"
#include "core/dolphin.h"
#include "io/Resource.h"
#include "Ninja/njtypes.h"

/**
 * @size 0xA088
 */
class TDvdFile : public TFile {
public:
	TDvdFile(const char* path);
	virtual ~TDvdFile();
	virtual int seek(int offset, int from);
	virtual int tell();
	virtual void cancel();
	virtual int getSize();
	virtual int getStatus();
	virtual int read(void* dest, int size);

	int fillBuffer(int pos);
	int fillAt(int pos, int size);
	int isBuffered(int pos);

	DVDFileInfo mInfo; // _04
	u8 _40[0xA020];
	u8* mBuffer;       // _A060 32 byte aligned inside the padding above
	int mBufferPos;    // _A064 -1 = empty
	int mBufferSize;   // _A068
	int mFileSize;     // _A06C
	int mBufferOffset; // _A070
	int mState;        // _A074
	int mPos;          // _A078
	int mLeft;         // _A07C
	int mUseCache;     // _A080
	u8* mDest;         // _A084
};

/**
 * One entry of ResCache.
 * @size 0x34
 */
struct ResCacheNode {
	ResCacheNode();

	ResCacheNode* mNext;     // _00 in its bucket or the free list
	ResCacheNode* mPrev;     // _04
	ResName mName;           // _08
	void* mValue;            // _28
	ResCacheNode* mListPrev; // _2C in load order
	ResCacheNode* mListNext; // _30
};

struct ResCacheBucket {
	ResCacheNode* mNext;
	ResCacheNode* mPrev;
};

/**
 * Finds loaded files by name, so loading a file twice shares it. The name hash is just the sum of its characters.
 * @size 0x24
 */
struct ResCache {
	ResCache();
	~ResCache();

	void remove(ResCacheNode* node);
	ResCacheNode* insert(const ResName* name, u8* isNew);
	ResCacheNode* find(const ResName* name);
	ResCacheNode* add(const ResName* name, void** value, u8* isNew);
	void init(int nodes, int buckets);
	void initBuckets(int count);
	ResCacheNode* allocNode();
	void removeNode(ResCacheNode* node);
	ResCacheBucket* getBucket(const ResName* name);
	ResCacheNode* findInBucket(const ResName* name, ResCacheBucket* bucket);
	void initNodes(int count);

	ResCacheNode* mNodes;     // _00
	ResCacheNode* mFree;      // _04
	ResCacheNode* mFirst;     // _08
	ResCacheNode* mLast;      // _0C
	ResCacheBucket* mBuckets; // _10
	int mBucketCount;         // _14
	int mCount;               // _18
	int mCapacity;            // _1C
	u32 _20;                  // _20 never read here gResCache's .bss span needs it
};

struct ResPackNode {
	ResPackNode* mNext;
	ResPackNode* mPrev;
	class ResPack* mPack;
};

/**
 * The files whose contents can stand in for loose files.
 * @size 0x40
 */
struct ResPackList {
	ResPackList();
	~ResPackList();

	ResPackNode* append();
	ResPackNode* allocNode();

	ResPackNode* mFirst;   // _00
	ResPackNode* mLast;    // _04
	int mCount;            // _08
	ResPackNode mNodes[4]; // _0C
	ResPackNode* mFree;    // _3C
};

// What a caller asks to have read
struct ResLoadRequest {
	ResName mName;                   // _00
	int mId;                         // _20
	void* mDest;                     // _24
	void (*mDone)(int size, int id); // _28
};

struct ResLoadNode {
	ResLoadNode();

	ResLoadNode* mNext;              // _00
	ResLoadNode* mPrev;              // _04
	ResName mName;                   // _08
	int mId;                         // _28
	void* mDest;                     // _2C
	void (*mDone)(int size, int id); // _30
};

struct ResLoadIter {
	ResLoadIter() { mNode = 0; }

	ResLoadNode* mNode;
};

/**
 * @size 0x14
 */
struct ResLoadList {
	ResLoadList();

	void removeBy(ResLoadNode* const* node);
	void popFront();
	void pushBack(const ResLoadRequest* request);
	void reserve(int count);
	void remove(ResLoadNode* node);
	void initNodes(int count);
	ResLoadIter insertBefore(ResLoadIter where, const ResLoadRequest* from);
	ResLoadNode* allocNode();

	ResLoadNode* mNodes; // _00
	ResLoadNode* mFree;  // _04
	ResLoadNode* mHead;  // _08
	int mCount;          // _0C
	int mCapacity;       // _10
};

/**
 * Reads whole files in the background, one request at a time. It is ticked once a frame.
 * @size 0x84
 */
struct TResLoader {
	TResLoader();

	void update();
	int request(const char* path, void* dest, void (*done)(int, int));
	int cancel(int id);

	s8 mState; // _00 0 = reading 1 = cancelling 2 = idle
	u8 _01[3];
	ResLoadList mList;       // _04
	int mNextId;             // _18
	DVDFileInfo mInfo;       // _1C
	ResName mName;           // _58
	int mId;                 // _78
	void* mDest;             // _7C
	void (*mDone)(int, int); // _80
};

/** One file in a ResToc. @size 0x10 */
struct ResTocEntry {
	u32 mName;   // _00 offset into ResToc::mNames
	u32 mOffset; // _04 from the start of the packed file
	u32 mSize;   // _08
	u32 _0C;     // _0C
};

/**
 * A table of contents for files packed into one big file. ResManager's openResource looks names up in it before it
 * goes to the disc. The names are ours. ResToc.cpp, 0x800686F4.
 * @size 0x28
 */
struct ResToc {
	u8 _00[0x14];
	u32 mCount; // _14
	u8 _18[8];
	ResTocEntry* mEntries; // _20
	char* mNames;          // _24
};

extern "C" {
int resTocFind(const ResToc* toc, const char* name); // 0x800686F4 -1 when missing
u32 resTocOffset(ResToc* toc, int index);            // 0x80068764
u32 resTocSize(ResToc* toc, int index);              // 0x80068778
}

class BinFile;
class ChunkFile;
class TArc;
class TResource;

extern "C" {
void addResPack(const char*);
int cancelDiscLoad(int);
void checkResName(const char*);
int findCachedArc(const char*, TArc**);
int findCachedOwned(const char*, ChunkFile**);
int findCachedSized(const char*, BinFile**);
int getResPackSize(const char*, void**);
void initResources();
int loadArc(const char*, TArc**);
int loadOwnedResource(const char*, u32, ChunkFile**);
int loadSizedResource(const char*, BinFile**);
int openResource(const char*, TResource*);
void printResources();
void registerResource(const char*, TResource*);
int releaseResource(TResource*);
extern TResLoader* sResLoaderObj;
extern int sResTocBase;
void updateResources();
}

extern "C" {
void numberTextures(NJS_TEXLIST*);
int readFromResPack(const char*, void*);
}

extern "C" {
void readResPackRange(void*, int, void*);
int loadChunkFile(const char*, int);
}

extern "C" {
int requestDiscLoad(const char* path, void* dest, void (*done)(int, int));
}

#endif
