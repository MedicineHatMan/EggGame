#ifndef _VIEWPORT_H
#define _VIEWPORT_H

#include "stage/Stage.h"

extern "C" {
void viewportInit();                                // 0x8001542C empty
void viewportTerm();                                // 0x80015430 empty
void setViewport(int split);                        // 0x80015434 0 = full screen 1 to 8 halves and quarters
void getViewport(f32* x, f32* y, f32* w, f32* h);   // 0x80015620 any pointer can be null
f32 quatNormalize(Quat* q);                         // 0x80015664 approximate returns the old length
void quatScale(Quat* q, f32 scale);                 // 0x80015694
void quatAdd(Quat* q, const Quat* add);             // 0x800156B0
}

#endif
