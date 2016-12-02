#include <boost/algorithm/string.hpp>
#include "aim.h"
#include "PY.h"
#include "CharUtil.h"
#include "indextree/IndexTree.h"
#include "Util.h"
#include "Log.h"
#include "indextree_item.h"
#include "Configure.h"

#undef PRINTF
//#define PRINTF(fmt, args...)  printf(fmt, ##args)
#define PRINTF(fmt, args...)

#define CACHE_ITEMS_MAX 100

#define SEP_CHAR '-'
#define WRITE_USERDB_TH  50
#define UPDATE_USERDB_TH 50
#define DEFAULT_PRIORITY  20
#define REFRESH_PRIORITY_TH  4000
#define MAX_USRDB_ENTRY     5000
#define MAX_PRIORITY    255
#define MAX_PHRASE_LEN  10

PY::PY(): m_addCnt(0), m_selCnt(0)
{
}

PY::PY(const string& pydb,
       const string& phdb,
       const string& usrPhdb,
       const string& handb): 
       m_addCnt(0), m_selCnt(0),
       m_usrPhDB(1, INXTREE_NOT_HAS_DUPINX)
{
    m_pyDB.load(pydb, 0xB4B3);
    m_phDB.load(phdb, 0xB4B3);
    m_hanDB.load(handb, 0xB4B3, true);

    if (!m_usrPhDB.load(usrPhdb, 0xB4B3, true)) {
        log.d("Load usr phd, failure\n");
        memset(&m_usrPhDB.m_header, 0, sizeof(struct inxtree_header));
        m_usrPhDB.m_header.magic[0] = 0xB3;
        m_usrPhDB.m_header.magic[1] = 0xB4;
        m_usrPhDB.m_header.d_coding[0] = INXTREE_UTF8;
        inxtree_write_u16(m_usrPhDB.m_header.i_size, 1);
    }

    m_phDBs[0] = &m_phDB;
    m_phDBs[1] = &m_usrPhDB;
    m_phDBsLen = 2;

    m_selCnt = Configure::getRefrence().readSelcnt();
}

PY::~PY()
{
    log(LOG_INFO, "~PY write userphd\n");
    if (m_addCnt > 0) {
        m_addCnt = 0;
        m_usrPhDB.write();
    }
}

void PY::close()
{
}

void PY::reset()
{
    if (m_addCnt > 0) {
        m_addCnt = 0;
        m_usrPhDB.write();
    }
}

string PY::lookup(const string& input, deque<IMItem>& items)
{
    return lookup(input, items, true);
}

string PY::lookup(const string& input, deque<IMItem>& items, bool firstRound)
{
    string key = input;
    string ret = "";
    string validkey;
    PRINTF("lookup1 %s\n", key.c_str());
    if (!key.empty()) {
        //@ Start from a valid PY.
        int validlen = m_pyDB.validLen(key);
 
       // A whole invlid pinyin, delete the first char, search again.
        if (validlen == 0) {
            string rest = key.substr(1);
            ret += key[0]; // Append the invalid char to candidate string.
            ret += lookup(rest, items, true);
            return ret;
        }

        validkey = input.substr(0, validlen);

        //@ Search phrase and filter
        if (validkey.length() < input.length()) {
            lookupPhrase(validkey, input, items);
        }

       //PRINTF("lookup1 validkey: %s, rest: %s\n", validkey.c_str(), rest.c_str());  
        //@ Check if appending PinYin items
        if (firstRound || (items.size() == 0)) {
            if (!lookupCache(m_InputMap, validkey, items)) {
                deque<IMItem> pyitems;
                getPYItems(validkey, pyitems);
                cache(m_InputMap, validkey, pyitems);
                int end = firstRound ? pyitems.size() : 1;
                for (int i = 0; i < end; i++)
                    items.push_back(pyitems[i]);
            }
        }

        if (items.size() > 0) {
            string rest = key.substr(items[0].key.length());
            ret  = items[0].val;  // Append to candidate string.

            // Contain partly invalid pinyin string, search slice by slice.
            deque<IMItem> cacheItems;
            ret += lookup(rest, cacheItems, false);
            return ret;
        }
    }
    return "";
}

