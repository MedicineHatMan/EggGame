#include "io/ResManager.h"
#include "io/ResPack.h"
#include "io/FileReader.h"
#include "io/ResFile.h"
#include "io/AramFile.h"
#include "core/dolphin.h"
#include "Dolphin/OS/OSCache.h"
#include "io/NinjaFile.h"
#include "stage/Stage.h"
#include "Ninja/ninja.h"
#include "Dolphin/os.h"
#include "core/PlacementNew.h"

static ResToc sResToc;
static ResCache gResCache;
static ResPackList gResPacks;

// 0x80032174
TDvdFile::TDvdFile(const char* path)
{
	DVDOpen((char*)path, &mInfo);
	mBuffer       = (u8*)(((u32)this + 0x5F) & ~0x1F);
	mBufferOffset = 0;
	mPos          = 0;
	mBufferPos    = -1;
	mBufferSize   = 0;
	mUseCache     = 1;
	mState        = 0;
	mFileSize     = mInfo.length;
}

TDvdFile::~TDvdFile()
{
	DVDClose(&mInfo);
}

int TDvdFile::seek(int offset, int from)
{
	switch (from) {
	case 1:
		mPos += offset;
		break;
	case 0:
		mPos = offset;
		break;
	case 2:
		mPos = mFileSize - offset;
		break;
	}
	return mPos;
}

int TDvdFile::tell()
{
	return mPos;
}

// 0x800322D8
int TDvdFile::read(void* dest, int size)
{
	if (size == 0) {
		return 1;
	}
	switch (mState) {
	case 2:
		switch (DVDGetCommandBlockStatus(&mInfo.cBlock)) {
		case 0:
			mState = 1;
			mBufferOffset = 0;
			break;
		case 10:
			mState = 5;
			break;
		default:
			mState = 6;
			return -1;
		case 1:
		case 2:
		case 4:
		case 5:
		case 6:
		case 11:
			break;
		}
		return 0;
	case 0:
	case 3:
		mLeft  = size;
		mDest  = (u8*)dest;
		mState = 1;
	case 1:
		while (mState != 3) {
			if (mFileSize < mPos + mLeft) {
				mLeft = mFileSize - mPos;
			}
			mBufferOffset = mPos - ((mBufferPos + 3) & ~3);
			if (mPos > mFileSize) {
				return -1;
			}
			if (isBuffered(mPos) > 0) {
				int offset = mBufferOffset;
				int have   = mBufferSize - offset;
				if (mLeft <= have) {
					syMemCopy(mDest, mBuffer + offset, mLeft);
					if (mUseCache != 0) {
						DCFlushRange(mDest, (mLeft + 0x1F) & ~0x1F);
					}
					mPos += mLeft;
					mLeft = 0;
					mDest = 0;
					mState = 3;
					mBufferOffset = mPos - ((mBufferPos + 3) & ~3);
					if (mBufferOffset == 0 && mPos < mFileSize) {
						fillBuffer(mPos);
						mState = 0;
					}
					return 1;
				}
				if (mUseCache != 0) {
					syMemCopy(mDest, mBuffer + offset, have);
				}
				DCFlushRange(mDest, (have + 0x1F) & ~0x1F);
				mPos += have;
				mLeft -= have;
				mDest += have;
			} else {
				return fillAt(mPos, mLeft);
			}
		}
		break;
	case 4:
		if (DVDGetCommandBlockStatus(&mInfo.cBlock) == 10) {
			mState = 5;
		}
		break;
	case 6:
		return -1;
	}
	return -1;
}

void TDvdFile::cancel()
{
	if (DVDCancelAsync(&mInfo.cBlock, 0) != 0) {
		OSReport("cancel success");
	}
	mState = 4;
}

int TDvdFile::getSize()
{
	return mFileSize;
}

// 0x800325F4
int TDvdFile::getStatus()
{
	switch (mState) {
	case 0:
		return 0;
	case 1:
	case 2:
		return 1;
	case 4:
		int status = DVDGetCommandBlockStatus(&mInfo.cBlock);
		if (status == 0 || status == 10) {
			mState = 5;
			return 3;
		}
		return 1;
	case 3:
		return 2;
	case 5:
		return 3;
	case 6:
		break;
	}
	return 4;
}

