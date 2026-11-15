#include "ui/TAeObjGallery.h"
#include "engine/DrawTask.h"
#include "ui/AeArc.h"
#include "ui/AeUtil.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;

// TAeObjGallery class at 0x801B4904 then TModeGallery's helpers to 0x801B4E9C
// TModeGallery.cpp follows

extern "C" {
extern const u32 sAeGalleryItemCells[] = {
	0x0000025C, 0x00000261, 0x00000266, 0x0000026B, 0x00000270, 0x00000275, 0x0000027A, 0x0000027F, 0x00000284, 0x00000289, 0x0000028F,
	0x00000295, 0x0000029B, 0x000002A1, 0x000002A7, 0x000002AD, 0x000002B3, 0x000002B9, 0x000002BF, 0x000002C5, 0x000002CB, 0x000002D1,
	0x000002D7, 0x000002DD, 0x000002E3, 0x000002E9, 0x000002EF, 0x000002F5, 0x000002FB, 0x00000301, 0x00000307, 0x0000030D, 0x00000313,
	0x00000319, 0x0000031F, 0x00000325, 0x0000032B, 0x00000331, 0x00000337, 0x0000033D, 0x00000343, 0x00000349, 0x0000034F, 0x00000355,
	0x0000035B, 0x00000361, 0x00000367, 0x0000036D, 0x00000373, 0x00000379, 0x0000037F, 0x00000385, 0x0000038B, 0x00000391, 0x00000397,
	0x0000039D, 0x000003A3, 0x000003A9, 0x000003AF, 0x000003B5, 0x000003BB, 0x000003C1, 0x000003C7, 0x000003CD, 0x000003D3, 0x000003D9,
	0x000003DF, 0x000003E5, 0x000003EB, 0x000003F1, 0x000003F7, 0x000003FD,
};
extern const u32 sAeGalleryEggCells[] = {
	0x000000FD, 0x00000101, 0x00000105, 0x00000109, 0x0000010D, 0x00000111, 0x00000115, 0x00000119, 0x0000011D, 0x00000121, 0x00000126,
	0x0000012B, 0x00000130, 0x00000135, 0x0000013A, 0x0000013F, 0x00000144, 0x00000149, 0x0000014E, 0x00000153, 0x00000158, 0x0000015D,
	0x00000162, 0x00000167, 0x0000016C, 0x00000171, 0x00000176, 0x0000017B, 0x00000180, 0x00000185, 0x0000018A, 0x0000018F, 0x00000194,
	0x00000199, 0x0000019E, 0x000001A3, 0x000001A8, 0x000001AD, 0x000001B2, 0x000001B7, 0x000001BC, 0x000001C1, 0x000001C6, 0x000001CB,
	0x000001D0, 0x000001D5, 0x000001DA, 0x000001DF, 0x000001E4, 0x000001E9, 0x000001EE, 0x000001F3, 0x000001F8, 0x000001FD, 0x00000202,
	0x00000207, 0x0000020C, 0x00000211, 0x00000216, 0x0000021B, 0x00000220, 0x00000225, 0x0000022A, 0x0000022F, 0x00000234, 0x00000239,
	0x0000023E, 0x00000243, 0x00000248, 0x0000024D, 0x00000252, 0x00000257,
}; // the cell of each egg
extern const u32 sAeGalleryPalettes[] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000013, 0x00000014, 0x00000015, 0x00000016,
}; // palette banks
}

typedef char TAeObjGallery_size_check[(sizeof(TAeObjGallery) == 0x54) ? 1 : -1];

const char* gTAeObjGalleryName = "TAeObjGallery";

// 0x801B4710
extern "C" void loadGallery()
{
	loadArc(getLangFileName("ae_gallery.arc"), &sGalleryArc);
	if (sGalleryArc != 0) {
		AeArcBlock* block = aeArcBlock(sGalleryArc);
		sGalleryAe        = block->mAe;
		relocateAe(sGalleryAe);
		sGalleryTexList = block->mTexList;
		bindGvmTextures((u8*)block->mGvm, sGalleryTexList);
		u8* buf = new u8[0x800];
		if (block->mPalettes != 0) {
			loadE3Palette(block->mPalettes[0], (E3Palette*)&buf, sAeGalleryPalettes[5]);
			loadE3Palette(block->mPalettes[1], (E3Palette*)&buf, sAeGalleryPalettes[6]);
			loadE3Palette(block->mPalettes[2], (E3Palette*)&buf, sAeGalleryPalettes[7]);
			loadE3Palette(block->mPalettes[3], (E3Palette*)&buf, sAeGalleryPalettes[8]);
		}
		delete[] buf;
		njSetTexture(sGalleryTexList);
		njSetPaletteBankNum(5, sAeGalleryPalettes[5]);
		njSetPaletteBankNum(6, sAeGalleryPalettes[6]);
		njSetPaletteBankNum(7, sAeGalleryPalettes[7]);
		njSetPaletteBankNum(8, sAeGalleryPalettes[8]);
		GXInvalidateTexAll();
	}
}

