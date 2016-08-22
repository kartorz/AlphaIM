#include <boost/algorithm/string.hpp>
#include "aim.h"
#include "PY.h"
#include "CharUtil.h"
#include "indextree/IndexTree.h"
#include "Util.h"

#undef PRINTF
//#define PRINTF(fmt, args...)  printf(fmt, ##args)
#define PRINTF(fmt, args...)

#define CACHE_ITEMS_MAX 10

#define SEP_CHAR '-'

void HanTDFParser::parseRow(int row, string& str, vector<string>& columns)
{
    if (columns.size() == 3) {
        HC hc;
        string key      = columns[0];
        hc.py       = columns[1];
        hc.priority = Util::stringToInt(columns[2]);
        m_owner->m_hcMap[key] = hc;
    }
}

PY::PY()
{
}

PY::PY(const string& pydb, const string& phdb, const string& pytbl)
{
    m_pyDB.load(pydb, 0xB4B3);
    m_phDB.load(phdb, 0xB4B3);

    HanTDFParser han_tdf(this);
    han_tdf.parser(pytbl);

/*    IndexList index_list;
    int size = m_phDB.getIndexList(index_list, "wm", 0, -1);
    for (int i = 0; i < size; i++) {
        inxtree_dataitem& d = index_list[i]->d;
        printf("%s\n", (char *)d.ptr_data);
    }*/
    //printf("size == %d\n", size);
    //vector<inxtree_dataitem> items;
    //m_phDB.lookup("wm", items);
    /*for (int i = 0; i < items.size(); i++) {
        inxtree_dataitem& d = items[i];
        printf("%s\n", (char *)d.ptr_data);
    }*/
}

PY::~PY()
{
}

string PY::lookup(const string& input, deque<IMItem>& items)
{
    string ret = "";
    string key = input;
    int validlen;
    string validkey;
    string rest;

    PRINTF("lookup1 %s\n", key.c_str());
    if (!key.empty()) {
        //@ Start from a valid PY.
        validlen = m_pyDB.validLen(key);
        // A whole invlid pinyin, delete the first char, search again.
        if (validlen == 0) {
            rest = key.substr(1, validlen - 1);
            ret += key[0]; // Append the invalid char to candidate string.
            ret += lookup(rest, items);
            return ret;
        }

        //@ search phrase and filter
        validkey = key.substr(0, validlen);
        rest = key.substr(validlen, key.length() - validlen);
PRINTF("lookup1 validkey: %s, rest: %s\n", validkey.c_str(), rest.c_str());
        validlen = m_pyDB.validLen(rest); // The second PY.
        if (validlen >  0) {
            int validlen2;
            if ((validlen2 = lookupCache(input, items)) == 0) {
                IndexList indexList;
                m_phDB.getIndexList(indexList, validkey, 0, -1);
                validlen = lookup(input, indexList, items);printf("phd validlen:%d \n", validlen);
                if (validlen > 0) {
                    cache(input, validlen, items);
                    rest = input.substr(validlen, input.length() - validlen);
                }
            } else {
                validlen = validlen2;
                rest = input.substr(validlen, input.length() - validlen);
            }
            PRINTF("lookup phdb valid:%d rest: %s\n", validlen, rest.c_str());
        }

        if (!lookupCache(validkey, items)) {
            getPYItems(validkey, items);
            cache(validkey, validkey.length(), items);
        }

        if (items.size() > 0)
            ret += items[0].val;  // Append to candidate string.

        // Contain partly invalid pinyin string, search slice by slice.
        deque<IMItem> cacheItems;
        ret += lookup(rest, cacheItems);
        return ret;
    }
    return "";
}

// Lookup from phrase db.
int PY::lookup(string key, IndexList& indexList, deque<IMItem>& items)
{
    int validlen = 0;

    if (key != "" && indexList.size() > 0) {
        bool found = false;
        int validlen = m_pyDB.validLen(key);
        //printf("lookup phrase key: %s, validlen: %d\n", key.c_str(), validlen);
        if (validlen > 0) {
            string validkey = key.substr(0, validlen);

            vector<iIndexItem*>::iterator iter;
            for(iter = indexList.begin(); iter != indexList.end();) {
                string inx = (*iter)->index;
                if (inx == "") {
                    ++iter;
                    continue;
                }
                int e = inx.find_first_of(SEP_CHAR, 0);
                if (e != string::npos)
                    inx = inx.substr(0, e);

                if (inx.find(validkey, 0) != 0) {
                    iter = indexList.erase(iter);
                } else {
                    found = true;
                    (*iter)->data_s += validkey;
                    if (e != string::npos && e < (*iter)->index.length() - 1)
                        (*iter)->index = (*iter)->index.substr(e + 1, inx.length() - e - 1);
                    else {
                        (*iter)->index = "";  // Match
                        inxtree_dataitem& d = (*iter)->d;
                        string val((char *)d.ptr_data);
                        IMItem item = {(*iter)->data_s, val};
                        items.push_front(item);
                    }
                    ++iter;
                }
            }

            if (found) {
                key = key.substr(validlen, key.length() - validlen);
                //printf("lookup phrase, next %s\n", key.c_str());
                return validlen + lookup(key, indexList, items);
            }
        }
    }
    return 0;
}

int PY::lookupCache(const string& input, deque<IMItem>& items)
{
    deque<CacheItem>::iterator iter;
    for (iter = m_cache.begin(); iter != m_cache.end(); iter++) {
        if (iter->input == input) {
            PRINTF("joni debug lookup from cache %s, %d\n", input.c_str(), iter->validlen);
            for (int i = 0; i < iter->items.size(); i++)
                items.push_back(iter->items[i]);
            return iter->validlen;
        }
    }
    return 0;
}

void PY::cache(const string& input, int validlen, deque<IMItem>& items)
{
    if (m_cache.size() > CACHE_ITEMS_MAX) {
        m_cache.pop_front();
    }

    CacheItem i;
    i.input = input;
    i.validlen = validlen;
    i.items = items;
    m_cache.push_back(i);
}

void PY::getPYItems(const string& key, deque<IMItem>& items)
{
    IndexList indexList;

    int size = m_pyDB.getIndexList(indexList, key, 0, -1);
    for (int i = 0; i < size; i++) {
        inxtree_dataitem& d = indexList[i]->d;

        const char* hanstr = (char *)d.ptr_data;
        int haninx = 0;
        char* han = CharUtil::nextu8char(hanstr + haninx, &haninx);
        while (han != NULL) {
            IMItem item = {key, han};
            items.push_back(item);

            free(han);
            han = CharUtil::nextu8char(hanstr + haninx, &haninx);
        }
    }
}
#if 0
void AddToUserDBTask::doWork()
{
    const char* hanstr = (char *)m_phrase.c_str();
    int haninx = 0;
    char* han = CharUtil::nextu8char(hanstr + haninx, &haninx);
    string key = m_owner->m_hcMap[han];
    key += SEP_CHAR;

    while (han != NULL) {
        string key = m_owner->m_hcMap[han];
        key += SEP_CHAR;
    
        free(han);
        han = CharUtil::nextu8char(hanstr + haninx, &haninx);
    }
}
#endif