// 0x800326B0
int TDvdFile::fillBuffer(int pos)
{
	if (pos >= mFileSize || pos < 0) {
		return -1;
	}
	if (pos == mBufferPos) {
		return 0;
	}
	u32 start = (pos + 3) & ~3;
	if (mFileSize > (int)(start + 0xA000)) {
		mBufferSize = 0xA000;
	} else {
		mBufferSize = mFileSize - start;
	}
	DVDReadAsyncPrio(&mInfo, mBuffer, (mBufferSize + 0x1F) & ~0x1F, start, 0, 2);
	mBufferPos = start;
	mState     = 2;
	return 0;
}

int TDvdFile::fillAt(int pos, int size)
{
	return fillBuffer(pos);
}

// 0x80032798
int TDvdFile::isBuffered(int pos)
{
	if (mBufferPos < 0) {
		return -1;
	}
	int last = mBufferPos + (mBufferSize - 1);
	if (mBufferPos <= pos && pos <= last) {
		return 1;
	}
	return -1;
}

extern "C" void checkResName(const char* name) { }

TResLoader::TResLoader()
{
	mNextId = 0;
	mList.reserve(0x20);
	mState = 2;
}

// 0x80032830
void TResLoader::update()
{
	if (mState == 1) {
		if (DVDGetCommandBlockStatus(&mInfo.cBlock) == 10) {
			DVDClose(&mInfo);
			mState = 2;
		}
	} else if (mState == 0 && DVDGetCommandBlockStatus(&mInfo.cBlock) == 0) {
		if (mDone != 0) {
			mDone(mInfo.length, mId);
		}
		DVDClose(&mInfo);
		mState = 2;
	}
	if (mState == 2 && mList.mCount != 0) {
		ResLoadRequest* req = (ResLoadRequest*)&mList.mHead->mName;
		mState = 0;
		DVDOpen(req->mName.mText, &mInfo);
		DVDReadAsyncPrio(&mInfo, req->mDest, (mInfo.length + 0x1F) & ~0x1F, 0, 0, 2);
		resNameCopy(&mName, &req->mName);
		mId   = req->mId;
		mDest = req->mDest;
		mDone = req->mDone;
		mList.popFront();
	}
}

// 0x80032950
int TResLoader::request(const char* path, void* dest, void (*done)(int, int))
{
	ResLoadRequest request;

	if (mList.mCount == mList.mCapacity) {
		return -1;
	}
	resNameSet(&request.mName, path);
	request.mDest = dest;
	request.mDone = done;
	request.mId   = mNextId++;
	mList.pushBack(&request);
	return request.mId;
}

// 0x800329C8
int TResLoader::cancel(int id)
{
	if ((u32)mId == (u32)id && mState == 0) {
		DVDCancelAsync(&mInfo.cBlock, 0);
		mState = 1;
		return 1;
	}
	ResLoadNode* volatile it;
	for (it = mList.mHead; it != 0; it = it->mPrev) {
		if ((u32)id == (u32)it->mId) {
			ResLoadNode* node = it;
			mList.removeBy(&node);
			return 1;
		}
	}
	return 0;
}

extern "C" void initResources()
{
	gResCache.init(0x100, 0x35);
	sResLoaderObj = new TResLoader;
	initPrdPools();
}

extern "C" void updateResources()
{
	if (sResLoaderObj != 0) {
		sResLoaderObj->update();
	}
}

extern "C" int requestDiscLoad(const char* path, void* dest, void (*done)(int, int))
{
	if (sResLoaderObj != 0) {
		return sResLoaderObj->request(path, dest, done);
	}
	return -1;
}

extern "C" int cancelDiscLoad(int id)
{
	if (sResLoaderObj != 0) {
		return sResLoaderObj->cancel(id);
	}
	return 0;
}

// 0x80032B7C
extern "C" void registerResource(const char* name, TResource* res)
{
	u8 isNew;
	ResName key;

	ResCacheNode* node = gResCache.add(resNameMake(&key, name), (void**)&res, &isNew);
	resNameSet(&((TResource*)node->mValue)->mName, name);
}

