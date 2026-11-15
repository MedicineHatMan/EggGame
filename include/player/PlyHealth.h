#ifndef _PLAYER_PLYHEALTH_H_
#define _PLAYER_PLYHEALTH_H_

#include "core/types.h"

struct PlyHealth;

extern "C" {
void plyClearWord(u32*);
void plyCountDown(u16*);
void plyHealthInit(PlyHealth*);
void plyHealthSet(PlyHealth*, int);
}

#endif
