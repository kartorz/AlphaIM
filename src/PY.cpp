#include <boost/algorithm/string.hpp>
#include "aim.h"
#include "PY.h"
#include "CharUtil.h"
#include "indextree/IndexTree.h"
#include "Util.h"
#include "Log.h"

#undef PRINTF
#define PRINTF(fmt, args...)  printf(fmt, ##args)
//#define PRINTF(fmt, args...)

#define CACHE_ITEMS_MAX 10

#define SEP_CHAR '-'
#define WRITE_USERDB_TH  50

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

PY::PY(): m_addCnt(0)
{
}

PY::PY(const string& pydb,
       const string& phdb,
       const string& usrPhdb,
       const string& pytbl): m_addCnt(0)
{
    m_pyDB.load(pydb, 0xB4B3);
    m_phDB.load(phdb, 0xB4B3);
    if (!m_usrPhDB.load(usrPhdb, 0xB4B3, true)) {
        log.d("Load usr phd, failure\n");
        memset(&m_usrPhDB.m_header, 0, sizeof(struct inxtree_header));
        m_usrPhDB.m_header.magic[0] = 0xB3;
        m_usrPhDB.m_header.magic[1] = 0xB4;
        m_usrPhDB.m_header.d_coding[0] = INXTREE_UTF8;
    }

    HanTDFParser han_tdf(this);
    han_tdf.parser(pytbl);
    m_phDBs[0] = &m_phDB;
    m_phDBs[1] = &m_usrPhDB;
    m_phDBsLen = 2;
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
    //printf("joni debug ~PY\n");
    //log(LOG_INFO, "~PY write userphd\n");
    close();

    if (m_addCnt > 0) {
        m_addCnt = 0;
        m_usrPhDB.write();
        log.d("~PY: write user phrase db\n");
    }
}

void PY::close()
{
    if (m_addCnt > 0) {
        m_addCnt = 0;
        m_usrPhDB.write();
    }
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
            //if ((validlen2 = lookupCache(input, items)) == 0) {
                IndexList indexList;
                for (int i = 0; i < m_phDBsLen; i++)
                    (*m_phDBs[i]).getIndexList(indexList, validkey, 0, -1);

                int size1 = items.size();
                validlen = lookup(input, indexList, items);
                if (validlen > 0 && (items.size() - size1) > 0 /* larger 'index', not match*/) {
                    //cache(input, validlen, items);
                    rest = input.substr(validlen, input.length() - validlen);printf("lookup from phd done\n");
                }
            /*} else {
                validlen = validlen2;
                rest = input.substr(validlen, input.length() - validlen);
            }*/
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
                //printf("inx: %s, validkey: %s\n", inx.c_str(), validkey.c_str());
                if (inx.find(validkey, 0) != 0) {
                    iter = indexList.erase(iter);
                } else {
                    found = true;
                    (*iter)->data_s += validkey;
                    if (e != string::npos && e < (*iter)->index.length() - 1) {
                        // 'index' is larger then 'key', so cut 'index' and search continuely.
                        (*iter)->index = (*iter)->index.substr(e + 1, inx.length() - e - 1);
                        ++iter;
                    } else {
                        (*iter)->index = "";  // Match
                        inxtree_dataitem& d = (*iter)->d;
                        if (d.len_data == 0 || d.ptr_data == NULL) {
                            iter = indexList.erase(iter);
                        } else {
                            string val((char *)d.ptr_data);
                            IMItem item = {(*iter)->data_s, val};
                            items.push_front(item);
                            ++iter;
                            //printf("got a index %s --> %s\n", item.key.c_str(), item.val.c_str());
                        }
                    }
                }
            }

            if (found) {
                key = key.substr(validlen, key.length() - validlen);
                //printf("lookup phrase,validlen:%d next %s\n", validlen, key.c_str());
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

void PY::addUserPhrase(const string& phrase)
{
    string py;
    const char* hanstr = phrase.c_str();
    int haninx = 0;
    char* han = CharUtil::nextu8char(hanstr + haninx, &haninx);
    int count = 0;
    while (han != NULL) {
         map<string, HC>::iterator iter = m_hcMap.find(han);
         if(iter == m_hcMap.end()) {
            log(LOG_INFO, "addUserPhrase: no py with han(%s), return.\n", han); 
            free(han);
            return;
        }
        free(han);
        py += iter->second.py + SEP_CHAR;
        han = CharUtil::nextu8char(hanstr + haninx, &haninx);
        ++count; 
    }

    if (py == "" || count <= 1) {
        log(LOG_ERROR, "addUserPhrase: py is null, got a invalid phrase(%s).\n",phrase.c_str()); 
        return;
    }

    py.erase(py.length()-1);

    PRINTF("addUserPhrase: (%s)-->(%s)\n", py.c_str(), phrase.c_str());

    if (!phraseExists(py, phrase)) {   
        u32* keyPtr = NULL;
        int keyLen = CharUtil::utf8StrToUcs4Str(py.c_str(), &keyPtr);printf("add to indextree\n");
        if (keyLen > 0) {
            m_usrPhDB.add(keyPtr, keyLen, (void *)phrase.c_str(), phrase.length() + 1/*'\0'*/);
            free(keyPtr);
            if (++m_addCnt > WRITE_USERDB_TH) {
                m_addCnt = 0;
                m_usrPhDB.write();
            }
        }
    }
}

void PY::addUserPhraseAsync(const string& phrase)
{
    AddToUserDBTask* tsk = new AddToUserDBTask(phrase);
    TaskManager::getInstance()->addTask(tsk);
}

bool PY::phraseExists(const string& py, const string& phrase) 
{
    vector<inxtree_dataitem> items;
    for (int i = 0; i < m_phDBsLen; i++)
        (*m_phDBs[i]).lookup(py,items);
    
    vector<inxtree_dataitem>::iterator iter;
    for(iter = items.begin(); iter != items.end(); ++iter) {
        inxtree_dataitem& d = (*iter);
        if (d.len_data > 0 && d.ptr_data != NULL) {
            string val((char *)d.ptr_data);
            if (val.compare(phrase) == 0)
                return true;
        }
    }
    return false;
}

AddToUserDBTask::AddToUserDBTask(string phrase):
Task(0, false), m_phrase(phrase)
{
}

void AddToUserDBTask::doWork()
{
    m_owner->addUserPhrase(m_phrase);
}
