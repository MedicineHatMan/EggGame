#include "ui/TAeObjBattleSelect.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "ui/AeArc.h"
#include "ui/AeUtil.h"
#include "core/TMainTask.h"
#include "ui/TAeObject.h"
struct NJS_TEXLIST;

// Palette banks 0 and 7 are unused
extern "C" const u32 sAeBattleSelectPalettes[] = {
	0xFFFFFFFF, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0xFFFFFFFF, 0x19,
};
static const u32 sAeBattleSelectTitleLayers[]  = { 0xCB, 0xD5, 0xDF, 0xE9 };
static const u32 sAeBattleSelectIconSlots[][4] = {
	{ 0x143, 0x147, 0x14B, 0x14F },
	{ 0x144, 0x148, 0x14C, 0x150 },
	{ 0x145, 0x149, 0x14D, 0x151 },
	{ 0x146, 0x14A, 0x14E, 0x152 },
};
static const u32 sAeBattleSelectRowLayers[]    = { 0xCF, 0xD9, 0xE3, 0xED };
static const u32 sAeBattleSelectPairSlots[]    = { 0xCD, 0xD7, 0xE1, 0xEB };
static const u32 sAeBattleSelectRowSlots[]     = { 0x10, 0x11, 0x12, 0x13 };
static const u32 sAeBattleSelectAttachSlotsA[] = { 0x18, 0x19, 0x1A, 0x1B };
static const u32 sAeBattleSelectAttachSlotsB[] = { 0x4, 0x5, 0x6, 0x7 };
static const u32 sAeBattleSelectCells[]        = { 0x9E, 0xA3, 0xA8, 0xAD };
static const u32 sAeBattleSelectFrames[]       = { 0x38, 0x39, 0x3A, 0x3B };
static const u32 sAeBattleSelectOtherFrames[]  = { 0xF, 0x10, 0x11, 0x12 };
static const u32 sAeBattleSelectChoiceSlots[]  = { 0x52, 0x53, 0x54 };
static const u32 sAeBattleSelectPairA[]        = { 0x4C, 0x4D };
static const u32 sAeBattleSelectPairB[]        = { 0x4E, 0x50 };
static const u32 sAeBattleSelectPairC[]        = { 0x4F, 0x51 };
static const u32 sAeBattleSelectChoiceFrames[] = { 0x51, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57 };
static const u32 sAeBattleSelectValueFrames[]  = { 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50 };
static const u32 sAeBattleSelectValueCells[]   = { 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F };
static const u32 sAeBattleSelectFrameCells[]   = { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47 };

const char* gTAeObjBattleSelectName = "TAeObjBattleSelect";

typedef char TAeObjBattleSelect_size_check[(sizeof(TAeObjBattleSelect) == 0xB4) ? 1 : -1];

static void taskList3C(TAeObjBattleSelect* self);
static void taskList3E(TAeObjBattleSelect* self);

