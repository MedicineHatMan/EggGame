#ifndef _ANIMAL_ANIFLAGS_H_
#define _ANIMAL_ANIFLAGS_H_

#include "animal/TAniMount.h"
#include "animal/AniFlagBits.h"

template <class T>
void flagByteClear(T* flags, int bit)
{
	flags->mBits &= ~(1 << bit);
}

// 0x801C09B0
// for FlagWord
template <class T>
void flagClear(T* flags, int bit)
{
	flags->mBits &= ~(1 << bit);
}

#pragma defer_codegen on

// 0x801C0910
int TAniMount::sendToHandle(ObjMsg* msg)
{
	switch (*(u16*)msg) {
	case 0x48:
		return mountMsg(msg);
	case 0x45:
		return dismountMsg(msg);
	}
	switch (*(u16*)msg) {
	case 0x44:
		return onMsg44(msg);
	case 0x46:
		return onMsg46(msg);
	case 0x47:
		return onMsg47(msg);
	case 0x23:
		return onMsg23(msg);
	}
	return 0;
}

#endif
