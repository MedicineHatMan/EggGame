#ifndef _TAEOBJBOSSCONSOLE_H
#define _TAEOBJBOSSCONSOLE_H
#include "core/types.h"
struct DrawTask;

#include "ui/TAeLayer.h"

/**
 * @size 0x48
 */
class TAeObjBossConsole : public TAeObject {
public:
	TAeObjBossConsole(TObject* parent, void* data, void* assets);
	virtual ~TAeObjBossConsole();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();           // _0C
	virtual void render();         // _1C
	virtual int vfn_20(s16* code); // _20 a BossConsoleMsg

	int setLife(struct BossConsoleMsg* msg); // 0x800F2A24
	int close();                             // 0x800F2A6C

	TAeLayer* mBar;     // _34 id 0xB7
	TAeLayer* mBarBack; // _38 id 0xBA
	TAeLayer* mFace;    // _3C sBossConsoleOpen bound into mBar
	DrawTask* mTask;    // _40
	BOOL mClosing;      // _44
};

extern "C" int bossConsoleMsg(BossConsoleMsg*); // 0x800F29DC

extern "C" {
void drawTube(const f32*, u8, const u32*, f32, f32, f32);
void ringDrawInit();
void ringDrawTerm();
}

/** Message 0xB9 sets the boss's life 0 to 1 */
struct BossConsoleMsg {
	s16 mCode; // _00
	u8 _02[2];
	f32 mLife; // _04
};

extern "C" {
extern const u32 sRingSides;
extern const u32 sRingSegments;
extern const u32 sRingMtxIds[10];
}

#endif