void PY::lookupPhrase(string key, string input, deque<IMItem>& items)
{
    // Gets All phrases beging with 'key[0 .. -1]'.
    // Fix the 'dier' , 'wangu' issue.
    //    - dier: di'er  die'r
    //    - wangu: wang'u  wan'gu
    if (key.length() > 2)
        key = key.substr(0, key.length() - 1);
    //printf("lookupPhrase %s\n", key.c_str());
    IndexList indexList;
    for (int i = 0; i < m_phDBsLen; i++) {
        m_phDBs[i]->getIndexList(indexList, key, true);
    }

    deque<IMItem> imitemList[MAX_PHRASE_LEN + 1];
    vector<iIndexItem*>::iterator iter;
    for(iter = indexList.begin(); iter != indexList.end(); iter++) {
        lookupPhrase(input, *iter, imitemList);
    }

    // Start from two hans.
    for (int i = MAX_PHRASE_LEN; i > 1; i--) {
        if (imitemList[i].size() > 0) {
            sort(imitemList[i]);
            for (int ii = 0; ii < imitemList[i].size(); ii++) {
                items.push_back(imitemList[i].at(ii));
            }
        }
    }

    IndexTree::freeItems(indexList);
}

void PY::lookupPhrase(string key, iIndexItem* item,  deque<IMItem> imitemList[])
{
    char han[10];
    int npos = 0;
    int len = 0;
    int number = 0;
    bool found;
    string imkey = "";
    string imval = "";

    string prefix;
    string phs = item->index;
    int e = phs.find_first_of(SEP_CHAR, 0);
    if (e != string::npos) {
        item->index = phs.substr(e + 1);
        prefix = phs.substr(0, e + 1);
    }

    char *phstr = (char *)item->index.c_str();
    int phstrlen = item->index.length();

    do {
        found = false;
        int len = CharUtil::nextu8char(phstr + npos, han);
        if (len >= MIN_HAN_U8BYTES) {
            imval += item->index.substr(npos, len);
            npos += len;
            ++number;
            //printf("lookupPhrase %s --> %s,\n", imval.c_str(), phstr);

            vector<inxtree_dataitem> pyitems;
            m_hanDB.lookup(han, pyitems);
            for (int i = 0; i < pyitems.size(); i++) {
                HanItem hani(pyitems[i].ptr);
                string py = hani.py;
                int len = Util::stringCommonLen(py, key);
                if (len > 0) {
                    found = true;
                    imkey += key.substr(0, len);
                    key = key.substr(len);
                    break;
                }
            }
            IndexTree::freeItems(pyitems);
        } else {
            break;
        }
    } while(found && key != "" && npos < phstrlen);

    if (found == true)
    {
        if (npos <  phstrlen) {       
            //printf("check if exist %s, npos:%d, phstrlen:%d\n", (prefix+imval).c_str(), npos, phstrlen);
            for (int i = 0; i < m_phDBsLen; i++) {                
                if (m_phDBs[i]->isExist(prefix+imval)) /* Don't append the duplicate item.*/ 
                    return;
            }
        }

        IMItem imitem;
        imitem.key = imkey;
        imitem.val = imval;
        imitem.priority = item->d.buf[0];
        //printf("found %s, priorid %d\n", phstr, imitem.priority);

        // input: guojia
        // guo'jia   guo'ji
        if (key == "") 
            imitemList[MAX_PHRASE_LEN].push_back(imitem);  // perfect match
        else
            imitemList[number].push_back(imitem);
    }
}

bool PY::lookupCache(map<string, deque<IMItem> >& cache, const string& key, deque<IMItem>& items)
{
    map<string, deque<IMItem> >::iterator iter = cache.find(key);
    if(iter != cache.end()) {
        PRINTF("lookup from cache %s\n", key.c_str());
        deque<IMItem> cacheItems = iter->second;
        for (int i = 0; i < cacheItems.size(); i++) {
            items.push_back(cacheItems[i]);
        }
        return true;
    }
    return false;
}

void PY::cache(map<string, deque<IMItem> >& cache, const string& key, deque<IMItem>& items)
{
    map<string, deque<IMItem> >::iterator iter = cache.find(key);
    if(iter == cache.end()) {
        if(cache.size() > CACHE_ITEMS_MAX) {
            for (int i = 0; i < CACHE_ITEMS_MAX/2; i++)
                cache.erase(cache.begin());
        }
        cache[key] = items;
    }
}

