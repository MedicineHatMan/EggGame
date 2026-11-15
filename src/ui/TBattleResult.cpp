#include "ui/TBattleResult.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/LangFile.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "ui/TAeLayer.h"
#include "egg/EggCreature.h"
#include "ui/TFade.h"
#include "ui/AeObjControl.h"
#include "ui/AeArc.h"
#include "ui/TAeObjBattle.h"
#include "core/Pad.h"
#include "core/Mem.h"
#include "sound/SoundCtrl.h"
#include "ui/TSimpleText.h"
#include "object/TObjPlayer.h"
#include "player/Player.h"
#include "core/SdkCalls.h"
#include "ui/TextVec.h"
#include "stl/math.h"
#include "ui/AeUtil.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;

// 0x80286808
// and 0x80286814 set up by the static init at 0x801A80FC
static TextVec battleTextGlyph = textVec(0.82f, 0.82f, 0.82f);
static TextVec battleTextBox   = textVec(120.0f, 20.0f, 0.0f);

struct TObjPlayer;
struct TSimpleText;

// This file calls aeGetPlayerSlot through an s8 parameter each call sign-extends its argument again
#define playerSlot(i) (((s8 (*)(s8))aeGetPlayerSlot)(i))

// The .rodata block at 0x8024C520
extern "C" {
extern const u32 gAeBattlePalettes[6] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000014,
}; // palette banks
const u32 sAeBattleScoreLayers[6]       = { 0xB0, 0xB0, 0xBF, 0xBF, 0xBF, 0xBF }; // per screen slot
const u32 sAeBattleScoreChangeLayers[6] = { 0xB2, 0xB2, 0xC1, 0xC1, 0xC1, 0xC1 }; // per screen slot

extern const u32 gAeBattleScoreSlots[6] = { 0xE4, 0xE2, 0xE4, 0xE3, 0xE2, 0xE1 }; // leader mark slot per character
// TBattleResult's rank slots per player cleared cleared and bound in readPick
extern const u32 sBattleResultSlotsA[6]  = { 0xEA, 0xE9, 0xE8, 0xE7, 0xE6, 0xE5 };
extern const u32 sBattleResultSlotsB[6]  = { 0xF0, 0xEF, 0xEE, 0xED, 0xEC, 0xEB };
extern const u32 sBattleResultSlotsC[6]  = { 0xF6, 0xF5, 0xF4, 0xF3, 0xF2, 0xF1 };
extern const u32 sAeBattleDigitCells[10] = {
	0x74, 0x76, 0x78, 0x7A, 0x7C, 0x7E, 0x80, 0x82, 0x84, 0x86,
}; // big digit cells 0-9
// The small digit cells 0-9 for the score change
const u32 sAeBattleSmallDigitCells[10] = {
	0x88, 0x8A, 0x8C, 0x8E, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9A,
};
const u32 sAeBattleIconCells[4] = { 0x62, 0x67, 0x6C, 0x70 }; // player icon cell per player
// TBattleResult's layout per player one x and the y of each row
extern const s16 sBattleResultPosX[6]  = { 0x102, 0x102, 0x79, 0x1AE, 0x79, 0x1AE };
extern const s16 sBattleResultPosY0[6] = { 0x50, 0x120, 0x5A, 0x5A, 0x13E, 0x13E };
extern const s16 sBattleResultPosY1[6] = { 0x66, 0x136, 0x70, 0x70, 0x154, 0x154 };
extern const s16 sBattleResultPosY2[6] = { 0x7A, 0x14A, 0x84, 0x84, 0x168, 0x168 };
extern const s16 sBattleResultPosY3[6] = { 0x8E, 0x15E, 0x98, 0x98, 0x17C, 0x17C };
}

const char* gTBattleResultName = "TBattleResult"; // its string sits between the position tables and the marks

extern "C" {

extern const int sBattleResultMarks[] = {
	120,
	72,
	66,
	50,
};
}

// ae_battle.arc and mes_battle.bin Names of the slots are ours
extern "C" {
TArc* gBattleResultArc;
void* gBattleResultAe0;
void* gBattleResultAe1;
BinFile* sBattleMsgFile;
int sBattleMsgCount;
char** sBattleMsgTable;
}

static inline TAeClip* clipOf(TAeLayer* layer)
{
	return (TAeClip*)((u8*)layer + 0x20);
}

static inline void markLayer(TAeLayer* layer)
{
	if (layer != 0) {
		layer->mBanks = gAeBattlePalettes;
	}
}

