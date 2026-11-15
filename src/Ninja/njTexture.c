#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

typedef struct GjPaletteFile {
	u32 magic; // _00 'GVPL' in file order
	u32 size;  // _04
	u16 _08;   // _08
	s16 _0A;   // _0A
	s16 _0C;   // _0C
	s16 _0E;   // _0E
	u8 data[]; // _10
} GjPaletteFile;

typedef struct GjPalette {
	void* data; // _00
	u16 _04;    // _04
	s16 _06;    // _06
	s16 _08;    // _08
	s16 _0A;    // _0A
} GjPalette;

void gjTexMemSetTlut(GjTexMem* mem, s32 tlut);
void gjTexClearErr(void);
void gjTexSwap32(u32* value);
void gvrSwap32(u32* value);
s32 gjTexSurfaceCreate(GjTexInfo* surface, s32 w, s32 h, s32 fmt);
void gjTexCopyFlush(void* dst, const void* src, s32 size);
void gjTexListSetSlot(NJS_TEXLIST* list, s32 num, s32 slot);
s32 gjTexHeapBusy(void);

// The chunk names as they read after the swap
#define GVR_GBIX 0x58494247
#define GVR_GVRT 0x54525647

GjTexMem* gjTexSlots;
s32 gjTexSlotCount; // number of slots
GjTexHeader* gjTexHeaders;
s32 gjTexHeaderCount; // number of headers

u8* gvrFindImage(GjTexDesc* desc, u32* gbix, u8* p);
s32 gjTexMemCreateSurface(GjTexMem* mem, const GjTexDesc* desc);
s32 gjTexListDecode(NJS_TEXLIST* list, u32 num, u32* gbix, s32* bank);
void gjTexMakePath(char* out, const char* name, const char* ext);
s32 gjTexFormatHasTlut(u32 format);
void gjTexSlotsInit(GjTexMem* mems, s32 count);
void gjTexHeadersInit(GjTexHeader* hdrs, s32 count);
s32 gjTexFindGbix(u32 gbix);
s32 gjTexFindFree(GjTexMem** outMem, GjTexHeader** outHdr);
s32 gjTexFindGbixBank(u32 gbix, s32 bank);
void gjTexHeaderTidy(GjTexMem* mem);
s32 gjTexWriteImage(GjTexInfo* surface, const void* src, u32 offset, u32 size);
s32 gjTexLookup(u32 gbix, s32 bank, u32 format, s32* out);
s32 gjTexCreateSlot(u32 gbix, s32 bank, const GjTexDesc* desc);
s32 gjTexLoadSlot(s32 slot, const void* src);
s32 gjTexListBind(NJS_TEXLIST* list, s32 num, u32 gbix, s32 bank, const GjTexDesc* desc, s32* slot);
void gjTexMemStub(GjTexMem* mem);
s32 gjTexMemRelease(GjTexMem* mem);
s32 gjTexSurfaceSetup(GjTexInfo* surface, u32 w, u32 h, u32 format);
u32 gjTexSurfaceSize(const GjTexInfo* surface);
u32 gjTexCalcSize(int w, int h, u32 format);
u32 gjTexDescSize(GjTexDesc* desc);
s32 gjTexSurfaceAlloc(GjTexInfo* surface);
s32 gjTexSurfaceFree(GjTexInfo* surface);

// 0x802105F4
// A 32 bit swap
void gvrSwap32(u32* value)
{
	u32 v  = *value;
	*value = (v & 0xFF) << 24 | ((v & 0xFF00) << 8 | ((v & 0xFF000000) >> 24 | (v & 0xFF0000) >> 8));
}

// 0x8021062C
// Walks a GVR file's chunks to the GVRT one
u8* gvrFindImage(GjTexDesc* desc, u32* gbix, u8* p)
{
	u32 magic;
	u32 size;
	u32* q = (u32*)p;

	if (gbix != NULL) {
		*gbix = -1;
	}
	do {
		magic = *q;
		gvrSwap32(&magic);
		switch (magic) {
		case GVR_GVRT:
			syMemCopy(desc, q + 2, 8);
			return (u8*)(q + 4);
		case GVR_GBIX:
			q++;
			size = *q++;
			gvrSwap32(&size);
			if (gbix != NULL) {
				*gbix = *q;
			}
			q += size >> 2;
			break;
		default:
			gTexErrStat.err = 4;
			return NULL;
		}
	} while (magic != GVR_GVRT);
	return (u8*)q;
}

