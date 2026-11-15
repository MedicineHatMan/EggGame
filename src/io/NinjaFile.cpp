#define TFILE_DTOR_OUT_OF_LINE

#include "core/types.h"
#include "Ninja/ninja.h"
#include "Dolphin/OS/OSCache.h"
#include "io/NinjaFile.h"
#include "io/Resource.h"
#include "core/dolphin.h"
#include "core/Mem.h"
#include "stl/string.h"
#include "stl/string.h"

enum NinjaState {
	NINJA_START   = 0,
	NINJA_READING = 1,
	NINJA_DONE    = 2,
	NINJA_STATIC  = 3, // The memory isn't ours to free
};

enum NinjaStep {
	NINJA_STEP_MAGIC,
	NINJA_STEP_SIZE,
	NINJA_STEP_DATA,
	NINJA_STEP_POF_MAGIC,
	NINJA_STEP_POF_SIZE,
	NINJA_STEP_POF_DATA,
};

// The chunk names as they read after the swap
#define GVM_GVMH 0x484D5647
#define GVM_GVRT 0x54525647

enum GvmHeaderFlags {
	GVM_HAS_NAME   = 0x1,   // a 0x1C byte name after the index
	GVM_HAS_FORMAT = 0x2,   // two bytes of format
	GVM_HAS_SIZE   = 0x4,   // a byte of log2 width and height
	GVM_HAS_GBIX   = 0x8,   // a global index
	GVM_EXTERNAL   = 0x400, // the global indexes are someone else's
};

enum GvmLoaderState {
	GVM_START   = 0,
	GVM_READING = 1,
	GVM_DONE    = 2,
	GVM_STATIC  = 3, // loaded from memory
};

enum GvmStep {
	GVM_STEP_HEADER  = 0,
	GVM_STEP_RECORDS = 1,
	GVM_STEP_TEXTURE = 3, // a GVR's GVRT header
	GVM_STEP_IMAGE   = 4,
};

extern "C" {
// 0x802A5950
// 0x80240A10
// Texture sides from the log2 in a record

}

// 0x80018E10
// The debug tools are stubbed out in this build
extern "C" void debugEditExec()
{
}

// 0x80018E14
extern "C" void debugEditDraw()
{
}

// 0x80018E18
// The stage calls it once it is set up
extern "C" int debugEditInit()
{
	return 0;
}

// 0x80018E20
// Would skip the range checks and keep the pause menu shut
extern "C" int debugEditActive()
{
	return 0;
}

// 0x80018E28
// Would let objects draw their debug shapes
extern "C" int debugInfoActive()
{
	return 0;
}

// 0x80018E30
TFile::~TFile()
{
}

// 0x80018E78
TResource::TResource()
{
	mRefCount = 1;
}

// 0x80018E90
TResource::~TResource()
{
}

// 0x80018ED8
// Runs load until it finishes
int resLoadAll(TResource* res, TFile* file)
{
	int done = 0;
	while (done == 0) {
		done = res->load(file);
		if (done < 0) {
			break;
		}
	}
	return done;
}

// 0x80018F34
// A simple hash of a name the sum of its characters
u32 resNameHash(const ResName* name)
{
	const char* c = name->mText;
	int sum       = 0;
	while (*c != 0) {
		sum += *c;
		c++;
	}
	return sum;
}

// 0x80018F5C
// The key for a lookup
ResName* resNameMake(ResName* dst, const char* src)
{
	strcpy(dst->mText, src);
	return dst;
}

// 0x80018F8C
ResName* resNameSet(ResName* dst, const char* src)
{
	strcpy(dst->mText, src);
	return dst;
}

// 0x80018FBC
ResName* resNameCopy(ResName* dst, const ResName* src)
{
	strcpy(dst->mText, src->mText);
	return dst;
}

// 0x80018FEC
int resNameCompare(const ResName* a, const ResName* b)
{
	return strcmp(a->mText, b->mText);
}

// 0x8001900C
// A little endian word to big endian in place
asm void ninjaSwap32(u32* word)
{
	nofralloc
	lwz r4, 0(r3)
	stwbrx r4, r0, r3
	blr
}

// 0x80019018
NinjaFile* ninjaFileInit(NinjaFile* file)
{
	file->mData  = NULL;
	file->mPof   = NULL;
	file->mState = NINJA_START;
	file->mKind  = '****';
	file->mKinds = NULL;
	return file;
}

