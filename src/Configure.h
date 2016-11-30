#ifndef _CONFIGURE_H_
#define _CONFIGURE_H_

#include "tinyxml2/tinyxml2.h"

#include <string>
#include <vector>
#include <map>

#include "SpinLock.h"

using namespace std;
using namespace tinyxml2;

class Configure
{
public:
    static Configure& getRefrence();
    Configure();
    ~Configure();

    int  initialization();
    int  readSelcnt();
    void writeSelcnt(int cnt);

    void writeXml();

    string m_dataDir;
    string m_homeDir;

private:
    int  load(const string& xmlpath);


    tinyxml2::XMLDocument m_doc;
    bool m_dirty;
    int m_selcnt;
    string m_configFile;
    SpinCriticalSection m_cs;
};

#endif
