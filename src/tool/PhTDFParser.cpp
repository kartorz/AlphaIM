/**
 *    @Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
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
#include "PhTDFParser.h"
#include "indextree_item.h"

/* A phrase entry's data is its priority: exactly 1 byte. Passing 1 here makes
 * IndexTreeWriter::add() store fixed 1-byte items with no length prefix, and
 * PinyinTDFParser::parseInxtreeHeaderEnd() declares the same width in the
 * header, which is what readers use (IndexTree::load -> m_dataItemSize, then
 * PY.cpp: "imitem.priority = item->d.buf[0]").
 */
PhTDFParser::PhTDFParser() : PinyinTDFParser(1)
{
}

PhTDFParser::~PhTDFParser()
{
}

void PhTDFParser::parseRow(int row, string& str, vector<string>& columns)
{
    if (columns.size() == 2) {
        string key = columns[0];
        u8 priority = Util::stringToInt(columns[1]);

        u32* keyPtr = NULL;
        int keyLen = CharUtil::utf8StrToUcs4Str(key.c_str(), &keyPtr);
        if (keyLen > 0) {
            m_writer.add(keyPtr, keyLen, (void *)&priority, 1);
            free(keyPtr);
        }
    }
}
