#ifndef _CRI_CRI_STD_H
#define _CRI_CRI_STD_H

#include <core/types.h>
#include <stdarg.h>

double cos(double);
double __frsqrte(double);
char* strcpy(char* dst, const char* src);
void DCFlushRange(void*, u32);
int strncmp(const char* a, const char* b, unsigned long n);
char* strcat(char* dst, const char* src);
int vsprintf(char*, const char*, va_list);
unsigned long strtoul(const char*, char**, int);
int OSDisableInterrupts(void);
int OSRestoreInterrupts(int level);

#endif
