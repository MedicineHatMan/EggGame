#include "item/TItemManage.h"
#include "item/TItemEquipComb.h"
#include "item/TItemEquipWing.h"
#include "item/TItemEquipShoeHiJump.h"
#include "item/TItemEquipShoeSpeed.h"
#include "item/TItemImm.h"
#include "item/TItemOnce.h"
#include "object/TObjSensorBomb.h"
#include "egg/ObjEgg.h"
#include "stage/GameState.h"
#include "io/ResManager.h"
#include "core/Pad.h"
#include "object/ObjAction.h"
#include "object/TObjItem.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "object/TObjWholeAttack.h"
#include "ui/AeArc.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "animal/AniMount.h"
#include "Dolphin/os.h"
#include "stage/TGameState.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "object/mission/TObjMissionBomb.h"
#include "object/TObjKey00.h"
#include "object/TObjExplode.h"
#include "object/TObjPlayer.h"
#include "Ninja/ninja.h"
struct NJS_TEXLIST;

class TObjMissionBomb;

/** The player's slots past TObjAction's as items call them */

int gItemBattle;
static BlockPool* sBlockPool;
static TItemManage* sItemManage;
static TItemManage* sInstance;

static const char* sClassName = "TItem";

static const ItemSpec sItemSpecs[] = {
	{ 0x00, 0x01, 0, 0x0C, 30 },  { 0x01, 0x01, 0, 0x0D, 30 },  { 0x03, 0x82, 0, 0x0B, 10 },
	{ 0x04, 0x82, 0, 0x0A, 15 },  { 0x05, 0x02, 1, 0x0C, 30 },  { 0x06, 0x02, 1, 0x00, 30 },
	{ 0x07, 0x02, 1, 0x01, 30 },  { 0x08, 0x02, 1, 0x02, 30 },  { 0x09, 0x02, 1, 0x03, 30 },
	{ 0x0A, 0x03, 0, 0x06, 30 },  { 0x0B, 0x03, 0, 0x00, 30 },  { 0x0C, 0x03, 1, 0x01, 30 },
	{ 0x0D, 0x03, 1, 0x04, 10 },  { 0x0E, 0x03, 1, 0x02, 0 },   { 0x0F, 0x03, 1, 0x03, 0 },
	{ 0x10, 0x00, 0, 0x28, 0 },   { 0x11, 0x00, 0, 0x64, 0 },   { 0x12, 0x03, 0, 0x05, 5 },
	{ 0x13, 0x45, 0, 0x00, 0 },   { 0x15, 0x20, 0, 0x00, 0 },   { 0x24, 0x44, 0, 0x7B, 0 },
	{ 0x27, 0x44, 0, 0x7D, 0 },   { 0x26, 0x44, 0, 0x7E, 0 },   { 0x25, 0x44, 0, 0x7F, 0 },
	{ 0x28, 0x44, 0, 0x7C, 0 },   { 0x16, 0x11, 0, 0x05, 0 },   { 0x17, 0x11, 0, 0x06, 0 },
	{ 0x18, 0x11, 0, 0x07, 0 },   { 0x19, 0x11, 0, 0x08, 0 },   { 0x1A, 0x11, 0, 0x09, 0 },
	{ 0x1B, 0x11, 0, 0x0A, 0 },   { 0x1C, 0x11, 0, 0x0B, 0 },   { 0x1D, 0x11, 1, 0x00, 0 },
	{ 0x1E, 0x11, 1, 0x02, 0 },   { 0x1F, 0x12, 2, 0x07, 0 },   { 0x20, 0x11, 2, 0x01, 0 },
	{ 0x21, 0x11, 0, 0x03, 0 },   { 0x22, 0x11, 0, 0x04, 0 },   { 0x23, 0x12, 0, 0x08, 0 },
};

typedef void (TItem::*ItemEffect)(const ItemSpec* spec, TObjAction* owner);

