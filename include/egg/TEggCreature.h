#ifndef _TEGGCREATURE_H
#define _TEGGCREATURE_H

#include "object/ObjAction.h"

class TEggCreatureTypeE;
struct AniList;
struct ObjMsg;

/** How an animal hangs in TAniManager's list */
struct AniListLink {           // size 0x10
	AniListLink* mNext;        // _00
	AniListLink* mPrev;        // _04
	TEggCreatureTypeE* mOwner; // _08
	AniList* mList;            // _0C
};

/**
 * @size 0x4C
 */
class TEggCreature : public TObjAction {
public:
	TEggCreature(TObject* parent, int kind);
	virtual ~TEggCreature();

	virtual int getKind();                           // _2C 4
	virtual int sendToHandle(ObjMsg* msg);           // _4C
	virtual void setPose(const Vec3f* pos, int rot); // _6C
	virtual void vfn_70();                           // _70 message 0x44
	virtual int vfn_74();                            // _74 message 0x47
	virtual void vfn_78();                           // _78 message 0x46 let the rider go
	virtual void vfn_7C();                           // _7C die
	virtual void vfn_80(const void* body);           // _80 message 0x23 given what follows the type
	virtual int vfn_84();
	virtual int vfn_88();
	virtual void vfn_8C(u32 flags);
	virtual void vfn_90(u32 flags);
	virtual void vfn_94(f32* out);

	static TEggCreature* find(int id);
	static TEggCreature* cast(TObjAction* obj);

	int onMsg44(ObjMsg* msg);
	int onMsg46(ObjMsg* msg);
	int onMsg47(ObjMsg* msg);
	int onMsg23(ObjMsg* msg);

	AniListLink mLink; // _30 in TAniManager's list
	int mCreatureKind; // _40
	int _44;           // _44
	u16 mEggId;        // _48 the egg it hatched from
	u8 _4A[2];
};

#endif
