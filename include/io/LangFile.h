#ifndef _LANGFILE_H
#define _LANGFILE_H

extern "C" {
char* getLangFileName(const char* name);                      // 0x801A31C0
char* insertFileSuffix(const char* name, const char* suffix); // 0x801A31E4 puts suffix in front of every '.'
}

#endif