// 0x801B485C
extern "C" void freeGallery()
{
	if (sGalleryArc != 0) {
		releaseTextures((NJS_TEXLIST*)sGalleryTexList);
		sGalleryTexList = 0;
		releaseResource(sGalleryArc);
		sGalleryArc = 0;
		sGalleryAe  = 0;
	}
}

// 0x801B48A8
extern "C" TAeObjGallery* createGallery()
{
	TAeObjGallery* made;

	if (sGalleryArc == 0) {
		return 0;
	}
	made = new TAeObjGallery(gTL_07, sGalleryAe, sGalleryTexList);
	return made;
}

static inline void make(TAeObjGallery* self, TAeLayer*& field, int id, u32 flags)
{
	field = self->createLayer(id, flags);
	if (field != 0) {
		field->mBanks = sAeGalleryPalettes;
	}
}

// 0x801B4904
TAeObjGallery::TAeObjGallery(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	int i;

	_00   = (u32)gTAeObjGalleryName;
	mSize = 0x54;
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3C, 0);
	make(this, _38, 0x41D, 0x10);
	make(this, _3C, 0x41B, 0x10);
	make(this, _40, 0x418, 0x10);
	make(this, _44, 0x403, 0);
	if (_38 != 0) {
		for (i = 0; i < 0x48; i++) {
			_38->setCell(0x66 - i, (int)sAeGalleryItemCells[i]);
		}
		_38->attach(0xAE, _44, 0);
	}
	if (_3C != 0) {
		TAeClip* clip = _3C;
		clip->pause();
	}
	if (_40 != 0) {
		_40->setCell(0x419, 0);
	}
	mCursor   = 0;
	mGridDone = 0;
}

// 0x801B4AC0
TAeObjGallery::~TAeObjGallery()
{
}

// 0x801B4B1C
void TAeObjGallery::draw()
{
	if (_38 != 0) {
		_38->update();
		if (_38->isDone() != 0) {
			mGridDone = 1;
		}
	}
	if (_3C != 0) {
		_3C->update();
		if (_3C->isDone() != 0) {
			mFrameDone = 1;
		}
	}
	if (_40 != 0) {
		_40->update();
	}
	if (_44 != 0) {
		_44->update();
	}
}

// 0x801B4BBC
void TAeObjGallery::render()
{
	if (_38 != 0) {
		_38->present();
	}
	if (_3C != 0) {
		_3C->present();
	}
	if (_40 != 0) {
		_40->present();
	}
}

// 0x801B4C14
void TAeObjGallery::showEgg(int egg)
{
	if (_38 != 0) {
		_38->setCell(0x66 - egg, sAeGalleryEggCells[egg]);
	}
}

// 0x801B4C58
void TAeObjGallery::setCursor(int egg)
{
	if (mCursor != egg && _38 != 0 && _44 != 0) {
		_38->clearSlot(0xAE - mCursor);
		_38->attach(0xAE - egg, _44, 0);
		mCursor = egg;
	}
}

// 0x801B4CDC
void TAeObjGallery::setCount(int count)
{
	TAeLayer* clip = _38;
	int shown;
	if (count < 0) {
		shown = 0;
	} else {
		shown = count > 0x48 ? 0x48 : count;
	}
	aeClipSetDigits2(clip, shown, 0x1A, 0x420, 0x421);
}

// 0x801B4D30
void TAeObjGallery::close()
{
	mGridDone = 0;
	if (_38 != 0) {
		_38->setMode(0x20);
		_38->wake();
	}
}

// 0x801B4D84
void TAeObjGallery::openView()
{
	mFrameDone = 0;
	if (_3C != 0) {
		_3C->setMode(0x10);
		_3C->wake();
	}
}

// 0x801B4DD8
void TAeObjGallery::closeView()
{
	mFrameDone = 0;
	if (_3C != 0) {
		_3C->setMode(0x20);
		_3C->wake();
	}
}

// 0x801B4E2C
void TAeObjGallery::markFound()
{
	if (_40 != 0) {
		_40->setCell(0x419, 0);
	}
}

// 0x801B4E64
void TAeObjGallery::markMissing()
{
	if (_40 != 0) {
		_40->setCell(0x419, 3);
	}
}
