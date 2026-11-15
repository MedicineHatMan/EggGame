#ifndef _TOBJREDMAGMAOBJ_H
#define _TOBJREDMAGMAOBJ_H

#include "object/red/TObjRedMagma.h"

/**
 * @size 0xB8
 */
class TObjRedMagmaObj : public TObjRedMagma {
public:
	TObjRedMagmaObj(TObject* parent, SetDesc* desc);
	virtual ~TObjRedMagmaObj() { }

	virtual void initFromDesc(SetDesc* desc); // _30
	virtual f32 getRange();                   // _7C returns gRedMagmaObjRange
	virtual f32 getFarAdd();                  // _80 returns gRedMagmaObjFarAdd

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
};

#endif
