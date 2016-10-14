/** 
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#ifndef _PHTDFPARSER_H_
#define _PHTDFPARSER_H_

#include <string.h>

#include "indextree/IndexTreeWriter.h"
#include "PinyinTDFParser.h"

//#define PINYIN_MAGIC  0xB4B3

class PhTDFParser : public PinyinTDFParser
{
public:
    PhTDFParser();
    virtual ~PhTDFParser();

protected:
    virtual void parseRow(int row, string& str, vector<string>& columns);
    virtual void parseInxtreeHeaderEnd();
};

#endif