// What items with mode 3 do by spec->mParam
static ItemEffect sItemEffects[] = {
	&TItem::effectEggMsg, &TItem::effectShatterEggs, &TItem::effectBreakEggs, &TItem::effectSteal,
	&TItem::effectStatus, &TItem::effectTime,        &TItem::effectBonus,
};

static TFixedArray<ItemArcHandler, 8> sArcHandlers;

static inline ItemArcHandler* findHandler(ItemArcHandler* first, ItemArcHandler* last, const int& kind)
{
	while (first != last) {
		if (kind == first->mKind) {
			break;
		}
		first++;
	}
	return first;
}

static inline ItemArcHandler* findHandlerOf(ItemArcHandler* first, ItemArcHandler* last, const int& kind)
{
	while (first != last) {
		if (first->mKind == kind) {
			break;
		}
		first++;
	}
	return first;
}

static inline f32 randUnit()
{
	return (1.0f / 32768.0f) * rand();
}

// 0x801867F0
void initItems()
{
	addItemArcHandler(2, &gBombArcFuncs);
}

// 0x80186818
void createItemManage()
{
	sBlockPool  = new BlockPool(0x5000, 6);
	sItemManage = new TItemManage();
	gItemBattle = 0;
}

// 0x8018687C
void destroyItemManage()
{
	delete sItemManage;
	delete sBlockPool;
	sItemManage = NULL;
	sBlockPool  = NULL;
}

// 0x801868D0
void itemLoad(int kind)
{
	if (sItemManage != NULL) {
		sItemManage->load(kind);
	}
}

// 0x80186904
int itemHolderAttach(int kind, ItemHolder* holder)
{
	if (sItemManage != NULL) {
		return sItemManage->attach(kind, holder);
	}
	return 0;
}

// 0x80186948
void itemHolderRelease(ItemHolder* holder)
{
	if (sItemManage != NULL) {
		sItemManage->release(holder);
	}
}

// 0x8018697C
void spawnItem(u16 owner, int kind, Vec3f* pos)
{
	if (kind == 2) {
		spawnTObjMissionBomb(owner, pos);
	} else if (kind == 0x14) {
		((void (*)(u16, const Vec3f*))spawnChickenBomb)(owner, pos);
	} else {
		spawnItemDrop(owner, kind, pos);
	}
}

// 0x801869C4
const ItemSpec* findItemSpec(int kind)
{
	for (int i = 0; i < 0x27; i++) {
		if (kind == sItemSpecs[i].mKind) {
			return &sItemSpecs[i];
		}
	}
	return NULL;
}

// 0x80186A00
TItem* createEquipItem(TObject* parent, const ItemSpec* spec)
{
	switch (spec->mType) {
	case 0:
		return new TItemEquipComb(parent, spec);
	case 1:
		return new TItemEquipWing(parent, spec);
	case 2:
		switch (spec->mKind) {
		case 0x20:
			return new TItemEquipShoeHiJump(parent, spec);
		case 0x1F:
			return new TItemEquipShoeSpeed(parent, spec);
		default:
			return NULL;
		}
	default:
		return NULL;
	}
}

// 0x80186B2C
TItem* createBombItem(TObject* parent, const ItemSpec* spec)
{
	switch (spec->mKind) {
	case 0x15:
		return new TObjSensorBomb(parent);
	default:
		return NULL;
	}
}

// 0x80186B90
TItem* createItem(TObject* parent, const ItemSpec* spec)
{
	u8 flags = spec->mFlags;
	if (flags & 0x10) {
		return createEquipItem(parent, spec);
	}
	if (flags & 0x20) {
		return createBombItem(parent, spec);
	}
	if (flags & 0x40) {
		return new TItemImm(parent, spec);
	}
	return new TItemOnce(parent, spec);
}

// 0x80186C3C
TItem* giveItem(int owner, int kind)
{
	TObjAction* parent = findObjById(owner);
	TItem* item        = createItem(parent, findItemSpec(kind));
	if (item != NULL) {
		item->mKind    = kind;
		item->mOwnerId = owner;
	}
	return item;
}