// 0x8001903C
NinjaFile* ninjaFileFree(NinjaFile* file, s16 flags)
{
	if (file != NULL) {
		if (file->mState != NINJA_STATIC) {
			if (file->mData != NULL) {
				memFree(file->mData);
			}
			if (file->mPof != NULL) {
				memFree(file->mPof);
			}
		}
		if (flags > 0) {
			delete file;
		}
	}
	return file;
}

// 0x800190AC
// Reads more of the file
int ninjaFileRead(NinjaFile* file, TFile* src)
{
	int ret;

	switch (file->mState) {
	case NINJA_DONE:
		if (file->mData != NULL) {
			memFree(file->mData);
			file->mData = NULL;
		}
		file->mState = NINJA_START;
	case NINJA_START:
		ninjaFileExpect(file, NINJA_STEP_MAGIC, &file->mMagic, 4);
		file->mState = NINJA_READING;
	case NINJA_READING:
		while ((ret = src->read(file->mDst, file->mDstSize)) > 0) {
			switch (file->mStep) {
			case NINJA_STEP_MAGIC:
				ninjaFileExpect(file, NINJA_STEP_SIZE, &file->mSize, 4);
				break;
			case NINJA_STEP_SIZE:
				ninjaSwap32(&file->mSize);
				file->mData = ninjaFileAlloc(file, file->mMagic, file->mSize, &file->mKind);
				if (file->mData == NULL) {
					file->mState = NINJA_DONE;
					return 1;
				}
				ninjaFileExpect(file, NINJA_STEP_DATA, file->mData, file->mSize);
				break;
			case NINJA_STEP_DATA:
				DCFlushRange(file->mData, file->mSize);
				ninjaFileExpect(file, NINJA_STEP_POF_MAGIC, &file->mMagic, 4);
				break;
			case NINJA_STEP_POF_MAGIC:
				ninjaFileExpect(file, NINJA_STEP_POF_SIZE, &file->mSize, 4);
				break;
			case NINJA_STEP_POF_SIZE:
				ninjaSwap32(&file->mSize);
				if (file->mMagic != 'POF0') {
					file->mState = NINJA_DONE;
					return 1;
				}
				file->mPof = memAlloc(file->mSize);
				ninjaFileExpect(file, NINJA_STEP_POF_DATA, file->mPof, file->mSize);
				break;
			case NINJA_STEP_POF_DATA:
				ninjaApplyPof(file->mData, file->mPof, file->mSize);
				memFree(file->mPof);
				file->mPof   = NULL;
				file->mState = NINJA_DONE;
				return 1;
			}
		}
		return ret;
	default:
		return -1;
	}
}

// 0x800192C0
void* ninjaFileTake(NinjaFile* file, u32* kind)
{
	void* data  = file->mData;
	file->mData = NULL;
	if (kind != NULL) {
		*kind = file->mKind;
	}
	return data;
}

// 0x800192E4
// What the next read fills
void ninjaFileExpect(NinjaFile* file, int step, void* dst, u32 size)
{
	file->mStep    = step;
	file->mDst     = dst;
	file->mDstSize = size;
}

// 0x800192F4
// Allocates for a chunk of a known kind
void* ninjaFileAlloc(NinjaFile* file, u32 magic, u32 size, u32* kind)
{
	switch (magic) {
	case 'GCAM':
	case 'GJBM':
	case 'GJCM':
	case 'GJTL':
	case 'GLIM':
	case 'GMDM':
	case 'GSSM':
	case 'NJCA':
	case 'NJCM':
	case 'NJIN':
	case 'NJLI':
	case 'NJTL':
	case 'NMDM':
		*kind = magic;
		return memAlloc(size);
	}
	if (file->mKinds != NULL) {
		for (int i = 0; file->mKinds[i] != 0; i++) {
			if (magic == file->mKinds[i]) {
				*kind = magic;
				return memAlloc(size);
			}
		}
	}
	*kind = '****';
	return NULL;
}

// 0x8001945C
// Applies POF0 the top two bits of a byte say how many more bytes hold the distance to the next word to fix in words
void ninjaApplyPof(void* data, void* pof, u32 size)
{
	u8* src = (u8*)pof;
	u32 offset = 0;
	while (size != 0) {
		u8 b0, b1, b2, b3;
		syMemCopy(&b0, src, 1);
		src++;
		size--;
		u32 delta;
		switch (b0 & 0xC0) {
		case 0x00:
			continue;
		case 0x40:
			delta = b0 & 0x3F;
			break;
		case 0x80:
			syMemCopy(&b1, src, 1);
			src++;
			size--;
			delta = ((b0 & 0x3F) << 8) | b1;
			break;
		case 0xC0:
			syMemCopy(&b1, src, 1);
			syMemCopy(&b2, src + 1, 1);
			syMemCopy(&b3, src + 2, 1);
			src += 3;
			size -= 3;
			delta = ((b0 & 0x3F) << 24) | (b1 << 16) | (b2 << 8) | b3;
			break;
		default:
			continue;
		}
		offset += delta << 2;
		u32* word = (u32*)((u8*)data + offset);
		if (*word != 0) {
			*word += (u32)data;
		}
	}
}