static inline void callPlayer(void* player, int slot)
{
	void** vt = *(void***)((u8*)player + 0x18);
	((void (*)(void*)) * (void**)((u8*)vt + slot))(player);
}

static inline void flagText(void* obj, u16 bit)
{
	if (obj != 0) {
		((TObject*)obj)->mFlags |= bit;
	}
}

typedef char TBattleResult_size_probe[(sizeof(TBattleResult) == 0x170) ? 1 : -1];

// 0x801A5944
extern "C" TBattleResult* createTBattleResult(TObject* parent)
{
	if (gBattleResultArc == 0) {
		return 0;
	}
	return new TBattleResult(parent, gBattleResultAe0, gBattleResultAe1);
}

// 0x801A59AC
// From the battle mode for each port still in the round
void TBattleResult::setRow(u8 slot, const BattleResultRow* row)
{
	mLive[slot]        = 1;
	mRow[slot].mPlace  = row->mPlace;
	mRow[slot].mWins   = row->mWins;
	mRow[slot].mPoints = row->mPoints;
	mRow[slot].mLost   = row->mLost;
	mRow[slot]._08     = row->_08;
}

const char* gTAeObjBattleName = "TAeObjBattle";

// 0x801A59F4
TBattleResult::TBattleResult(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	_00   = (u32)gTBattleResultName;
	mSize = 0x170;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3E, 0);
	setup();
}

// 0x801A5A64
TBattleResult::~TBattleResult()
{
	alphaFadeDelete();
	for (s8 i = 0; i < 4; i++) {
		releaseRow(i);
		if (mNum[0][i] != 0) {
			delete mNum[0][i];
			mNum[0][i] = 0;
		}
		if (mNum[1][i] != 0) {
			delete mNum[1][i];
			mNum[1][i] = 0;
		}
		if (mNum[2][i] != 0) {
			delete mNum[2][i];
			mNum[2][i] = 0;
		}
		if (mNum[3][i] != 0) {
			delete mNum[3][i];
			mNum[3][i] = 0;
		}
	}
	removeDrawTask(mDraw);
}

// 0x801A5B60
void TBattleResult::setup()
{
	mDone  = 0;
	mTimer = 0;
	for (s8 i = 0; i < 4; i++) {
		mLive[i]        = 0;
		mPicked[i]      = 0;
		mRow[i].mPlace  = 0xFF;
		mRow[i].mWins   = 0;
		mRow[i].mPoints = 0;
		mRow[i].mLost   = 0;
		mRow[i]._08     = 0;
		mPlace[i]       = 0;
		mRank[i]        = 0;
		mText[0][i][1]  = 0;
		mText[0][i][0]  = 0;
		mText[1][i][1]  = 0;
		mText[1][i][0]  = 0;
		mText[2][i][1]  = 0;
		mText[2][i][0]  = 0;
		mText[3][i][1]  = 0;
		mText[3][i][0]  = 0;
		mNum[0][i]      = new char[2];
		mNum[1][i]      = new char[2];
		mNum[2][i]      = new char[2];
		mNum[3][i]      = new char[2];
	}
	mLayer = createLayer(0x142, 0);
	markLayer(mLayer);
	for (s8 i = 0; i < 4; i++) {
		Vec3f a;
		a.x            = (f32)sBattleResultPosX[i];
		a.y            = (f32)sBattleResultPosY1[i];
		a.z            = -2.0f;
		mText[1][i][0] = createSimpleText(this, 1, -1, &a, &battleTextBox, &battleTextGlyph, sBattleMsgTable[0]);
		flagText(mText[1][i][0], 0x10);

		Vec3f b;
		b.x            = (f32)sBattleResultPosX[i];
		b.y            = (f32)sBattleResultPosY2[i];
		b.z            = -2.0f;
		mText[2][i][0] = createSimpleText(this, 1, -1, &b, &battleTextBox, &battleTextGlyph, sBattleMsgTable[1]);
		flagText(mText[2][i][0], 0x10);

		Vec3f c;
		c.x            = (f32)sBattleResultPosX[i];
		c.y            = (f32)sBattleResultPosY3[i];
		c.z            = -2.0f;
		mText[3][i][0] = createSimpleText(this, 1, -1, &c, &battleTextBox, &battleTextGlyph, sBattleMsgTable[2]);
		flagText(mText[3][i][0], 0x10);
	}
	mState = 0;
}

// 0x801A5E6C
void TBattleResult::draw()
{
	switch ((s8)mState) {
	case 0:
		open();
		break;
	case 1:
		waitOpen();
		break;
	case 2:
		readPick();
		break;
	case 3:
		waitEnd();
		break;
	}
}

