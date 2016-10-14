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
#include "Util.h"
#include "HanTDFParser.h"
#include "indextree_item.h"

HanTDFParser::HanTDFParser()
{
}

HanTDFParser::~HanTDFParser()
{
}

void HanTDFParser::parseRow(int row, string& str, vector<string>& columns)
{
    if (columns.size() == 3) {
        string key = columns[0];
        string hanstr = columns[1];

        int itemlen = hanstr.length() + 1/*'\0'*/ + 1;
        han_item  *item = (han_item *) malloc(itemlen);
        memset(item, 0, itemlen);
        item->priority[0] = Util::stringToInt(columns[2]);
        memcpy(item->pystr, (void *)hanstr.c_str(), hanstr.length());

        u32* keyPtr = NULL;
        int keyLen = CharUtil::utf8StrToUcs4Str(key.c_str(), &keyPtr);
        if (keyLen > 0) {
            m_writer.add(keyPtr, keyLen, (void *)item, itemlen);
            free(keyPtr);
        }
        free(item);
    }
}
