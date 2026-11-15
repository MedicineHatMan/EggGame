#include "io/ResManager.h"
#include "core/dolphin.h"
#include "io/Arc.h"
#include "Ninja/ninja.h"
#include "core/Mem.h"

TArc::TArc()
{
	mData   = 0;
	mRelocs = 0;
	mExports = 0;
	mImports = 0;
	mNames  = 0;
	mState  = 0;
}

TArc::TArc(void* memory)
{
	u8* file = (u8*)memory;
	syMemCopy(&mFileSize, file, 0x20);
	mExportsSize = *(int*)(file + 0xC) << 3;
	mImportsSize = *(int*)(file + 0x10) << 3;
	u8* next = file + 0x20;
	mData    = next;
	next += *(int*)(file + 4);
	mRelocs  = next;
	next += *(int*)(file + 8);
	mExports = (int*)next;
	next += mExportsSize;
	mImports = (int*)next;
	next += mImportsSize;
	mNames   = (char*)next;
	relocateArc(mData, mRelocs, *(int*)(file + 8));
	mState = 3;
}

TArc::~TArc()
{
	if (mState != 3) {
		if (mData != 0) {
			memFree(mData);
		}
		if (mRelocs != 0) {
			memFree(mRelocs);
		}
		if (mExports != 0) {
			memFree(mExports);
		}
		if (mImports != 0) {
			memFree(mImports);
		}
		if (mNames != 0) {
			memFree(mNames);
		}
	}
}

// Finds an export by name
extern "C" void* findArcExport(TArc* arc, const char* name)
{
	int* entry = arc->mExports;
	int i = arc->mExportCount;
	while (i > 0) {
		if (strcmp(name, arc->mNames + entry[1]) == 0) {
			return arc->mData + entry[0];
		}
		i--;
		entry += 2;
	}
	return 0;
}

const char* TArc::getImportName(int index)
{
	return mNames + mImports[index * 2 + 1];
}

// Fills in a named import
void TArc::link(const char* name, void* value)
{
	int* entry = mImports;
	u32 i = 0;
	while (i < (u32)mImportCount) {
		if (strcmp(name, mNames + entry[1]) == 0) {
			u32 offset = entry[0];
			while (offset != 0xFFFFFFFF) {
				int* place = (int*)(mData + offset);
				offset = *place;
				*place = (int)value;
			}
			return;
		}
		i++;
		entry += 2;
	}
}

int TArc::load(TFile* file)
{
	switch (mState) {
	case 0:
		mStep  = 0;
		mState = 1;
	case 1:
		int result;
		while ((result = loadStep(file)) > 0) {
			if (mStep == 6) {
				mState = 2;
				return 1;
			}
			return 0;
		}
		return result;
	case 2:
		return 1;
	}
	return -1;
}

int TArc::getState()
{
	switch (mState) {
	case 0:
	case 1:
		return 0;
	case 2:
	case 3:
		return 1;
	}
	return 2;
}

int TArc::loadStep(TFile* file)
{
	int result = 0;

	switch (mStep) {
	case 0:
		result = file->read(&mFileSize, 0x20);
		if (result == 1) {
			mExportsSize = mExportCount << 3;
			mImportsSize = mImportCount << 3;
			mNamesSize = mFileSize - 0x20 - mDataSize - mRelocSize - mExportsSize - mImportsSize;
			if (mDataSize != 0) {
				mData = (u8*)memAlloc(mDataSize);
			}
			if (mRelocSize != 0) {
				mRelocs = (u8*)memAlloc(mRelocSize);
			}
			if (mExportsSize != 0) {
				mExports = (int*)memAlloc(mExportsSize);
			}
			if (mImportsSize != 0) {
				mImports = (int*)memAlloc(mImportsSize);
			}
			if (mNamesSize != 0) {
				mNames = (char*)memAlloc(mNamesSize);
			}
			result = 1;
			mStep  = 1;
		}
		break;
	case 1:
		result = file->read(mData, mDataSize);
		if (result == 1) {
			result = 1;
			mStep  = 2;
		}
		break;
	case 2:
		result = file->read(mRelocs, mRelocSize);
		if (result == 1) {
			relocateArc(mData, mRelocs, mRelocSize);
			result = 1;
			mStep  = 3;
		}
		break;
	case 3:
		result = file->read(mExports, mExportsSize);
		if (result == 1) {
			result = 1;
			mStep  = 4;
		}
		break;
	case 4:
		result = file->read(mImports, mImportsSize);
		if (result == 1) {
			result = 1;
			mStep  = 5;
		}
		break;
	case 5:
		result = file->read(mNames, mNamesSize);
		if (result == 1) {
			result = 1;
			mStep  = 6;
		}
		break;
	}
	return result;
}

// The relocation table is a list of distances in words from one pointer to the next
extern "C" void relocateArc(u8* data, u8* list, u32 size)
{
	u8 b0;
	u8 b1;
	u8 b2;
	u8 b3;
	u8* relocs = list;
	int pos = 0;

	while (size != 0) {
		u32 step;
		syMemCopy(&b0, relocs, 1);
		relocs++;
		size--;
		switch (b0 & 0xC0) {
		case 0:
			continue;
		case 0x40:
			step = b0 & 0x3F;
			break;
		case 0x80:
			syMemCopy(&b1, relocs, 1);
			relocs++;
			size--;
			step = ((b0 & 0x3F) << 8) | b1;
			break;
		case 0xC0:
			syMemCopy(&b1, relocs, 1);
			syMemCopy(&b2, relocs + 1, 1);
			syMemCopy(&b3, relocs + 2, 1);
			relocs += 3;
			size -= 3;
			step = ((b0 & 0x3F) << 24) | (b1 << 16) | (b2 << 8) | b3;
			break;
		default:
			continue;
		}
		pos += step * 4;
		if (*(u32*)(data + pos) != 0) {
			*(int*)(data + pos) += (int)data;
		}
	}
}

void TArc::linkFrom(TArc* other)
{
	for (u32 i = 0; i < (u32)mImportCount; i++) {
		const char* name = getImportName(i);
		if (name != 0) {
			void* value = findArcExport(other, name);
			if (value != 0) {
				link(name, value);
			}
		}
	}
}