void PY::getPhraseKey(const string& phrase, vector<string>& phkeys)
{
    char han[8];
    int len = CharUtil::nextu8char(phrase.c_str(), han);
    if (len >= MIN_HAN_U8BYTES) {
        vector<inxtree_dataitem> pyitems;
        m_hanDB.lookup(han, pyitems);
        if (pyitems.size() == 0) {
            log(LOG_INFO, "getPhraseKey: no py with han(%s), return.\n", han); 
            return;
        }

        if (len == phrase.length()) {
            return;
        }

        for (int i = 0; i < pyitems.size(); i++) {
            HanItem hani(pyitems[i].ptr);
            string key = hani.py + SEP_CHAR + phrase; 
            phkeys.push_back(key);
        }
    }
}

void PY::selectUsrPhrase(const IMItem& imitem)
{
    if (imitem.priority < MAX_PRIORITY) {
        vector<string> phkeys;
        getPhraseKey(imitem.val, phkeys);
        for(int i = 0; i < phkeys.size(); i++) {
            u8 p[1];
            p[0] = imitem.priority + 1;
            //printf("im priority %d\n", imitem.priority);
            if (m_usrPhDB.update(phkeys[i], p, 1)) {
                if (++m_selCnt > REFRESH_PRIORITY_TH) {
                    u8 *buf = NULL;
                    int size = m_usrPhDB.readData(&buf);
                    if (size > 0) {
                        for (int i = 0; i < size; i++) {
                            if (buf[i] > 5)
                                --buf[i];
                        }

                        m_usrPhDB.writeData(buf, size);

                        m_selCnt = 0;
                    }
                    if (buf != NULL)
                        free(buf);

                    log(LOG_INFO, "selectUsrPhrase: > REFRESH_PRIORITY_TH, decrease priority\n");
                }
                Configure::getRefrence().writeSelcnt(m_selCnt);
            }
        }
    }
}

void PY::getPYItems(const string& py, deque<IMItem>& items)
{
    vector<inxtree_dataitem> pyItems[2];
    IndexList indexList;
    int size = m_pyDB.getIndexList(indexList, py, INXTREE_LOAD);
    for (int i = 0; i < size; i++) {
        inxtree_dataitem& d = indexList[i]->d;
        if (indexList[i]->index.length() == py.length()) {  // perfect match. 
            //printf("perfect match %s\n", indexList[i]->index.c_str());
            pyItems[0].push_back(d);
        } else
            pyItems[1].push_back(d);
    }

    for (int i = 0; i < pyItems[0].size(); i++) {
        inxtree_dataitem& d = pyItems[0][i];
        getHanItems(py, d, items);
    }
    sort(items);
    //m_pyDB.freeItems(pyItems[0]);
    {
    deque<IMItem> imItems;
    for (int i = 0; i < pyItems[1].size(); i++) {
        inxtree_dataitem& d = pyItems[1][i];
        getHanItems(py, d, imItems);
    }
    sort(imItems);
    //m_pyDB.freeItems(pyItems[1]);

    IndexTree::freeItems(indexList); // This will free  inxtree_dataitem->ptr, check if double free.

    for (int j = 0; j < imItems.size(); j++) {
        items.push_back(imItems[j]);
    }
    }

    for (int i = py.length() - 1;  i > 1; i--) {
        string py2 = py.substr(0, i);
        deque<IMItem> py2ImItems;

        if (!lookupCache(m_PyMap, py2, py2ImItems)) {
            vector<inxtree_dataitem> py2Items;
            m_pyDB.lookup(py2, py2Items);
            for (int ii = 0; ii < py2Items.size(); ii++) {
                getHanItems(py2, py2Items[ii], py2ImItems);
            }
            m_pyDB.freeItems(py2Items);
            sort(py2ImItems);
            cache(m_PyMap, py2, py2ImItems);
        }

        for (int j = 0; j < py2ImItems.size(); j++) {
            items.push_back(py2ImItems[j]);
        }
    }
}

void PY::getHanItems(const string& py, inxtree_dataitem& d, deque<IMItem>& items)
{
    char* hanstr = (char *)d.ptr;
    int haninx = 0;
    char han[8];
    int len = CharUtil::nextu8char(hanstr + haninx, han);
    while (len >= MIN_HAN_U8BYTES) {
        IMItem item;
        item.key = py;
        item.val = han;

        HanItem hani(m_hanDB.find(item.val));
        item.priority = hani.priority;

        items.push_back(item);

        haninx += len;
        len = CharUtil::nextu8char(hanstr + haninx, han);
    }
}

