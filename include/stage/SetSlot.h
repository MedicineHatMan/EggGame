#ifndef _STAGE_SETSLOT_H_
#define _STAGE_SETSLOT_H_
struct SetLink;

#include "core/types.h"

extern "C" {
void notifySetLinkDeath(SetLink*, void*);
void setLinkFree(SetLink*);
void setLinkDone(SetLink*);
void setLinkForever(SetLink*);
}

extern "C" {
void setLinkTimed(SetLink*, int);
}

#endif
