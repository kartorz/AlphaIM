/**
 *    @Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#ifndef _PINYINTDFPARSER_H_
#define _PINYINTDFPARSER_H_

#include <string.h>

#include "indextree/IndexTreeWriter.h"
#include "TDFParser.h"

//#define PINYIN_MAGIC  0xB4B3

class PinyinTDFParser : public TDFParser
{
public:
    /* dataItemSize: (0 = variable size; 1 = fixed size).
     Subclasses pass the width their rows produce.*/
    explicit PinyinTDFParser(int dataItemSize = 0);
    virtual ~PinyinTDFParser();

    void write(const string& dbpath);
    unsigned int getTotalEntry();

protected:
    virtual void parseRow(int row, string& str, vector<string>& columns);
    virtual void parseInxtreeHeaderStart();
    virtual void parseInxtreeHeader(vector<string>& columns);
    virtual void parseInxtreeHeaderEnd();

    //FILE *m_dbFile;
    IndexTreeWriter m_writer;
};

#endif
