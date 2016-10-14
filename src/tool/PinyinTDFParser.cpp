/** 
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>   /* above boost 1.32.0 */

#include "CharUtil.h"
#include "PinyinTDFParser.h"

PinyinTDFParser::PinyinTDFParser()
{
    printf("PinyinTDFParser()\n");
    m_writer.open();
    m_writer.setStrinxThreshold(0, 8);
}

PinyinTDFParser::~PinyinTDFParser()
{
}

void PinyinTDFParser::parseRow(int row, string& str, vector<string>& columns)
{
    if (/*m_dbFile && */columns.size() == 2) {
        string key = columns[0];
        string hanstr = columns[1];

        u32* keyPtr = NULL;
        int keyLen = CharUtil::utf8StrToUcs4Str(key.c_str(), &keyPtr);
        if (keyLen > 0) {
            m_writer.add(keyPtr, keyLen, (void *)hanstr.c_str(), hanstr.length() + 1/*'\0'*/);
            free(keyPtr);
        }
    }
}

void PinyinTDFParser::write(const string& dbpath)
{
    m_writer.write(dbpath);
}

void PinyinTDFParser::parseInxtreeHeaderStart()
{
    memset(&m_writer.m_header, 0, sizeof(struct inxtree_header));
    m_writer.m_header.magic[0] = 0xB3;
    m_writer.m_header.magic[1] = 0xB4;
    m_writer.m_header.d_coding[0] = INXTREE_UTF8;
}

void PinyinTDFParser::parseInxtreeHeader(vector<string>& columns)
{
    if (columns.size() == 3) {
        if (columns[1] == "identify")
            strncpy((char *)(m_writer.m_header.d_identi), columns[2].c_str(), 59);
        else if (columns[1] == "version") {
            inxtree_helper_w_version(m_writer.m_header, columns[2]);
        }
    }
    //printf("%d, %d, %s", row, column, str.c_str());
}

void PinyinTDFParser::parseInxtreeHeaderEnd()
{
}

unsigned int PinyinTDFParser::getTotalEntry()
{
    return m_writer.getTotalEntry();
}
