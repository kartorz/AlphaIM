# ifdef WIN32
#include <Windows.h>
# endif

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
# ifdef _LINUX
#include <dirent.h>
# endif

#include "Log.h"
#include "Application.h"
#include "Util.h"
#include "CharUtil.h"
#include "Configure.h"
#include "SpinLock.h"

using namespace tinyxml2;
using namespace boost::filesystem;

#define CONF_VERSION  1

Configure& Configure::getRefrence()
{
    static Configure configure;
    return configure;
}

Configure::Configure():
m_homeDir(""),
m_configFile(""),
m_dirty(false),
m_selcnt(0)
{
}

Configure::~Configure()
{
    m_doc.SaveFile(m_configFile.c_str());
}

int Configure::initialization()
{
    int ret = 0;
    m_homeDir = Util::usrProfileDir("AlphaIM");
    m_configFile = m_homeDir + "/configure.xml";
    log(LOG_INFO, "home direcotry:(%s)\n", m_homeDir.c_str());

    m_dataDir = Util::execDir();
    m_dataDir +=  "/system";

#ifdef _LINUX
    if (!Util::isDirExist(m_dataDir))
        m_dataDir = DATADIR;
#endif
    log(LOG_INFO, "system dir :(%s)\n", m_dataDir.c_str());

    if (!Util::isDirExist(m_homeDir)) {
         if (Util::createDir(m_homeDir)) {
             Util::copyFile(m_dataDir + "/configure.xml.in", m_configFile);
         } else {
             log.e("can't create home direcotry\n");
             return -1;
         }
    }

    if (!Util::isFileExist(m_configFile)) {
        if (Util::copyFile(m_dataDir + "/configure.xml.in", m_configFile) == false) {
            log.e("{Configure} can't copy cofigure.xml.in \n");
            return -2;
        }
    }

    ret = load(m_configFile);
    if (ret != 0) {
        // Reload configure file
        Util::copyFile(m_dataDir + "/configure.xml.in", m_configFile);
        ret = load(m_configFile);
        log.w("{Configure} load configure.xml failure, restore to default setting\n");
    }

    return ret;
}

int Configure::load(const string& xmlpath)
{
    if (m_doc.LoadFile(xmlpath.c_str()) != XML_NO_ERROR) {
        log.e("{Configure} can't load xml %s\n", xmlpath.c_str());
        return -1;
    }

    XMLElement* rootElement = m_doc.RootElement();
    if (rootElement == NULL) {
        log.e("{Configure} can't get root element %s\n", xmlpath.c_str());
        return -2;    
    }

#if 0
    int ver = rootElement->IntAttribute("version");
    if (ver != CONF_VERSION)
        return ERR_LDCFG; // Recreate configure.xml
#endif

    XMLElement* tempElement = rootElement->FirstChildElement("phrase");
    if (!tempElement)
        return -3;   
    m_selcnt = tempElement->IntAttribute("selcnt");

    //m_doc.SaveFile(xmlpath.c_str());
    return 0;
}

int Configure::readSelcnt()
{
    return m_selcnt;
}

void Configure::writeSelcnt(int cnt)
{
    if (m_selcnt != cnt) {
        SpinLock lock(m_cs);
        m_selcnt = cnt;
        XMLElement* phraseElement = XMLHandle(m_doc.RootElement()).FirstChildElement("phrase").ToElement();
        if (phraseElement) {
            phraseElement->SetAttribute("selcnt", cnt);
            m_dirty = true;
        }
    }
}

void Configure::writeXml()
{
    SpinLock lock(m_cs);
    if (m_dirty) {
       m_dirty = false;
       m_doc.SaveFile(m_configFile.c_str());
    }
}