// 0x801A5ED4
void TBattleResult::render()
{
	if (mLayer != 0) {
		mLayer->present();
	}
}

// 0x801A5F00
void TBattleResult::waitOpen()
{
	s8 i;
	if (mLayer != 0) {
		mLayer->update();
	}
	for (i = 0; i < 4; i++) {
		if (mPlace[i] != 0) {
			mPlace[i]->update();
		}
	}
	int count = 0;
	for (i = 0; i < 4; i++) {
		if (mLive[i] != 0) {
			count += 1;
		}
	}
	int marks[4];
	marks[0] = sBattleResultMarks[0];
	marks[1] = sBattleResultMarks[1];
	marks[2] = sBattleResultMarks[2];
	marks[3] = sBattleResultMarks[3];
	if (count == 2) {
		marks[1] -= 2;
	}
	for (i = 0; i < count; i++) {
		if (mTimer == marks[i]) {
			if (i == 0) {
				sePlay(0x3B);
			} else {
				sePlay(0x3C);
			}
		}
	}
	if (mTimer == 0) {
		reveal();
		mState = 2;
	} else {
		mTimer -= 1;
	}
}

// 0x801A6054
void TBattleResult::waitEnd()
{
	if (mLayer != 0) {
		mLayer->update();
	}
	for (s8 i = 0; i < 4; i++) {
		if (mPlace[i] != 0) {
			mPlace[i]->update();
		}
		if (mRank[i] != 0) {
			mRank[i]->update();
		}
	}
	if (mTimer == 0) {
		mDone = 1;
	} else {
		mTimer -= 1;
	}
}

// 0x801A60F0
void TBattleResult::readPick()
{
	s8 ready = 0;
	for (s8 i = 0; i < 4; i++) {
		s8 who = playerSlot(i);
		if (mPlace[i] != 0) {
			mPlace[i]->update();
		}
		if (mRank[i] != 0) {
			mRank[i]->update();
		}
		if (mLive[i] != 0) {
			if (*(s16*)((u8*)padGet(i) + 0x34) != 0) {
				if (mPicked[i] == 0) {
					hideRow(i);
					mLayer->clearSlot(sBattleResultSlotsB[who]);
					if (who != -1 && mLayer != 0 && mRank[i] != 0) {
						mRank[i]->reset(0x123, 0x10);
						mRank[i]->mBanks = gAeBattlePalettes;
						mLayer->clearSlot(sBattleResultSlotsA[who]);
						mLayer->bind(sBattleResultSlotsC[who], mRank[i]);
					}
					mPicked[i] = 1;
				}
			} else if ((*(u32*)((u8*)padGet(i) + 8) & 0x1000) != 0 && mPicked[i] == 0) {
				sePlay(0x1C);
				hideRow(i);
				mLayer->clearSlot(sBattleResultSlotsB[who]);
				if (who != -1 && mLayer != 0 && mRank[i] != 0) {
					mRank[i]->reset(0x123, 0x10);
					mRank[i]->mBanks = gAeBattlePalettes;
					mLayer->clearSlot(sBattleResultSlotsA[who]);
					mLayer->bind(sBattleResultSlotsC[who], mRank[i]);
				}
				mPicked[i] = 1;
			}
			if (mPicked[i] != 0) {
				ready += 1;
			}
		} else {
			ready += 1;
		}
	}
	if (ready == 4) {
		mTimer = 0x78;
		new TBlackFadeInOut(this, 1);
		mState = 3;
	}
}

