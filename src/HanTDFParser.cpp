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

HanTDFParser::HanTDFParser()
{
    m_writer.setStrinxThreshold(0, 8);
}

HanTDFParser::~HanTDFParser()
{
}

void HanTDFParser::parseRow(int row, string& str, vector<string>& columns)
{
    if (m_dbFile && columns.size() == 2) {
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

