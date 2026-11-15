#ifndef _OBJECT_TOBJECT_H_
#define _OBJECT_TOBJECT_H_

#include "core/types.h"

class TObject;

extern "C" {
void drawObjectTask(TObject*);
void objApplyFlags(TObject*);
void objMarkChildrenDeleted(TObject*);
}

extern "C" {
void objDeleteTree(TObject*);
void objUpdateTree(TObject*);
void objWalkChildren(TObject*);
void objSetParent(TObject*, TObject*);
void objPrintTree(TObject*);
}

extern "C" {
int objCountChildren(TObject*);
}

extern "C" {
extern const char* gTObjectName;
extern int sObjAliveCount;
extern int sObjWalkDepth;
extern char sObjDumpSpaces[];
}

#endif