// 0x80196620
// The block is the ae the texture list the gvm and the palettes
void loadBattleSelect()
{
	loadArc("ae_battle_menu.arc", &sAeBattleSelectArc);
	if (sAeBattleSelectArc != 0) {
		void** block        = (void**)sAeBattleSelectArc->mData;
		sAeBattleSelectData = (u32)block[0];
		relocateAe((void*)sAeBattleSelectData);
		sAeBattleSelectTex = (u32)block[1];
		bindGvmTextures((u8*)block[2], (NJS_TEXLIST*)sAeBattleSelectTex);
		u8* buf = new u8[0x800];
		if (block[3] != 0) {
			loadE3Palette(((u32*)block[3])[0], (E3Palette*)&buf, sAeBattleSelectPalettes[1]);
			loadE3Palette(((u32*)block[3])[1], (E3Palette*)&buf, sAeBattleSelectPalettes[2]);
			loadE3Palette(((u32*)block[3])[2], (E3Palette*)&buf, sAeBattleSelectPalettes[3]);
			loadE3Palette(((u32*)block[3])[3], (E3Palette*)&buf, sAeBattleSelectPalettes[4]);
			loadE3Palette(((u32*)block[3])[4], (E3Palette*)&buf, sAeBattleSelectPalettes[5]);
			loadE3Palette(((u32*)block[3])[5], (E3Palette*)&buf, sAeBattleSelectPalettes[6]);
			loadE3Palette(((u32*)block[3])[6], (E3Palette*)&buf, sAeBattleSelectPalettes[8]);
		}
		delete[] buf;
		njSetTexture((void*)sAeBattleSelectTex);
		njSetPaletteBankNum(0, sAeBattleSelectPalettes[1]);
		njSetPaletteBankNum(1, sAeBattleSelectPalettes[2]);
		njSetPaletteBankNum(2, sAeBattleSelectPalettes[3]);
		njSetPaletteBankNum(3, sAeBattleSelectPalettes[4]);
		njSetPaletteBankNum(4, sAeBattleSelectPalettes[5]);
		njSetPaletteBankNum(5, sAeBattleSelectPalettes[6]);
		njSetPaletteBankNum(6, sAeBattleSelectPalettes[8]);
		GXInvalidateTexAll();
	}
}

// 0x801967F8
void freeBattleSelect()
{
	if (sAeBattleSelectArc != 0) {
		releaseTextures((NJS_TEXLIST*)sAeBattleSelectTex);
		sAeBattleSelectTex = 0;
		releaseResource(sAeBattleSelectArc);
		sAeBattleSelectArc  = 0;
		sAeBattleSelectData = 0;
	}
}

// 0x80196844
TAeObjBattleSelect* createBattleSelect()
{
	TAeObjBattleSelect* made;

	if (sAeBattleSelectArc == 0) {
		return 0;
	}
	made                  = new TAeObjBattleSelect((TObject*)gTL_07, (void*)sAeBattleSelectData, (void*)sAeBattleSelectTex);
	sAeBattleSelectScreen = made;
	return made;
}

// 0x801968A4
// Stores the new layer in field then gives it the palettes
#define MAKE_LAYER(field, id, flags)             \
	field = createLayer(id, flags);              \
	if (field != 0) {                            \
		field->mBanks = sAeBattleSelectPalettes; \
	}

TAeObjBattleSelect::TAeObjBattleSelect(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	int i;

	_00    = (u32)gTAeObjBattleSelectName;
	mSize  = 0xB4;
	mTask  = addDrawTask(this, (void (*)(void*))taskList3C, 0x3C, 0);
	mTask2 = addDrawTask(this, (void (*)(void*))taskList3E, 0x3E, 0);
	MAKE_LAYER(_3C, 0x137, 0x10)
	MAKE_LAYER(_40, 0, 0x10)
	MAKE_LAYER(_44, 0xB2, 0)
	for (i = 0; i < 4; i++) {
		MAKE_LAYER(_7C[i], 0x6E, 0x10)
		if (_7C[i] != 0) {
			_7C[i]->setCell(0x6F, (int)sAeBattleSelectCells[i]);
			_7C[i]->setFrame(0x74, (int)sAeBattleSelectFrames[i]);
			if (_40 != 0) {
				_40->bind((int)sAeBattleSelectRowSlots[i], static_cast<TAeClip*>(_7C[i]));
			}
		}
		MAKE_LAYER(_8C[i], 0x6B, 0x10)
		if (_8C[i] != 0) {
			static_cast<TAeClip*>(_8C[i])->pause();
		}
		MAKE_LAYER(_6C[i], 0x63, 0x10)
		if (_6C[i] != 0) {
			static_cast<TAeClip*>(_6C[i])->pause();
		}
	}
	if (_3C != 0 && _40 != 0) {
		_3C->bind(0x138, static_cast<TAeClip*>(_40));
	}
	MAKE_LAYER(_48, 0x13F, 0x10)
	for (i = 0; i < 4; i++) {
		MAKE_LAYER(_4C[i], (int)sAeBattleSelectTitleLayers[i], 0)
		MAKE_LAYER(_5C[i], (int)sAeBattleSelectRowLayers[i], 0x10)
		if (_5C[i] != 0) {
			static_cast<TAeClip*>(_5C[i])->pause();
		}
	}
	MAKE_LAYER(_9C, 0x109, 0)
	MAKE_LAYER(_A0, 0x12E, 0x10)
	MAKE_LAYER(_A4, 0x131, 0x10)
	MAKE_LAYER(_A8, 0x10B, 0)
	for (i = 0; i < 4; i++) {
		_AC[i] = 0;
	}
	_B1 = 0;
	_B0 = 0;
}

