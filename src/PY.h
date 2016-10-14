#ifndef _PY_H_
#define _PY_H_

#include <deque>
#include <string>

#include "MutexLock.h"
#include "iIM.h"
#include "indextree/IndexTree.h"
#include "indextree/IndexTreeWriter.h"
#include "TDFParser.h"
#include "TaskManager.h"
#include "UsrPhrase.h"

using namespace std;

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
    PY(const string& pydb,
       const string& phdb,
       const string& usrPhdb,
       const string& handb);

    virtual ~PY();
    virtual string lookup(const string& input, deque<IMItem>& items);
    virtual void selectUsrPhrase(const IMItem& imitem);

    virtual void addUserPhrase(const string& phrase);
    virtual void addUserPhraseAsync(const string& phrase);
    virtual void close();
    virtual void reset();

private:
    string lookup(const string& input, deque<IMItem>& items,  bool firstRound);
    void lookupPhrase(string key, string input, deque<IMItem>& items);
    void lookupPhrase(string key, iIndexItem* item,  deque<IMItem> itemList[]);

    bool lookupCache(map<string, deque<IMItem> >& cache, const string& key, deque<IMItem>& items);
    void cache(map<string, deque<IMItem> >& cache, const string& key, deque<IMItem>& items);

    void getPYItems(const string& key, deque<IMItem>& items);
    void getHanItems(const string& py, inxtree_dataitem& d, deque<IMItem>& items);
    void sort(deque<IMItem>& items);
    void sort(IndexList& items);
    void addToPhDB(const string& phrase, u8 priority);
    void trackUsrInput(const string phrase);
    void parseUsrInput(vector<string> &phrases);
    void addToUsrDB(const string& phrase);
    void getPhraseKey(const string& phrase, vector<string>& phkeys);

    MutexCriticalSection m_phdbCS;

    IndexTree m_pyDB;
    IndexTree m_phDB;
    IndexTreeWriter  m_usrPhDB;
    IndexTreeWriter  m_hanDB;
    int m_addCnt;
    int m_selCnt;

    map<string, deque<IMItem> > m_InputMap;
    map<string, deque<IMItem> > m_PyMap;

    IndexTree* m_phDBs[2];
    int  m_phDBsLen;
    string m_pendingPhrase;
    string m_usrPhraseTmp;
    FILE  *m_usrPhraseFile;

    UsrPhrase m_usrPhrase;
};

class AddToUserDBTask: public Task
{
public:
    AddToUserDBTask(PY* owner, string phrase);
    virtual ~AddToUserDBTask() {}
    virtual void doWork();

private:
    PY *m_owner;
    string m_phrase;
};

#endif
