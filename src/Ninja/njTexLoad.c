#include "Ninja/ninja.h"
#include "Dolphin/dvd.h"
#include "Ninja/njInternal.h"

typedef s32 (*GjTexLoader)(NJS_TEXLIST* list, s32 num);

extern GjTexLoader gjTexLoaders[4];
u8 gjTexHeaderBuf[0x20] __attribute__((aligned(32)));
char gjTexExtension[] = ".GVR"; // the file extension in .sdata

s32 njLoadTextureNum(NJS_TEXLIST* list, s32 num);

// 0x80215540
s32 njLoadTextureNum(NJS_TEXLIST* list, s32 num)
{
	s32 loader;
	switch (list->textures[num].attr & 0x5C000000) {
	case 0:
		loader = 0;
		break;
	case 0x40000000:
		loader = 1;
		break;
	case 0x08000000:
		loader = 2;
		break;
	case 0x04000000:
		loader = 3;
		break;
	default:
		loader = 2;
		break;
	}
	return gjTexLoaders[loader](list, num);
}

// 0x802155E8
// From a file reads the first 0x20 bytes for the header then the rest into the surface
s32 gjTexLoadFile(NJS_TEXLIST* list, s32 num)
{
	DVDFileInfo file;
	char path[0x20];
	GjTexDesc desc;
	u32 index;
	u32 gbix;
	s32 bank;
	s32 slot;
	u8* header = gjTexHeaderBuf;
	s32 ret;
	u8* data;
	s32 offset;
	DVDFileInfo* fp;
	u32 size;

	gjTexClearErr();
	ret = gjTexListDecode(list, num, &gbix, &bank);
	if (ret != 0) {
		return ret;
	}
	gjTexMakePath(path, list->textures[num].filename, gjTexExtension);
	fp = &file;
	if (DVDOpen(path, fp) == 0) {
		gTexErrStat.err = 2;
		return -1;
	}
	size = (fp->length + 0x1F) & ~0x1F;
	if (DVDReadPrio(fp, header, 0x20, 0, 2) == -1) {
		DVDClose(fp);
		return -1;
	}
	data = gvrFindImage(&desc, &index, header);
	if (data == NULL) {
		DVDClose(fp);
		return -1;
	}
	if (gbix != 0xFFFFFFFF) {
		index = gbix;
	}
	ret = gjTexListBind(list, num, index, bank, &desc, &slot);
	if (ret != 0) {
		DVDClose(fp);
		return ret;
	}
	offset = data - header;
	DVDReadPrio(fp, gjTexSlots[slot].hdr->info.image, size - offset, offset, 2);
	DVDClose(fp);
	return 1;
}

// 0x80215780
// From memory the name is a GVR file a pointer to one or an NJS_TEXINFO
s32 gjTexLoadMem(NJS_TEXLIST* list, s32 num)
{
	GjTexDesc desc;
	u32 index;
	u32 gbix;
	s32 bank;
	s32 slot;
	s32 ret;
	u8* data;

	gjTexClearErr();
	ret = gjTexListDecode(list, num, &gbix, &bank);
	if (ret != 0) {
		return ret;
	}
	data = gvrFindImage(&desc, &index, list->textures[num].filename);
	if (data == NULL) {
		data = gvrFindImage(&desc, &index, *(u8**)list->textures[num].filename);
		if (data == NULL) {
			NJS_TEXINFO* info = list->textures[num].filename;
			desc.width        = info->info.width;
			desc.height       = info->info.height;
			desc.format       = info->info.format;
			data              = info->texaddr;
		}
	}
	if (gbix != 0xFFFFFFFF) {
		index = gbix;
	}
	ret = gjTexListBind(list, num, index, bank, &desc, &slot);
	if (ret != 0) {
		return ret;
	}
	if (gjTexLoadSlot(slot, data) < 0) {
		gjTexReleaseSlot(slot);
		return -1;
	}
	while (gjTexHeapReady() != 1) { }
	return 1;
}

// 0x802158CC
// An NJS_TEXINFO that only reserves the surface
s32 gjTexLoadInfo(NJS_TEXLIST* list, s32 num)
{
	GjTexDesc desc;
	u32 gbix;
	s32 bank;
	s32 slot;
	s32 ret;
	NJS_TEXINFO* info;
	u32 index;

	gjTexClearErr();
	ret = gjTexListDecode(list, num, &gbix, &bank);
	if (ret != 0) {
		return ret;
	}
	info        = list->textures[num].filename;
	desc.width  = info->info.width;
	desc.height = info->info.height;
	desc.format = info->info.format;
	index       = (u32)list->textures[num].texaddr;
	if (gbix != 0xFFFFFFFF) {
		index = gbix;
	}
	if (gjTexListBind(list, num, index, bank, &desc, &slot) >= 0) {
		return 1;
	}
	return -1;
}

// 0x80215998
// Takes a slot without loading anything
s32 gjTexLoadEmpty(NJS_TEXLIST* list, s32 num)
{
	u32 gbix;
	s32 bank;
	s32 slot;
	GjTexMem* mem;
	GjTexHeader* hdr;
	u32 index;
	s32 ret;

	gjTexClearErr();
	ret = gjTexListDecode(list, num, &gbix, &bank);
	if (ret != 0) {
		return ret;
	}
	index = (u32)list->textures[num].texaddr;
	if (gbix != 0xFFFFFFFF) {
		index = gbix;
	}
	switch (gjTexLookup(index, bank, 0, &slot)) {
	case -1:
		return -1;
	case 0:
		slot = gjTexFindFree(&mem, &hdr);
		if (slot < 0) {
			return -1;
		}
		mem->hdr = hdr;
		if (index != 0xFFFFFFFF) {
			mem->hdr->texaddr = (void*)index;
		}
		gjTexHeadersUsed++;
		gjTexListSetSlot(list, num, slot);
		break;
	}
	return 1;
}

// 0x80215AB4
// Stops at the first failure and says which
s32 njLoadTexture(NJS_TEXLIST* list)
{
	u32 i;
	if (list->nbTexture == 0) {
		gTexErrStat.err = 1;
		return -1;
	}
	for (i = 0; i < list->nbTexture; i++) {
		if (njLoadTextureNum(list, i) < 0) {
			gTexErrStat.texno = i;
			return -1;
		}
	}
	return 1;
}

GjTexLoader gjTexLoaders[4] = { gjTexLoadFile, gjTexLoadMem, gjTexLoadInfo, gjTexLoadEmpty };