// 0x80186C9C
ItemHolder::ItemHolder()
    : mNode(this)
{
	mEntry = NULL;
}

// 0x80186CE4
int ItemHolder::isPinned()
{
	return mFlags.mBits & 1;
}

// 0x80186CF0
TItem::TItem(TObject* parent)
    : TObjAction(parent)
{
	mOwnerId = 0xFFFF;
	_00      = (u32)sClassName;
	mSize    = 0x38;
	setObjId(this, 0xFFFF);
}

// 0x80186D50
TItem::~TItem()
{
	unregisterId();
}

// 0x80186DB4
TObjAction* TItem::getOwner() const
{
	return (TObjAction*)findPlayer(mOwnerId);
}

// 0x80186DD8
void TItem::apply(const ItemSpec* spec, TObjAction* owner)
{
	switch (spec->mFlags & 0xF) {
	case 3:
		if (sItemEffects[spec->mParam]) {
			(this->*sItemEffects[spec->mParam])(spec, owner);
		}
		break;
	case 4:
		setKeyBit(spec->mParam + 0x400);
		break;
	case 5: {
		ObjMsg msg;
		msg.mType = 0x9A;
		sendGameMsg(&msg);
		break;
	}
	default:
		((ItemUser*)owner)->useSpec(spec);
		break;
	}
}

// 0x80186EC0
void TItem::playFx(int fx)
{
	f32 mtx[12];
	mtxSetTransV(mtx, getPos());
	fxSpawnAt(1, 0, fx, mtx);
}

// 0x80186F18
void TItem::effectOthers(const ItemSpec* spec, TObjAction* owner, int status)
{
	for (int i = 0; i < 4; i++) {
		if (i != owner->mObjId) {
			ItemUser* player = (ItemUser*)findPlayer((u16)i);
			if (player != NULL) {
				player->setStatus(status, 60.0f * spec->mTime);
			}
		}
	}
	playFx(0x4F);
}

// 0x80186FC8
void TItem::effectAll(const ItemSpec* spec, TObjAction* owner, int status)
{
	for (int i = 0; i < 4; i++) {
		ItemUser* player = (ItemUser*)findPlayer((u16)i);
		if (player != NULL) {
			player->setStatus(status, 60.0f * spec->mTime);
		}
	}
	playFx(0x4F);
}

// 0x80187084
void TItem::effectEggMsg(const ItemSpec* spec, TObjAction* owner)
{
	ObjIdIter it;
	ObjMsg msg;
	msg.mType = 0x75;
	it        = objIdIter();
	for (it.begin(); !it.atEnd(); it.next()) {
		u32 name = it.get()->_00;
		if (name == (u32)gTObjEggName) {
			it.get()->sendToHandle(&msg);
		}
	}
	playFx(0x4F);
}

// 0x80187138
void TItem::effectShatterEggs(const ItemSpec* spec, TObjAction* owner)
{
	ObjIdIter it;
	it = objIdIter();
	for (it.begin(); !it.atEnd(); it.next()) {
		u32 name = it.get()->_00;
		if (name == (u32)gTObjEggName) {
			TObjAction* egg = it.get();
			u32 eggId = ((ItemUser*)owner)->getEggId();
			if (eggId != (u32)egg->mObjId) {
				shatterEgg(it.getEgg());
			}
		}
	}
	playFx(0x59);
	createWholeAttack(owner->getPos());
}

// 0x80187220
void TItem::effectBreakEggs(const ItemSpec* spec, TObjAction* owner)
{
	ObjIdIter it;
	it = objIdIter();
	for (it.begin(); !it.atEnd(); it.next()) {
		u32 name = it.get()->_00;
		if (name == (u32)gTObjEggName) {
			TObjAction* egg = it.get();
			u32 eggId = ((ItemUser*)owner)->getEggId();
			if (eggId != (u32)egg->mObjId) {
				it.getEgg()->shatterSilently();
			}
		}
	}
	playFx(0x5B);
	createWholeAttack(owner->getPos());
}

