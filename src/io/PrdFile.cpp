#include "stage/Stage.h"
#include "io/Resource.h"
#include "core/dolphin.h"
#include "io/FileReader.h"
#include "io/BinFileEx.h"

extern "C" {

PrdFile* sPrdPools[PRD_POOL_COUNT];
}

PrdFile::PrdFile(PrdFile* list)
{
	mPrev = 0;
	mNext = 0;
	if (list != 0) {
		while (list->mNext != 0) {
			list = list->mNext;
		}
		list->mNext = this;
		mPrev       = list;
	}

	mFile     = 0;
	mState    = 0;
	mRefCount = 0;
	for (int i = 0; i < 32; i++) {
		mName[i] = '\0';
	}
}

PrdFile::~PrdFile()
{
	releaseResource(mFile);
	if (mPrev != 0) {
		mPrev->mNext = mNext;
	}
	if (mNext != 0) {
		mNext->mPrev = mPrev;
	}
}

void* PrdFile::find(const char* path, u32* outSize)
{
	ARCFileInfo fileInfo;
	void* data = 0;
	u32 size   = 0;

	if (mState == 1 && ARCOpen(&mHandle, path, &fileInfo)) {
		data = ARCGetStartAddrInMem(&fileInfo);
		size = ARCGetLength(&fileInfo);
		ARCClose(&fileInfo);
	}

	if (outSize != 0) {
		*outSize = size;
	}
	return data;
}

int PrdFile::isNamed(const char* name)
{
	return (u8)(strncmp(name, mName, 32) == 0);
}

void PrdFile::addRef()
{
	mRefCount++;
}

static void prdLoadCallback(int state, int, void*, void* arg)
{
	PrdFile* prd = (PrdFile*)arg;

	if (state == 2) {
		ARCInitHandle(prd->mFile->mData, &prd->mHandle);
		DCStoreRange(prd->mFile->mData, prd->mFile->getSize());
		prd->mState    = 1;
		prd->mRefCount = 1;
	}
}

int PrdFile::startLoad(const char* name)
{
	mFile  = (BinFile*)new BinFileEx;
	mState = 2;
	FileReader::get()->read(name, mFile, prdLoadCallback, this);

	int i;
	for (i = 0; i < 31; i++) {
		char c   = name[i];
		mName[i] = c;
		if (c == '\0') {
			break;
		}
	}
	mName[31] = '\0';
	return 1;
}

void initPrdPools()
{
	for (int i = 0; i < PRD_POOL_COUNT; i++) {
		sPrdPools[i] = new PrdFile(0);
	}
}

PrdFile* getPrdPool(int pool)
{
	if (pool >= 0 && pool < PRD_POOL_COUNT) {
		return sPrdPools[pool];
	}
	return 0;
}

u8 loadPrd(int pool, const char* name)
{
	PrdFile* list = getPrdPool(pool);
	if (list == 0) {
		return 0;
	}

	PrdFile* prd;
	for (prd = list->mNext; prd != 0; prd = prd->mNext) {
		if (prd->isNamed(name)) {
			prd->addRef();
			break;
		}
	}

	if (prd == 0) {
		prd = new PrdFile(list);
		if (!prd->startLoad(name) && prd != 0) {
			delete prd;
			prd = 0;
		}
	}

	return prd != 0;
}

u8 loadBootPrd(const char* name)
{
	return loadPrd(PRD_POOL_BOOT, name);
}

u8 loadModePrd(const char* name)
{
	return loadPrd(PRD_POOL_MODE, name);
}

void freePrdPool(int pool)
{
	PrdFile* list = getPrdPool(pool);
	if (list == 0) {
		return;
	}

	PrdFile* prd = list->mNext;
	if (prd == 0) {
		return;
	}
	PrdFile* next;
	while ((next = prd->mNext) != 0) {
		prd = next;
	}

	while (prd != list) {
		PrdFile* prev = prd->mPrev;
		if (prd != 0) {
			delete prd;
		}
		prd = prev;
	}
}

void freeModePrds()
{
	freePrdPool(PRD_POOL_MODE);
}

void* findInPrds(const char* path, u32* outSize)
{
	void* data = 0;
	PrdFile* prd;

	for (int i = 0; i < PRD_POOL_COUNT; i++) {
		for (prd = sPrdPools[i]->mNext; prd != 0; prd = prd->mNext) {
			data = prd->find(path, outSize);
			if (data != 0) {
				break;
			}
		}
		if (data != 0) {
			break;
		}
	}
	return data;
}
