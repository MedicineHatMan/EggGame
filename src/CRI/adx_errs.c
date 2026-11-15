#include <CRI/adx.h>

char adxerr_msg[0x100];
char adxerr_tmp[0x20];
void* adxerr_obj;
void (*adxerr_func)(void* obj, char* msg);

// 0x8021FDA4
void ADXERR_Init(void)
{
	memset(adxerr_msg, 0, sizeof(adxerr_msg));
	adxerr_func = 0;
	adxerr_obj = 0;
}

// 0x8021FD5C
void ADXERR_Finish(void)
{
	memset(adxerr_msg, 0, sizeof(adxerr_msg));
	adxerr_func = 0;
	adxerr_obj = 0;
}

// 0x8021FCF8
void ADXERR_CallErrFunc1(const char* msg)
{
	strncpy(adxerr_msg, msg, 0xFF);
	if (adxerr_func != 0) {
		adxerr_func(adxerr_obj, adxerr_msg);
	}
	SVM_CallErr(adxerr_msg);
}

// 0x8021FC78
void ADXERR_CallErrFunc2(const char* msg1, const char* msg2)
{
	strncpy(adxerr_msg, msg1, 0xFF);
	strncat(adxerr_msg, msg2, 0xFF);
	if (adxerr_func != 0) {
		adxerr_func(adxerr_obj, adxerr_msg);
	}
	SVM_CallErr(adxerr_msg);
}

/* Writes n in decimal The digits are taken from the scratch buffer in reverse as CRI wrote it */
static void adxerr_ItoA(int n, char* str, int len)
{
	int i;
	int l;

	for (i = 0; i < 32; i++) {
		str[i] = n % 10;
		n /= 10;
		if (n == 0) {
			str[i] = '\0';
			break;
		}
	}
	l = strlen(adxerr_tmp);
	l = l < len - 1 ? l : len - 1;
	for (i = 0; i < l; i++) {
		str[i] = adxerr_tmp[l - 1 - i];
	}
	str[i] = '\0';
}

// 0x8021F79C
void ADXERR_ItoA2(int n1, int n2, char* str, int len)
{
	int l;

	adxerr_ItoA(n1, str, len);
	strncat(str, " ", len - strlen(str) - 1);
	l = 4 - strlen(str);
	str += strlen(str);
	adxerr_ItoA(n2, str, l);
}