// 0x80032BDC
extern "C" int openResource(const char* name, TResource* res)
{
	ResToc* toc = &sResToc;
	checkResName(name);
	int index = resTocFind(toc, name);
	if (index >= 0) {
		TFile* file = new TAramFile(sResTocBase + resTocOffset(toc, index), resTocSize(toc, index));
		resLoadAll(res, file);
		delete file;
	} else {
		TFile* file = new TDvdFile(name);
		resLoadAll(res, file);
		delete file;
	}
	return 0;
}

// 0x80032CF8
extern "C" int loadSizedResource(const char* name, BinFile** out)
{
	u32 size;

	if (findCachedSized(name, out) == 0) {
		void* inPack = findInPrds(name, &size);
		if (inPack != 0) {
			*out = new BinFile(inPack, size);
		} else {
			*out = new BinFile;
			openResource(name, *out);
		}
		registerResource(name, *out);
	}
	return 1;
}

extern "C" int loadOwnedResource(const char* name, u32 magic, ChunkFile** out)
{
	if (findCachedOwned(name, out) == 0) {
		void* inPack = findInPrds(name, 0);
		if (inPack != 0) {
			*out = new ChunkFile(magic, inPack);
		} else {
			*out = new ChunkFile(magic);
			openResource(name, *out);
		}
		registerResource(name, *out);
	}
	return 1;
}

extern "C" int loadArc(const char* name, TArc** out)
{
	if (findCachedArc(name, out) == 0) {
		void* inPack = findInPrds(name, 0);
		if (inPack != 0) {
			*out = new TArc(inPack);
		} else {
			*out = new TArc;
			openResource(name, *out);
		}
		registerResource(name, *out);
	}
	return 1;
}

// Dropping the last reference forgets the name and deletes the file
// 0x80032F04
extern "C" int releaseResource(TResource* arc)
{
	if (arc == 0) {
		return 0;
	}
	if (--arc->mRefCount == 0) {
		if (arc->mName.mText[0] != 0) {
			ResCacheNode* node = gResCache.find(&arc->mName);
			if (node != 0) {
				gResCache.remove(node);
			}
		}
		delete arc;
	}
	return 0;
}

extern "C" void printResources()
{
	int line = 0;
	for (ResCacheNode* node = gResCache.mFirst; node != 0; node = node->mListNext) {
		njPrintC((line + 5) | 0x140000, node->mName.mText);
		line++;
	}
}

// 0x80032FF8
extern "C" int loadChunkFile(const char* name, int arg)
{
	GvmLoader* chunk = new GvmLoader;
	checkResName(name);
	gvmLoaderSetList(chunk, (NJS_TEXLIST*)arg);
	void* inPack = findInPrds(name, 0);
	if (inPack != 0) {
		gvmLoaderFromMemory(chunk, (u8*)inPack);
	} else {
		TFile* file = new TDvdFile(name);
		gvmLoaderReadAll(chunk, file);
		delete file;
	}
	gvmLoaderTakeList(chunk);
	delete chunk;
	return 1;
}

static inline u32 swap32(u32 x)
{
	return (x >> 24) | ((x >> 8) & 0xFF00) | ((x & 0xFF00) << 8) | (x << 24);
}

// Parses a GVM texture bank
// 0x800330D4
extern "C" void bindGvmTextures(u8* gvm, NJS_TEXLIST* list)
{
	s16 count;
	u8* entry;
	u16 height;
	u16 width;
	u32 size;
	u32 index;
	u32 i;
	int texIndex;
	u32 format;
	int offset;
	entry = gvm;
	index = 0;
	count = *(s16*)(entry + 10);
	entry += swap32(*(u32*)(entry + 4));
	entry += 8;

	while ((int)index < count) {
		size   = swap32(*(u32*)(entry + 4));
		format = *(u32*)(entry + 8);
		height = *(u16*)(entry + 0xE);
		width  = *(u16*)(entry + 0xC);
		offset = 0;
		entry += 0x10;
		for (i = 0; i < list->nbTexture; i++) {
			if (index == (*(u32*)((u8*)list->textures + offset + 4) & 0xFFFF)) {
				GjTexMem* slot;
				GjTexHeader* tex;
				texIndex = gjTexFindFree(&slot, &tex);
				if (texIndex < 0) {
					return;
				}
				slot->hdr = tex;
				gjTexSurfaceSetup(&slot->hdr->info, (int)width, (int)height, (int)format);
				tex->info.image = entry;
				tex->texaddr    = (void*)0x101B;
				slot->count     = slot->hdr->count;
				gjTexListSetSlot(list, i, texIndex);
			}
			offset += 0xC;
		}
		index++;
		entry += size - 8;
	}
	DCStoreRange(gvm, entry - gvm);
}