// 0x801A637C
void TBattleResult::reveal()
{
	for (s8 i = 0; i < 4; i++) {
		s8 who = playerSlot(i);
		if (who == -1) {
			continue;
		}
		if (mPlace[i] != 0) {
			switch (clipOf(mPlace[i])->getId()) {
			case 0x130:
				clipOf(mPlace[i])->reset(0x138, 1);
				mPlace[i]->mBanks = gAeBattlePalettes;
				break;
			case 0x118:
				clipOf(mPlace[i])->reset(0x11D, 1);
				mPlace[i]->mBanks = gAeBattlePalettes;
				break;
			case 0xFE:
				clipOf(mPlace[i])->reset(0x104, 1);
				mPlace[i]->mBanks = gAeBattlePalettes;
				break;
			case 0x10D:
				clipOf(mPlace[i])->reset(0x110, 1);
				mPlace[i]->mBanks = gAeBattlePalettes;
				break;
			}
		}
		if (mRow[i].mPlace != 0xFF) {
			mRank[i] = createLayer(0x127, 0);
			markLayer(mRank[i]);
			if (mLayer != 0) {
				if (mRank[i] != 0) {
					mLayer->bind(sBattleResultSlotsA[who], mRank[i]);
				}
				mLayer->setFrame(sBattleResultSlotsB[who], 0x41);
			}
			if (mText[1][i][0] != 0) {
				((TObject*)mText[1][i][0])->mFlags &= ~0x10;
				((TSimpleText*)mText[1][i][0])->mAt.set((f32)sBattleResultPosX[who], (f32)sBattleResultPosY1[who], -2.0f);
			}
			if (mText[2][i][0] != 0) {
				((TObject*)mText[2][i][0])->mFlags &= ~0x10;
				((TSimpleText*)mText[2][i][0])->mAt.set((f32)sBattleResultPosX[who], (f32)sBattleResultPosY2[who], -2.0f);
			}
			if (mText[3][i][0] != 0) {
				((TObject*)mText[3][i][0])->mFlags &= ~0x10;
				((TSimpleText*)mText[3][i][0])->mAt.set((f32)sBattleResultPosX[who], (f32)sBattleResultPosY3[who], -2.0f);
			}
			sprintf(mNum[0][i], "%2d", mRow[i].mWins);
			Vec3f a;
			a.x            = 120.0f + (f32)sBattleResultPosX[who];
			a.y            = (f32)sBattleResultPosY0[who];
			a.z            = -2.0f;
			mText[0][i][1] = createSimpleText(this, 1, -1, &a, &battleTextBox, &battleTextGlyph, mNum[0][i]);

			sprintf(mNum[1][i], "%2d", mRow[i].mPoints);
			Vec3f b;
			b.x            = 120.0f + (f32)sBattleResultPosX[who];
			b.y            = (f32)sBattleResultPosY1[who];
			b.z            = -2.0f;
			mText[1][i][1] = createSimpleText(this, 1, -1, &b, &battleTextBox, &battleTextGlyph, mNum[1][i]);

			sprintf(mNum[2][i], "%2d", mRow[i].mLost);
			Vec3f c;
			c.x            = 120.0f + (f32)sBattleResultPosX[who];
			c.y            = (f32)sBattleResultPosY2[who];
			c.z            = -2.0f;
			mText[2][i][1] = createSimpleText(this, 1, -1, &c, &battleTextBox, &battleTextGlyph, mNum[2][i]);

			sprintf(mNum[3][i], "%2d", mRow[i]._08);
			Vec3f d;
			d.x            = 120.0f + (f32)sBattleResultPosX[who];
			d.y            = (f32)sBattleResultPosY3[who];
			d.z            = -2.0f;
			mText[3][i][1] = createSimpleText(this, 1, -1, &d, &battleTextBox, &battleTextGlyph, mNum[3][i]);
		}
	}
	alphaFadeDim(0x3C);
}

// 0x801A6918
void TBattleResult::open()
{
	int i;
	int count = 0;
	for (i = 0; i < 4; i++) {
		if (mLive[i] != 0) {
			count += 1;
		}
	}
	for (i = 0; i < 4; i++) {
		s8 who = aeGetPlayerSlot((s8)i);
		if (who == -1) {
			continue;
		}
		switch (mRow[i].mPlace) {
		case 1:
			if (count == 2) {
				mPlace[i] = createLayer(0x130, 0x10);
			} else {
				mPlace[i] = createLayer(0xFE, 0x10);
			}
			if (mLive[i] != 0) {
				TObjPlayer* player = findPlayer((u16)i);
				if (player != 0) {
					player->vfn_CC();
				}
			}
			break;
		case 0:
		case 2:
			if (count == 2) {
				mPlace[i] = createLayer(0x118, 0x10);
			} else {
				mPlace[i] = createLayer(0x109, 0x10);
			}
			if (mLive[i] != 0) {
				TObjPlayer* player = findPlayer((u16)i);
				if (player != 0) {
					player->vfn_D0();
				}
			}
			break;
		case 3:
			mPlace[i] = createLayer(0x10B, 0x10);
			if (mLive[i] != 0) {
				TObjPlayer* player = findPlayer((u16)i);
				if (player != 0) {
					player->vfn_D0();
				}
			}
			break;
		case 4:
			mPlace[i] = createLayer(0x10D, 0x10);
			if (mLive[i] != 0) {
				TObjPlayer* player = findPlayer((u16)i);
				if (player != 0) {
					player->vfn_D0();
				}
			}
			break;
		}
		if (mRow[i].mPlace != 0xFF) {
			markLayer(mPlace[i]);
			if (mLayer != 0 && mPlace[i] != 0) {
				mLayer->bind(gAeBattleScoreSlots[who], mPlace[i]);
			}
		}
	}
	mTimer = 0x78;
	mState = 1;
}