// 0x80196CA8
TAeObjBattleSelect::~TAeObjBattleSelect()
{
	sAeBattleSelectScreen = 0;
}

// 0x80196D0C
void TAeObjBattleSelect::draw()
{
	int i;

	if (_3C != 0) {
		_3C->update();
	}
	if (_40 != 0) {
		_40->update();
	}
	if (_44 != 0) {
		_44->update();
	}
	if (_48 != 0) {
		_48->update();
	}
	for (i = 0; i < 4; i++) {
		if (_4C[i] != 0) {
			_4C[i]->update();
		}
		if (_5C[i] != 0) {
			_5C[i]->update();
		}
		if (_7C[i] != 0) {
			_7C[i]->update();
		}
		if (_8C[i] != 0) {
			_8C[i]->update();
		}
		if (_6C[i] != 0) {
			_6C[i]->update();
		}
	}
	if (_9C != 0) {
		_9C->update();
	}
	if (_A0 != 0) {
		_A0->update();
	}
	if (_A4 != 0) {
		_A4->update();
	}
	if (_A8 != 0) {
		_A8->update();
	}
}

// 0x80196E20
static void presentBack(TAeObjBattleSelect* self)
{
	if (self->_3C != 0) {
		self->_3C->present();
	}
}

// 0x80196E4C
static void presentFront(TAeObjBattleSelect* self)
{
	if (self->_48 != 0) {
		self->_48->present();
	}
}

// 0x80196E78
// The screen is the word at sAeBattleSelectScreen
void aeBattleSelectSetDigits(s8 index, u8 value)
{
	u8 clamped;

	if (sAeBattleSelectScreen == 0) {
		return;
	}
	clamped = value > 99 ? 99 : value;
	if (sAeBattleSelectScreen->_7C[index] != 0) {
		aeClipSetDigits2(sAeBattleSelectScreen->_7C[index], clamped, 0x18, 0x72, 0x73);
	}
}

// 0x80196EE0
void aeBattleSelectShowIcon(s8 row, s8 which)
{
	const u32* slots;
	TAeLayer* layer;
	int i;

	if (sAeBattleSelectScreen == 0) {
		return;
	}
	layer = sAeBattleSelectScreen->_48;
	if (layer == 0) {
		return;
	}
	slots = sAeBattleSelectIconSlots[row];
	for (i = 0; i < 4; i++) {
		if (sAeBattleSelectScreen->_4C[i] != 0) {
			if (i == which) {
				layer->bind(slots[i], sAeBattleSelectScreen->_4C[row]);
			} else {
				layer->clearSlot(slots[i]);
			}
		}
	}
}

// 0x80196FA0
void aeBattleSelectClearIcons()
{
	TAeLayer* layer;
	int row;
	int i;

	if (sAeBattleSelectScreen == 0) {
		return;
	}
	layer = sAeBattleSelectScreen->_48;
	if (layer == 0) {
		return;
	}
	for (row = 0; row < 4; row++) {
		for (i = 0; i < 4; i++) {
			layer->clearSlot(sAeBattleSelectIconSlots[row][i]);
		}
	}
}