extern "C" void releaseTextures(NJS_TEXLIST* list)
{
	njReleaseTexture(list);
}

// 0x8003323C
extern "C" void numberTextures(NJS_TEXLIST* list)
{
	for (u32 i = 0; i < list->nbTexture; i++) {
		list->textures[i].attr = i | (list->textures[i].attr & 0xFFFF0000);
	}
}

extern "C" void addResPack(const char* name)
{
	ResPackNode* node = gResPacks.append();
	if (node != 0) {
		node->mPack = new ResPack;
		FileReader::get()->read(name, node->mPack, 0, 0);
	}
}

// 0x800332EC
extern "C" ResPackNode* findInResPacks(const char* name, ARCFileInfo* entry)
{
	int index;
	ResPackNode* node = gResPacks.mFirst;
	while (node != 0) {
		index = ARCConvertPathToEntrynum(&node->mPack->mHandle, name);
		if (index >= 0) {
			break;
		}
		node = node->mNext;
	}
	if (node == 0) {
		return 0;
	}
	if (ARCFastOpen(&node->mPack->mHandle, index, entry) != 0) {
		return node;
	}
	return 0;
}

// 0x80033388
extern "C" int readFromResPack(const char* name, void* dest)
{
	ARCFileInfo entry;
	u8 stream[0xA8];

	ResPackNode* node = findInResPacks(name, &entry);
	if (node == 0) {
		return 0;
	}
	int pos = ARCGetStartOffset(&entry) - node->mPack->mFileStart;
	void* size = (void*)ARCGetLength(&entry);
	TAramFile* file = new (stream) TAramFile(pos + node->mPack->mAram, (u32)size);
	fileReadAll(file, dest, (u32)size);
	ARCClose(&entry);
	return 1;
}

// 0x80033430
extern "C" void readResPackRange(void* dest, int pos, void* size)
{
	u8 stream[sizeof(TAramFile)];
	TAramFile* file = new (stream) TAramFile(pos, (u32)size);
	fileReadAll(file, dest, (u32)size);
}

extern "C" int getResPackSize(const char* name, void** size)
{
	ARCFileInfo entry;

	ResPackNode* node = findInResPacks(name, &entry);
	if (node == 0) {
		return -1;
	}
	int top  = node->mPack->mAram;
	int base = node->mPack->mFileStart;
	int pos  = top + ARCGetStartOffset(&entry) - base;
	if (size != 0) {
		*size = (void*)ARCGetLength(&entry);
	}
	ARCClose(&entry);
	return pos;
}

extern "C" int findCachedArc(const char* name, TArc** out)
{
	ResName key;
	ResCacheNode* node = gResCache.find(resNameMake(&key, name));
	if (node != 0) {
		TResource* value = (TResource*)node->mValue;
		value->mRefCount++;
		*out = (TArc*)node->mValue;
		return 1;
	}
	return 0;
}

extern "C" int findCachedOwned(const char* name, ChunkFile** out)
{
	ResName key;
	ResCacheNode* node = gResCache.find(resNameMake(&key, name));
	if (node != 0) {
		TResource* value = (TResource*)node->mValue;
		value->mRefCount++;
		*out = (ChunkFile*)node->mValue;
		return 1;
	}
	return 0;
}

extern "C" int findCachedSized(const char* name, BinFile** out)
{
	ResName key;
	ResCacheNode* node = gResCache.find(resNameMake(&key, name));
	if (node != 0) {
		TResource* value = (TResource*)node->mValue;
		value->mRefCount++;
		*out = (BinFile*)node->mValue;
		return 1;
	}
	return 0;
}

