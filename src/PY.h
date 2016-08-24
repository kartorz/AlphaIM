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
    int validlen;
}CacheItem;

class PY: public iIM
{
friend class HanTDFParser;
friend class AddToUserDBTask;

public:
    PY();
    PY(const string& pydb,
       const string& phdb,
       const string& usrPhdb,
       const string& pytbl);

    virtual ~PY();

    virtual string lookup(const string& input, deque<IMItem>& items);
    virtual void addUserPhrase(const string& phrase);
    virtual void addUserPhraseAsync(const string& phrase);
    virtual void close();

private:
    int  lookup(string key, IndexList& indexList, deque<IMItem>& items);
    int  lookupCache(const string& key, deque<IMItem>& items);
    void cache(const string& input, int validlen, deque<IMItem>& items);
    void getPYItems(const string& key, deque<IMItem>& items);
    bool phraseExists(const string& py, const string& phrase);

    MutexCriticalSection m_phdbCS;

    IndexTree m_pyDB;
    IndexTree m_phDB;
    IndexTreeWriter  m_usrPhDB;
    int m_addCnt;

    FILE *m_usrTbl;

    map<string, HC>   m_hcMap;
    deque<CacheItem>  m_cache;
    IndexTree* m_phDBs[2];
    int  m_phDBsLen;
};

class HanTDFParser : public TDFParser
{
public:
    HanTDFParser(PY* py) {m_owner = py;}
    ~HanTDFParser() {};

protected:
    virtual void parseRow(int row, string& str, vector<string>& columns);

private:
    PY *m_owner;
};

class AddToUserDBTask: public Task
{
public:
    AddToUserDBTask(string phrase);
    virtual ~AddToUserDBTask() {}
    virtual void doWork();

private:
    PY *m_owner;
    string m_phrase;
};

#endif
