#ifndef _GAMESTATEROOT_H
#define _GAMESTATEROOT_H

#include "player/PlyState.h"

class TGameContext;
class TGameContextBattle;

extern "C" {
void* allocPlyState(u32 size, void* pool);
void releasePlyState(void* pool, void* state);
}

/**
 * The bottom of the state classes. The DOL has its vtable, but no name for it.
 *
 * @size 0x14
 */
class TGameStateRoot : public TActionState {
public:
	TGameStateRoot() { }
	static void* operator new(unsigned long size, void* pool) { return allocPlyState(size, pool); }
	virtual ~TGameStateRoot() { }                 // _08 0x800429E8
	virtual void enter(TGameContext* context) { } // _0C 0x80042A40
	virtual void exit(TGameContext* context) { }  // _10 0x80042A44
	virtual void run(TGameContext* context) { }   // _14 0x80042A48
};

/**
 * The same shape for the battle modes, whose context is a different class.
 *
 * @size 0x14
 */
class TBattleStateRoot : public TActionState {
public:
	TBattleStateRoot() { }
	static void* operator new(unsigned long size, void* pool) { return allocPlyState(size, pool); }
	virtual ~TBattleStateRoot() { }                     // _08 0x8019EF18
	virtual void enter(TGameContextBattle* context) { } // _0C 0x8019EF70
	virtual void exit(TGameContextBattle* context) { }  // _10 0x8019EF74
	virtual void run(TGameContextBattle* context) { }   // _14 0x8019EF78
};

#endif