// 0x80197024
void aeBattleSelectJoin(s8 slot, s8 other)
{
	TAeLayer* a;
	TAeLayer* b;
	TAeLayer* c;
	TAeLayer* d;
	TAeLayer* e;
	TAeLayer* host;

	if (sAeBattleSelectScreen == 0) {
		return;
	}
	host = sAeBattleSelectScreen->_40;
	a    = sAeBattleSelectScreen->_4C[slot];
	b    = sAeBattleSelectScreen->_5C[slot];
	c    = sAeBattleSelectScreen->_7C[slot];
	d    = sAeBattleSelectScreen->_8C[slot];
	e    = sAeBattleSelectScreen->_6C[other];
	if (a != 0) {
		a->setMode(0x10);
		if (b != 0) {
			a->bind((int)sAeBattleSelectPairSlots[slot], static_cast<TAeClip*>(b));
			b->wake();
		}
	}
	if (c != 0) {
		c->setFrame(0x6E, (int)sAeBattleSelectOtherFrames[other]);
	}
	if (host != 0 && d != 0) {
		host->attach((int)sAeBattleSelectAttachSlotsA[slot], static_cast<TAeClip*>(d), 0);
		d->wake();
	}
	if (host != 0 && e != 0) {
		host->attach((int)sAeBattleSelectAttachSlotsB[other], static_cast<TAeClip*>(e), 0);
		e->wake();
	}
}

// 0x8019715C
void aeBattleSelectLeave(s8 slot, s8 other)
{
	TAeLayer* a;
	TAeLayer* b;
	TAeLayer* c;
	TAeLayer* d;
	TAeLayer* e;
	TAeLayer* host;

	if (sAeBattleSelectScreen == 0) {
		return;
	}
	host = sAeBattleSelectScreen->_40;
	a    = sAeBattleSelectScreen->_4C[slot];
	b    = sAeBattleSelectScreen->_5C[slot];
	c    = sAeBattleSelectScreen->_7C[slot];
	d    = sAeBattleSelectScreen->_8C[slot];
	e    = sAeBattleSelectScreen->_6C[other];
	if (a != 0) {
		a->mPlayFlags &= 0xFFFFFAFF;
		a->setMode(1);
		a->clearSlot((int)sAeBattleSelectPairSlots[slot]);
	}
	if (b != 0) {
		b->cue(0);
		static_cast<TAeClip*>(b)->pause();
	}
	if (c != 0) {
		c->clearSlot(0x6E);
	}
	if (d != 0) {
		d->cue(0);
		static_cast<TAeClip*>(d)->pause();
	}
	if (e != 0) {
		e->cue(0);
		static_cast<TAeClip*>(e)->pause();
	}
	if (host != 0) {
		host->clearSlot((int)sAeBattleSelectAttachSlotsA[slot]);
		host->clearSlot((int)sAeBattleSelectAttachSlotsB[other]);
	}
}

// 0x801972AC
void aeBattleSelectBindIcon(s8 row, s8 which)
{
	TAeObjBattleSelect* self;
	TAeLayer* layer;

	self = sAeBattleSelectScreen;
	if (self == 0) {
		return;
	}
	layer = self->_4C[row];
	if (self->_48 != 0 && layer != 0) {
		self->_48->bind((int)sAeBattleSelectIconSlots[row][which], static_cast<TAeClip*>(layer));
	}
}

// 0x8019732C
void aeBattleSelectClearRow(s8 row)
{
	TAeObjBattleSelect* self;
	int i;

	self = sAeBattleSelectScreen;
	if (self == 0) {
		return;
	}
	const u32* slots;
	TAeLayer* layer = self->_48;
	if (layer == 0) {
		return;
	}
	slots = sAeBattleSelectIconSlots[row];
	for (s8 j = 0; j < 4; j++) {
		layer->clearSlot((int)slots[j]);
	}
}

// 0x801973A8
void aeBattleSelectBindBanner()
{
	TAeObjBattleSelect* self;

	self = sAeBattleSelectScreen;
	if (self != 0) {
		TAeLayer* host = self->_3C;
		TAeLayer* clip = self->_44;
		if (host != 0 && clip != 0) {
			host->bind(0x13A, static_cast<TAeClip*>(clip));
		}
	}
}

