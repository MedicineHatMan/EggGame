#ifndef _TITEMMANAGE_H
#define _TITEMMANAGE_H

#include "object/ObjCommon.h"
struct NJS_TEXLIST;
#include "core/BlockPool.h"
#include "item/TItem.h"
#include "item/TItemEquipment.h"

class TArc;

/** What runs when an item archive has loaded, and before it is freed. @size 0x8 */
struct ItemArcFuncs {
	void (*mBind)(TArc* arc);    // _00
	void (*mRelease)(TArc* arc); // _04
};

/** @size 0xC */
struct ItemArcHandler {
	int mKind;           // _00
	ItemArcFuncs mFuncs; // _04
};

/** The start of an item archive with no handler: its textures. @size 0x10 */
struct ItemArcData {
	u8 _00[8];      // _00
	NJS_TEXLIST* mTexList; // _08
	void* mGvm;     // _0C
};

/** An item id and the archive it loads from. @size 0x8 */
struct ItemArcName {
	int mKind;          // _00 0xFFFF ends the table
	const char* mName;  // _04
};

/** What the item code asks of a player its bonuses its held item and the specs it is handed */
class ItemUser : public TObjAction {
public:
	virtual void vfn_6C();
	virtual void addBonus(int n);                     // _70
	virtual void addBigBonus(int n);                  // _74
	virtual int getItem(int* kind, int* arg);         // _78 nonzero when it holds one
	virtual void dropItem();                          // _7C
	virtual void setItem(const ItemSpec* spec);       // _80
	virtual void vfn_84();
	virtual void vfn_88();
	virtual void vfn_8C();
	virtual void vfn_90();
	virtual void vfn_94();
	virtual void useSpec(const ItemSpec* spec);       // _98
	virtual void setStatus(int status, int frames);   // _9C
	virtual u16 getEggId();                           // _A0
};

/**
 * Loads item archives on demand into blocks of a BlockPool and counts their holders. There is one, made by
 * createItemManage.
 *
 * @size 0x18
 */
class TItemManage {
public:
	typedef TListNode<ItemArc> Node;

	TItemManage();
	~TItemManage();

	void load(int kind);
	int attach(int kind, ItemHolder* holder);
	void release(ItemHolder* holder);
	void debugPrint(int x, int y);
	Node* find(int kind);
	Node* allocNode(int kind);
	void freeArc(ItemArc* arc);

	static void onLoaded(int result, int id);

	TNodePool<ItemArc> mPool; // _00
	TList<ItemArc> mList;     // _0C the archives in use or loading
};

/**
 * Lets the debug pad (pad 4) pick an item id, load it, or drop it at object 0. Its constructor is not in the DOL.
 * The name is the one its rodata string gives.
 *
 * @size 0x28
 */
class TItemDebug : public TObject {
public:
	virtual ~TItemDebug();
	virtual void draw(); // _0C

	int mKind;    // _20
	int mEnabled; // _24
};

extern "C" {
void initItems();
void createItemManage();
void destroyItemManage();
void itemLoad(int kind);
int itemHolderAttach(int kind, ItemHolder* holder);
void itemHolderRelease(ItemHolder* holder);
void spawnItem(u16 owner, int kind, Vec3f* pos);
const ItemSpec* findItemSpec(int kind);
TItem* createEquipItem(TObject* parent, const ItemSpec* spec);
TItem* createBombItem(TObject* parent, const ItemSpec* spec);
TItem* createItem(TObject* parent, const ItemSpec* spec);
TItem* giveItem(int owner, int kind);
void addItemArcHandler(int kind, const ItemArcFuncs* funcs);
const char* getItemArcName(int kind);

extern int gItemBattle; // nonzero in battle where most effects reach the other players instead
}

extern "C" {
extern ItemArcFuncs gBombArcFuncs;
}

#endif
