#include <boost/algorithm/string.hpp>

#ifdef _LINUX
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif

#include "Application.h"
#include "IMPreedit.h"
#include "Log.h"

#undef PRINTF
//#define PRINTF(fmt, args...)  printf(fmt, ##args)
#define PRINTF(fmt, args...)

IMPreedit::IMPreedit():m_bTrigger(false), m_curPage(0),m_uiStringMax(100)
{
}

IMPreedit::~IMPreedit()
{
    log.d(":~IMPreedit\n");
}

bool IMPreedit::commit(int i)
{
    i = m_pageWin[0] + i - 1; // 'i' starts from 1.
    if (i == 0 && m_bCandiItem) // commit candidate string.
        return true;

    i -= m_bCandiItem;  // offset to m_items
    printf("commit: i: %d, size: %d\n", i, m_items.size());
    if(m_items.size() > i) {
        IMItem item = m_items[i];

        int key_pos = m_input.find(item.key, 0);
        if (key_pos != string::npos) {
            int next_pos = key_pos + item.key.length();
            printf("next pos: %d\n", next_pos);
            if (next_pos < m_input.length()) {
                m_ci += m_input.substr(0, key_pos); // invalid PY before the 'key'.
                m_ci += item.val; // key --> val;
                m_ciItems.push_back(item);
                m_input = m_input.substr(next_pos, m_input.length() - next_pos);
                m_candidate = lookup(m_input);
                page(1);
            } else {
                //boost::algorithm::replace_first(m_input, item.key,  item.val);
                //boost::algorithm::replace_first(m_candidate, m_items[0].val,  m_items[i].val);
                m_input = "";
                m_candidate = "";
                m_ci += m_input.substr(0, key_pos);
                m_ci += item.val;
                m_ciItems.push_back(item);
                page(1);
                //TODO: show pharse db beging with commit string.
                return true; // Done, commit the 'commit' string.
            }
        } else {
            log.e("[XimSrv::commit]: can't find key %s\n", item.key.c_str());
            return true; //Something wrong, just commit, don't lose user input.
        }
    }
    return false;
}

u32 IMPreedit::mapCNPun(char *key)
{
    switch (*key) {
    case '.':
        return 0x3002;
    case '?':
        return 0xFF1F;
    case '!':
        return 0xFF01;
    case ',':
        return 0xFF0C;
    case '`':
        return 0x3001;
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
        return 0x2014;
    case '<':
        return 0x300A;
    case '>':
        return 0x300B;
    }
    return 0;
}

string IMPreedit::mapCNPunToU8Str(char *key)
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
    return ret;
}

