
#include <map>
#include <string.h>
#include <stdio.h>

#include "UsrPhrase.h"
#include "aim.h"
#include "indextree/indextree_inner.h"
#include "Util.h"
#include "CharUtil.h"
#include "Log.h"

#define USRPH_FILENAME "usrphrasetmp"

UsrPhrase::UsrPhrase()
{
    string path = home_dir + "/" + USRPH_FILENAME;
    m_phraseFile = fopen(path.c_str(), "a+");
    m_fileCache = "";
    m_phraseTmp = "";
    m_roundBase = 0;
    //m_usrPhraseTmp = "我的国家无之芒果西我的无之西恩的,我的国家无之想无之西地得到 , . ,我的国家无之西我的无之西恩的,我的国家无之西地得,地得,aabbbvvv,sdfs,之西恩的,我的我呒dfsdf得就额额fsfl马f那而而地国家得得aa<<我的";
}

UsrPhrase::~UsrPhrase()
{
    if (m_phraseFile) {
        fclose(m_phraseFile);
    }
}

void UsrPhrase::trackUsrInput(const string phrase,   vector<string> &phrases)
{
    m_phraseTmp += phrase;
    m_fileCache += phrase;
    if (m_phraseTmp.length() - m_roundBase >=  USRPHTMP_ROUND_SIZE) {
        m_roundBase = m_phraseTmp.length();
        //printf("is USRPHTMP_ROUND_SIZE %s %d\n", m_phraseTmp.c_str(), m_phraseTmp.length());
        log.d("is USRPHTMP_ROUND_SIZE %s %d\n", m_phraseTmp.c_str(), m_phraseTmp.length());
        parseUsrInput(m_phraseTmp, phrases);

        if (m_phraseTmp.length() >= USRPHTMP_MAX_SZIE) {
            m_phraseTmp.clear();
        }
        if (m_phraseFile) {
            //printf("write to phrase file\n");
            fwrite(m_fileCache.c_str(), m_fileCache.length(), 1, m_phraseFile);
            m_fileCache.clear();

            if (ftello(m_phraseFile)  >=  USRPHFILE_MAX_SZIE) {
                log.d("clear phrase file\n");

                util::ReadFile read;
                const char *ptr = (const char *)read(m_phraseFile, -1);               
                parseUsrInput(ptr, phrases); // It may take a wile.

                fclose(m_phraseFile);
                string path = home_dir + "/" + USRPH_FILENAME;
                m_phraseFile = fopen(path.c_str(), "w");
            }
        }
   }
}

/*
 * N : The duplicate number of Hans.
 * M : The duplicate string of a Han.
 * L : The length of a dumplicate string.
 * T : cost of 'parseUsrInput'
 * T =  N*M*M*L
 */
void UsrPhrase::parseUsrInput(string usrph, vector<string> &phrases)
{
    #define MAX_USRPH_LEN   8

    u32 *u32str = NULL;
    size_t total = CharUtil::utf8StrToUcs4Str(usrph.c_str(),  &u32str);
    map<u32, vector<int> > prefixMap;
    for (int i = 0; i < total; i++) {
        prefixMap[ u32str[i] ].push_back(i);
    }

    for (int i = 0; i < total; i++) {
        //[ U+4E00～U+9FFF ]
        if (u32str[i] < 0x4E00 || u32str[i] > 0x9FFF)
            continue;

        vector<LCS> lcsVec;
        vector<int>& indexVec = prefixMap[u32str[i]];
        //printf("0x%x: %d --> number:%d\n", u32str[i], i, indexVec.size());

        for (int j = 0; j < indexVec.size(); j++) {
            int iPos = indexVec[j];
            LCS lcs = {0, iPos};

            for (int ii = 0; ii < indexVec.size(); ii++) {
                if (ii == j) continue;
                int iiPos = indexVec[ii];
                int len; // Length of the common substr.
                for (len = 0; *(u32str+iPos+len) != '\0' && *(u32str+iiPos+len) != '\0' && *(u32str+iPos+len) == *(u32str+iiPos+len); len++) {}
                //printf("len == %d\n",  len);
                if (len > 1 && len > lcs.len) {
                    // More than one han char.
                    lcs.len = len;
                }
            }

            lcsVec.push_back(lcs);
        }

        for (int k = 0; k < lcsVec.size(); k++) {
            //printf("check %d, len:%d\n",  lcsVec[k].inx, lcsVec[k].len);
            if (lcsVec[k].len < 2) continue;

            bool bAdd = true;
            /* Fix get substr from the dumplicate phrase.
             *     eg: abcdexxxabcde. we get 'abcde  bcde, cde, de'.
             *  But maybe 'cde' can match other phrase.
             *  So, just keep one duplicate string.
             */
            for (int kk = k + 1; kk < lcsVec.size(); kk ++) {
                if (lcsVec[kk].len < 2) continue;

                int l = 0;
                int s1 = lcsVec[k].inx;
                int s2 = lcsVec[kk].inx;
                for (l = 0;  *(u32str+s1+l) == *(u32str+s2+l); l++) {}
                //printf("--->check kk %d --> %d  common len:%d\n", s1, s2, l);
                if (lcsVec[k].len < lcsVec[kk].len) {
                    // 'k' contained by 'kk'
                    if (l == lcsVec[k].len) {
                        //printf("k < kk, %d is contained by %d,clear(%d, %d wchars)\n", s1, s2, s1, l);
                        bAdd = false;
                    }
                    continue;
                }

                if (lcsVec[k].len == lcsVec[kk].len) {
                    //printf("k === kk  %d,%d \n", *(u32str+s2+l), *(u32str+s1+l) );
                    // s1/s2 is cleared partly.
                    if (l == lcsVec[kk].len || *(u32str+s2+l) == '\0') {
                        // Keep one dumplicate string.
                        //printf("%d is the same as %d, clear (%d --> %d wchars)\n", s1, s2, s2, l);
                        memset(u32str + s2, '\0', sizeof(u32) * l);
                        lcsVec[kk].len = 0;
                    } else if (*(u32str+s1+l) == '\0') {
                        // s1 is partly cleared, keep the other duplicate string.
                        //printf("%d is partly cleared, clear it, keep other\n", s1);
                        bAdd = false;
                    }

                    continue;
                }

                // 'kk'  is contained by 'k'
                if (l == lcsVec[kk].len) {
                    //printf("k > kk, %d is contained by %d, clear (%d -->%d wchars)\n", s2, s1, s2, l);
                    memset(u32str + s2, '\0', sizeof(u32) * l);
                    lcsVec[kk].len = 0;
                }
            }

            if (bAdd) {
                u32 ucs[MAX_USRPH_LEN + 1];
                int phlen = lcsVec[k].len <= MAX_USRPH_LEN ? lcsVec[k].len : MAX_USRPH_LEN;
                memcpy(ucs, u32str + lcsVec[k].inx, sizeof(u32) * phlen);
                ucs[phlen] = 0;
                char *u8s = CharUtil::ucs4StrToUTF8Str(ucs, NULL);
                if (u8s != NULL) {
                    //printf("%s\n", u8s);
                    phrases.push_back(u8s);
                    free(u8s);
                }
            } else {
                memset(u32str + lcsVec[k].inx, '\0', sizeof(u32) * lcsVec[k].len);
                lcsVec[k].len = 0;
            }
        }

        prefixMap.erase(u32str[i]);
    }

    if (u32str != NULL)
        free(u32str);
}

