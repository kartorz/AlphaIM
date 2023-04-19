# ifdef WIN32
#include <Windows.h>
# endif

#include <wchar.h>
#include <stdio.h>
#ifdef _LINUX
#include <stdlib.h>
#include <string.h>
#endif
#include "CharUtil.h"

#define UCS4C_TO_U8BYTES_MAX    6

int CharUtil::ucs4CharToUTF16Byte(u32 uchr, u16* ub)
{
    if(ub != NULL ) {
        if (uchr <= 0xFFFF) {
            *ub = static_cast<wchar_t>(uchr);  
            return 1;
        } else if(uchr <= 0xEFFFF) {
            ub[0] = static_cast<u16>(0xD800 + (uchr >> 10) - 0x40);
            ub[1] = static_cast<u16>(0xDC00 + (uchr & 0x03FF));
            return 2;  
        }
    }
    return 0;  
}

int CharUtil::ucs4CharToUTF8Byte(u32 uchr, char* ub)  
{
    const unsigned char prefix[] = {0, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
    const u32  codeup[] = {  
        0x80,           // U+00000000 �� U+0000007F  
        0x800,          // U+00000080 �� U+000007FF  
        0x10000,        // U+00000800 �� U+0000FFFF  
        0x200000,       // U+00010000 �� U+001FFFFF  
        0x4000000,      // U+00200000 �� U+03FFFFFF  
        0x80000000      // U+04000000 �� U+7FFFFFFF  
    };

    int i, len;  
    len = sizeof(codeup) / sizeof(u32);  
    for (i = 0; i < len; i++)  
    {  
        if (uchr < codeup[i])
            break;
    }
    if( i == len )
        return 0;

    len = i + 1;
    if (ub != NULL)
    {
        for( ; i > 0; i-- )  
        {
            ub[i] = static_cast<char>((uchr & 0x3F) | 0x80);  
            uchr >>= 6;  
        }
        ub[0] = static_cast<char>(uchr | prefix[len - 1]);  
    }
    return len;  
}

char* CharUtil::u8charat(const char* u8str, int pos, int *o_start)
{
    if(u8str == NULL)
        return NULL;

    const char* s = u8str;
    int len = 0;
    int i = 0;
    unsigned char b;

    do  {   
        b = *(s+len);
        if (b == 0x00) {
            if (pos == -1 && len > 0) {
                char *ret = (char *)malloc(len+1);
                strncpy(ret, s, len);
                ret[len] = '\0';
                if (o_start) *o_start = s - u8str;
                return ret;
            }
            return NULL;
        }
        s += len;
 
        if(b < 0x80) {
            len = 1;
        } else if(b < 0xC0 || b > 0xFD ) {
            return NULL;
        } else if(b < 0xE0) {
            len = 2;
        } else if(b < 0xF0) {
            len = 3;  
        } else if( b < 0xF8) {
            len = 4;	
        } else if(b < 0xFC) {  
            len = 5;
        } else {
            len = 6;
        }

        if ((pos != -1) && (i == pos)) {
            char *ret = (char *)malloc(len+1);
            strncpy(ret, s, len);
            ret[len] = '\0';
            if (o_start) *o_start = s - u8str;
            return ret;
        }
        i++;
    } while (1);
}

// Caller should free the returned pointer
// 'o_len' should be a loop variable, adds the previous value.
char* CharUtil::nextu8char(const char* u8str, int *o_len)
{
    int i, len;
    unsigned char b;

    if(u8str == NULL)
        return NULL;

    b = *u8str;
    if (b == 0x00)
        return NULL;

    if(b < 0x80) {
        len = 1;
    } else if(b < 0xC0 || b > 0xFD ) {
        return NULL;
    } else if(b < 0xE0) {
        len = 2;
    } else if(b < 0xF0) {
        len = 3;  
    } else if( b < 0xF8) {
        len = 4;	
    } else if(b < 0xFC) {  
        len = 5;
    } else {
        len = 6;
    }

    char *ret = (char *)malloc(len+1);
    strncpy(ret, u8str, len);
    ret[len] = '\0';
    if (o_len) *o_len += len;
    return ret;
}

int CharUtil::nextu8char(const char* u8str, char* u8chr)
{
    int i, len;
    unsigned char b;

    b = *u8str;
    if (b == 0x00)
        return 0;

    if(b < 0x80) {
        len = 1;
    } else if(b < 0xC0 || b > 0xFD ) {
        return -1;
    } else if(b < 0xE0) {
        len = 2;
    } else if(b < 0xF0) {
        len = 3;  
    } else if( b < 0xF8) {
        len = 4;	
    } else if(b < 0xFC) {  
        len = 5;
    } else {
        len = 6;
    }

    strncpy(u8chr, u8str, len);
    u8chr[len] = '\0';
    return len;
}

u32 CharUtil::utf8byteToUCS4Char(const char** ub)
{
    int i, len;
    unsigned char b;
    u32 uchr;

    if(ub == NULL)
        return 0; // '\0'

    b = **ub;
    (char *)(*ub)++; //Move to next byte.

    if(b < 0x80) {
        uchr = b;
        return uchr;
    }

    if(b < 0xC0 || b > 0xFD ) {
        uchr = -1;
        return -1; // invalid utf8 string.
    }

    if(b < 0xE0) {
        uchr = b & 0x1F;  
        len = 2;
    } else if(b < 0xF0) {
        uchr = b & 0x0F;  
        len = 3;  
    } else if( b < 0xF8) {
        uchr = b & 7;
        len = 4;	
    } else if(b < 0xFC) {  
        uchr = b & 3;
        len = 5;
    } else {
        uchr = b & 1;
        len = 6;
    }

    for (i = 1; i < len; i++) {
        b = **ub;
        (char *)(*ub)++;
        if(b < 0x80 || b > 0xBF)
            break;
        uchr = (uchr << 6) + (b & 0x3F);
    }

    if(i < len)
        return -1;
    return uchr;
}

int CharUtil::ucs4slen(const u32 *ucs)
{
    int len = 0;
    while (*ucs++ != 0)
        len++;
    return len;
}

// Caller should release *u32Ptr;
size_t  CharUtil::utf8StrToUcs4Str(const char *u8s,  u32** u32Ptr)
{
    size_t len = strlen(u8s);
    u4char_t *u4s = (u4char_t *)malloc((len+1)*sizeof(u4char_t));
    size_t offset = 0;
    int u4slen = 0;
    while (*u8s != '\0') {
        u4char_t u4chr = utf8byteToUCS4Char(&u8s);
        if (u4chr == 0)
            break;
        if (u4chr == -1)
            continue; //return 0;
        u4s[offset++] = u4chr;
    }
    u4s[offset] = '\0';
    u4slen = offset;
    *u32Ptr = u4s;
    return u4slen;
}

// Caller should release the returned pointer char*
char* CharUtil::ucs4StrToUTF8Str(const u32 *ucs, size_t* u8slen)
{
    size_t total = ucs4slen(ucs);
    total = total * UCS4C_TO_U8BYTES_MAX + 1;
    char *u8s = (char *)malloc(total);
    if (u8s == NULL)
        return NULL;

    memset(u8s, '\0', total);
    u32 uchr;
    size_t offset = 0;
    while((uchr = *ucs++) != 0) {
        size_t nbytes = ucs4CharToUTF8Byte(uchr, u8s+offset);
        offset += nbytes;
    }
    u8s[offset] = '\0';
    if (u8slen != NULL)
        *u8slen = offset;
    return u8s;
}

char* CharUtil::mbsrtoutf8s(const char *mbs)
{
#ifdef WIN32
    size_t total = strlen(mbs);
    total = (total+1) * 4;
    wchar_t *ws = (wchar_t *)malloc(total);
    memset(ws, '\0', total);
    int len = MultiByteToWideChar(CP_ACP, 0, mbs, -1, ws, total);
    if (len > 0 && len < 0xFFFD) {
        len = len * UCS4C_TO_U8BYTES_MAX + 1;
        char *u8s = (char *)malloc(len);
        memset(u8s, '\0', len);
        WideCharToMultiByte(CP_UTF8, 0, ws, -1, u8s, len, NULL, NULL);
        free(ws);
        return u8s;
    }
#endif
    return NULL;
}

wchar_t* CharUtil::utf8srtowcs(const char *u8s)
{
#ifdef WIN32
    size_t total = (strlen(u8s) + 1)*4;
    wchar_t *ws = (wchar_t *)malloc(total);
    memset(ws, '\0', total);
    int len = MultiByteToWideChar(CP_UTF8, 0, u8s, -1, ws, total);
    if (len > 0 && len < 0xFFFD)
        return ws;
#endif
    return NULL;
}
/* This piece of code comes from an example of mbrtowc function of GNU libc. */
wchar_t CharUtil::mbrtowc(char** mb)
{
	wchar_t wctmp[1];
	size_t len = strlen(*mb);
	size_t nbytes = ::mbrtowc(wctmp, *mb, len, NULL);
	if (nbytes > 0) {
		if (nbytes > (size_t)-2)
			return 0;
		*mb += nbytes;
		return wctmp[0];
	}
    return 0;
}

int CharUtil::wcrtomb(char* s, wchar_t *wc)
{
    int nbytes = ::wcrtomb(s, *wc, NULL);
    if (nbytes == (size_t) -1)
        /* Error in the conversion. */
        return -1;
    return nbytes;
}

// Caller should release pointer 'wchar_t*'
wchar_t* CharUtil::mbsrtowcs(const char *mb)
{
    size_t len = strlen(mb);
    len = (len+1)*sizeof(wchar_t);
    wchar_t *result = (wchar_t *)malloc(len);
    memset(result, L'\0', len);
    size_t ret = ::mbsrtowcs(result, &mb, len, NULL);
    if (ret == (size_t)-1) {
        free(result);
        return NULL;
    }
    return result;
}

char* CharUtil::wcsrtombs(const wchar_t *wc)
{
    size_t len = wcslen(wc);
    len = len*sizeof(wchar_t) + 1;
    char *result = (char *)malloc(len);
    memset(result, '\0', len);
    size_t ret = ::wcsrtombs(result, &wc, len, NULL);
    if (ret == (size_t)-1) {
        if (result[0] == '\0') {
            free(result);
            printf("{wcsrtombs}: invalid wc string\n");
            return NULL;
        } else {
            printf("{wcsrtombs}: (%s), encounter a invalid wide character(0x%x)\n", result, *wc);
        }
    }
    return result;
}

wchar_t* CharUtil::gb2312towcs(const char *gbbytes)
{
    return NULL;
}
