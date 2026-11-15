#ifndef _SCROLLMODEL_H
#define _SCROLLMODEL_H

#include "stage/SetManager.h"

/**
 * @size 0x20
 */
struct ScrollModel {
	ScrollModel();
	~ScrollModel();
	void reset();
	void update();
	void begin();
	void end();
	void bindModel(void* node, SetTransform* trans);
	void draw(SetTransform* trans, void* model, void* texList);
	void readDesc(SetDesc* desc);
	void debugPrint();
	void debugPrintTrans(const SetTransform* trans);

	f32 mScrollU;  // _00
	f32 mScrollV;  // _04
	f32 mSpeedU;   // _08 the third float of the record times 0.01
	f32 mSpeedV;   // _0C the fourth float of the record times 0.01
	int mAngle;    // _10
	f32 mSin;      // _14
	f32 mScale;    // _18 the second float of the record plus 0.1
	int mTexIndex; // _1C the second int of the record minus 2
};

#endif
