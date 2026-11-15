#include "io/FileReader.h"
#include "Ninja/ninja.h"
#include "stl/string.h"
#include "stl/string.h"

static char sLangFileName[0x40] = "";

// 0x801A31C0
char* getLangFileName(const char* name)
{
	return insertFileSuffix(name, "_e");
}

// 0x801A31E4
// Puts suffix in front of every '.'
char* insertFileSuffix(const char* name, const char* suffix)
{
	char c;
	int n = 0;
	int i = 0;
	memset(sLangFileName, 0, sizeof(sLangFileName));
	for (; name[i] != '\0'; i++) {
		c = name[i];
		if (c == '.') {
			strcat(sLangFileName, suffix);
			n += strlen(suffix);
		}
		sLangFileName[n++] = c;
	}
	sLangFileName[n] = '\0';
	return sLangFileName;
}
