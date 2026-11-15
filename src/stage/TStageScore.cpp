#include "stage/Stage.h"
#include "ui/AeObjControl.h"
#include "stage/TStageScore.h"
#include "stage/TEventCtrl.h"
#include "core/TMainTask.h"

// 0x801B32CC
extern "C" int framesToMs(int frames)
{
	return (frames * 1000) / 60;
}

// 0x801B32DC
extern "C" int kindIcon(int kind)
{
	switch (kind) {
	case 0:
		return 0;
	case 1:
		return 1;
	case 2:
		return 2;
	case 3:
		return 3;
	case 4:
		return 4;
	}
	return 0;
}

// 0x801B3338
extern "C" int patternIcon(int pattern)
{
	switch (pattern) {
	case 0:
		return 5;
	case 1:
		return 6;
	case 2:
		return 7;
	case 3:
		return 8;
	}
	return 0;
}

// 0x801B3388
TStageScore::TStageScore(TObject* parent)
    : TObject(parent)
{
	mFrames = 0;
	mTime   = 0;
	for (int i = 0; i < 16; i++) {
		mChains[i].mId       = 0;
		mChains[i].mKind     = 0;
		mChains[i].mLastTime = 0;
		mChains[i].mHits     = 0;
		mChains[i].mRank     = 0;
	}
	mCombo       = 0;
	mComboTime   = 0;
	mComboWindow = 7000;
	mComboCount  = 0;
	for (int i = 0; i < 3; i++) {
		mKinds[i] = 0;
	}
	mNextId = 1;
	stageRecordClear(&mRecord);
	mDirty = 0;
}

// 0x801B3458
TStageScore::~TStageScore()
{
	if (sStageScore == this) {
		sStageScore = 0;
	}
}

// 0x801B34CC
int TStageScore::hitsNeeded(int kind)
{
	return kind == 0 ? 2 : 3;
}

// 0x801B34DC
void TStageScore::addKind(int kind)
{
	if (1 <= mCombo && mCombo <= 3) {
		mKinds[mCombo - 1] = kind;
	}
	switch (kind) {
	case 0:
		mRecord.mCount[2]++;
		break;
	case 1:
		mRecord.mCount[3]++;
		break;
	case 2:
		mRecord.mCount[4]++;
		break;
	case 3:
		mRecord.mCount[5]++;
		break;
	case 4:
		mRecord.mCount[6]++;
		break;
	}
}

// 0x801B3578
// 0 = three of one kind 2 = one of each 1 = three different 3 = more than three -1 otherwise
int TStageScore::comboPattern()
{
	if (mCombo < 3) {
		return -1;
	}
	if (mCombo > 3) {
		return 3;
	}
	u32 mask;
	int a = mKinds[0];
	int b = mKinds[1];
	int c = mKinds[2];
	if (a == b && b == c) {
		return 0;
	}
	mask = (1u << a) | (1u << b);
	mask |= 1u << c;
	if (mask == 7) {
		return 2;
	}
	if (a != b && b != c && c != a) {
		return 1;
	}
	return -1;
}

// 0x801B360C
void TStageScore::addPatternBonus()
{
	int pattern = comboPattern();
	if (pattern < 0 || pattern >= 4) {
		return;
	}
	mRecord.mScore[2] += sScorePoints.mPattern[pattern];
}

// 0x801B366C
void TStageScore::touchCombo()
{
	mComboTime = mTime;
}

// 0x801B3678
// Takes a free chain or else the one hit longest ago
int TStageScore::newChain(int kind)
{
	if (kind < 0 || kind >= 5) {
		kind = 0;
	}
	int slot = -1;
	for (int i = 0; i < 16; i++) {
		if (mChains[i].mId <= 0) {
			slot = i;
			break;
		}
	}
	if (slot < 0) {
		int oldest = mChains[0].mLastTime;
		slot       = 0;
		for (int i = 1; i < 16; i++) {
			if (mChains[i].mLastTime < oldest) {
				slot   = i;
				oldest = mChains[i].mLastTime;
			}
		}
	}
	mChains[slot].mId       = 0;
	mChains[slot].mKind     = 0;
	mChains[slot].mLastTime = 0;
	mChains[slot].mHits     = 0;
	mChains[slot].mRank     = 0;
	mChains[slot].mId       = mNextId++;
	mChains[slot].mKind     = kind;
	mChains[slot].mLastTime = mTime;
	return mChains[slot].mId;
}

// 0x801B374C
// An enemy hit by the throw with this id
void TStageScore::hit(int id)
{
	if (id < 0) {
		return;
	}
	mRecord.mCount[0]++;
	if (id <= 0) {
		mRecord.mScore[0] += sScorePoints.mHit;
		return;
	}

	ScoreChain* chain;
	int points = mRecord.mScore[0];
	for (int i = 0; i < 16; i++) {
		chain = &mChains[i];
		if (chain->mId == id) {
			chain->mHits++;
			chain->mLastTime = mTime;
			if (chain->mRank <= 0) {
				int need = hitsNeeded(chain->mKind);
				if (chain->mHits >= need) {
					mCombo++;
					chain->mRank = mCombo;
					addKind(chain->mKind);
					addPatternBonus();
					points += (need - 1) * (sScorePoints.mHitCombo * chain->mRank - sScorePoints.mHit);
					mComboCount += need - 1;
					mDirty |= 1;
					mDirty |= 2;
				} else {
					points += sScorePoints.mHit;
				}
			}
			if (chain->mRank > 0) {
				touchCombo();
				points += sScorePoints.mHitCombo * chain->mRank;
				mComboCount++;
				mDirty |= 1;
			}
			break;
		}
	}
	if (chain == 0) {
		points += sScorePoints.mHit;
		if (mCombo > 0) {
			touchCombo();
		}
	}
	mRecord.mScore[0] = points;
}

