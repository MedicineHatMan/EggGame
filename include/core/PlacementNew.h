#ifndef _CORE_PLACEMENTNEW_H_
#define _CORE_PLACEMENTNEW_H_

inline void* operator new(unsigned long, void* mem)
{
	return mem;
}

#endif