// 0x8021073C
s32 gjTexMemCreateSurface(GjTexMem* mem, const GjTexDesc* desc)
{
	if (gjTexSurfaceCreate(&mem->hdr->info, desc->width, desc->height, desc->format) != 0) {
		gTexErrStat.err = 6;
		return -1;
	}
	return 1;
}

// 0x80210798
s32 gjTexHeapReady(void)
{
	if (gjTexHeapBusy() == 0) {
		return 1;
	}
	return -1;
}

// 0x802107CC
// Decodes a list entry before loading
s32 gjTexListDecode(NJS_TEXLIST* list, u32 num, u32* gbix, s32* bank)
{
	NJS_TEXNAME* textures = list->textures;
	NJS_TEXNAME* t        = &textures[num];
	u32 attr              = t->attr;
	GjTexMem* mem;
	GjTexMem* prev;
	s32 slot;

	if ((attr & 0x10000) && num != 0) {
		*bank = ((u32)t->texaddr & 0xFC000000) >> 26;
		prev  = ((GjTexMem**)&textures[(s32)(num - 1)])[2];
		*gbix = (u32)prev->hdr->texaddr;
		slot  = gjTexFindGbixBank(*gbix, *bank);
		if (slot < 0) {
			if (gjTexFindFree(&mem, NULL) < 0) {
				return -1;
			}
			t->texaddr = mem;
			mem->tlut  = *bank;
			mem->hdr   = prev->hdr;
			mem->count = 1;
			mem->hdr->count++;
			mem->_14 = prev->_14;
			gjTexMemStub(mem);
		} else {
			t->texaddr = mem = &gjTexSlots[slot];
			mem->hdr->count++;
			mem->count++;
		}
		return 1;
	}
	if (attr & 0x800000) {
		*gbix = (u32)t->texaddr;
		*bank = -1;
	} else if (attr & 0x1000000) {
		*gbix = (u32)t->texaddr & 0x3FFFFFF;
		*bank = ((u32)t->texaddr & 0xFC000000) >> 26;
	} else {
		*gbix = -1;
		*bank = ((u32)t->texaddr & 0xFC000000) >> 26;
	}
	return 0;
}

// 0x80210984
// Copies a file name adding the extension unless it's already there
void gjTexMakePath(char* out, const char* name, const char* ext)
{
	char tail[5];
	u32 len;

	len = strlen(name);
	strcpy(tail, &name[len - 4]);
	strupr(tail);
	if (tail[0] == '.') {
		if (strcmp(tail, ext) != 0) {
			strcpy(out, name);
			strcat(out, ext);
		} else {
			strcpy(out, name);
		}
	} else {
		strcpy(out, name);
		strcat(out, ext);
	}
}

// 0x80210A50
// Formats 8 to 10 use a palette
s32 gjTexFormatHasTlut(u32 format)
{
	switch ((u8)format) {
	case 8:
	case 9:
	case 10:
		return 1;
	}
	return 0;
}

// 0x80210A78
s32 gjTexSurfaceCreate(GjTexInfo* surface, s32 w, s32 h, s32 fmt)
{
	s32 err = gjTexSurfaceSetup(surface, w, h, fmt);
	if (err != 0) {
		return err;
	}
	return gjTexSurfaceAlloc(surface);
}

// 0x80210AB8
s32 njReleaseTexture(NJS_TEXLIST* list)
{
	u32 i;
	s32 ret = 1;
	for (i = 0; i < list->nbTexture; i++) {
		NJS_TEXNAME* t = &list->textures[i];
		if (t->attr & 0x200000) {
			if (gjTexMemRelease(t->texaddr) < 0 && ret != -1) {
				gTexErrStat.texno = i;
				ret               = -1;
			}
			list->textures[i].attr &= ~0x200000;
			list->textures[i].texaddr = NULL;
		}
	}
	return ret;
}

// 0x80210B84
void njSetTextureInfo(NJS_TEXINFO* info, void* texaddr, s32 type, s32 width, s32 height)
{
	info->texaddr     = texaddr;
	info->info.width  = width;
	info->info.height = height;
	info->info.format = type;
}

// 0x80210B98
// Both branches do the same
void njSetTextureName(NJS_TEXNAME* t, void* filename, void* texaddr, u32 attr)
{
	if (attr & 0x40000000) {
		t->filename = filename;
		t->attr     = attr;
		t->texaddr  = texaddr;
		return;
	}
	t->filename = filename;
	t->attr     = attr;
	t->texaddr  = texaddr;
}

