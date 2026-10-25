#include <boost/algorithm/string.hpp>

#ifdef _LINUX
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif

#include "Application.h"
#include "Configure.h"
#include "IMPreedit.h"
#include "Log.h"

IMPreedit::IMPreedit():refs(0), m_bTrigger(false), m_bCN(true),
m_bCNPun(true),m_curPage(0), m_uiStringMax(80)
{
}

IMPreedit::~IMPreedit()
{
    //logger.d(":~IMPreedit\n");
}

// ret:
//    true - close the ic-window
//    false - otherwise.
bool IMPreedit::commit(int i)
{
    int start = m_curPage > 1 ? m_pageWin[m_curPage - 2] : 0;
    bool candistr = showCandiStr(m_curPage);
    if (candistr && i == 1) // commit candidate string.
    {
        //printf("commit candidate %s\n", m_items[i].val.c_str());
        if (m_items.size() > 0)
            gApp->curIM()->onCommit(m_items.front());
        return true;
    }

    i = start + i - 1 - candistr;
    //printf("commit: i: %d, size: %d\n", i, m_items.size());
    if (i < 0 || i > m_items.size()) {
        logger.e("commit: error number i: %d,\n", i);
        return false;
    }

    IMItem item = m_items[i];
    gApp->curIM()->onCommit(item);

    int key_pos = m_input.find(item.key, 0);
    if (key_pos != string::npos) {
        int next_pos = key_pos + item.key.length();
        //printf("next pos: %d\n", next_pos);
        if (next_pos < m_input.length()) {
            m_ci += m_input.substr(0, key_pos); // invalid PY before the 'key'.
            m_ci += item.val; // key --> val;
            m_ciItems.push_back(item);
            m_input = m_input.substr(next_pos, m_input.length() - next_pos);
            m_candidate = lookup(m_input);
            page(1);
            return false; // commit string, go on.
        } else {
            //boost::algorithm::replace_first(m_input, item.key,  item.val);
            //boost::algorithm::replace_first(m_candidate, m_items[0].val,  m_items[i].val);
            m_input = "";
            m_candidate = "";
            m_ci += m_input.substr(0, key_pos);
            m_ci += item.val;
            m_ciItems.push_back(item);
            page(1);
            return true;  // Done, commit the 'commit' string, close ic-window.
        }
    }

    logger.e("commit: can't find key %s\n", item.key.c_str());
    return true; //Something wrong, just commit, don't lose user input.
}

u32 IMPreedit::mapCNPun(char key)
{
    switch (key) {
    case '.':
        return 0x3002;
    case '?':
        return 0xFF1F;
    case '!':
        return 0xFF01;
    case ',':
        return 0xFF0C;
    case '`':
        return 0xFF40;
    case ';':
        return 0xFF1B;
    case ':':
        return 0xFF1A;
    case '\"':
        if (m_bPreQuo) {
            m_bPreQuo = false;
            return 0x201C;
        } else {
            m_bPreQuo = true;
            return 0x201D;
        }
    case  '(':
        return 0xFF08;
    case ')':
        return 0xFF09;
    case '[':
        return 0x3010;
    case ']':
        return 0x3011;
    case '-':
        return 0xFF0D;
    case '_':
        return 0xFF3F;
    case '<':
        return 0x300A;
    case '>':
        return 0x300B;
    case '{':
        return 0xFF5B;
    case '}':
        return 0xFF5D;
    }
    return 0;
}

string IMPreedit::mapCNPunToU8Str(char key)
{
    string ret = "";
    u32 CNPunUnicode = mapCNPun(key);
    if (CNPunUnicode) {
        char* ub = (char *)malloc(6);
        int len = CharUtil::ucs4CharToUTF8Byte(CNPunUnicode, ub);
        ub[len] = '\0';
        ret += ub;
        free(ub);
    }
    //PRINTF("mapCNPunToU8Str, %c --> %s\n", key, ret.c_str());
    return ret;
}