// 0x80197404
void aeBattleSelectResetBanner()
{
	TAeObjBattleSelect* self;
	TAeLayer* layer;

	self = sAeBattleSelectScreen;
	if (self == 0) {
		return;
	}
	layer = self->_44;
	if (layer != 0) {
		layer->reset(0xB4, 0x10);
		layer->mBanks = sAeBattleSelectPalettes;
	}
}

// 0x8019745C
void aeBattleSelectClearBanner()
{
	TAeObjBattleSelect* self;

	self = sAeBattleSelectScreen;
	if (self != 0 && self->_3C != 0) {
		self->_3C->clearSlot(0x13A);
	}
}

// 0x8019749C
void aeBattleSelectEnterChara()
{
	TAeLayer* host;
	TAeLayer* back;
	TAeLayer* front;
	TAeLayer* banner;
	int i;

	if (sAeBattleSelectScreen == 0) {
		return;
	}
	host   = sAeBattleSelectScreen->_40;
	back   = sAeBattleSelectScreen->_3C;
	front  = sAeBattleSelectScreen->_48;
	banner = sAeBattleSelectScreen->_44;
	if (host != 0) {
		host->reset(0, 0x10);
		host->mBanks = sAeBattleSelectPalettes;
	}
	if (back != 0) {
		back->clearSlot(0x13A);
	}
	if (front != 0) {
		front->setMode(0x10);
		front->wake();
	}
	if (banner != 0) {
		banner->reset(0xB2, 1);
		banner->mBanks = sAeBattleSelectPalettes;
	}
	for (i = 0; i < 4; i++) {
		TAeLayer* row = sAeBattleSelectScreen->_7C[i];
		if (host != 0 && row != 0) {
			host->bind(sAeBattleSelectRowSlots[i], row);
		}
	}
}

// 0x801975AC
void aeBattleSelectLeaveChara()
{
	if (sAeBattleSelectScreen == 0) {
		return;
	}
	if (sAeBattleSelectScreen->_40 != 0) {
		sAeBattleSelectScreen->_40->setMode(0x20);
		sAeBattleSelectScreen->_40->wake();
	}
	if (sAeBattleSelectScreen->_48 != 0) {
		sAeBattleSelectScreen->_48->setMode(0x20);
		sAeBattleSelectScreen->_48->wake();
	}
	if (sAeBattleSelectScreen->_3C != 0) {
		sAeBattleSelectScreen->_3C->clearSlot(0x13A);
	}
}

// 0x80197644
void aeBattleSelectEnterRules()
{
	TAeLayer* host;
	TAeLayer* back;
	TAeLayer* banner;
	int i;

	if (sAeBattleSelectScreen == 0) {
		return;
	}
	host   = sAeBattleSelectScreen->_40;
	back   = sAeBattleSelectScreen->_3C;
	banner = sAeBattleSelectScreen->_44;
	if (host != 0) {
		host->reset(0x24, 0x10);
		host->mBanks = sAeBattleSelectPalettes;
		host->setCell(0x48, 0x107);
		for (i = 0; i < 8; i++) {
			host->setCell(sAeBattleSelectFrameCells[i], 0x10E);
		}
	}
	if (banner != 0) {
		banner->reset(0xB6, 0x10);
		banner->mBanks = sAeBattleSelectPalettes;
		if (back != 0) {
			back->bind(0x13A, banner);
		}
	}
}

// 0x80197730
void aeBattleSelectLeaveRules()
{
	if (sAeBattleSelectScreen == 0) {
		return;
	}
	if (sAeBattleSelectScreen->_40 != 0) {
		sAeBattleSelectScreen->_40->setMode(0x20);
		sAeBattleSelectScreen->_40->wake();
	}
	if (sAeBattleSelectScreen->_44 != 0) {
		sAeBattleSelectScreen->_44->setMode(0x20);
		sAeBattleSelectScreen->_44->wake();
	}
}