// 0x80187308
// This file passes findPlayer a u16 id the DOL masks an int index at the call
#define findPlayerById(id) (((TObjPlayer* (*)(u16))findPlayer)(id))

void TItem::effectSteal(const ItemSpec* spec, TObjAction* owner)
{
	if (gItemBattle != 0) {
		int self  = owner->mObjId;
		int count = 0;
		int kinds[4];
		for (int i = 0; i < 4; i++) {
			if (self != i) {
				ItemUser* player = (ItemUser*)findPlayerById(i);
				if (player != NULL) {
					int kind;
					int arg;
					if (player->getItem(&kind, &arg) && kind != 0xFFFF) {
						kinds[count++] = kind;
					}
				}
			}
		}
		if (count != 0) {
			const ItemSpec* got = findItemSpec(kinds[(int)(count * randUnit())]);
			((ItemUser*)owner)->setItem(got);
		}
		for (int i = 0; i < 4; i++) {
			if (self != i) {
				ItemUser* player = (ItemUser*)findPlayerById(i);
				if (player != NULL) {
					player->dropItem();
				}
			}
		}
		playFx(0x4F);
	} else {
		s8 kinds[] = { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0x10, 0x11, 0x12 };
		apply(findItemSpec((s8)kinds[(int)(13.0f * randUnit())]), owner);
	}
}

// 0x80187500
void TItem::effectStatus(const ItemSpec* spec, TObjAction* owner)
{
	if (gItemBattle != 0) {
		effectOthers(spec, owner, 5);
	} else {
		((ItemUser*)owner)->setStatus(0xE, 60.0f * spec->mTime);
	}
}

// 0x8018757C
void TItem::effectTime(const ItemSpec* spec, TObjAction* owner)
{
	if (gItemBattle != 0) {
		effectOthers(spec, owner, 6);
	} else {
		AddTimeMsg msg;
		msg.mType   = 0x9B;
		msg.mMinute = 1;
		msg.mSecond = 0;
		sendGameMsg(&msg);
	}
}

// 0x801875D0
void TItem::effectBonus(const ItemSpec* spec, TObjAction* owner)
{
	if (gItemBattle != 0) {
		effectAll(spec, owner, 4);
	} else if (randUnit() < 0.5f) {
		((ItemUser*)owner)->addBigBonus(200);
	} else {
		((ItemUser*)owner)->addBonus(10);
	}
}

// 0x80187678
TItemManage::TItemManage()
{
	mPool.create(6);
	sInstance = this;
}

// 0x801876C8
TItemManage::~TItemManage()
{
	for (Node* node = mList.mHead; node != NULL; node = node->mNext) {
		if (node->mData.mArc != NULL) {
			freeArc(&node->mData);
		} else {
			cancelDiscLoad(node->mData.mLoadId);
		}
		sBlockPool->release(node->mData.mBlock);
	}
	sInstance = NULL;
}

// 0x80187764
void TItemManage::load(int kind)
{
	Node* node;
	void* block;
	const char* name;
	if (find(kind) != NULL) {
		return;
	}
	name = getItemArcName(kind);
	if (name == NULL) {
		OSReport("TItemManage: filename not found\n");
		return;
	}
	block = sBlockPool->alloc();
	if (block == NULL) {
		OSReport("TItemManage: cannot alloc block\n");
		return;
	}
	node = allocNode(kind);
	if (node == NULL) {
		OSReport("TItemManage: cannot alloc sysinfo\n");
		sBlockPool->release(block);
		return;
	}
	mList.append(node);
	node->mData.mKind   = kind;
	node->mData.mBlock  = block;
	node->mData.mArc    = NULL;
	node->mData.mLoadId = requestDiscLoad(name, block, onLoaded);
}

