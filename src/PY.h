#ifndef _PY_H_
#define _PY_H_

#include <deque>
#include <string>
#include <vector>

#include "MutexLock.h"
#include "iIM.h"
#include "indextree/IndexTree.h"
#include "indextree/IndexTreeWriter.h"
#include "TDFParser.h"
#include "TaskManager.h"

using namespace std;

#define CACHE_ITEMS_MAX 100

#define SEP_CHAR '-'
#define UPDATE_USERDB_TH 50
#define DEFAULT_PRIORITY  20
#define REFRESH_PRIORITY_TH  4000
#define MAX_USRDB_ENTRY     5000
#define MAX_PRIORITY    255

#define MAX_PHRASE_LEN  10

typedef struct
{
    string py;
    int    priority;
}HC;

typedef struct
{
    deque<IMItem> items;
    string input;
}CacheItem;

class PY: public iIM
{
friend class AddToUserDBTask;

public:
    PY();

    virtual ~PY();
    virtual string lookup(const string& input, deque<IMItem>& items);
    virtual void onCommit(const IMItem& imitem);

    virtual void addUserPhrase(const string& phrase);
    virtual void addUserPhraseAsync(const vector<string>& phrase);
    virtual void close();
    virtual void reset();

    int initialization();
    void syncUserPhrases();

private:
    string lookup(const string& input, deque<IMItem>& items,  bool firstRound);
    void lookupPhrase(string key, string input, deque<IMItem>& items, bool firstRound);
    void lookupPhrase(string key, iIndexItem* item,  deque<IMItem> itemList[2], bool firstRound);

    bool lookupCache(map<string, deque<IMItem> >& cache, const string& key, deque<IMItem>& items);
    void cache(map<string, deque<IMItem> >& cache, const string& key, deque<IMItem>& items);

    void getPYItems(const string& key, deque<IMItem>& items);
    void getHanItems(const string& py, inxtree_dataitem& d, deque<IMItem>& items);
    void sort(deque<IMItem>& items);
    void sort(IndexList& items);
    void sortByOff(deque<IMItem>& items);
    void addToPhDB(const string& phrase, u8 priority);
    void addToUsrDB(const string& phrase);

    // 0: no pinyin, 1: only one han, 2: got it
    int getPhraseKey(const string& phrase, vector<string>& phkeys);

    void refreshHanPriority(IndexTreeWriter& hanDB);
    void refreshPhrasePriority(IndexTreeWriter& phDB);


    MutexCriticalSection m_phraseCS;

    IndexTree m_pyDB;
    IndexTreeWriter  m_phDB;
    IndexTreeWriter  m_usrPhDB;
    IndexTreeWriter  m_hanDB;
    int m_addCnt;
    int m_selCnt;

    map<string, deque<IMItem> > m_InputMap;
    map<string, deque<IMItem> > m_PyMap;

    IndexTree* m_phDBs[2];
    int  m_phDBsLen;
};

class AddToUserDBTask: public Task
{
public:
    AddToUserDBTask(PY* owner, const vector<string>& phrases);
    virtual ~AddToUserDBTask() {}
    virtual void doWork();

private:
    PY *m_owner;
    vector<string> m_phrases;
};

#endif