// 0x801977AC
void aeBattleSelectPickRule(s8 choice)
{
	TAeLayer* host;
	TAeLayer* clip;
	s8 picked;
	int i;

	if (sAeBattleSelectScreen == 0) {
		return;
	}
	host = sAeBattleSelectScreen->_40;
	clip = sAeBattleSelectScreen->_9C;
	if (host == 0) {
		return;
	}
	picked = choice < 0 ? 0 : (choice > 2 ? 2 : choice);
	for (i = 0; i < 3; i++) {
		if (i == picked) {
			if (i != 2) {
				host->setCell(sAeBattleSelectPairA[i], 0x134);
			}
			if (clip != 0) {
				host->bind(sAeBattleSelectChoiceSlots[i], clip);
			}
		} else {
			host->clearSlot(sAeBattleSelectChoiceSlots[i]);
			if (i != 2) {
				host->clearSlot(sAeBattleSelectPairA[i]);
				host->clearSlot(sAeBattleSelectPairB[i]);
				host->clearSlot(sAeBattleSelectPairC[i]);
			}
		}
	}
}

// 0x801978CC
void aeBattleSelectBindPairB(s8 choice)
{
	TAeLayer* host;
	TAeLayer* clip;
	s8 picked;
	int i;

	if (sAeBattleSelectScreen == 0) {
		return;
	}
	host = sAeBattleSelectScreen->_40;
	clip = sAeBattleSelectScreen->_A0;
	if (host != 0 && clip != 0) {
		picked = choice < 0 ? 0 : (choice > 1 ? 1 : choice);
		for (i = 0; i < 2; i++) {
			if (i == picked) {
				host->bind(sAeBattleSelectPairB[i], clip);
				clip->cue(0);
				clip->wake();
			} else {
				host->clearSlot(sAeBattleSelectPairB[i]);
			}
		}
	}
}

// 0x801979A8
void aeBattleSelectBindPairC(s8 choice)
{
	TAeLayer* host;
	TAeLayer* clip;
	s8 picked;
	int i;

	if (sAeBattleSelectScreen == 0) {
		return;
	}
	host = sAeBattleSelectScreen->_40;
	clip = sAeBattleSelectScreen->_A4;
	if (host != 0 && clip != 0) {
		picked = choice < 0 ? 0 : (choice > 1 ? 1 : choice);
		for (i = 0; i < 2; i++) {
			if (i == picked) {
				host->bind(sAeBattleSelectPairC[i], clip);
				clip->cue(0);
				clip->wake();
			} else {
				host->clearSlot(sAeBattleSelectPairC[i]);
			}
		}
	}
}

// 0x80197A84
void aeBattleSelectShowStage(s8 choice)
{
	TAeLayer* host;
	TAeLayer* clip;
	int i;

	if (sAeBattleSelectScreen == 0) {
		return;
	}
	host = sAeBattleSelectScreen->_40;
	clip = sAeBattleSelectScreen->_A8;
	if (host == 0) {
		return;
	}
	host->setFrame(0x49, sAeBattleSelectChoiceFrames[choice]);
	for (i = 0; i < 8; i++) {
		if (i == choice) {
			if (clip != 0) {
				host->bind(sAeBattleSelectFrameCells[i], clip);
			}
		} else {
			host->setCell(sAeBattleSelectFrameCells[i], 0x10E);
		}
	}
}

// 0x80197B4C
void aeBattleSelectOpenStage(s8 choice)
{
	TAeObjBattleSelect* self;

	self = sAeBattleSelectScreen;
	if (self != 0 && self->_40 != 0) {
		self->_40->setFrame((int)sAeBattleSelectValueCells[choice], (int)sAeBattleSelectValueFrames[choice]);
	}
}

// 0x80197BA8
static void taskList3E(TAeObjBattleSelect* self)
{
	presentFront(self);
}

// 0x80197BC8
static void taskList3C(TAeObjBattleSelect* self)
{
	presentBack(self);
}
