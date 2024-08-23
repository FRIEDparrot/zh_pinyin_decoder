#include <string>
#include <string.h>
#include <stdlib.h>
#include "codeconv.h"

#ifdef _WIN32
#include <windows.h>

string GbkToUtf8(const char* src_str)
{
	int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	string strTemp = str;
	if (wstr) delete[] wstr;
	if (str) delete[] str;
	return strTemp;
}

string Utf8ToGbk(const char* src_str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	string strTemp(szGBK);
	if (wszGBK) delete[] wszGBK;
	if (szGBK) delete[] szGBK;
	return strTemp;
}
#else
#include <iconv.h>

int GbkToUtf8(char* str_str, size_t src_len, char* dst_str, size_t dst_len)
{
	iconv_t cd;
	char** pin = &str_str;
	char** pout = &dst_str;

	cd = iconv_open("utf8", "gbk");
	if (cd == 0)
		return -1;
	memset(dst_str, 0, dst_len);
	if (iconv(cd, pin, &src_len, pout, &dst_len) == -1)
		return -1;
	iconv_close(cd);
	*pout = '\0';

	return 0;
}

int Utf8ToGbk(char* src_str, size_t src_len, char* dst_str, size_t dst_len)
{
	iconv_t cd;
	char** pin = &src_str;
	char** pout = &dst_str;

	cd = iconv_open("gbk", "utf8");
	if (cd == 0)
		return -1;
	memset(dst_str, 0, dst_len);
	if (iconv(cd, pin, &src_len, pout, &dst_len) == -1)
		return -1;
	iconv_close(cd);
	*pout = '\0';

	return 0;
}


#endif

int test(void)
{
	const char* src_str = "∆œÃ—√¿æ∆“ππ‚±≠";
	cout << "origin string: " << src_str << endl;

#ifdef _WIN32
	// windows default is gbk
	string dst_str = GbkToUtf8(src_str);
	cout << "gbk to utf8: " << dst_str << endl;

	string str_utf8 = Utf8ToGbk(dst_str.c_str());
	cout << "utf8 to gbk: " << str_utf8 << endl;
#else
	// unix default is utf8
	char dst_gbk[1024] = { 0 };
	Utf8ToGbk(src_str, strlen(src_str), dst_gbk, sizeof(dst_gbk));
	cout << "utf8 to gbk: " << dst_gbk << endl;

	char dst_utf8[1024] = { 0 };
	GbkToUtf8(dst_gbk, strlen(dst_gbk), dst_utf8, sizeof(dst_utf8));
	cout << "gbk to utf8: " << dst_utf8 << endl;
#endif

	return 0;
}