// 0x80187848
int TItemManage::attach(int kind, ItemHolder* holder)
{
	Node* node = find(kind);
	if (node == NULL) {
		return 0;
	}
	holder->mEntry = node;
	node->mData.mHolders.append(&holder->mNode);
	return 1;
}

// 0x80187898
void TItemManage::release(ItemHolder* holder)
{
	ItemArc* arc = &holder->mEntry->mData;
	arc->mHolders.remove(&holder->mNode);
	if (arc->mHolders.mCount == 0) {
		if (arc->mArc != NULL) {
			freeArc(arc);
		} else {
			cancelDiscLoad(arc->mLoadId);
		}
		mList.remove(holder->mEntry);
		mPool.free(holder->mEntry);
		sBlockPool->release(arc->mBlock);
		holder->mEntry = NULL;
	}
}

// 0x80187934
void addItemArcHandler(int kind, const ItemArcFuncs* funcs)
{
	ItemArcHandler handler;
	handler.mKind  = kind;
	handler.mFuncs = *funcs;
	sArcHandlers.push_back(handler);
}

// 0x80187978
void TItemManage::debugPrint(int x, int y)
{
	njPrint((x << 16) | y, "active:    %d", mList.mCount);
	int loaded = 0;
	for (Node* node = mList.mHead; node != NULL; node = node->mNext) {
		if (node->mData.mArc != NULL) {
			loaded++;
		}
	}
	njPrint((x << 16) | (y + 1), "load num:  %d", loaded);
	njPrint((x << 16) | (y + 2), "free mem:  %d", sBlockPool->mCount);
}

// 0x80187A1C
TItemManage::Node* TItemManage::find(int kind)
{
	for (Node* node = mList.mHead; node != NULL; node = node->mNext) {
		if (kind == node->mData.mKind) {
			return node;
		}
	}
	return NULL;
}

// 0x80187A44
// With no free node takes back the first archive none of whose holders is pinned
TItemManage::Node* TItemManage::allocNode(int kind)
{
	Node* node = mPool.alloc();
	if (node == NULL) {
		for (node = mList.mHead; node != NULL; node = node->mNext) {
			TRefList<ItemHolder>::Node* ref;
			for (ref = node->mData.mHolders.mHead; ref != NULL; ref = ref->mNext) {
				if (ref->mObj->isPinned()) {
					break;
				}
			}
			if (ref == NULL) {
				node->mData.mHolders.clear();
				freeArc(&node->mData);
				sBlockPool->release(node->mData.mBlock);
				return node;
			}
		}
		return NULL;
	}
	return node;
}

// 0x80187AE4
void TItemManage::freeArc(ItemArc* arc)
{
	if (arc->mArc != NULL) {
		ItemArcHandler* handler = findHandler(sArcHandlers.begin(), sArcHandlers.end(), arc->mKind);
		ItemArcHandler* end     = sArcHandlers.end();
		if (handler != end) {
			if (handler->mFuncs.mRelease != NULL) {
				handler->mFuncs.mRelease(arc->mArc);
			}
		} else {
			ItemArcData* data = (ItemArcData*)arc->mArc->mData;
			if (data->mTexList != NULL) {
				releaseTextures(data->mTexList);
			}
		}
		releaseResource(arc->mArc);
	}
}

// 0x80187B88
// The disc load's callback wraps the block in a TArc and binds it
void TItemManage::onLoaded(int result, int id)
{
	Node* node;
	for (node = sInstance->mList.mHead; node != NULL; node = node->mNext) {
		if ((u32)id == node->mData.mLoadId) {
			break;
		}
	}
	if (node == NULL) {
		return;
	}
	TArc* arc = new TArc(node->mData.mBlock);
	node->mData.mArc = arc;
	ItemArcHandler* handler = findHandlerOf(sArcHandlers.begin(), sArcHandlers.end(), node->mData.mKind);
	ItemArcHandler* end     = sArcHandlers.end();
	if (handler != end) {
		if (handler->mFuncs.mBind != NULL) {
			handler->mFuncs.mBind(arc);
		}
	} else {
		ItemArcData* data = (ItemArcData*)arc->mData;
		if (data->mTexList != NULL) {
			numberTextures(data->mTexList);
			bindGvmTextures((u8*)data->mGvm, data->mTexList);
		}
	}
}