// 0x801A6BBC
void TBattleResult::hideRow(s8 index)
{
	if (mText[0][index][0] != 0) {
		((TObject*)mText[0][index][0])->mFlags |= 0x10;
	}
	if (mText[0][index][1] != 0) {
		((TObject*)mText[0][index][1])->mFlags |= 0x10;
	}
	if (mText[1][index][0] != 0) {
		((TObject*)mText[1][index][0])->mFlags |= 0x10;
	}
	if (mText[1][index][1] != 0) {
		((TObject*)mText[1][index][1])->mFlags |= 0x10;
	}
	if (mText[2][index][0] != 0) {
		((TObject*)mText[2][index][0])->mFlags |= 0x10;
	}
	if (mText[2][index][1] != 0) {
		((TObject*)mText[2][index][1])->mFlags |= 0x10;
	}
	if (mText[3][index][0] != 0) {
		((TObject*)mText[3][index][0])->mFlags |= 0x10;
	}
	if (mText[3][index][1] != 0) {
		((TObject*)mText[3][index][1])->mFlags |= 0x10;
	}
}

// 0x801A6CA8
void TBattleResult::releaseRow(s8 index)
{
	if (mText[0][index][0] != 0) {
		((TObject*)mText[0][index][0])->mFlags |= 1;
		mText[0][index][0] = 0;
	}
	if (mText[0][index][1] != 0) {
		((TObject*)mText[0][index][1])->mFlags |= 1;
		mText[0][index][1] = 0;
	}
	if (mText[1][index][0] != 0) {
		((TObject*)mText[1][index][0])->mFlags |= 1;
		mText[1][index][0] = 0;
	}
	if (mText[1][index][1] != 0) {
		((TObject*)mText[1][index][1])->mFlags |= 1;
		mText[1][index][1] = 0;
	}
	if (mText[2][index][0] != 0) {
		((TObject*)mText[2][index][0])->mFlags |= 1;
		mText[2][index][0] = 0;
	}
	if (mText[2][index][1] != 0) {
		((TObject*)mText[2][index][1])->mFlags |= 1;
		mText[2][index][1] = 0;
	}
	if (mText[3][index][0] != 0) {
		((TObject*)mText[3][index][0])->mFlags |= 1;
		mText[3][index][0] = 0;
	}
	if (mText[3][index][1] != 0) {
		((TObject*)mText[3][index][1])->mFlags |= 1;
		mText[3][index][1] = 0;
	}
}

// 0x801A6DD4
// Loads ae_battle.arc and mes_battle.bin
extern "C" void loadBattleResult()
{
	u32* data;
	u32 text;
	int i;

	loadArc("ae_battle.arc", &gBattleResultArc);
	if (gBattleResultArc != 0) {
		void** block     = (void**)gBattleResultArc->mData;
		gBattleResultAe0 = block[0];
		relocateAe(gBattleResultAe0);
		void* tex        = block[1];
		gBattleResultAe1 = tex;
		bindGvmTextures((u8*)block[2], (NJS_TEXLIST*)tex);
		u8* buf = new u8[0x800];
		if (block[3] != 0) {
			loadE3Palette(*(u32*)block[3], (E3Palette*)&buf, gAeBattlePalettes[5]);
		}
		delete[] buf;
		njSetTexture(gBattleResultAe1);
		njSetPaletteBankNum(5, (int)gAeBattlePalettes[5]);
		GXInvalidateTexAll();
	}
	loadSizedResource(getLangFileName("mes_battle.bin"), &sBattleMsgFile);
	data = (u32*)sBattleMsgFile->mData;
	text = *data;
	text += (u32)data++;
	u32 size        = *data++;
	sBattleMsgTable = (char**)data;
	sBattleMsgCount = size >> 2;
	for (i = 0; i < sBattleMsgCount; i++) {
		sBattleMsgTable[i] += text + 8;
	}
}