ResCache::ResCache()
{
	mFree     = 0;
	mNodes    = 0;
	mBuckets  = 0;
	mBucketCount = 0;
	mLast     = 0;
	mFirst    = 0;
	mCapacity = 0;
	mCount    = 0;
}

// 0x80033678
ResCache::~ResCache()
{
	if (mBuckets != 0) {
		delete[] mBuckets;
	}
	if (mNodes != 0) {
		delete[] mNodes;
	}
}

void ResCache::remove(ResCacheNode* node)
{
	removeNode(node);
}

ResCacheNode* ResCache::insert(const ResName* name, u8* isNew)
{
	ResCacheBucket* bucket = getBucket(name);
	ResCacheNode* node     = findInBucket(name, bucket);
	if (node != 0) {
		*isNew = 0;
	} else {
		node = allocNode();
		if (node != 0) {
			resNameCopy(&node->mName, name);
			node->mNext  = bucket->mNext;
			node->mPrev  = (ResCacheNode*)bucket;
			bucket->mNext->mPrev = node;
			bucket->mNext = node;
			if (mFirst == 0) {
				mLast = node;
				mFirst = node;
				node->mListNext = 0;
				node->mListPrev = 0;
			} else {
				node->mListNext = 0;
				node->mListPrev = mLast;
				mLast->mListNext = node;
				mLast = node;
			}
			*isNew = 1;
		} else {
			*isNew = 0;
		}
	}
	return node;
}

ResCacheNode* ResCache::find(const ResName* name)
{
	return findInBucket(name, getBucket(name));
}

ResCacheNode* ResCache::add(const ResName* name, void** value, u8* isNew)
{
	ResCacheNode* node = insert(name, isNew);
	if (node != 0 && *isNew != 0) {
		node->mValue = *value;
	}
	return node;
}

// 0x80033880
void ResCache::init(int nodes, int buckets)
{
	if (mBuckets != 0) {
		delete[] mBuckets;
	}
	if (mNodes != 0) {
		delete[] mNodes;
	}
	initBuckets(buckets);
	initNodes(nodes);
	mBucketCount = buckets;
	mLast  = 0;
	mFirst = 0;
	mFree  = mNodes;
	mCount = 0;
}

// 0x8003390C
void ResCache::initBuckets(int count)
{
	mBuckets = (ResCacheBucket*)operator new[](count * sizeof(ResCacheBucket));
	for (int i = 0; i < count; i++) {
		ResCacheBucket* bucket = &mBuckets[i];
		bucket->mPrev = (ResCacheNode*)bucket;
		mBuckets[i].mNext = (ResCacheNode*)bucket;
	}
}

// 0x80033970
ResCacheNode* ResCache::allocNode()
{
	ResCacheNode* node = mFree;
	if (mFree != 0) {
		mFree = mFree->mPrev;
		mCount++;
	}
	return node;
}

// 0x8003399C
void ResCache::removeNode(ResCacheNode* node)
{
	if (node->mListPrev != 0) {
		node->mListPrev->mListNext = node->mListNext;
	} else {
		mFirst = node->mListNext;
	}
	if (node->mListNext != 0) {
		node->mListNext->mListPrev = node->mListPrev;
	} else {
		mLast = node->mListPrev;
	}
	node->mListNext = 0;
	node->mListPrev = 0;
	node->mNext->mPrev = node->mPrev;
	node->mPrev->mNext = node->mNext;
	node->mPrev = mFree;
	mFree = node;
	mCount--;
}

ResCacheBucket* ResCache::getBucket(const ResName* name)
{
	u32 hash = resNameHash(name);
	return &mBuckets[hash % mBucketCount];
}

ResCacheNode* ResCache::findInBucket(const ResName* name, ResCacheBucket* bucket)
{
	ResCacheNode* node = bucket->mPrev;
	while (node != (ResCacheNode*)bucket) {
		if (resNameCompare(&node->mName, name) == 0) {
			return node;
		}
		node = node->mPrev;
	}
	return 0;
}

// 0x80033AC8
void ResCache::initNodes(int count)
{
	mNodes = new ResCacheNode[count];
	for (int i = 0; i < count - 1; i++) {
		mNodes[i].mPrev = &mNodes[i + 1];
	}
	mNodes[count - 1].mPrev = 0;
	mCapacity = 0;
}

