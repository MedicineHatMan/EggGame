#ifndef _UI_AEDRAW_H_
#define _UI_AEDRAW_H_
struct AeXform;

#include "core/types.h"

struct AeFrame;
struct AeSlot;

extern "C" {
void aeApplyRecord(AeFrame*, int);
void aeCollectSlots(void*, s16, s16*, int, int*);
void aeDrawGroup(AeFrame*, int, const u32*);
AeSlot* aeFindSlot(AeFrame*, u32);
void aeGetRange(void*, u32, f32*, f32*);
void aeKey1(f32, f32*, f32*);
void aeKey2(f32, f32*, f32*, f32*);
void aeKey3(f32, f32*, f32*, f32*, f32*);
void aeScanRange(void*, u32, f32*, f32*, f32, f32, f32);
}

extern "C" {
void aeMultXform(f32*, AeXform*);
}

#endif