// 0x801A6EF0
// Name is ours
extern "C" void freeBattleResult()
{
	if (gBattleResultArc != 0) {
		releaseTextures((NJS_TEXLIST*)gBattleResultAe1);
		gBattleResultAe1 = 0;
		releaseResource(gBattleResultArc);
		gBattleResultArc = 0;
		gBattleResultAe0 = 0;
	}
	if (sBattleMsgFile != 0) {
		releaseResource(sBattleMsgFile);
		sBattleMsgFile = 0;
	}
}

struct TArc;

struct BattleTimeMsg;
struct BattlePlayerMsg;

inline Vec3f vec3f(f32 x, f32 y, f32 z)
{
	Vec3f a;
	a.x = x;
	a.y = y;
	a.z = z;
	return a;
}

typedef char TAeObjBattle_size_check[(sizeof(TAeObjBattle) == 0x9C) ? 1 : -1];

// 0x801A6F54
// The battle HUD built from the arc TBattleResult loads
extern "C" TAeObjBattle* createTAeObjBattle()
{
	if (gBattleResultArc == 0) {
		return 0;
	}
	return new TAeObjBattle(gTL_07, gBattleResultAe0, gBattleResultAe1);
}

static inline TAeLayer* make(TAeObjBattle* self, int id, u32 flags)
{
	TAeLayer* layer = self->createLayer(id, flags);
	if (layer != 0) {
		layer->mBanks = gAeBattlePalettes;
	}
	return layer;
}

// Stores the layer first and tests the member it stored
#define MAKE_LAYER(self, member, id, flags)  \
	member = (self)->createLayer(id, flags); \
	if (member != 0) {                       \
		member->mBanks = gAeBattlePalettes;  \
	}

static void clearSlots(TAeObjBattle* self);
static void buildLayers(TAeObjBattle* self);

// 0x801A6FB0
TAeObjBattle::TAeObjBattle(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	_00   = (u32)gTAeObjBattleName;
	mSize = 0x9C;
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3C, 0);
	clearSlots(this);
	buildLayers(this);
}

// 0x801A7028
TAeObjBattle::~TAeObjBattle()
{
	removeDrawTask(mTask);
	aeForgetBattle(this);
}

extern "C" {

// 0x801A7098
static void buildLayers(TAeObjBattle* self)
{
	MAKE_LAYER(self, self->_38, 0x144, 0);
	MAKE_LAYER(self, self->_3C, 0x39, 0x100);
	MAKE_LAYER(self, self->_40, 0x142, 0);
	MAKE_LAYER(self, self->_44, 0x129, 0x10);
	battleSetPlayers(self, aeGetPlayerCount());
	self->mRule = 0;
}

// 0x801A7184
void battleRuleSurvival(TAeObjBattle* self)
{
	self->mRule = 1;
}

// 0x801A7190
void battleRuleTime(TAeObjBattle* self)
{
	self->mRule = 2;
}

// 0x801A719C
// Hatch battles show each player's score from the start
void battleRuleHatch(TAeObjBattle* self)
{
	for (int i = 0; i < 4; i++) {
		if (self->_48[i] == 0 && playerSlot(i) != -1) {
			self->_48[i] = self->createLayer(sAeBattleScoreLayers[playerSlot(i)], 0x10);
			if (self->_48[i] != 0) {
				self->_48[i]->mBanks = gAeBattlePalettes;
			}
			self->_48[i]->setCell(0xC6, sAeBattleIconCells[i]);
			battleShowScore(self, i);
			self->_48[i]->setFrame(0xC9, 0x20);
			battlePlaceLayer(self, self->_48[i], playerSlot(i));
		}
	}
	self->mRule = 3;
}
}

// 0x801A729C
void TAeObjBattle::draw()
{
	if (_3C != 0) {
		if (_3C->isDone() != 0) {
			_78 = 1;
		} else {
			_3C->update();
		}
	}
	if (_40 != 0) {
		_40->update();
	}
	if (_44 != 0) {
		_44->update();
	}
	for (int i = 0; i < 4; i++) {
		if (_48[i] != 0) {
			_48[i]->update();
		}
	}
	for (int i = 0; i < 4; i++) {
		if (_58[i] != 0) {
			_58[i]->update();
			if (_58[i]->isDone() != 0 && _8C[i] >= 0 && _8C[i] < 4 && _48[_8C[i]] != 0) {
				battleShowScore(this, _8C[i]);
				_8C[i] = -1;
			}
		}
	}
}

// 0x801A73B8
static void clearSlots(TAeObjBattle* self)
{
	int i;

	self->_6C = 9999;
	self->_70 = 0;
	self->_74 = 0;
	self->_78 = 0;
	self->_38 = 0;
	self->_3C = 0;
	self->_40 = 0;
	self->_44 = 0;
	for (i = 0; i < 4; i++) {
		self->_48[i] = 0;
		self->_58[i] = 0;
		self->_7C[i] = 0;
		self->_8C[i] = -1;
	}
}