ResCacheNode::ResCacheNode() { }

ResPackNode* ResPackList::append()
{
	ResPackNode* node = allocNode();
	if (node == 0) {
		return 0;
	}
	if (mFirst == 0) {
		mLast  = node;
		mFirst = node;
	} else {
		node->mPrev = mLast;
		mLast->mNext = node;
		mLast = node;
	}
	mCount++;
	return node;
}

ResPackNode* ResPackList::allocNode()
{
	ResPackNode* node = mFree;
	if (node != 0) {
		mFree = node->mNext;
		node->mNext = 0;
		node->mPrev = 0;
	}
	return node;
}

// 0x80033C0C
ResPackList::ResPackList()
{
	mFirst = 0;
	mLast  = 0;
	mCount = 0;
	for (int i = 0; i < 3; i++) {
		mNodes[i].mNext = &mNodes[i + 1];
	}
	mNodes[3].mNext = 0;
	mFree = mNodes;
}

ResPackList::~ResPackList() { }

ResLoadList::ResLoadList()
{
	mNodes    = 0;
	mFree     = 0;
	mHead     = 0;
	mCapacity = 0;
	mCount    = 0;
}

// 0x80033CB8
void ResLoadList::removeBy(ResLoadNode* const* node)
{
	remove(*node);
}

void ResLoadList::popFront()
{
	if (mCount != 0) {
		remove(mHead);
	}
}

// 0x80033D0C
void ResLoadList::pushBack(const ResLoadRequest* request)
{
	insertBefore(ResLoadIter(), request);
}

// 0x80033D48
void ResLoadList::reserve(int count)
{
	if (count > mCapacity) {
		ResLoadNode* old = mNodes;
		initNodes(count);
		if (old != 0) {
			delete[] old;
		}
		mFree  = mNodes;
		mHead  = 0;
		mCount = 0;
	}
}

void ResLoadList::remove(ResLoadNode* node)
{
	if (node->mNext == node) {
		mHead = 0;
	} else if (mHead == node) {
		mHead = node->mPrev;
		node->mNext->mPrev = 0;
		if (node->mPrev != 0) {
			node->mPrev->mNext = node->mNext;
		}
	} else {
		node->mNext->mPrev = node->mPrev;
		if (node->mPrev != 0) {
			node->mPrev->mNext = node->mNext;
		} else {
			mHead->mNext = node->mNext;
		}
	}
	node->mPrev = mFree;
	mFree = node;
	mCount--;
}

// 0x80033E44
void ResLoadList::initNodes(int count)
{
	mNodes = new ResLoadNode[count];
	for (int i = 0; i < count - 1; i++) {
		mNodes[i].mPrev = &mNodes[i + 1];
	}
	mNodes[count - 1].mPrev = 0;
	mCapacity = count;
}

ResLoadNode::ResLoadNode() { }

// 0x80033EEC
ResLoadIter ResLoadList::insertBefore(ResLoadIter where, const ResLoadRequest* from)
{
	ResLoadIter result;
	ResLoadNode* before = where.mNode;
	ResLoadNode* node   = allocNode();
	if (before == 0) {
		if (mHead == 0) {
			node->mNext = node;
			node->mPrev = 0;
			mHead = node;
		} else {
			node->mNext = mHead->mNext;
			node->mPrev = 0;
			mHead->mNext->mPrev = node;
			mHead->mNext = node;
		}
	} else {
		if (mHead == before) {
			mHead = node;
		} else {
			before->mNext->mPrev = node;
		}
		node->mNext = before->mNext;
		node->mPrev = before;
		before->mNext = node;
	}
	resNameCopy(&node->mName, &from->mName);
	node->mId   = from->mId;
	node->mDest = from->mDest;
	node->mDone = from->mDone;
	result.mNode = node;
	return result;
}

// 0x80033FD0
ResLoadNode* ResLoadList::allocNode()
{
	ResLoadNode* node = mFree;
	if (node != 0) {
		mFree = mFree->mPrev;
		mCount++;
	} else {
		OSPanic("dlist.cp", 0xE1, "dlist alloc error");
	}
	return node;
}
