/**
  * There preedit modes (m_icMode):
  *    0) One preedit per App (default)
  *    1) One preedit per OS
  *    2) One preedit per IC (non-use)
  */
#include "IC.h"
#include "Log.h"

int IC::dpyW = 1024;
int IC::dpyH = 768;


ICRect IC::adjRect(int x, int y, int w, int h)
{
    ICRect ret;

    if ((x <= 0 || y <= 0) && dpyW) {
        // Client don't set cursor.
        x = (dpyW - ICWIN_W)/2;
        y = dpyH - ICWIN_H - 60;
    }

    if (x + w > dpyW - 20) {
        x = dpyW - ICWIN_W - 20;
    }

    if (y + h > dpyH - 10) {
        y = y - 2*h;
    }

    //PRINTF("getICWin (%d, %d, %d, %d)\n",  x, y, w, h);
    ret.x = x;
    ret.y = y;
    ret.w = w;
    ret.h = h;
    return ret;
}

IC::IC(): id(0),cursorX(0),cursorY(0)
{
}

IC::~IC()
{
}

void IC::close()
{
    preedit->close();
}

void IC::setCursorLocation(int x, int y, int w, int h)
{
    //PRINTF("setCursor (%d, %d, %d, %d)\n", x, y, w, h);
    cursorX = x;
    cursorY = y;
}

void IC::reset()
{
    //PRINTF("reset\n");
    preedit->reset();
}

void IC::enable()
{
}

void IC::disable()
{
}

bool IC::isEnabled()
{
    return true;
}

void IC::setCapabilities(u32 cpas)
{
}

void IC::propertyActivate(const char *name, int state)
{
}

void IC::setSurroundingText()
{
}

IMPreedit *IC::createPreedit(ICManager* icm)
{
    PRINTF("error no preedit\n");
    return NULL;
}

ICRect IC::getRect()
{
    return adjRect(cursorX, cursorY + 40, ICWIN_W, ICWIN_H);
}

ICManager::ICManager(): m_icid(0), m_icFocus(0), m_icMode(0)
{
    /* Dumy IC for error proc, no need to check
     * IC* and IMPreedit* everywhere */
    m_ics[0] = new IC();
    m_ics[0]->preedit = m_ics[0]->createPreedit(this);
}

ICManager::~ICManager()
{
    std::map<int, IC*>::iterator iter;
    for (iter = m_ics.begin(); iter != m_ics.end(); iter++)
        delete(iter->second);

    std::map<string, IMPreedit*>::iterator preedit;
    for (preedit = m_preedits.begin(); preedit != m_preedits.end(); preedit++)
        delete preedit->second;
}

void ICManager::focusIn(u32 id)
{
    //PRINTF("focusIn %d\n", id);
    //m_icFocus = id;
    if (!m_icMode)
        get(id)->preedit->guiReload();
}

/*
 * Click at a input(45),
 * 45 focus in, 45 rest, 1 focus out
 -----------------------------------
 * m_icFocus: obsolete  
 */
int ICManager::focusOut(u32 id)
{
    //PRINTF("focusOut %d\n", id);
#if 0
    if (m_icFocus == id)
        m_icFocus = 0;
#endif
    return 0;
}

void ICManager::setMode(int mode)
{
    m_icMode = mode == 1 ? 1 : 0;
}

/**/
int ICManager::mode() const
{
    return m_icMode;
}

u32 ICManager::add(IC *ic)
{
    m_ics[++m_icid] = ic;
    ic->id = m_icid;
    return ic->id;
}

/* '*app' must not be NULL */
u32 ICManager::add(IC *ic, const char *app)
{
    if (!m_icMode) {
        std::map<std::string, IMPreedit*>::iterator iter = m_preedits.find(app);
        if (iter == m_preedits.end())
            m_preedits[app] = ic->createPreedit(this);
        ic->preedit = m_preedits[app];
    } else {
        ic->preedit = m_ics[0]->preedit;
    }

    return add(ic);
}

void ICManager::destroy(u32 id)
{
    if (id > 0) {
        std::map<int, IC*>::iterator iter = m_ics.find(id);
        if(iter != m_ics.end()) {
            delete(iter->second);
            m_ics.erase(iter);
            //printf("destroy id:%d\n", icid);
        }
    }
}

IC* ICManager::get(u32 id)
{
    //PRINTF("ICManager::get id: %d\n", icid);
    std::map<int, IC*>::iterator iter = m_ics.find(id);
    if(iter != m_ics.end()) {
        return iter->second;
    }
    logger.e("ICManager:: return the dummy IC\n");
    return m_ics[0];
}
