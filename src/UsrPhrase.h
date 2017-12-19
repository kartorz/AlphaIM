#ifndef _USRPHRASE_H_
#define _USRPHRASE_H_

#include <stdio.h>
#include <string>
#include <vector>

#define USRPHTMP_ROUND_SIZE  512
#define USRPHTMP_MAX_SZIE    USRPHTMP_ROUND_SIZE * 8
#define USRPHFILE_MAX_SZIE   USRPHTMP_MAX_SZIE * 5
#define USRPH_MAX_SIZE       16 // about four hans.

using namespace std;

typedef struct {
    int len;
    int inx;
}LCS;

class UsrPhrase
{
public:
    UsrPhrase();
    ~UsrPhrase();
    void trackUsrInput(const string phrase,   vector<string> &phrases);

private:
    /*
     * N : The duplicate number of Hans.
     * M : The duplicate string of a Han.
     * L : The length of a dumplicate string.
     * T : cost of 'parseUsrInput'
     * T =  N*M*M*L
     */
    void parseUsrInput(string usrph, vector<string> &phrases);

    FILE *m_phraseFile;

    string m_fileCache;
    string m_phraseTmp;
    int    m_roundBase;
};
#endif