// 0x801A7410
void TAeObjBattle::render()
{
	int i;

	if (_40 != 0) {
		_40->present();
	}
	if (_38 != 0) {
		_38->present();
	}
	if (_3C != 0) {
		_3C->present();
	}
	for (i = 0; i < 4; i++) {
		if (_48[i] != 0) {
			_48[i]->present();
		}
	}
	for (i = 0; i < 4; i++) {
		if (_58[i] != 0) {
			_58[i]->present();
		}
	}
}

// Plays banner id on the _3C layer and hangs it in slot 0x144 of _38
static inline void showBanner(TAeObjBattle* self, u32 id)
{
	if (self->_38 != 0 && self->_3C != 0) {
		self->_3C->reset(id, 0x10);
		self->_3C->mBanks = gAeBattlePalettes;
		self->_38->bind(0x144, self->_3C);
	}
}

extern "C" {

// 0x801A74C0
// "Ready" Starts the 60 frame wait at _70
void battleShowReady(TAeObjBattle* self)
{
	showBanner(self, 0x44);
	self->_78 = 0;
	self->_70 = 60;
}

// 0x801A7554
// "Go"
void battleShowGo(TAeObjBattle* self)
{
	showBanner(self, 0x4B);
	self->_78 = 0;
}

// 0x801A75E0
// Banner 0 the empty one
void battleShowNothing(TAeObjBattle* self)
{
	showBanner(self, 0);
	self->_78 = 0;
}

// 0x801A766C
// "3" of the closing count which also clears the clock digits
void battleShowCount3(TAeObjBattle* self)
{
	showBanner(self, 0x5B);
	if (self->_40 != 0) {
		self->_40->clearSlot(0xF8);
		self->_40->clearSlot(0xF9);
		self->_40->clearSlot(0xFA);
		self->_40->clearSlot(0xFB);
		self->_40->clearSlot(0xFC);
	}
	self->_78 = 0;
}

// 0x801A7750
// "2"
void battleShowCount2(TAeObjBattle* self)
{
	showBanner(self, 0x59);
	self->_78 = 0;
}

// 0x801A77DC
// "1"
void battleShowCount1(TAeObjBattle* self)
{
	showBanner(self, 0x57);
	self->_78 = 0;
}

// 0x801A7868
// "Time up"
void battleShowTimeUp(TAeObjBattle* self)
{
	showBanner(self, 0x55);
	self->_78 = 0;
}

// 0x801A78F4
// "Sudden death"
void battleShowSuddenDeath(TAeObjBattle* self)
{
	showBanner(self, 0x51);
	self->_78 = 0;
}

// 0x801A7980
// Shows the 2 3 or 4 player frame
void battleSetPlayers(TAeObjBattle* self, int count)
{
	TAeLayer* layer = self->_40;
	if (layer == 0) {
		return;
	}
	int n;
	if (count < 2) {
		n = 2;
	} else {
		n = 4;
		if (count <= 4) {
			n = count;
		}
	}
	switch (n) {
	case 2:
		layer->setCell(0xF7, 0xCB);
		break;
	case 3:
		layer->setCell(0xF7, 0xD2);
		break;
	case 4:
		layer->setCell(0xF7, 0xD5);
		break;
	}
}

// 0x801A7A24
// Shows the clock and keeps the time left in frames at _6C
int battleOnTimeLeft(TAeObjBattle* self, BattleTimeMsg* msg)
{
	if (self->_40 != 0) {
		aeClipSetDigit(self->_40, msg->mMinutes, 0x4E, 0xFA);
		aeClipSetDigits2(self->_40, msg->mSeconds, 0x4E, 0xFB, 0xFC);
		self->_40->setFrame(0xF9, 0x58);
		self->_40->setCell(0xF8, 0x140);
	}
	self->_6C = 60.0f * (msg->mMinutes * 60) + 60.0f * msg->mSeconds;
	return 1;
}

// 0x801A7B08
// Moves the leader mark onto the new leader
int battleOnLeader(TAeObjBattle* self, BattlePlayerMsg* msg)
{
	if (self->_40 != 0 && self->_44 != 0) {
		self->_44->home();
		self->_44->wake();
		for (int i = 0; i < 4; i++) {
			s8 who = playerSlot(i);
			if (who == -1) {
				continue;
			}
			if (i == msg->mPlayer) {
				self->_40->bind(gAeBattleScoreSlots[who], self->_44);
				sePlay(0x3A);
			} else {
				self->_40->clearSlot(gAeBattleScoreSlots[who]);
			}
		}
	}
	return 1;
}

// 0x801A7BF0
// Adds a score change to a player builds the player's score layer the first time and pops up the change
int battleOnScore(TAeObjBattle* self, BattlePlayerMsg* msg)
{
	if ((int)self->mRule != 3) {
		return 0;
	}
	u16 player = msg->mPlayer;
	if (player >= 4) {
		return 0;
	}
	u16 slot = player;
	if (playerSlot(slot) == -1) {
		return 0;
	}
	if (msg->mDelta == 0) {
		return 1;
	}

	if (self->_48[player] == 0 && playerSlot(slot) != -1) {
		self->_48[player] = self->createLayer(sAeBattleScoreLayers[playerSlot(slot)], 0x10);
		if (self->_48[player] != 0) {
			self->_48[player]->mBanks = gAeBattlePalettes;
		}
		self->_48[player]->setCell(0xC6, sAeBattleIconCells[player]);
		battleShowScore(self, player);
		self->_48[player]->setFrame(0xC9, 0x20);
		battlePlaceLayer(self, self->_48[player], playerSlot(slot));
	}
	self->_48[player]->cue(0);
	((TAeClip*)self->_48[player])->pause();

	int delta = msg->mDelta;
	self->_7C[player] += delta;
	int score = self->_7C[player];
	if (score < 0) {
		score = 0;
	} else if (score > 99) {
		score = 99;
	}
	self->_7C[player] = score;

	if (self->_58[player] == 0) {
		self->_58[player] = self->createLayer(sAeBattleScoreChangeLayers[playerSlot(slot)], 0x10);
		if (self->_58[player] != 0) {
			self->_58[player]->mBanks = gAeBattlePalettes;
		}
	} else {
		self->_58[player]->reset(sAeBattleScoreChangeLayers[playerSlot(slot)], 0x10);
		self->_58[player]->mBanks = gAeBattlePalettes;
	}
	battlePlaceLayer(self, self->_58[player], playerSlot(slot));
	self->_8C[player] = player;

	BOOL gain = TRUE;
	if (delta < 0) {
		gain = FALSE;
	}
	int size  = FABS((f32)delta);
	int shown = (size < 0) ? 0 : (size > 99 ? 99 : size);
	if (shown < 10) {
		if (gain) {
			self->_58[player]->setCell(0xBB, 0x9E);
		} else {
			self->_58[player]->setCell(0xBB, 0x9C);
		}
		self->_58[player]->setCell(0xBC, sAeBattleSmallDigitCells[shown]);
		self->_58[player]->setFrame(0xBD, 0x20);
	} else {
		if (gain) {
			self->_58[player]->setCell(0xB7, 0x9E);
		} else {
			self->_58[player]->setCell(0xB7, 0x9C);
		}
		self->_58[player]->setCell(0xB8, sAeBattleSmallDigitCells[shown / 10]);
		self->_58[player]->setCell(0xB9, sAeBattleSmallDigitCells[shown % 10]);
		self->_58[player]->setFrame(0xBA, 0x20);
	}
	return 1;
}

// 0x801A7F9C
// Writes player's score into the two digits of its score layer
void battleShowScore(TAeObjBattle* self, int player)
{
	if (self->_48[player] != 0) {
		int ones;
		int tens;
		tens = self->_7C[player] / 10;
		ones = self->_7C[player] % 10;
		self->_48[player]->setCell(0xC7, sAeBattleDigitCells[tens]);
		self->_48[player]->setCell(0xC8, sAeBattleDigitCells[ones]);
		self->_48[player]->wake();
	}
}

// 0x801A8030
// Puts a player's layer in its corner of the screen
void battlePlaceLayer(TAeObjBattle* self, TAeLayer* layer, int slot)
{
	if (layer == 0) {
		return;
	}
	switch (slot) {
	case 0:
		layer->setTrans(15.0f, 21.0f);
		break;
	case 2:
		layer->setTrans(5.0f, 16.0f);
		break;
	case 1:
		layer->setTrans(15.0f, 250.0f);
		break;
	case 4:
		layer->setTrans(5.0f, 245.0f);
		break;
	case 3:
		layer->setTrans(330.0f, 16.0f);
		break;
	case 5:
		layer->setTrans(330.0f, 245.0f);
		break;
	}
}
}
