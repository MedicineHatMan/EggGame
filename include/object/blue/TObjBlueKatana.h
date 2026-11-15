#ifndef _TOBJBLUEKATANA_H
#define _TOBJBLUEKATANA_H
struct SetDesc;
struct TObject;

#include "engine/DrawTask.h"
struct NJS_TEXLIST;
struct NJS_OBJECT;
#include "object/ObjModel.h"

class TMorphEggAnim;
#include "stage/SetManager.h"

extern "C" void objHeapFree(void*, void*);

/**
 * A katana sitting in the blue stage. Touching it with the bit-0x40 contact spawns the 0xCC effect,
 * and that effect's byte at +0x30 bit 0 retires the katana.
 *
 * @size 0xA4
 */
class TObjBlueKatana : public TSetObj {
public:
	TObjBlueKatana(TObject* parent, SetDesc* desc);
	virtual ~TObjBlueKatana();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10
	virtual void render(); // _1C

	void take();
	void stateWait();
	void stateHeld();

	// _80 is TSetObj's word reused as the collision
	DrawTask* mDrawTask;    // _90
	TMorphEggAnim* mEffect; // _94
	DrawState* mDrawState;  // _98 the default state until take() then mEffect's
	u8 mRecord3E;           // _9C SetRecord::_3E Nothing in this class reads it
	u8 _9D;                 // _9D
	u16 mHitId;             // _9E id from the contact
	int mAct;               // _A0 0 = waiting 1 = taken
};

extern "C" {
void initBlueKatana();
void termBlueKatana();
}

extern "C" {
TObject* createTObjBlueKatana(TObject*, SetDesc*);
}

#include "io/ResBank.h"
extern "C" {
extern const char* gTObjBlueKatanaName;
extern ResBank sBlueKatanaBank;
extern NJS_OBJECT** sBlueKatanaModels;
extern NJS_TEXLIST** sBlueKatanaTextures;
}

#endif