void IMPreedit::add(char key)
{
    m_bStart = true;
    if (m_input.length() < IM_INPUT_MAX) {
        if (m_bCNPun) {
            string strCNPun = mapCNPunToU8Str(key);
            if (strCNPun != "") {
                m_input += strCNPun;
            } else {
                m_input += key;
            }
        } else {
            m_input += key;
        }

        m_candidate = lookup(m_input);
        page(1);
    }
}

void IMPreedit::del()
{
    if (m_input != "") {
        char* r = CharUtil::u8charat(m_input.c_str(), -1, NULL);
        if (r != NULL) {
            boost::algorithm::erase_last(m_input, r);
            free(r);
        }
    }

    if (m_input == "" && m_ciItems.size () > 0) {
        IMItem it = m_ciItems.back();
        m_ciItems.pop_back();
        boost::algorithm::erase_last(m_ci, it.val);
        m_input = it.key + m_input;
    }

    if (m_input != "") {
        m_candidate = lookup(m_input);
        page(1);
    }
}

string IMPreedit::lookup(string input)
{
    string ret;
    m_curPage = 0;
    m_pageWin.clear();
    m_candidate = "";

    m_items.clear();
    ret = gApp->curIM()->lookup(input, m_items);
    return ret;
}

void IMPreedit::clear()
{
    m_bStart = false;
    m_input = "";
    m_ci = "";
    m_candidate = "";
    m_curPage = 0;
    m_pageWin.clear();
    m_items.clear();
    m_ciItems.clear();
    m_uiItems.clear();
}

/*
 * Click at a input
 * gtk:
 *   focus_in(41)
 *   reset(41)
 */
void IMPreedit::reset()
{
}

void IMPreedit::close()
{
    clear();
    guiAction(MSG_IM_OFF, GUIACT_SYSTRAY);
    gApp->curIM()->close();
}

/*
 * The number of items per page depends on phrase length.
 * eg:   1 xx  2 xx                 .. 9 xx
 *       1 xxxxxxxxx 2 xxxx         .. 5 xx
 * -----------------------------------------------
 * m_items: all the items
 * m_pageWin: exclusive page ends, e.g. 8, 12, 17, 22
 * m_uiItems: items at ic win
 */
void IMPreedit::page(int pg)
{
    const int padding = 6;
    IMItem candiItem;

    if (m_items.empty()) {
        m_curPage = 0;
        m_pageWin.clear();
        m_uiItems.clear();
        candiItem.key = "";
        candiItem.val = m_candidate;
        m_uiItems.push_front(candiItem);
        return;
    }

    if (pg < 1)
        pg = 1;
    if (pg > m_pageWin.size() + 1)
        return;

    // The range is [start, end).
    bool hasCandiStr = showCandiStr(pg);
    if (pg == m_pageWin.size() + 1) {
        int end = m_pageWin.empty() ? 0 : m_pageWin.back();
        bool firstPage = m_pageWin.empty();
        int maxItems = IM_PAGE_MAX_ITEMS - (firstPage && hasCandiStr);
        int length = padding;
        if (firstPage && hasCandiStr)
            length += m_candidate.length();

        for (int count = 0; count < maxItems && end < m_items.size(); ++count) {
            length += m_items[end++].val.length() + padding;
            if (length > m_uiStringMax)
                break;
        }
        m_pageWin.push_back(end);
    }

    int start = pg == 1 ? 0 : m_pageWin[pg - 2];
    int end = m_pageWin[pg - 1];
    m_uiItems.assign(m_items.begin() + start, m_items.begin() + end);
    if (hasCandiStr) {
        candiItem.key = "";
        candiItem.val = m_candidate;
        m_uiItems.push_front(candiItem);
    }
    m_curPage = pg;
    PRINTF("pg:%d, [%d, %d)\n", pg, start, end);
}

void IMPreedit::doTriggerOn()
{
    m_bTrigger = true;
    //m_bCN = true;
    //m_bCNPun = true;
    m_bStart = false;
}

void IMPreedit::doTriggerOff()
{
    m_bTrigger = false;
    //m_bCN = false;
    //m_bCNPun = false;
    flushUsrInput();
}