static ItemArcName sItemArcNames[] = {
	{ 0x00, "item_loop.arc" },        { 0x01, "item_spine.arc" },        { 0x02, "obj_ms_bomb.arc" },
	{ 0x03, "item_ani_bat.arc" },     { 0x04, "item_crow.arc" },         { 0x05, "item_ani_gori.arc" },
	{ 0x06, "item_ani_leon.arc" },    { 0x07, "item_ani_rat.arc" },      { 0x08, "item_ani_turtle.arc" },
	{ 0x09, "item_ani_lion.arc" },    { 0x0C, "item_ani_tiger.arc" },    { 0x0D, "item_ani_sheep.arc" },
	{ 0x0E, "item_ani_hawk.arc" },    { 0x0F, "item_ani_fox.arc" },      { 0x10, "item_ani_btfly2.arc" },
	{ 0x11, "item_ani_btfly.arc" },   { 0x13, "item_extend.arc" },       { 0x12, "item_watch.arc" },
	{ 0x14, "item_chickenbomb.arc" }, { 0x15, "item_sensorbomb.arc" },   { 0x0A, "item_dice.arc" },
	{ 0x20, "item_j_shoes.arc" },     { 0x1F, "item_s_shoes.arc" },      { 0x16, "item_comb_fire.arc" },
	{ 0x17, "item_comb_water.arc" },  { 0x18, "item_comb_thunder.arc" }, { 0x19, "item_comb_ice.arc" },
	{ 0x1A, "item_comb_wind.arc" },   { 0x1B, "item_comb_iron.arc" },    { 0x1C, "item_comb_light.arc" },
	{ 0x22, "item_cap_psi.arc" },     { 0x21, "item_cap_circus.arc" },   { 0x23, "item_cap_heart.arc" },
	{ 0x0B, "item_maxi.arc" },        { 0x1E, "item_rocket.arc" },       { 0x1D, "item_wing.arc" },
	{ 0x24, "item_gba_chu.arc" },     { 0x27, "item_gba_nights.arc" },   { 0x25, "item_gba_egg.arc" },
	{ 0x26, "item_gba_egg.arc" },     { 0x28, "item_gba_puyo.arc" },     { 0xFFFF, NULL },
};

// 0x80187C70
const char* getItemArcName(int kind)
{
	for (int i = 0; sItemArcNames[i].mName != NULL; i++) {
		if (kind == sItemArcNames[i].mKind) {
			return sItemArcNames[i].mName;
		}
	}
	return NULL;
}

// 0x80187CBC
void TItemDebug::draw()
{
	if ((padGet(3)->mPressed & 4) && (padGet(3)->mHold & 0x800)) {
		mEnabled = mEnabled == 0;
	}
	if (mEnabled == 0) {
		return;
	}
	njPrintSize(0xC);
	const char* name = getItemArcName(mKind);
	if (name == NULL) {
		njPrint(0x14, "no file");
	} else {
		njPrint(0x14, "load %s", name);
	}
	if (sItemManage != NULL) {
		sItemManage->debugPrint(0, 0x15);
	}
	if (padGet(3)->mRepeat & 4) {
		if (mKind > 0) {
			mKind--;
		}
	} else if (padGet(3)->mRepeat & 8) {
		if (mKind < 0x28) {
			mKind++;
		}
	} else if (padGet(3)->mRepeat & 0x100) {
		itemLoad(mKind);
	} else if (padGet(3)->mRepeat & 0x400) {
		TObjAction* obj = findObjById(0);
		if (obj != NULL) {
			spawnItem(0, mKind, obj->getPos());
		}
	}
	njPrintSize(0x10);
}

// 0x80187E44
TItemDebug::~TItemDebug() { }