// 0x80210BC4
void gjTexSlotsInit(GjTexMem* mems, s32 count)
{
	s32 i;
	gjTexSlots     = mems;
	gjTexSlotCount = count;
	for (i = 0; i < count; i++) {
		mems->_00   = 0;
		mems->_04   = 0;
		mems->tlut  = -1;
		mems->hdr   = NULL;
		mems->count = 0;
		mems->_14   = 0;
		mems++;
	}
}

// 0x80210D08
void gjTexHeadersInit(GjTexHeader* hdrs, s32 count)
{
	s32 i;
	gjTexHeaders     = hdrs;
	gjTexHeaderCount = count;
	for (i = 0; i < count; i++) {
		hdrs->texaddr     = (void*)-1;
		hdrs->info.format = 0;
		hdrs->info._04    = 0;
		hdrs->info._08    = 0;
		hdrs->info.width  = 0;
		hdrs->info.height = 0;
		hdrs->info._14    = 0;
		hdrs->info.flags  = 0;
		hdrs->info.image  = NULL;
		hdrs->count       = 0;
		hdrs++;
	}
}

// 0x80210EDC
// The slot holding a global index
s32 gjTexFindGbix(u32 gbix)
{
	s32 i;
	for (i = 0; i < gjTexSlotCount; i++) {
		if (gjTexSlots[i].hdr != NULL && gbix == (u32)gjTexSlots[i].hdr->texaddr) {
			return i;
		}
	}
	return -1;
}

// 0x80210F28
// Finds a free slot and if asked a free header
s32 gjTexFindFree(GjTexMem** outMem, GjTexHeader** outHdr)
{
	s32 i;
	GjTexHeader* hdr;
	GjTexMem* mem;
	s32 index;

	*outMem = NULL;
	mem     = gjTexSlots;
	for (i = 0; i < gjTexSlotCount; i++) {
		if (mem->hdr == NULL) {
			*outMem = mem;
			index   = i;
			break;
		}
		mem++;
	}
	if (*outMem == NULL) {
		gTexErrStat.err = 7;
		return -1;
	}
	if (outHdr != NULL) {
		*outHdr = NULL;
		hdr     = gjTexHeaders;
		for (i = 0; i < gjTexHeaderCount; i++) {
			if ((u32)hdr->texaddr == 0xFFFFFFFF) {
				*outHdr = hdr;
				break;
			}
			hdr++;
		}
		if (*outHdr == NULL) {
			gTexErrStat.err = 7;
			return -1;
		}
		return index;
	}
	return index;
}

// 0x80211008
// The slot holding a global index in a palette bank
s32 gjTexFindGbixBank(u32 gbix, s32 bank)
{
	s32 i;
	GjTexMem* mem = gjTexSlots;
	for (i = 0; i < gjTexSlotCount; i++) {
		if ((u32)mem->tlut == bank && mem->hdr != NULL && (u32)mem->hdr->texaddr == gbix) {
			return i;
		}
		mem++;
	}
	return -1;
}

// 0x80211060
// Clears what no longer has users
void gjTexHeaderTidy(GjTexMem* mem)
{
	GjTexHeader* hdr = mem->hdr;
	if (hdr->count <= 0) {
		hdr->texaddr     = (void*)-1;
		hdr->info.format = 0;
		hdr->info._04    = 0;
		hdr->info._08    = 0;
		hdr->info.width  = 0;
		hdr->info.height = 0;
		hdr->info._14    = 0;
		hdr->info.flags  = 0;
		hdr->info.image  = NULL;
		gjTexHeadersUsed--;
	}
	if (mem->count <= 0) {
		mem->tlut = -1;
		mem->_00  = 0;
		mem->_04  = 0;
		mem->hdr  = NULL;
		mem->_14  = 0;
	}
}

// 0x802110D4
void njInitTextureEx(void* mems, s32 memCount, void* hdrs, s32 hdrCount)
{
	gjTexSlotsInit(mems, memCount);
	gjTexHeadersInit(hdrs, hdrCount);
}

// 0x80211118
s32 njSetTexture(NJS_TEXLIST* list)
{
	_nj_current_ctx_->texlist = list;
	return 1;
}

// 0x80211128
NJS_TEXLIST* njGetCurrentTexList(void)
{
	return _nj_current_ctx_->texlist;
}

// 0x80211134
s32 njSetTextureNum(s32 num)
{
	NJS_CONTEXT* ctx      = _nj_current_ctx_;
	GjTexMem* mem         = ctx->texlist->textures[num].texaddr;
	ctx->tex              = &mem->hdr->info;
	_nj_current_ctx_->_18 = mem->tlut;
	return 1;
}