// 0x801B390C
// An egg hatched a fresh one joins the combo
void TStageScore::egg(int fresh)
{
	mRecord.mCount[1]++;
	if (fresh != 0) {
		mCombo++;
		mRecord.mScore[1] += sScorePoints.mEggCombo * mCombo;
		touchCombo();
		addKind(4);
		addPatternBonus();
		mDirty |= 2;
	} else {
		mRecord.mScore[1] += sScorePoints.mEgg;
	}
}

// 0x801B39B4
void TStageScore::addExtra(int points)
{
	mRecord.mScore[3] += points;
}

// 0x801B39C4
void TStageScore::addBonus()
{
	addExtra(sScorePoints.mBonus);
}

// 0x801B39F0
// clock is in milliseconds
void TStageScore::addTime(int clock)
{
	addExtra((clock / 1000) * sScorePoints.mPerSecond);
}

// 0x801B3A28
void TStageScore::getRecord(StageRecord* out)
{
	*out = mRecord;
}

// 0x801B3A84
void TStageScore::setRecord(StageRecord* record)
{
	mRecord = *record;
}

// 0x801B3AE0
void TStageScore::draw()
{
	if (!eventIsBusy()) {
		mFrames++;
		mTime = framesToMs(mFrames);
	}
	int now = mTime;
	if (mCombo > 0) {
		if (now - mComboTime > mComboWindow) {
			mComboCount = 0;
			mCombo      = 0;
			for (int i = 0; i < 16; i++) {
				if (mChains[i].mRank > 0) {
					mChains[i].mHits = 0;
					mChains[i].mRank = 0;
				}
			}
			for (int i = 0; i < 3; i++) {
				mKinds[i] = 0;
			}
			aeFadeComboCounts();
		} else {
			if (mDirty & 1) {
				aeShowComboCount(mComboCount);
			}
			if (mDirty & 2) {
				int icon;
				int pattern = comboPattern();
				if (pattern == -1) {
					icon = kindIcon(mKinds[mCombo - 1]);
				} else {
					icon = patternIcon(pattern);
				}
				aeShowComboRank(mCombo, icon);
			}
		}
	}
	mDirty = 0;
}

extern "C" {
// 0x801B3C14
void stageRecordClear(StageRecord* record)
{
	record->mScore[0] = 0;
	record->mScore[1] = 0;
	record->mScore[2] = 0;
	record->mScore[3] = 0;
	record->mCount[0] = 0;
	record->mCount[1] = 0;
	record->mCount[2] = 0;
	record->mCount[3] = 0;
	record->mCount[4] = 0;
	record->mCount[5] = 0;
	record->mCount[6] = 0;
}

// 0x801B3C48
int stageRecordTotal(StageRecord* record)
{
	return record->mScore[0] + record->mScore[1] + record->mScore[2] + record->mScore[3];
}

// 0x801B3C68
void scoreInit()
{
	sStageScore = 0;
}

// 0x801B3C74
void scoreExit()
{
	if (sStageScore != 0) {
		sStageScore->mFlags |= 1;
		sStageScore = 0;
	}
}

// 0x801B3C98
void scoreCreate()
{
	if (sStageScore == 0) {
		sStageScore = new TStageScore(gTL_02);
	}
}

// 0x801B3CE4
// The points scaled by the time limit over the time taken kept between 0.5 and 2
int scoreWithTime(int points, int time, int limit)
{
	f32 t = (f32)limit / (f32)time;
	if (t > 2.0f) {
		t = 2.0f;
	} else if (t < 0.5f) {
		t = 0.5f;
	}
	return (int)((f32)points * t);
}

// 0x801B3D74
// 4 is the best rank
int scoreRank(int score, int a, int b, int c, int d)
{
	if (score >= a) {
		return 4;
	}
	if (score >= b) {
		return 3;
	}
	if (score >= c) {
		return 2;
	}
	return (score >= d) ? 1 : 0;
}

// 0x801B3DB8
int scoreNewChain(int kind)
{
	if (sStageScore != 0) {
		return sStageScore->newChain(kind);
	}
	return 0;
}

// 0x801B3DF4
void scoreHit(int id)
{
	if (sStageScore != 0) {
		sStageScore->hit(id);
	}
}

// 0x801B3E28
void scoreEgg(int fresh)
{
	if (sStageScore != 0) {
		sStageScore->egg(fresh);
	}
}

// 0x801B3E5C
void scoreAddExtra(int points)
{
	if (sStageScore != 0) {
		sStageScore->addExtra(points);
	}
}

// 0x801B3E90
void scoreAddBonus()
{
	if (sStageScore != 0) {
		sStageScore->addBonus();
	}
}

// 0x801B3EBC
void scoreAddTime(int clock)
{
	if (sStageScore != 0) {
		sStageScore->addTime(clock);
	}
}

// 0x801B3EF0
void scoreGetRecord(StageRecord* out)
{
	if (sStageScore != 0) {
		sStageScore->getRecord(out);
	}
}

// 0x801B3F24
void scoreSetRecord(StageRecord* record)
{
	if (sStageScore != 0) {
		sStageScore->setRecord(record);
	}
}
}
