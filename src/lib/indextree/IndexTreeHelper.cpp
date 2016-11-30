
#include "IndexTreeHelper.h"

#define UCS4C_TO_U8BYTES_MAX    6

int IndexTreeHelper::ucs4slen(const u32 *ucs)
{
    int len = 0;
    while (*ucs++ != 0)
        len++;
    return len;
}

int IndexTreeHelper::ucs4CharToUTF8Byte(u32 uchr, char* ub)
{
    const char prefix[] = {0, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
    const u32  codeup[] = {
        0x80,           // U+00000000 ～ U+0000007F
        0x800,          // U+00000080 ～ U+000007FF
        0x10000,        // U+00000800 ～ U+0000FFFF
        0x200000,       // U+00010000 ～ U+001FFFFF
        0x4000000,      // U+00200000 ～ U+03FFFFFF
        0x80000000      // U+04000000 ～ U+7FFFFFFF
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

// Caller should release the returned pointer char*
char* IndexTreeHelper::ucs4StrToUTF8Str(const u32 *ucs, size_t* u8slen)
{
    size_t total = ucs4slen(ucs);
    total = total * UCS4C_TO_U8BYTES_MAX + 1;
    char *u8s = (char *)malloc(total);
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

u32 IndexTreeHelper::utf8byteToUCS4Char(const char** ub)
{
    int i, len;
    unsigned char b;
    u32 uchr;

    if(ub == NULL)
        return 0;

    b = **ub;
    (char *)(*ub)++;

    if(b < 0x80) {
        uchr = b;
        return uchr;
    }

    if(b < 0xC0 || b > 0xFD ) {
        uchr = 0;
        return 0;
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
        return 0;
    return uchr;
}

// Caller should release *u32Ptr;
size_t  IndexTreeHelper::utf8StrToUcs4Str(const char *u8s,  u32** u32Ptr)
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

/*@return  - offset aganist current pos. caller should use SEEK_CUR */
off_t IndexTreeHelper::checkBlockBound(off_t pos, int nbytes)
{
	off_t remain = INXTREE_BLOCK - pos%INXTREE_BLOCK;
	if (nbytes > remain){
		return remain; /* seek to next block */
    }
	return 0; /* don't seek */
}


void IndexTreeHelper::mergeFile(FILE *det, FILE *src)
{
	unsigned char buf[2048];
	int nr;

	while ((nr = fread(buf, 1, 2048, src)) > 0) {
		fwrite(buf, 1, nr, det);
	}
}

int IndexTreeHelper::strComLen(const char* pstr1, const char* pstr2, int start)
{
    int ret = 0;
    for (int i=start; *(pstr1+i) != '\0' && *(pstr2+i) != '\0'; i++)
        if (*(pstr1+i) == *(pstr2+i)) ++ret;

    return ret;
}

namespace indextree {

size_t ReadFile::operator()(FILE *f, void *ptr, size_t length)
{
    size_t rdbytes = 0;
    size_t rsize;
    if (f == NULL)
        return 0;
    do {
        int bytes = length - rdbytes;
	rsize = fread(ptr, 1, bytes, f);
        rdbytes += rsize;
    }while(rsize > 0 && rdbytes < length);
    return rdbytes;
}

void* ReadFile::operator()(FILE *f, size_t length)
{
    if (f == NULL)
        return NULL;

    if (ptr != NULL)
        free(ptr);
    if (length == -1) {
        fseek(f, 0, SEEK_END);
        length = ftello(f);
        fseek(f, 0, SEEK_SET);
    }
    ptr = malloc(length);
    size_t rdbytes = 0;
    size_t rsize;
    do {
        int bytes = length - rdbytes;
	rsize = fread(ptr, 1, bytes, f);
        rdbytes += rsize;
    }while(rsize > 0 && rdbytes < length);
    return ptr;
}

MutexCriticalSection::MutexCriticalSection(bool re)
{
#ifdef WIN32
    m_mutex = CreateMutex( NULL, FALSE, NULL);
#else
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	if (re)
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	else
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);

	pthread_mutex_init(&m_mutex, &attr);

	pthread_mutexattr_destroy(&attr);
#endif
}


MutexLock::MutexLock(MutexCriticalSection &mcs)
:m_criticalSection(mcs)
{
	m_criticalSection.lock();
}

MutexLock::~MutexLock()
{
	m_criticalSection.unlock();
}

}