// 0x8021116C
void gjTexClearErr(void)
{
	gTexErrStat.texno = -1;
	gTexErrStat.n     = -1;
	gTexErrStat.err   = 0;
	gTexErrStat._0C   = 0;
	gTexErrStat._10   = 0;
}

// 0x80211194
void gjTexCopyFlush(void* dst, const void* src, s32 size)
{
	syMemCopy(dst, src, size);
	DCFlushRange(dst, size);
}

// 0x802111D8
// Copies image data in in whole words
s32 gjTexWriteImage(GjTexInfo* surface, const void* src, u32 offset, u32 size)
{
	u32 room;
	u32 at = offset & ~3;
	size &= ~3;
	if (size == 0) {
		return 0;
	}
	room = surface->_14 - at;
	if (at >= surface->_14) {
		return 7;
	}
	if (room > size) {
		room = size;
	}
	gjTexCopyFlush((u8*)(at + (u32)surface->image), src, room);
	return 0;
}

// 0x80211240
// Returns 1 if the texture's already loaded or if only a palette bank's slot was missing and 0 if it must be loaded
s32 gjTexLookup(u32 gbix, s32 bank, u32 format, s32* out)
{
	s32 found = gjTexFindGbix(gbix);
	s32 slot;
	GjTexMem* mem;

	if (gjTexFormatHasTlut(format) != 0) {
		if (found < 0) {
			*out = -1;
			return 0;
		}
		slot = gjTexFindGbixBank(gbix, bank);
		if (slot < 0) {
			s32 n = gjTexFindFree(&mem, NULL);
			if (n < 0) {
				return -1;
			}
			mem->tlut = bank;
			mem->hdr  = gjTexSlots[found].hdr;
			mem->hdr->count++;
			mem->count = 1;
			mem->_14   = gjTexSlots[found]._14;
			gjTexMemStub(mem);
			*out = n;
			return 1;
		}
		*out = slot;
		gjTexSlots[slot].count++;
		gjTexSlots[slot].hdr->count++;
		return 1;
	}
	*out = found;
	if (found < 0) {
		return 0;
	}
	gjTexSlots[found].count++;
	gjTexSlots[found].hdr->count++;
	return 1;
}

// 0x802113CC
// Takes a free slot and header and allocates the surface
s32 gjTexCreateSlot(u32 gbix, s32 bank, const GjTexDesc* desc)
{
	GjTexMem* mem;
	GjTexHeader* hdr;
	s32 n = gjTexFindFree(&mem, &hdr);

	if (n < 0) {
		return -1;
	}
	mem->hdr = hdr;
	if (gjTexMemCreateSurface(mem, desc) != 1) {
		mem->hdr = NULL;
		return -1;
	}
	if (gbix != 0xFFFFFFFF) {
		mem->hdr->texaddr = (void*)gbix;
	}
	if (gjTexFormatHasTlut(desc->format) != 0) {
		mem->tlut = bank;
	} else {
		mem->tlut = -1;
	}
	gjTexMemStub(mem);
	mem->count      = 1;
	mem->hdr->count = 1;
	gjTexHeadersUsed++;
	return n;
}

// 0x802114D8
void gjTexListSetSlot(NJS_TEXLIST* list, s32 num, s32 slot)
{
	list->textures[num].texaddr = &gjTexSlots[slot];
	list->textures[num].attr |= 0x200000;
}

// 0x8021150C
s32 gjTexLoadSlot(s32 slot, const void* src)
{
	GjTexHeader* hdr = gjTexSlots[slot].hdr;
	if (hdr == NULL) {
		gTexErrStat.err = 1;
		return -1;
	}
	if (gjTexWriteImage(&hdr->info, src, 0, hdr->info._14) != 0) {
		gTexErrStat.err = 8;
		return -1;
	}
	return 1;
}

// 0x8021158C
s32 gjTexReleaseSlot(s32 slot)
{
	return gjTexMemRelease(&gjTexSlots[slot]);
}

// 0x802115B8
s32 gjTexListBind(NJS_TEXLIST* list, s32 num, u32 gbix, s32 bank, const GjTexDesc* desc, s32* slot)
{
	switch (gjTexLookup(gbix, bank, desc->format, slot)) {
	case -1:
		return -1;
	case 0:
		*slot = gjTexCreateSlot(gbix, bank, desc);
		if (*slot < 0) {
			return -1;
		}
		gjTexListSetSlot(list, num, *slot);
		return 0;
	case 1:
		gjTexListSetSlot(list, num, *slot);
		return 1;
	}
	return 0;
}

// 0x80211694
void gjTexMemStub(GjTexMem* mem)
{
}

