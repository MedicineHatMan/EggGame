#ifndef _STAGE_SETOBJ16_H_
#define _STAGE_SETOBJ16_H_

#include "core/types.h"

class SetDesc;
class TObject;

extern "C" {
TObject* createSetObj16(TObject*, SetDesc*);
}

#endif
