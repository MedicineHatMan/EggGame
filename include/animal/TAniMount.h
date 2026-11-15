#ifndef _ANIMAL_TANIMOUNT_H_
#define _ANIMAL_TANIMOUNT_H_

#include "core/types.h"

struct Vec3f;

extern "C" {
void aniMountSound(void*, int, const Vec3f*);
}

// Included by AniMount.h which every user includes first
/**
 * @size 0x9C
 */
class TAniMount : public TEggCreature {
public:
	TAniMount(TObject* parent, int kind);
	virtual ~TAniMount();

	virtual void draw();
	virtual int sendToHandle(ObjMsg* msg);
	virtual void setPose(const Vec3f* pos, int rot);
	virtual void vfn_78();
	virtual void vfn_7C();
	virtual int vfn_84() // 0x801C0908
	{
		return mKind;
	}
	virtual int vfn_88();
	virtual int canMount() const;              // _98
	virtual int canDismount();           // _9C
	virtual void onMount();              // _A0
	virtual void onDismount();           // _A4
	virtual void onLoaded();             // _A8
	virtual void runState(int state, int phase); // _AC
	virtual void preUpdate();            // _B0
	virtual void postUpdate();           // _B4

	void setState(int state);
	int mountMsg(ObjMsg* msg);
	int dismountMsg(ObjMsg* msg);
	void rumble(s16 id);
	void die();

	int mLoaded;       // _4C
	int mState;        // _50
	int mKind;         // _54
	FlagWord mMountFlags; // _58 1 = gone
	PlyInput mInput;   // _5C
	AniAsset mAsset;   // _74
	Vec3f mStartPos;   // _8C
	int mStartRot;     // _98
};

typedef char TAniMount_size_check[(sizeof(TAniMount) == 0x9C) ? 1 : -1];

#endif