// 0x80211698
// Drops one user and frees the image with the last one
s32 gjTexMemRelease(GjTexMem* mem)
{
	if (mem->hdr == NULL) {
		return -1;
	}
	if (mem->count > 0) {
		mem->count--;
	}
	if (mem->hdr->count > 0) {
		mem->hdr->count--;
		if (mem->hdr->count == 0) {
			gjTexSurfaceFree(&mem->hdr->info);
			_nj_current_ctx_->_14 = -1;
		}
	}
	gjTexHeaderTidy(mem);
	return 1;
}

// 0x80211730
// The same swap as gvrSwap32
void gjTexSwap32(u32* value)
{
	u32 v  = *value;
	*value = (v & 0xFF) << 24 | ((v & 0xFF00) << 8 | ((v & 0xFF000000) >> 24 | (v & 0xFF0000) >> 8));
}

// 0x80211768
void gjTexMemSetTlut(GjTexMem* mem, s32 tlut)
{
	mem->tlut = tlut;
}

// 0x80211770
void njSetPaletteBankNum(s32 num, s32 bank)
{
	gjTexMemSetTlut(_nj_current_ctx_->texlist->textures[num].texaddr, bank);
}

// 0x802117A8
s32 njLoadPaletteData(const GjPaletteFile* file, GjPalette* palette)
{
	u32 v;

	gjTexClearErr();
	v = file->magic;
	gjTexSwap32(&v);
	switch (v) {
	case 0x4C505647:
		v = file->size;
		gjTexSwap32(&v);
		syMemCopy(palette->data, file->data, v - 8);
		palette->_06 = file->_0A;
		palette->_08 = file->_0C;
		palette->_0A = file->_0E;
		palette->_04 = file->_08;
		break;
	default:
		return -1;
	}
	return 1;
}

// 0x8021185C
void gjSetPaletteData(u32 name, s32 fmt, u16 count, void* data)
{
	u8 obj[0x10];
	GXInitTlutObj(obj, data, fmt, (u16)count);
	GXLoadTlut(obj, name);
}

// 0x802118AC
s32 gjTexSurfaceSetup(GjTexInfo* surface, u32 w, u32 h, u32 format)
{
	s32 err = 0;
	u32 mipmap;
	u32 flags = 0;

	surface->format = (u8)format;
	surface->width  = w;
	surface->height = h;
	mipmap          = format & 0x100;
	if (mipmap && w != h) {
		err = 4;
	}
	surface->flags = 0;
	if (mipmap) {
		flags |= 0x80000000;
	}
	if (format & 0x200) {
		flags |= 0x8000;
	}
	if (format & 0x800) {
		flags |= 0x40000000;
	}
	surface->flags |= flags;
	surface->_14 = gjTexSurfaceSize(surface);
	return err;
}

// 0x8021195C
u32 gjTexSurfaceSize(const GjTexInfo* surface)
{
	u8 mipmap;
	if (surface->flags & 0x80000000) {
		mipmap = 1;
	} else {
		mipmap = 0;
	}
	return GXGetTexBufferSize(surface->width, surface->height, surface->format, mipmap, 100);
}

// 0x802119B4
u32 gjTexCalcSize(int w, int h, u32 format)
{
	s32 mipmap;
	u8 fmt = format;
	if (format & 0x100) {
		mipmap = 1;
	} else {
		mipmap = 0;
	}
	return GXGetTexBufferSize(w, h, fmt, mipmap, 100);
}

// 0x80211A04
u32 gjTexDescSize(GjTexDesc* desc)
{
	u16 w  = desc->width;
	u16 h  = desc->height;
	u8 fmt = desc->format;
	u8 mipmap;
	if (desc->format & 0x100) {
		mipmap = 1;
	} else {
		mipmap = 0;
	}
	return GXGetTexBufferSize(w, h, fmt, mipmap, 100);
}

// 0x80211A54
s32 gjTexSurfaceAlloc(GjTexInfo* surface)
{
	s32 err   = 0;
	void* mem = OSAllocFromHeap(gjTexHeapHandle, surface->_14);
	if (mem == NULL) {
		err = 3;
	} else {
		surface->image = mem;
	}
	return err;
}

// 0x80211AAC
s32 gjTexSurfaceFree(GjTexInfo* surface)
{
	u8* image = surface->image;
	if (image >= gjTexHeap && image < gjTexHeap + gjTexHeapSize) {
		OSFreeToHeap(gjTexHeapHandle, image);
		return 0;
	}
	return 7;
}

// 0x80211AFC
s32 gjTexHeapBusy(void)
{
	return 0;
}
