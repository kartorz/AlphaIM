/*
 * Convert xml file to  AlphaDictionary.
 *
 * See Doc/dictionary_inner.pdf for more inner format detail.
 *
 * Changes:
 * 11-Nov-2013, initial version: [LiQiong lee]
 *
 */

#ifdef WIN32
#include <Windows.h>
#include <mbctype.h>
#endif
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <locale.h>
#include <unistd.h>

#include <string>
#include <vector>


#ifdef WIN32
#include "win32/pgetopt.h"
#endif

#include "type.h"
#include "Util.h"

#include "PinyinTDFParser.h"
#include "HanTDFParser.h"
#include "PhTDFParser.h"

using namespace std;

#define VERSION     "0.1"

//#define INDEX_BLOCK_NR   2
//#define STRINX_WORDS_MAX 10
//#define STRINX_DEPTH_MAX 5
//#define CHRINX_DEPTH_MIN 1
//#define STRINX_LEN_MAX   60  /* a string index must be within a block. */

static void usage()
{
    printf("Usage: AIMTool [options] \n");
    printf("options:\n");
    printf("    -h help\n");
    printf("    -v version\n");
    printf("    -p tdf db  :create PY database \n");
    printf("    -a tdf db  :create Han database \n");
    printf("    -w tdf db  :create Phrase database \n");
    printf("       For example:\n");
    printf("       -p system/pinyin.tdf  system/pinyin.imdb \n");
}

static void make_pinyinDB(const string& pinyinPath, const string& dbPath);
static void make_hanDB(const string& hanPath, const string& dbPath);
static void make_phDB(const string& hanPath, const string& dbPath);

int main(int argc, char* argv[])
{
	int c;

#ifdef _LINUX
	setlocale(LC_ALL, "C.UTF-8");
#endif
    //Util::getTimeMS();

	while (( c = getopt(argc, argv, "hvpaw:")) != -1) {
	    switch (c) {
	    case 'h':
	    	usage();
            return 0;

        case 'v':
            printf("version: %s\n", VERSION);
            return 0;

	    case 'p':
            if (argc ==  4) {
                make_pinyinDB(argv[2], argv[3]);
                return 0;
            }

	    case 'a':
            if (argc ==  4) {
                make_hanDB(argv[2], argv[3]);
                return 0;
            }

	    case 'w':
            if (argc ==  4) {
                make_phDB(argv[2], argv[3]);
                return 0;
            }

	    }

        usage();
        return 0;
    }

    usage();
    return 0;
}

static void make_pinyinDB(const string& pinyinPath, const string& dbPath)
{
    PinyinTDFParser pyParser;
    //pyParser.setDbPath(dbPath);
    Util::getTimeMS();

    pyParser.parser(pinyinPath);
    pyParser.write(dbPath);

    printf("write db file, done: \n");
    printf("    entries: %u\n", pyParser.getTotalEntry());
    printf("    costs: (%u)s:(%u)ms\n", Util::getTimeMS()/1000, Util::getTimeMS()%1000);
}

static void make_hanDB(const string& hanPath, const string& dbPath)
{
    HanTDFParser hanParser;
    //pyParser.setDbPath(dbPath);
    Util::getTimeMS();

    hanParser.parser(hanPath);
    hanParser.write(dbPath);

    printf("write db file, done: \n");
    printf("    entries: %u\n", hanParser.getTotalEntry());
    printf("    costs: (%u)s:(%u)ms\n", Util::getTimeMS()/1000, Util::getTimeMS()%1000);
}

static void make_phDB(const string& path, const string& dbPath)
{
    PhTDFParser phParser;
    //pyParser.setDbPath(dbPath);
    Util::getTimeMS();

    phParser.parser(path);
    phParser.write(dbPath);

    printf("write db file, done: \n");
    printf("    entries: %u\n", phParser.getTotalEntry());
    printf("    costs: (%u)s:(%u)ms\n", Util::getTimeMS()/1000, Util::getTimeMS()%1000);
}
