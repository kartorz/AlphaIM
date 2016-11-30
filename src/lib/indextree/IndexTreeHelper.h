#ifndef _INDEXTREEHELPER_H_
#define _INDEXTREEHELPER_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
# ifdef WIN32
#include <windows.h>
#define mutex_handle  HANDLE
# else
#include <pthread.h>
#define mutex_handle  pthread_mutex_t
# endif

#include "indextree_inner.h"

class IndexTreeHelper
{
public:
    static int       ucs4slen(const u32 *ucs);
    static char*     ucs4StrToUTF8Str(const u32 *ucs, size_t* u8slen=NULL);
    static int       ucs4CharToUTF8Byte(u32 uchr, char* ub);
    static u32       utf8byteToUCS4Char(const char** ub);
    static size_t    utf8StrToUcs4Str(const char *u8s,  u32** u32Ptr);

    static int       strComLen(const char* pstr1, const char* pstr2, int start = 0);

    static void   mergeFile(FILE *det, FILE *src);
    static off_t  checkBlockBound(off_t pos, int nbytes);
};

namespace indextree {

class ReadFile {
public:
	ReadFile()
	:ptr(NULL)
	{ }

	~ReadFile()
	{
		if (ptr != NULL)
			free(ptr);
	}
	size_t operator()(FILE *f, void *ptr, size_t length);
	void*  operator()(FILE *f, size_t length);

	void *ptr;
};


class Malloc {
public:
	Malloc()
	:ptr(NULL) { }
	void* operator()(size_t size) {
	    ptr = malloc(size);
		return ptr;
	}
	~Malloc()
    {
		if (ptr != NULL)
			free(ptr);
	}
	void *ptr;
};

class MutexCriticalSection {
public:
	MutexCriticalSection(bool re=false);

	~MutexCriticalSection()
	{
        #ifdef WIN32
            CloseHandle(m_mutex);
        #else
	    pthread_mutex_destroy(&m_mutex);
        #endif
	}

	void lock()
	{
	#ifdef WIN32
            WaitForSingleObject(m_mutex, INFINITE);
        #else
	    pthread_mutex_lock(&m_mutex);
        #endif
	}

    void trylock()
	{
        #ifdef WIN32
            WaitForSingleObject(m_mutex, INFINITE);
	#else
	    pthread_mutex_trylock(&m_mutex);
        #endif
	}

	void unlock()
	{
        #ifdef WIN32
            ReleaseMutex(m_mutex);
	#else
	    pthread_mutex_unlock(&m_mutex);
        #endif
	}

	mutex_handle& acquire() {return m_mutex;}

private:
    mutex_handle m_mutex;
};

class MutexLock {
public:
	MutexLock(MutexCriticalSection &mcs);
	~MutexLock();
private:
	MutexCriticalSection& m_criticalSection;
};

}


// format of @strver: m.n, eg:2.1.
//--------------------
// #include <vector>
// #include <boost/lexical_cast.hpp>
// #include <boost/algorithm/string.hpp>   /* above boost 1.32.0 */
#define inxtree_helper_w_version(header, strver) do {\
	vector<string> splitVec;\
    boost::split(splitVec, strver, boost::is_any_of("."), boost::algorithm::token_compress_on);\
    if (splitVec.size() > 1) {\
        header.d_version[0] = boost::lexical_cast<int>(splitVec[1]);\
    } else {\
        header.d_version[0] = 0;\
    }\
    header.d_version[1] = boost::lexical_cast<int>(splitVec[0]);\
}while(0)

#endif