void PY::sort(deque<IMItem>& items)
{
    deque<IMItem> B(items.size());

    int C[0x100];
    memset(C, 0, sizeof(C));

    for (int j = 0; j < items.size(); j++) {
        int inx = MAX_PRIORITY - items[j].priority; // reverse
        ++C[inx];
    }

    for (int j = 1; j <= 0xff; j++) {
        C[j] += C[j-1];
    }

    for (int j = items.size() - 1; j >= 0; j--) {
        int p = MAX_PRIORITY - items[j].priority;
        int count = C[p];
        B[count - 1] = items[j];
        --C[p];
    }

    items = B;
}

void PY::sort(IndexList& items)
{
    IndexList B(items.size());

    int C[0x100];
    memset(C, 0, sizeof(C));

    for (int j = 0; j < items.size(); j++) {
        int inx = MAX_PRIORITY - items[j]->d.buf[0]; // reverse
        ++C[inx];
    }

    for (int j = 1; j <= 0xff; j++) {
        C[j] += C[j-1];
    }

    for (int j = items.size() - 1; j >= 0; j--) {
        int p = MAX_PRIORITY - items[j]->d.buf[0];
        int count = C[p];
        B[count - 1] = items[j];
        --C[p];
    }

    items = B;
}

void PY::addUserPhrase(const string& phrase)
{
    vector<string> phraseVec;
    m_usrPhrase.trackUsrInput(phrase, phraseVec);
    for (int i = 0; i < phraseVec.size(); i++)
        addToUsrDB(phraseVec[i]);

    addToUsrDB(phrase);
}

void PY::addToUsrDB(const string& phrase)
{
    char han[8];
    int len = CharUtil::nextu8char(phrase.c_str(), han);
    if (len >= MIN_HAN_U8BYTES) {
        vector<inxtree_dataitem> pyitems;
        m_hanDB.lookup(han, pyitems);
        if (pyitems.size() == 0) {
            log(LOG_INFO, "addToUsrDB: no py with han(%s), return.\n", han);
            return;
        }

        if (len == phrase.length()) {
            //printf("only one phrase\n");
            return;
        }

        if (m_usrPhDB.getTotalEntry() > MAX_USRDB_ENTRY) {
            IndexList indexList;
            m_usrPhDB.getIndexList(indexList, "", true);
            sort(indexList);
            m_usrPhDB.clear();
            for(int i = 0; i < MAX_USRDB_ENTRY / 2 ; i++) {
                u8 priority = indexList[i]->d.buf[0];
                u32* keyPtr = NULL;
                int keyLen = CharUtil::utf8StrToUcs4Str(indexList[i]->index.c_str(), &keyPtr);
                if (keyLen > 0) {
                    m_usrPhDB.add(keyPtr, keyLen, (void *)&priority, 1);
                    free(keyPtr);
                }
            }
            m_usrPhDB.write();
            IndexTree::freeItems(indexList);

            m_addCnt = 0;

            log(LOG_INFO, "addToUsrDB: > MAX_USRDB_ENTRY, clean up user db. \n");
        }

        for (int i = 0; i < pyitems.size(); i++) {
            HanItem hani(pyitems[i].ptr);
            string key = hani.py + SEP_CHAR + phrase; 
            for (int i = 0; i < m_phDBsLen; i++) {                
                if (m_phDBs[i]->isExist(key))
                    return;
            }

            //printf("addUserPhrase %s\n", key.c_str());
            u8 priority = DEFAULT_PRIORITY;
            u32* keyPtr = NULL;
            int keyLen = CharUtil::utf8StrToUcs4Str(key.c_str(), &keyPtr);
            if (keyLen > 0) {
                m_usrPhDB.add(keyPtr, keyLen, (void *)&priority, 1);
                free(keyPtr);
                if (++m_addCnt > WRITE_USERDB_TH) {
                    m_addCnt = 0;
                    m_usrPhDB.write();
                }
            }
        }
    } else {
        log(LOG_ERROR, "addUserPhrase: got a invalid phrase(%s).\n",phrase.c_str()); 
    }
}

void PY::addUserPhraseAsync(const string& phrase)
{
    AddToUserDBTask* tsk = new AddToUserDBTask(this, phrase);
    TaskManager::getInstance()->addTask(tsk);
}

AddToUserDBTask::AddToUserDBTask(PY* owner, string phrase):
Task(0, false), m_owner(owner), m_phrase(phrase)
{
}

void AddToUserDBTask::doWork()
{
    m_owner->addUserPhrase(m_phrase);
}
