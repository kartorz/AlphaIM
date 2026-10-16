/**
  * There preedit modes (m_icMode):
  *    0) One preedit per App (default)
  *    1) One preedit per OS
  *    2) One preedit per IC (non-use)
  */
#include "IC.h"
#include "Log.h"

vector<ScreenRect> IC::screens;

IC::IC(): id(0),cursor{0,0,0,32}
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
    cursor.x = x;
    cursor.y = y;
    if (w > 0) cursor.w = w;
    if (h > 0) cursor.h = h;
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

ICRect IC::getRect()
{
    #define margin 48

    ICRect ret = {cursor.x, cursor.y+margin, ICWIN_W,ICWIN_H};

    /* Don't set cursor firstly, or something wrong.
     * Specify 1280*720 for the dual screen issue.
     * When the monitor is left of the primary screen, x|y < 0 is valid.
     */
    if (cursor.x == 0 && cursor.y == 0) {
        ret.x = (1280 - ret.w)/2;
        ret.y = 720 - ret.h - 2*margin;
        return ret;
    }

    ScreenRect sc = IC::screens.front();
    for (const ScreenRect &candidate : IC::screens) {
        if (cursor.x >= candidate.x && cursor.x < candidate.x + candidate.w &&
            cursor.y >= candidate.y && cursor.y < candidate.y + candidate.h) {
            sc = candidate;
            break;
        }
    }

    PRINTF("getICWin before: (%d, %d) -- (%d,%d,%d,%d)\n",
           ret.x, ret.y, sc.x, sc.y, sc.w, sc.h);

    if (ret.x + ret.w + margin > sc.x + sc.w)
        ret.x = sc.x + sc.w - ret.w - margin;
    if (ret.x < sc.x + margin)
        ret.x = sc.x + margin;

    if (ret.y + ret.h + 4*margin > sc.y + sc.h)
        ret.y = sc.y + sc.h - 4*ret.h - 4*margin;
    if (ret.y < sc.y + margin)
        ret.y = sc.y + margin;

    PRINTF("getICWin after: (%d, %d) -- (%d,%d,%d,%d)\n",
           ret.x, ret.y, sc.x, sc.y, sc.w, sc.h);

    return ret;
}

ICManager::ICManager(IC* ic): m_icid(0), m_icFocus(0), m_icMode(0)
{
    /* m_icMode :
     * ----------
     *  0: Default IC for error case, no need to check
     *     IC* and IMPreedit* everywhere.
     *     If alphad crash, using the default IC for
     *     existing ics
     *  1: The global IC.
     */
    m_ics[0] = ic;
    m_ics[0]->preedit = ic->createPreedit();
    m_ics[0]->preedit->addRef();
}

ICManager::~ICManager()
{
    std::map<int, IC*>::iterator iter;
    for (iter = m_ics.begin(); iter != m_ics.end(); iter++) {
        releasePreedit(iter->second->preedit);
        delete(iter->second);
    }
}

void ICManager::releasePreedit(IMPreedit *preedit)
{

    if (!preedit || !preedit->releaseRef()) return;

    for (std::map<string, IMPreedit*>::iterator app = m_preedits.begin();
         app != m_preedits.end(); ) {
        if (app->second == preedit) {
            m_preedits.erase(app++);
            break;
        }
        ++app;
    }
    delete preedit;
	PRINTF("releasePreedit: %px\n", preedit);
}

void ICManager::focusIn(u32 id)
{
    //PRINTF("focusIn %d, %d\n", id, m_icFocus);
    /* There are many foucus event */
    if (m_icFocus == id)
        return;
    m_icFocus = id;
    if (!m_icMode)
        get(id)->preedit->guiReload();
}

/*
 * Qt/Gtk: Click at a input(45),
 * 45 focus in, 45 rest, 1 focus out
 -----------------------------------
 * m_icFocus: for X11
 */
int ICManager::focusOut(u32 id)
{
    //PRINTF("focusOut %d\n", id);
    if (m_icFocus == id)
        m_icFocus = 0;
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
    m_icFocus = ic->id;
    return ic->id;
}

/* '*app' must not be NULL */
u32 ICManager::add(IC *ic, const char *app)
{
    if (!m_icMode) {
        std::map<std::string, IMPreedit*>::iterator iter = m_preedits.find(app);
        if (iter == m_preedits.end()) {
            ic->preedit = ic->createPreedit();
            m_preedits[app] = ic->preedit;
        } else {
            ic->preedit = iter->second;
        }
        ic->preedit->addRef();
    } else {
        ic->preedit = m_ics[0]->preedit;
    }

    PRINTF("add ic:%d, Preedit refs:%d\n", ic->id, ic->preedit->refs);
    return add(ic);
}

void ICManager::destroy(u32 id)
{
    if (id > 0) {
        std::map<int, IC*>::iterator iter = m_ics.find(id);
        if(iter != m_ics.end()) {
            printf("destroy id:%d, ref: %d\n", id, iter->second->preedit->refs);
            releasePreedit(iter->second->preedit);
            delete(iter->second);
            m_ics.erase(iter);
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
    /* Error, eg: alphad crash, using the defaut IC. */
    logger.e("ICManager:: return the default IC\n");
    return m_ics[0];
}
