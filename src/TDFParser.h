/** 
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

/*
 *   TDF: Table Data File.
 *   Format of TDF.
 *  ======================
 *
 *  It's a table struct, item separated by 'space' or 'tab' .
 *      item  item  .. item
 *      item  item  .. item(mn)
 *  
 *  Comment Line:
 *      Begin with '%' or '#' .
 *   
 *  The last line must be ended by '\n', eg, must be ended by a blank line ('\n').
 */

#ifndef _TDFPARSER_H_
#define _TDFPARSER_H_

#include <stdio.h>
#include <vector>
#include <string>

#define TDF_HEADER_START   "@--header--"
#define TDF_HEADER_END     "@--header-end--"
#define SEPARATOR_CHARS    " 	"   //SPACE, TABLE
#define COMMENT_CHARS      "%#"

// CODING LIST: [utf8, utf16]

using namespace std;

class TDFParser
{
public:
    TDFParser();
    virtual ~TDFParser();
    // coding: see the comment CODING LIST.
    bool parser(const string& file, const string conding="utf8");

protected:
    // row and column start from 1.
    virtual void parseRow(int row, string& str, vector<string>& columns) {}
    virtual void parseColumn(int row, int column, string& str) {}
    virtual void parseInxtreeHeaderStart() {}
    virtual void parseInxtreeHeader(vector<string>& columns) {}
    virtual void parseInxtreeHeaderEnd() {}
};

#endif
