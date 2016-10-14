#ifndef _CHAR_UTIL_H_
#define _CHAR_UTIL_H_
#include "type.h"

class CharUtil
{
public:
    static u4char_t  utf8byteToUCS4Char(const char** ub);
    static int       ucs4CharToUTF16Byte(u32 uchr, u16* ub);
    static char*     ucs4StrToUTF8Str(const u32 *ucs, size_t* u8slen=NULL);
    static int       ucs4slen(const u32 *ucs);
    static size_t    utf8StrToUcs4Str(const char *u8s,  u32** u32Ptr);
    static int       ucs4CharToUTF8Byte(u32 uchr, char* ub);

    static char*    mbsrtoutf8s(const char *mbs);
    static wchar_t* utf8srtowcs(const char *u8s);
    static wchar_t  mbrtowc(char** mb);
    static wchar_t* mbsrtowcs(const char *mb);
    static char*    wcsrtombs(const wchar_t *wc);
    static int      wcrtomb(char* s, wchar_t *wc);

    // Caller shold free the returned (char*) ptr.
    static char*    nextu8char(const char* u8str, int *o_len);
    static int      nextu8char(const char* u8str, char* u8chr);

    // Caller shold free the returned (char*) ptr.
    static char*    u8charat(const char* u8str, int pos, int *o_start);

    static wchar_t* gb2312towcs(const char *gbbytes);
};

#endif