// 0x80019590
// The same file when it's already in memory
u8* ninjaFileFromMemory(NinjaFile* file, u8* mem)
{
	u32 size = *(u32*)(mem + 4);
	ninjaSwap32(&size);
	u8* src = mem + 8;
	syMemCopy(mem, src, size);
	file->mData = mem;
	src += size;
	u32 pofSize = *(u32*)(src + 4);
	ninjaSwap32(&pofSize);
	src += 8;
	file->mPof = src;
	src += pofSize;
	ninjaApplyPof(file->mData, file->mPof, pofSize);
	DCFlushRange(file->mData, size);
	file->mState = NINJA_STATIC;
	return src;
}

// 0x80019630
GvmLoader::GvmLoader()
{
	mList = NULL;
	mInfo = NULL;
	gvmLoaderFreeInfo(this);
	gvmLoaderReset(this);
}

// 0x80019674
GvmLoader::~GvmLoader()
{
	gvmLoaderFreeInfo(this);
	gvmLoaderRelease(this);
}

// 0x800196C4
void gvmLoaderSetList(GvmLoader* loader, NJS_TEXLIST* list)
{
	gvmLoaderRelease(loader);
	gvmLoaderFreeInfo(loader);
	loader->mList = list;
	gvmLoaderReset(loader);
}

// 0x80019708
NJS_TEXLIST* gvmLoaderTakeList(GvmLoader* loader)
{
	NJS_TEXLIST* list = loader->mList;
	loader->mList     = NULL;
	return list;
}

// 0x8001971C
void gvmLoaderReset(GvmLoader* loader)
{
	loader->mState = GVM_START;
	loader->mIndex = -1;
}

// 0x80019730
void gvmLoaderFreeInfo(GvmLoader* loader)
{
	if (loader->mInfo != NULL) {
		memFree(loader->mInfo);
		loader->mInfo = NULL;
	}
}

// 0x80019770
// Gives back what a half-done load took
void gvmLoaderRelease(GvmLoader* loader)
{
	if (loader->mList != NULL && loader->mIndex >= 0) {
		njReleaseTexture(loader->mList);
	}
}

// 0x800197AC
int gvmLoaderReadAll(GvmLoader* loader, TFile* file)
{
	int done = 0;
	while (done == 0) {
		done = gvmLoaderRead(loader, file);
		if (done < 0) {
			break;
		}
	}
	return done;
}

// 0x800197FC
// Reads more of the file
int gvmLoaderRead(GvmLoader* loader, TFile* file)
{
	int ret;

	switch (loader->mState) {
	case GVM_START:
		gvmLoaderExpect(loader, GVM_STEP_HEADER, &loader->mHeader, 0xC);
		loader->mState = GVM_READING;
	case GVM_READING:
		while ((ret = file->read(loader->mDst, loader->mDstSize)) > 0) {
			switch (gvmLoaderStep(loader, file, loader->mStep)) {
			case 0:
				break;
			case 1:
				loader->mState = GVM_DONE;
				return 1;
			case -1:
				return -1;
			}
		}
		return ret;
	case GVM_DONE:
		return 1;
	default:
		return -1;
	}
}

