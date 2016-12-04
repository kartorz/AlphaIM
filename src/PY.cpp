#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

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

using namespace boost::filesystem;

PY::PY(): m_addCnt(0), m_selCnt(0),
m_usrPhDB(1, INXTREE_NOT_HAS_DUPINX)
{
}

int PY::initialization()
{
    int ret = 0;

    string phPath = home_dir + "/phrase-utf8.imdb";
    if (!Util::isFileExist(phPath)) {
        log.d("copy phrase db to home\n");
        string phPathOri = system_dir + "/phrase-utf8.imdb";
        boost::filesystem::copy_file(phPathOri, phPath, copy_option::overwrite_if_exists);
    }
    ret += m_phDB.load(phPath, 0xB4B3);

    string hanPath = home_dir + "/han-utf8.imdb";
    if (!Util::isFileExist(hanPath)) {
        log.d("copy han db to home\n");
        string hanPathOri = system_dir + "/han-utf8.imdb";
        boost::filesystem::copy_file(hanPathOri, hanPath, copy_option::overwrite_if_exists);
        //permissions(file_path, add_perms|owner_write|group_write|others_write);
    }
    ret += m_hanDB.load(hanPath, 0xB4B3);

    string pyPath = system_dir + "/pinyin-utf8.imdb";
    ret += m_pyDB.load(pyPath, 0xB4B3);

    string usrPhPath = home_dir + "/user_phrase-utf8.imdb";
    string usrPhPathOk = usrPhPath + "_ok";
    if (!m_usrPhDB.load(usrPhPath, 0xB4B3, true)) {
        bool load =  false;

        log.d("Load usr phd, failure. check backup file. \n");
        if (Util::isFileExist(usrPhPathOk)) {
            boost::filesystem::copy_file(usrPhPathOk, usrPhPath, copy_option::overwrite_if_exists);
            load = m_usrPhDB.load(usrPhPath, 0xB4B3, true);
            log.d("Load backup user phrase : %d \n", load);
        }

        if (!load) {
            boost::filesystem::remove(usrPhPathOk);
            boost::filesystem::remove(usrPhPath);

            memset(&m_usrPhDB.m_header, 0, sizeof(struct inxtree_header));
            m_usrPhDB.m_header.magic[0] = 0xB3;
            m_usrPhDB.m_header.magic[1] = 0xB4;
            m_usrPhDB.m_header.d_coding[0] = INXTREE_UTF8;
            inxtree_write_u16(m_usrPhDB.m_header.i_size, 1);
        }
    } else {
        copy_file(usrPhPath, usrPhPathOk, copy_option::overwrite_if_exists);
    }

    m_phDBs[0] = &m_phDB;
    m_phDBs[1] = &m_usrPhDB;
    m_phDBsLen = 2;

    m_selCnt = Configure::getRefrence().readSelcnt();

    return ret;
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

    deque<IMItem> imitemList[2][MAX_PHRASE_LEN + 1];
    vector<iIndexItem*>::iterator iter;
    for(iter = indexList.begin(); iter != indexList.end(); iter++) {
        lookupPhrase(input, *iter, imitemList);
    }

    // Perfect match.
    for (int i = 2; i <= MAX_PHRASE_LEN; i++) {
        if (imitemList[0][i].size() > 0) {
            sort(imitemList[0][i]);
            for (int ii = 0; ii < imitemList[0][i].size(); ii++) {
                items.push_back(imitemList[0][i].at(ii));
            }
        }
    }

    // Start from two hans
    for (int i = MAX_PHRASE_LEN; i > 1; i--) {
        if (imitemList[1][i].size() > 0) {
            sort(imitemList[1][i]);
            for (int ii = 0; ii < imitemList[1][i].size(); ii++) {
                //printf("push_back %s, i:%d, size:%d\n",imitemList[j][i].at(ii).val.c_str(), i, imitemList[j][i].size());
                items.push_back(imitemList[1][i].at(ii));
            }
        }
    }

    IndexTree::freeItems(indexList);
}

