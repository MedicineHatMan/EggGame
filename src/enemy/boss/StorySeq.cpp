#include "enemy/boss/TEneYellowBossGreen.h"
#include "io/Resource.h"
#include "core/Mem.h"
#include "object/TObjKey00.h"
#include "stage/TGameState.h"
#include "Ninja/ninja.h"

extern "C" {
TArc* gStorySeqArc;  // 0x802C5A60
StorySeq* gStorySeq; // 0x802C5A64
}
// Where a run is the offset into the code and the stack
struct StoryRun {
	u32 mPc;    // _00
	int* mSp;   // _04
	int* mBase; // _08
};

// 0x801AA4C4
extern "C" void storySeqInit()
{
	gStorySeq = new StorySeq;
	if (loadArc("storyseq.arc", &gStorySeqArc)) {
		gStorySeq->mData = (StoryData*)gStorySeqArc->mData;
	}
}

// 0x801AA524
// Runs program id 0 before it is loaded
extern "C" int storySeqEval(int id)
{
	if (gStorySeq != 0) {
		return gStorySeq->eval(id);
	}
	return 0;
}

// 0x801AA560
u8* storyReadOp(u8* pc, u16* out)
{
	u8* to = (u8*)out;

	*to++ = *pc++;
	*to   = *pc++;
	return pc;
}

// 0x801AA57C
u8* storyReadOffset(u8* pc, s16* out)
{
	u8* to = (u8*)out;

	*to++ = *pc++;
	*to   = *pc++;
	return pc;
}

// 0x801AA598
u8* storyReadWord(u8* pc, int* out)
{
	u8* dst = (u8*)out;
	dst[0]  = *pc++;
	dst[1]  = *pc++;
	dst[2]  = *pc++;
	dst[3]  = *pc++;
	return pc;
}

// 0x801AA5C4
void storyPushRef(StoryRun* run, const int* value)
{
	int* sp  = run->mSp;
	run->mSp = sp + 1;
	*sp      = *value;
}

// 0x801AA5DC
void storyPush(StoryRun* run, int value)
{
	*run->mSp++ = value;
}

// 0x801AA5F0
int* storyPop(StoryRun* run)
{
	return --run->mSp;
}

// 0x801AA604
// Runs to op 0x10 which leaves the top in place of the program's own words
void storyRun(StoryRun* run, StoryProgram* program)
{
	int word;
	int result;
	u16 op;
	s16 offset;
	u8* pc;
	int* base;

	base     = run->mSp - program->mArgs;
	run->mSp = base + program->mLocals;
	syMemSet(base + program->mArgs, 0, (program->mLocals - program->mArgs) * 4);
	pc = program->mCode + run->mPc;
	do {
		pc = storyReadOp(pc, &op);
		switch (op) {
		case 3:
			pc = storyReadWord(pc, &word);
			storyPush(run, word);
			break;
		case 4:
			run->mSp--;
			break;
		case 5:
			pc = storyReadOffset(pc, &offset);
			pc += offset;
			break;
		case 6:
			pc = storyReadOffset(pc, &offset);
			if (*storyPop(run) != 0) {
				pc += offset;
			}
			break;
		case 7:
			pc = storyReadOffset(pc, &offset);
			if (*storyPop(run) != 0) {
				storyPush(run, 1);
				pc += offset;
			}
			break;
		case 8:
			pc = storyReadOffset(pc, &offset);
			if (*storyPop(run) == 0) {
				pc += offset;
			}
			break;
		case 9:
			pc = storyReadOffset(pc, &offset);
			if (*storyPop(run) == 0) {
				storyPush(run, 1);
				pc += offset;
			}
			break;
		case 10:
			word = *storyPop(run);
			storyPush(run, *storyPop(run) == word);
			break;
		case 11:
			word = *storyPop(run);
			storyPush(run, *storyPop(run) != word);
			break;
		case 12:
			word = *storyPop(run);
			storyPush(run, *storyPop(run) < word);
			break;
		case 13:
			word = *storyPop(run);
			storyPush(run, *storyPop(run) <= word);
			break;
		case 14:
			word = *storyPop(run);
			storyPush(run, *storyPop(run) > word);
			break;
		case 15:
			word = *storyPop(run);
			storyPush(run, *storyPop(run) >= word);
			break;
		case 16:
			result   = *storyPop(run);
			run->mSp = base;
			storyPushRef(run, &result);
			break;
		case 0x100:
			run->mSp--;
			storyPush(run, keyBitIsSet(*run->mSp + 0x400));
			break;
		case 0x101:
			setKeyBit(*storyPop(run) + 0x400);
			break;
		case 0x102:
			clearKeyBit(*storyPop(run) + 0x400);
			break;
		case 0x103:
			storyPush(run, countKeyBits());
			break;
		}
	} while (op != 16);
	run->mPc = pc - program->mCode;
}

// 0x801AAA6C
StorySeq::StorySeq()
{
	mData  = 0;
	mStack = (int*)memAlloc(0x200);
}

// 0x801AAAAC
int StorySeq::eval(int id)
{
	StoryData* data = mData;
	StoryRun run;

	if (data == 0) {
		return 0;
	}
	if (id < 0 || (u32)id >= data->mCount) {
		return 0;
	}
	run.mPc   = 0;
	run.mSp   = mStack;
	run.mBase = mStack;
	storyRun(&run, &data->mPrograms[id]);
	return mStack[0];
}