// 0x800198E8
// Handles what the last read brought in
int gvmLoaderStep(GvmLoader* loader, TFile* file, int step)
{
	GvmHeader* header  = &loader->mHeader;
	GvrHeader* texture = &loader->mTexture;

	switch (step) {
	case GVM_STEP_HEADER: {
		gvmSwap32(&header->mMagic);
		gvmSwap32(&header->mSize);
		if (header->mMagic != GVM_GVMH) {
			gTexErrStat.err = 4;
			return -1;
		}
		if (header->mCount == 0) {
			return 1;
		}
		loader->mNext        = header->mSize + file->tell() - 4;
		int count            = header->mCount;
		int size             = gvmRecordSize(header->mFlags);
		loader->mInfoSize    = size;
		loader->mList->nbTexture = count;
		if (loader->mInfo != NULL) {
			memFree(loader->mInfo);
		}
		size          = count * size;
		loader->mInfo = (u8*)memAlloc(size);
		gvmLoaderExpect(loader, GVM_STEP_RECORDS, loader->mInfo, size);
		break;
	}
	case GVM_STEP_RECORDS:
		if (gvmLoadTextures(loader->mList, header, loader->mInfo) < 0) {
			return -1;
		}
		file->seek(loader->mNext, 0);
		loader->mIndex = 0;
		gvmLoaderExpect(loader, GVM_STEP_TEXTURE, texture, 0x10);
		break;
	case GVM_STEP_TEXTURE:
		gvmSwap32(&texture->mMagic);
		gvmSwap32(&texture->mSize);
		loader->mNext = texture->mSize + file->tell() - 8;
		if (texture->mMagic != GVM_GVRT) {
			gTexErrStat.err = 4;
			return -1;
		}
		if (gvmLoaderNeedsImage(loader, loader->mIndex)) {
			GjTexMem* mem = loader->mList->textures[loader->mIndex].texaddr;
			gvmLoaderExpect(loader, GVM_STEP_IMAGE, mem->hdr->info.image, gjTexDescSize(&texture->mDesc));
			break;
		}
	case GVM_STEP_IMAGE:
		if (gvmLoaderNeedsImage(loader, loader->mIndex)) {
			DCFlushRange(loader->mDst, loader->mDstSize);
		}
		loader->mIndex++;
		if (loader->mIndex == header->mCount) {
			return 1;
		}
		file->seek(loader->mNext, 0);
		gvmLoaderExpect(loader, GVM_STEP_TEXTURE, &loader->mTexture, 0x10);
		return 0;
	default:
		return -1;
	}
	return 0;
}

// 0x80019B9C
// Whether texture i still has to be read
int gvmLoaderNeedsImage(GvmLoader* loader, int index)
{
	return *(u16*)(loader->mInfo + index * loader->mInfoSize) == 0 ? 1 : 0;
}

// 0x80019BB8
// Fills the list from the records and makes room for each texture that isn't loaded yet
int gvmLoadTextures(NJS_TEXLIST* list, GvmHeader* header, u8* record)
{
	int size;
	int num;
	int count;
	int i;
	u32 gbix;
	s32 bank = -1;

	size  = gvmRecordSize(header->mFlags);
	count = list->nbTexture;
	gvmReadRecords(list, header, record);
	for (i = 0; i < count; i++) {
		u16 format;
		u16 width;
		u16 height;
		gvmRecordInfo(header, record, &format, &width, &height);
		u32 index = i;
		for (num = 0; num < count; num++) {
			if (i == (list->textures[num].attr & 0xFFFF)) {
				break;
			}
		}
		s32 ret = gjTexListDecode(list, index, &gbix, &bank);
		if (ret == -1) {
			gTexErrStat.err = 7;
			return -1;
		}
		if (ret == 0) {
			GjTexDesc desc;
			s32 slot;
			desc.format = format;
			desc.width  = width;
			desc.height = height;
			switch (gjTexListBind(list, num, gbix, bank, &desc, &slot)) {
			case -1:
				gTexErrStat.err = 6;
				return -1;
			case 0:
				*(u16*)record = 0;
				break;
			case 1:
				*(u16*)record = 1;
				break;
			}
		}
		record += size;
	}
	return 0;
}

// 0x80019D3C
// What the next read fills
void gvmLoaderExpect(GvmLoader* loader, int step, void* dst, u32 size)
{
	loader->mStep    = step;
	loader->mDst     = dst;
	loader->mDstSize = size;
}

// 0x80019D4C
// The records aren't aligned
void gvmCopy16(u8* dst, u8* src)
{
	*dst++ = *src++;
	*dst++ = *src++;
}

// 0x80019D60
void gvmCopy32(u8* dst, u8* src)
{
	*dst++ = *src++;
	*dst++ = *src++;
	*dst++ = *src++;
	*dst++ = *src++;
}

// 0x80019D84
// The size of one texture's record in the GVMH from the header's flags
int gvmRecordSize(u16 flags)
{
	int size = 2;
	if (flags & GVM_HAS_NAME) {
		size = 0x1E;
	}
	if (flags & GVM_HAS_FORMAT) {
		size += 2;
	}
	if (flags & GVM_HAS_SIZE) {
		size += 2;
	}
	if (flags & GVM_HAS_GBIX) {
		size += 4;
	}
	return size;
}