void IMPreedit::add(char *key)
{
    m_bStart = true;
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

void IMPreedit::del()
{
#if 0
    if (m_ciItems.size () > 0) {
        IMItem it = m_ciItems.back();
        m_ciItems.pop_back();
        boost::algorithm::erase_last(m_ci, it.val);
        //int pos = m_ci.find_last_of(it.val);
        //if (pos != string::npos)
        //    m_ci.erase(pos, it.val.length());
        //printf("1111%s, %s\n", m_input.c_str(), it.key.c_str());
        //string temp = it.key + m_input;
        m_input = it.key + m_input;
    } else {
        if (m_input != "") {
            char* r = CharUtil::u8charat(m_input.c_str(), -1, NULL);
            if (r != NULL) {
                boost::algorithm::erase_last(m_input, r);
                free(r);
            }
        }
    }
#else
    if (m_input != "") {
        printf("del: m_input %s \n", m_input.c_str());
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
        //int pos = m_ci.find_last_of(it.val);
        //if (pos != string::npos)
        //    m_ci.erase(pos, it.val.length());
        //printf("1111%s, %s\n", m_input.c_str(), it.key.c_str());
        //string temp = it.key + m_input;
        m_input = it.key + m_input;
        printf("del: m_input2 %s \n", m_input.c_str());
    }

    m_candidate = lookup(m_input);
    page(1);
#endif
}

string IMPreedit::lookup(string input)
{
    string ret;
    m_curPage = 0;
    m_pageWin[0] = 0;
    m_pageWin[1] = 0;
    m_candidate = "";

    m_items.clear(); 
    ret = im->lookup(input, m_items);
    return ret;
}

void IMPreedit::clear()
{
    m_bStart = false;
    m_input = "";
    m_ci = "";
    m_candidate = "";
    m_curPage = 0;
    m_pageWin[0] = 0;
    m_pageWin[1] = 0;
    m_items.clear();
    m_ciItems.clear();
    m_uiItems.clear();
}

void IMPreedit::close(bool bReset)
{
    if (bReset) {
        clear();
    }
    guiAction(MSG_IM_OFF);
    im->close();
}

/*
 * The items number of a page is variable depending on length of phrase.
 * eg:   1 xx  2 xx                 .. 9 xx
 *       1 xxxxxxx 2 xxxxxxxxxxxx   .. 5 xx 
 *
 */

/*
 * The items number of a page is variable depending on length of phrase.
 * eg:   1 xx  2 xx                 .. 9 xx
 *       1 xxxxxxx 2 xxxxxxxxxxxx   .. 5 xx 
 *  
 * Current page: ( m_pageWin[0] , m_pageWin[1] ]
 */
void IMPreedit::page(int pg)
{
    //if (m_candidate == "") // Maybe, there are ""s in db.
    if (m_items.size() == 0)
        return;

    if (pg < 1)
        pg = 1;

    bool bPgDown = m_curPage <= pg;
    if (bPgDown && m_pageWin[1] >=  m_items.size())
        return;

    if (!bPgDown && m_pageWin[0] <= -1)
        return;

    int maxItems = IM_ITEM_PAGE_SIZE;
    int start, end;

    m_curPage = pg;
    m_uiItems.clear();
    m_bCandiItem = false;

    IMItem  candiItem;
    if (pg == 1) {

        if (m_candidate != m_items[0].val) {
            candiItem.key = "";
            candiItem.val = m_candidate;
            maxItems--;
        }

        start = 0;
        end = m_items.size() < maxItems ? m_items.size() : maxItems;
        bPgDown = true;
    } else {
        //printf("pg:%d, [%d, %d]\n", m_curPage, m_pageWin[0], m_pageWin[1]);
        if (bPgDown) {
            //  win[0] ... win[1] | 
            //             start ... end   
            start = m_pageWin[1];
            end = start + maxItems;
            end = m_items.size() < end ? m_items.size() : end;
        } else {
            // start ... end |
            //           win[0] ... win[1]
            end = m_pageWin[0];
            start = end - maxItems;
            start = start < 0 ? 0 : start;
        }
        //printf(" (%d, %d)\n", start, end);
    }

    int length = candiItem.val.length();
    int pos;
    for (pos = 0; pos < end - start; pos++) {
        IMItem it = bPgDown ? m_items[start + pos] : m_items[end - pos - 1] ;
        m_uiItems.push_back(it);
        length += it.val.length();
        if (length > m_uiStringMax) {
            ++pos; // Fix m_pageWin
            break;
        }
    }

    if (bPgDown) {
        m_pageWin[0] = start;
        m_pageWin[1] = start + pos;
    } else {
        m_pageWin[0] = end - pos;
        m_pageWin[1] = end;
    }
    //printf("done [%d, %d]\n", m_pageWin[0], m_pageWin[1]);

    if (candiItem.val != "") {
        m_bCandiItem = true;
        m_uiItems.push_front(candiItem);
    }   
}

void IMPreedit::doSwitchCEPun()
{
    m_bCNPun = !m_bCNPun;
    if (m_bCNPun)
        guiAction(MSG_IM_CPUN);
    else
        guiAction(MSG_IM_EPUN);
}

void IMPreedit::doSwitchCE(IMPreeditCallback *callback)
{
    m_bCN = !m_bCN;

    if (!m_bCN) {
        string candidate = m_input;
        callback->onCommit(callback->opaque, candidate);

        guiAction(MSG_IM_EN);
        doClose();
    } else {
        guiAction(MSG_IM_CN);
    }

    m_bCNPun = m_bCN;
}

void IMPreedit::doPageup()
{
    page(m_curPage-1);
}

void IMPreedit::doPagedown()
{
    page(m_curPage+1);
}

bool IMPreedit::doInput(char *key)
{
    if (*key == (XK_BackSpace & 0xff)) {
        del();
        if (m_input != "") {
            return true;
        }
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
    guiAction(MSG_IM_CLOSE);
}

void IMPreedit::doCommit(int i, IMPreeditCallback *callback)
{
    if (i >= 1 && i <= 9) {
        if (commit(i)) {
            // commit candidte string.
            string candidate = m_ci + m_candidate;
            PRINTF("commit %s\n", candidate.c_str());
            callback->onCommit(callback->opaque, candidate);

            doClose();
            im->addUserPhrase(candidate);
        }
    }
}

void IMPreedit::guiAction(int id)
{
    gApp->pGuiMsgQ->push(id);
}

void IMPreedit::guiShowCandidate(IMPreeditCallback *callback)
{
    if (m_bStart && m_uiItems.size() > 0) {
        ICRect rect = callback->onGetRect(callback->opaque);
        
        string input = m_ci + m_input;
        string candidate = m_ci + m_candidate;
        deque<IMItem>& items = m_uiItems;

        deque<IMItem> *pItems = new deque<IMItem>(items);

        PRINTF("preEdit %s, %d, %d , %d %d\n", input.c_str(), rect.x, rect.y, rect.w, rect.h);

        Message msg;
        msg.id = MSG_IM_INPUT;
        msg.iArg1   = rect.x;
        msg.iArg2   = rect.y;
        msg.fArg1   = rect.w;
        msg.fArg2   = rect.h;
        msg.strArg1 = input;
        msg.pArg1 =   pItems;
        
        gApp->pGuiMsgQ->push(msg);
    }
    //gApp->pGuiMsgQ->push(MSG_IM_OFF);
}

void IMPreedit::guiReload(IMPreeditCallback *callback)
{
    PRINTF("guiReload %d\n", m_bTrigger);

    if (!m_bTrigger) {        
        guiAction(MSG_IM_CLOSE);
        return;
    }

    guiAction(MSG_IM_ON);

    if (m_bCN)
        guiAction(MSG_IM_CN);
    else
        guiAction(MSG_IM_EN);

    guiShowCandidate(callback);
}

bool IMPreedit::isMatchKeys(int keysym, int modifier, TriggerKey *trigger)
{
    int i;
    int modifier2;
    int modifier2_mask;

    for (i = 0; trigger[i].keysym != 0; i++) {
	    modifier2      = trigger[i].modifier;
	    modifier2_mask = trigger[i].modifier_mask;
	    if (((KeySym)trigger[i].keysym == keysym)
	        && ((modifier & modifier2_mask) == modifier2))
	    return True;
    }
    return False;
}


