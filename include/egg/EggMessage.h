#ifndef _EGG_EGGMESSAGE_H_
#define _EGG_EGGMESSAGE_H_
struct BinFile;

#include "core/types.h"

extern "C" {
extern BinFile* sEggMessageFile;
extern int sEggMessageCount;
extern char** sEggMessages;
}

#endif
