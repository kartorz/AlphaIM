
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "Application.h"
#include "X11IMPreedit.h"
#include "Log.h"

/* Trigger Keys List */
static TriggerKey OnOffKeys[] = {
    {XK_space, ShiftMask, ShiftMask},
    {XK_space, ControlMask, ControlMask},
    {0L, 0L, 0L}
};

/* Conversion Keys List */
static TriggerKey CommitKeys[] = {
    {XK_space, 0L, 0L},
    {XK_Return, 0, 0},
    {0L, 0L, 0L}
};

/* Forward Keys List */
static TriggerKey ForwardKeys[] = {
    {XK_Tab, 0, 0},
    {0L, 0L, 0L}
};

static TriggerKey PageUpKeys[] = {
    {XK_f, ControlMask, ControlMask},
    {XK_Page_Up, 0, 0},
    {XK_Left, 0L, 0L},
    {XK_minus, 0L, 0L},
    {0L, 0L, 0L}
};

static TriggerKey PageDownKeys[] = {
    {XK_j, ControlMask, ControlMask},
    {XK_Page_Down, 0, 0},
    {XK_Right, 0L, 0L},
    {XK_equal, 0L, 0L},
    {0L, 0L, 0L}
};

static TriggerKey CancelKeys[] = {
    {XK_Escape, 0, 0},
    {XK_c, ControlMask, ControlMask},
    {0L, 0L, 0L}
};

static TriggerKey CESwitchKeys[] = {
    {XK_Shift_L, ShiftMask, ShiftMask},
    {0L, 0L, 0L}
};

static TriggerKey CEPSwitchKeys[] = {
    {XK_Shift_R, ShiftMask, ShiftMask},
    {0L, 0L, 0L}
};

X11IMPreedit::X11IMPreedit():m_preModKey(-1),m_preRetKey(0)
{
}

u32 X11IMPreedit::mapCNPun(char *key)
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

/*
 * There are some pains when comes to precess x11 keys.
 * 1) modifier + key
 *    - Press key event:
 *      kev state: 0 code: ffe1       key: 0
 *      kev state: 1 code: 'keycode'  key: 'keyval'
 *    - Release key event:
 *      kev state: 1 code: 'keycode'  char: 'keyval' 
 *      kev state: 1 code: ffe1       char: 0
 * 2) Forward key.
 *    Forward 'Release key event' not working.
 */
int X11IMPreedit::handleKey(int keysym, int modifier, char *key, int evtype, IMPreeditCallback *callback)
{
    //MutexLock lock(m_cs);
    if (isMatchKeys(keysym, modifier, ForwardKeys)) {
        return FORWARD_KEY;
    }

    // Check Shift + key
    if (keysym == XK_Shift_L || keysym == XK_Shift_R) {
        if (evtype == KeyPress) {
            return true;
        }
        if (m_preModKey == ShiftMask) {
            m_preModKey = -1;
            return true;
        }
    } else {
        if (evtype == KeyRelease)
            return m_preRetKey;
        //printf("m_preModKey == %d, current == %d\n", m_preModKey, modifier);
        m_preModKey = modifier;
    }

    m_preRetKey = NONE_KEY;

    // Check OnOff
    if (isMatchKeys(keysym, modifier, OnOffKeys)) {
        if (!m_bTrigger) {
            m_bTrigger = true;
            m_bCN = true;
            m_bCNPun = true;
            guiAction(MSG_IM_ON);
            return TRIGGER_ON_KEY;
        }
        m_bTrigger = false;
        callback->onIMOff(callback->opaque);
        guiAction(MSG_IM_OFF);
        return TRIGGER_OFF_KEY;
    }
    if (!m_bTrigger) {
        m_preRetKey = FORWARD_KEY;
        return FORWARD_KEY;
    }

    // Check  language 
    if (isMatchKeys(keysym, modifier, CESwitchKeys)) {
        doSwitchCE(callback);
        return SWITCH_CE_KEY;
    }
    if (!m_bCN) {
        m_preRetKey = FORWARD_KEY;
        return FORWARD_KEY;
    }

    if (isMatchKeys(keysym, modifier, CEPSwitchKeys)) {
        doSwitchCEPun();
        return SWITCH_CEP_KEY;
    }

    // Check if start
    if (!m_bStart) {
        if (modifier == 0  && isalpha(*key)) {
            doInput(key);
            guiShowCandidate(callback);
            return CONVERT_KEY;
        }

        if (m_bCNPun) {
            string cnPun = mapCNPunToU8Str(key);
            if (cnPun != "") {
                callback->onCommit(callback->opaque, cnPun);
                return COMMIT_KEY;           
            }
        }

        m_preRetKey = FORWARD_KEY;
        return FORWARD_KEY;
    }

    if (isMatchKeys(keysym, modifier, CommitKeys)) {
        doCommit(1, callback);
        guiShowCandidate(callback);
        return COMMIT_KEY;
    }

    if (isMatchKeys(keysym, modifier, PageUpKeys))   {
        doPageup();
        guiShowCandidate(callback);
        return PAGEUP_KEY;
    }

    if (isMatchKeys(keysym, modifier, PageDownKeys)) {
        doPagedown();
        guiShowCandidate(callback);
        return PAGEDOWN_KEY;
    }

    if (isMatchKeys(keysym, modifier, CancelKeys)) {
        doClose();
        return CANCEL_KEY;
    }

    if (modifier == 0 || modifier == 1) {
        if (isdigit(*key)) {
            int i = *key - 0x30;
            doCommit(i, callback);
            guiShowCandidate(callback);
            return COMMIT_KEY;
        }

        //if (*key == (XK_BackSpace & 0xff))
        if ((keysym & 0xff) == (XK_BackSpace & 0xff)) {
            doInput(key);
            guiShowCandidate(callback);
            return COMMIT_KEY;
        }

        // Must at the end. 
        if (isgraph(*key)) {
            doInput(key);
            guiShowCandidate(callback);
            return CONVERT_KEY;
        }
    }
    m_preRetKey = FORWARD_KEY;
    return FORWARD_KEY;
}

