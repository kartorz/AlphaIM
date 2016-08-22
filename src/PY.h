#ifndef _PY_H_
#define _PY_H_

#include <deque>
#include <string>

#include "MutexLock.h"
#include "iIM.h"
#include "indextree/IndexTree.h"
#include "TDFParser.h"

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
    PY(const string& pydb, const string& phdb, const string& pytbl);
    virtual ~PY();
    virtual string lookup(const string& input, deque<IMItem>& items);

private:
    int  lookup(string key, IndexList& indexList, deque<IMItem>& items);
    int  lookupCache(const string& key, deque<IMItem>& items);
    void cache(const string& input, int validlen, deque<IMItem>& items);
    void getPYItems(const string& key, deque<IMItem>& items);

    MutexCriticalSection m_phdbCS;
    IndexTree m_pyDB;
    IndexTree m_phDB;
    map<string, HC>   m_hcMap;
    deque<CacheItem>  m_cache;
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

#if 0
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
#endif