bool IMPreedit::doSwitchCEPun()
{
    m_bCNPun = !m_bCNPun;
    if (m_bCNPun)
        guiAction(MSG_IM_CPUN, GUIACT_SYSTRAY);
    else
        guiAction(MSG_IM_EPUN, GUIACT_SYSTRAY);
    return m_bCNPun;
}

bool IMPreedit::doSwitchCE()
{
    m_bCN = !m_bCN;

    if (!m_bCN) {
        doClose();
        flushUsrInput();
	}

    m_bCNPun = m_bCN;
    return m_bCN;
}

void IMPreedit::doPageup()
{
    page(m_curPage-1);
}

void IMPreedit::doPagedown()
{
    page(m_curPage+1);
}

bool IMPreedit::doInput(char key)
{
    if (key == (XK_BackSpace & 0xff)) {
        del();
        if (m_input != "") return true;
        doClose();
        return false;
    } else {
        add(key);
        return true;
    }
}

void IMPreedit::doClose()
{
    clear();
    guiAction(MSG_PREEDIT_CLOSE, GUIACT_SYSTRAY);
}

void IMPreedit::doCommit(int i, IMPreeditCallback *callback)
{
    if (i>=1 && i<=9 && commit(i)) {
        // commit candidte string.
        string candidate = m_ci + m_candidate;
        PRINTF("commit %s, %d\n", candidate.c_str(), i);
        callback->onCommit(callback->opaque, candidate);
        if (Configure::getRefrence().readLearnUserInput())
            m_usrPhrase.trackUsrInput(candidate);

        doClose();
    }
}

void IMPreedit::guiAction(int id, int flags)
{
    PRINTF("IMPreedit::guiAction id: %d flags: %d, %px\n", id, flags, this);
    gApp->getMessageQ()->push(id, flags, 0);
}

void IMPreedit::guiShowCandidate(IC* ic)
{
    if (m_bStart && !m_uiItems.empty()) {
        ICRect rect = ic->getRect();

        string input = m_ci + m_input;
        string candidate = m_ci + m_candidate;
        string items = "";

        for (int i=0; i < m_uiItems.size(); i++) {
            items += m_uiItems.at(i).val + " ";
        }

        /*PRINTF("guiShowCandidate %s-->%s,[%d,%d,%d,%d], \n",
               input.c_str(), items.c_str(), rect.x, rect.y, rect.w, rect.h);*/

        Message msg;
        msg.id = MSG_PREEDIT_UPDATE;
        msg.iArg1   = rect.x;
        msg.iArg2   = rect.y;
        msg.fArg1   = rect.w;
        msg.fArg2   = rect.h;
        msg.strArg1 = input;
        msg.strArg2 = items;

        gApp->getMessageQ()->push(msg);
    }
    //gApp->pSysMsgQ->push(MSG_IM_OFF);
}

/*
 * OPEN/CLOSE: im state, indicated by ic win
 * CN/EN     : lan state.
 */
void IMPreedit::guiReload()
{
    if (!m_bTrigger) {
        guiAction(MSG_IM_OFF, GUIACT_SYSTRAY);
        guiAction(MSG_PREEDIT_CLOSE, GUIACT_SYSTRAY);
        return;
    }

    if (m_bCN)
        guiAction(MSG_IM_CN, GUIACT_SYSTRAY);
    else
        guiAction(MSG_IM_EN, GUIACT_SYSTRAY);
}

void IMPreedit::flushUsrInput()
{
    if (Configure::getRefrence().readLearnUserInput()) {
        vector<string> phrases;
        m_usrPhrase.flush(phrases);
        gApp->curIM()->addUserPhraseAsync(phrases);
    }
}

/* Message from gui */
void IMPreedit::handleMessage(int msg, int arg)
{
    switch(msg) {
    case MSG_UI_LAN:
        doSwitchCE();
        return;
    case MSG_UI_PUN:
        doSwitchCEPun();
        return;
    }
}
