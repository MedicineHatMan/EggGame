#include "stage/Stage.h"
#include "stage/StageTime.h"

// 0x8010EFE4
extern "C" StageTime* stageTimeAdd(StageTime* time, const StageTime* add)
{
	stageTimeSet(time, time->toMillis() + add->toMillis());
	return time;
}

// 0x8010F078
// Stops at zero
extern "C" StageTime* stageTimeSub(StageTime* time, const StageTime* sub)
{
	u32 a = time->toMillis();
	u32 b = sub->toMillis();
	stageTimeSet(time, a < b ? 0 : a - b);
	return time;
}

// 0x8010F118
extern "C" void stageTimeSet(StageTime* time, u32 millis)
{
	millis %= 86400000;
	time->mHour = millis / 3600000;
	millis %= 3600000;
	time->mMinute = millis / 60000;
	millis %= 60000;
	time->mSecond = millis / 1000;
	millis %= 1000;
	time->mMilli = millis;
}

// 0x8010F178
extern "C" void stageTimerTick(u32* timer)
{
	if (*timer == 0) {
		return;
	}
	int left = *timer - 16;
	if (left < 0) {
		left = 0;
	}
	*timer = left;
}