// 0x80019DC0
asm void gvmSwap32(u32* word)
{
	nofralloc
	lwz r4, 0(r3)
	stwbrx r4, r0, r3
	blr
}

// 0x80019DCC
// The format and size a record gives
void gvmRecordInfo(GvmHeader* header, u8* record, u16* format, u16* width, u16* height)
{
	u32 flags  = header->mFlags;
	int offset = 2;
	if (flags & GVM_HAS_NAME) {
		offset = 0x1E;
	}
	if (flags & GVM_HAS_FORMAT) {
		gvmCopy16((u8*)format, record + offset);
		offset += 2;
	}
	if (flags & GVM_HAS_SIZE) {
		u16 size;
		gvmCopy16((u8*)&size, record + offset);
		*width  = sGvmSizes[size & 0xF];
		*height = sGvmSizes[(size >> 4) & 0xF];
	}
}

// 0x80019E64
// Fills the texture list from the GVMH records
void gvmReadRecords(NJS_TEXLIST* list, GvmHeader* header, u8* record)
{
	u16 flags     = header->mFlags;
	int hasName   = flags & GVM_HAS_NAME;
	int hasFormat = flags & GVM_HAS_FORMAT;
	int hasSize   = flags & GVM_HAS_SIZE;
	int external  = flags & GVM_EXTERNAL;
	int hasId     = flags & GVM_HAS_GBIX;
	int count     = header->mCount;
	NJS_TEXNAME* entry;
	int size;
	int i;

	for (i = 0; i < count; i++) {
		entry = &list->textures[i];
		u16 index;
		gvmCopy16((u8*)&index, record);
		size        = 2;
		entry->attr = (entry->attr & 0xFFFF0000) | index;
		if (hasName) {
			u32 len = strlen((char*)record + 2);
			if ((int)len > 0) {
				syMemCopy(entry->filename, record + 2, len);
			}
			size = 0x1E;
		}
		if (hasFormat) {
			size += 2;
		}
		if (hasSize) {
			size += 2;
		}
		entry->texaddr = NULL;
		if (external) {
			if (hasId) {
				gvmCopy32((u8*)&entry->texaddr, record + size);
				size += 4;
				entry->attr |= 0x1000000;
			}
		} else if (hasId) {
			gvmCopy32((u8*)&entry->texaddr, record + size);
			size += 4;
			entry->attr |= 0x800000;
		}
		record += size;
	}
}

// 0x80019F90
// Loads a whole GVM that's already in memory using the image data where it is
u8* gvmLoadFromMemory(void* file, NJS_TEXLIST* list)
{
	char* data = (char*)file;
	GjTexMem* mem;
	GjTexHeader* hdr;
	u32 size;
	u32 i;
	u8* dst;
	s32 slot;
	int format;
	int count;
	u8* p;
	u8* next;
	GvmHeader* header;
	u16 width;
	u16 height;

	p      = (u8*)data;
	header = (GvmHeader*)p;
	size   = header->mSize;
	gvmSwap32(&size);
	gvmReadRecords(list, header, p + 0xC);
	count = header->mCount;
	p += size;
	p += 8;
	for (i = 0; (int)i < count; i++) {
		size = ((GvrHeader*)p)->mSize;
		gvmSwap32(&size);
		format = ((GvrHeader*)p)->mDesc.format;
		next   = p + size;
		width  = ((GvrHeader*)p)->mDesc.width;
		height = ((GvrHeader*)p)->mDesc.height;
		p += 0x10;
		next += 8;
		if (((u32)p & 0x1F) != 0) {
			dst = p - 0x10;
			syMemCopy(dst, p, size - 8);
			DCFlushRange(dst, size - 8);
			p = dst;
		}
		for (u32 num = 0; num < list->nbTexture; num++) {
			if (i == (list->textures[num].attr & 0xFFFF)) {
				slot = gjTexFindFree(&mem, &hdr);
				if (slot < 0) {
					return p;
				}
				mem->hdr = hdr;
				gjTexSurfaceSetup(&mem->hdr->info, width, height, format);
				hdr->info.image = p;
				hdr->texaddr    = list->textures[num].texaddr;
				mem->count      = mem->hdr->count;
				gjTexListSetSlot(list, num, slot);
			}
		}
		p = next;
	}
	DCStoreRange(data, p - (u8*)data);
	return p;
}

// 0x8001A138
void gvmLoaderFromMemory(GvmLoader* loader, u8* mem)
{
	loader->mState = GVM_STATIC;
	gvmLoadFromMemory(mem, loader->mList);
}