void PY::lookupPhrase(string key, iIndexItem* item,  deque<IMItem> imitemList[2][MAX_PHRASE_LEN + 1])
{
    char han[10];
    int npos = 0;
    int len = 0;
    int number = 0;

    /*  0: not found
     *  1: partly match
     *  2: whole match
     */
    int found;
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
        found = 0;
        // Get a han from phrase.
        int len = CharUtil::nextu8char(phstr + npos, han);
        if (len >= MIN_HAN_U8BYTES) {
            //printf("lookupPhrase %s --> %s,\n", imval.c_str(), phstr);
            vector<inxtree_dataitem> pyitems;
            m_hanDB.lookup(han, pyitems);
            for (int i = 0; i < pyitems.size(); i++) {
                HanItem hani(pyitems[i].ptr);
                string py = hani.py;
                int len2 = Util::stringCommonLen(py, key);
                // Partly match.
                if (len2 > 0) {
                    found =  len2 == py.length() ? 2 : 1;
                    //printf("%s: py:%s, key:%s, len == %d, found == %d\n", phstr, py.c_str(), key.c_str(), len2, found);

                    imkey += key.substr(0, len2);
                    key = key.substr(len2);

                    // Save this han for result.
                    //    phrase: 有线广播; key: youxian; result: 有线
                    imval += item->index.substr(npos, len);
                    npos += len;
                    ++number;

                    break;
                }
            }
            IndexTree::freeItems(pyitems);
        } else {
            break;
        }
    } while(found && key != "" && npos < phstrlen);

    if (found > 0)
    {
        if (number > MAX_PHRASE_LEN)
            number = MAX_PHRASE_LEN;

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
        //printf("found %s, priorid %d, match val:%s remain key: %s, imkey(%s) \n",
        //       phstr, imitem.priority, imval.c_str(), key.c_str(), imkey.c_str());

        // input: guojia
        // guo'jia   guo'ji
        if (key == "" && found == 2) {
            //printf("perfect match %s, number:%d\n", phstr, number);
            imitemList[0][number].push_back(imitem);
        } else {
            imitemList[1][number].push_back(imitem);
        }
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
    #if 0
        if(cache.size() > CACHE_ITEMS_MAX) {
            for (int i = 0; i < CACHE_ITEMS_MAX/2; i++)
                cache.erase(cache.begin());
        }
    #endif
        cache[key] = items;
    }
}

int PY::getPhraseKey(const string& phrase, vector<string>& phkeys)
{
    char han[8];
    int len = CharUtil::nextu8char(phrase.c_str(), han);
    if (len >= MIN_HAN_U8BYTES) {
        vector<inxtree_dataitem> pyitems;
        m_hanDB.lookup(han, pyitems);
        if (pyitems.size() == 0) {
            log(LOG_INFO, "getPhraseKey: no py with han(%s), return.\n", han);
            return 0;
        }

        if (len == phrase.length()) {
            return 1;
        }

        for (int i = 0; i < pyitems.size(); i++) {
            HanItem hani(pyitems[i].ptr);
            string key = hani.py + SEP_CHAR + phrase; 
            phkeys.push_back(key);
        }
        return 1;
    }
    return 0;
}

// Reresh prprity by decreasing -1.
void PY::refreshHanPriority(IndexTreeWriter& hanDB)
{
    u8 *buf = NULL;
    int size = hanDB.readData(&buf);
    if (size > 0) {
        int npos = 0;
        while (npos < size){
            u8 *itemptr = buf + npos;
            if (itemptr[2] > 1)
                --itemptr[2];

            int len = inxtree_read_u16(itemptr) + 2;
            npos += len;
        }

        hanDB.writeData(buf, size);
    }

    if (buf != NULL)
        free(buf);

    log(LOG_INFO, "refreshHanPriority\n");
}

// Reresh prprity by decreasing -1.
void PY::refreshPhrasePriority(IndexTreeWriter& phDB)
{
    u8 *buf = NULL;
    int size = phDB.readData(&buf);
    if (size > 0) {
        for (int i = 0; i < size; i++) {
            if (buf[i] > 1)
                --buf[i];
        }

        phDB.writeData(buf, size);
    }
    if (buf != NULL)
        free(buf);

    log(LOG_INFO, "refreshPhrasePriority\n");
}

void PY::usrCommit(const IMItem& imitem)
{
    if (imitem.priority < MAX_PRIORITY) {
        vector<string> phkeys;
        int ret = getPhraseKey(imitem.val, phkeys);
        if (ret > 0) {
            bool update = false;
            u8 p[1];
            p[0] = imitem.priority + 1;

            if (ret == 1) {
                if (m_hanDB.update(imitem.val, p, 1, 0))
                    update = true;
            } else {
                for(int i = 0; i < phkeys.size(); i++) {
                    //printf("im priority %d\n", imitem.priority);

                    if (m_usrPhDB.update(phkeys[i], p, 1))
                        update = true;
                    else if (m_phDB.update(phkeys[i], p, 1))
                        update = true;
                }
            }

            if (update) {

                // Have updated priority
                m_InputMap.clear();
                m_PyMap.clear();

                if (++m_selCnt > REFRESH_PRIORITY_TH) {
                    refreshPhrasePriority(m_usrPhDB);
                    refreshPhrasePriority(m_phDB);
                    refreshHanPriority(m_hanDB);
                    m_selCnt = 0;
                }
                Configure::getRefrence().writeSelcnt(m_selCnt);
            }
        } // ret > 0
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
