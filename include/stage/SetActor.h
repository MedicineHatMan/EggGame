#ifndef _SETACTOR_H
#define _SETACTOR_H
struct CharaColli;

#include "stage/SetManager.h"

/**
 * The base most stage objects are built on. No name is stored, so TSetActor is ours. TSetObj is a
 * sibling with a larger transform. TObjStageObj derives from this.
 *
 * Two constructors. The one that takes a record is what a set file spawns. The one that takes only a
 * parent is built from code: the id is still 0xFFFF, so it takes a new one, and it does not read a record.
 *
 * @size 0x64
 */
class TSetActor : public TObjAction {
public:
	TSetActor(TObject* parent);
	TSetActor(TObject* parent, SetDesc* desc);
	virtual ~TSetActor();

	inline void operator delete(void* mem);

	virtual Vec3f* getPos();                  // _20
	virtual int getRotY();                    // _28
	virtual int getKind();                    // _2C a placed object 3
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34 empty
	virtual void setPos(const Vec3f* pos);    // _6C
	virtual SetLink* getLink();               // _70

	Vec3f mHome;         // _30
	Vec3f mPos;          // _3C what getPos returns
	SetRot mRot;         // _48 getRotY returns mRot.y
	CharaColli* mShape;  // _54
	SetLinkMember mLink; // _58
};

#endif
