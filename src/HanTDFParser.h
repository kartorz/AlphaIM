/** 
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#ifndef _HANTDFPARSER_H_
#define _HANTDFPARSER_H_

#include <string.h>

#include "indextree/IndexTreeWriter.h"
#include "TDFParser.h"

//#define PINYIN_MAGIC  0xB4B3

class HanTDFParser : public TDFParser
{
public:
    HanTDFParser();
    ~HanTDFParser();

protected:
    virtual void parseRow(int row, string& str, vector<string>& columns);
};

#endif
