#include "core/dolphin.h"
#include "stl/string.h"
#include "stage/Stage.h"
#include "io/ResFile.h"
#include "io/Resource.h"

extern "C" {
ChunkFile* sStageDefFile; // 0x802C4CE0
}

void loadStageDefs()
{
	loadOwnedResource("ge_stagedef.bin", 'GESD', &sStageDefFile);
}

StageDef* findStageDef(const char* name)
{
	StageDefTable* table = (StageDefTable*)sStageDefFile->mData;
	StageDef* def        = table->mEntries;

	for (int i = 0; i < table->mCount; i++, def++) {
		if (strcmp(name, def->mName) == 0) {
			return def;
		}
	}
	return 0;
}

StageDefTable* getStageDefTable()
{
	return (StageDefTable*)sStageDefFile->mData;
}
