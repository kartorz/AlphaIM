/** 
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#include <boost/algorithm/string.hpp>

#include "TDFParser.h"
#include "Log.h"

using namespace boost;

TDFParser::TDFParser()
{
}

TDFParser::~TDFParser()
{
}

bool TDFParser::parser(const string& file, const string conding)
{
    FILE *fDB = fopen(file.c_str(), "rb");
    if (fDB == NULL) {
        log.e("{TDFParser}: can't open %s\n", file.c_str());
        return false;
    }

    int rowCnt = 0;
    char buff[4096];
    string buffRemainder;
    do {
        int len;
        string str;
        bool eof = false;
        if (feof(fDB) != 0)
            eof = true;
        else {
            len = fread(buff, 1, 4096, fDB);
            if (len <= 0)
                eof = true;
        }

        if (!eof) {
            str = buffRemainder += string(buff, len);
        }
        else if (buffRemainder != "")
            str = buffRemainder += "\n";
        else
            break; // Done

        buffRemainder = "";
        int rowS = 0;
        bool bHeaderEnd = false;
        // Read line by line.
        do {
            rowS = str.find_first_not_of('\n', rowS); // Skip series '\n'
            if (rowS == string::npos)
                break;

            int rowE = str.find_first_of('\n', rowS);
            if (rowE == string::npos) {
                // The file must be ended by a \n
                //fseek(fDB, -(len - rowS), SEEK_CUR);
                buffRemainder = str.substr(rowS);
                break;
            }
            rowCnt++;

            if (buff[rowS] == '%' || buff[rowS] == '#') {
                rowS = rowE + 1;
            } else {
                vector<string> columns;
                string strRow = str.substr(rowS, rowE - rowS); // Not include '\n'.
                rowS = rowE + 1;
 
                algorithm::trim(strRow);
                if(strRow.length() == 0)
                    continue;

                bool isInxTreeHeader = bHeaderEnd ? false : strRow.at(0) == '@';
                if (isInxTreeHeader) {
                    if (strRow == TDF_HEADER_START) {
                        parseInxtreeHeaderStart();
                        continue;
                    } else if (strRow == TDF_HEADER_END) {
                        parseInxtreeHeaderEnd();
                        bHeaderEnd = true;
                        continue;
                    }
                }

                // Read column by column
                int colS = 0;
                int colCnt = 0;
                do {
                    colS = strRow.find_first_not_of(SEPARATOR_CHARS, colS);
                    if (colS != string::npos) {
                        colCnt++;

                        int colE = strRow.find_first_of(SEPARATOR_CHARS, colS);
                        if (colE != string::npos) {
                            string strColumn = strRow.substr(colS, colE-colS);
                            columns.push_back(strColumn);
                            if (!isInxTreeHeader)
                                parseColumn(rowCnt, colCnt, strColumn);

                            colS = colE + 1;
                        } else {
                            string strColumn = strRow.substr(colS);
                            columns.push_back(strColumn);
                            if (!isInxTreeHeader)
                                parseColumn(rowCnt, colCnt, strColumn);

                            break;
                        }
                    } else {
                        break;
                    }
                }while(1); // Column

                if (!isInxTreeHeader) {
                    parseRow(rowCnt, strRow, columns);
                } else {
                    parseInxtreeHeader(columns);
                }
            }
        }while(1); // Row
    }while(1);  // File

    return true;
}
