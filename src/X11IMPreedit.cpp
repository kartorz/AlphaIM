
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "Application.h"
#include "X11IMPreedit.h"
#include "Log.h"

/* Forward Keys List */
TriggerKey ForwardKeys[] = {
    {XK_Tab, 0, 0},
    {XK_ISO_Left_Tab, 0, 0},
    {XK_Caps_Lock, 0, 0},
    {XK_Num_Lock, 0, 0},
    {XK_Scroll_Lock, 0, 0},
    {XK_F1, 0, 0},
    {XK_F2, 0, 0},
    {XK_F3, 0, 0},
    {XK_F4, 0, 0},
    {XK_F5, 0, 0},
    {XK_F6, 0, 0},
    {XK_F7, 0, 0},
    {XK_F8, 0, 0},
    {XK_F9, 0, 0},
    {XK_F10, 0, 0},
    {XK_F11, 0, 0},
    {XK_F12, 0, 0},
    {0L, 0L, 0L}
};

/* Trigger Keys List */
static TriggerKey OnOffKeys[] = {
    {XK_space, ShiftMask, ShiftMask},
    {XK_space, ControlMask, ControlMask},
    {XK_space, ControlMask | Mod4Mask, ControlMask | Mod4Mask},
    {0L, 0L, 0L}
};

/* Conversion Keys List */
static TriggerKey CommitKeys[] = {
    {XK_space, 0L, 0L},
    {XK_Return, 0, 0},
    {0L, 0L, 0L}
};

static TriggerKey PageUpKeys[] = {
    {XK_Page_Up, 0, 0},
    {XK_Left, 0L, 0L},
    {XK_Up, 0L, 0L},
    {XK_minus, 0L, 0L},
    {0L, 0L, 0L}
};

static TriggerKey PageDownKeys[] = {
    {XK_Page_Down, 0, 0},
    {XK_Right, 0L, 0L},
    {XK_equal, 0L, 0L},
    {XK_Down, 0L, 0L},
    {0L, 0L, 0L}
};

static TriggerKey CancelKeys[] = {
    {XK_Escape, 0, 0},
    {0L, 0L, 0L}
};

static TriggerKey CESwitchKeys[] = {
    {XK_Shift_L, 0, 0},
    {0L, 0L, 0L}
};

static TriggerKey CEPSwitchKeys[] = {
    {XK_Shift_R, 0, 0},
    {0L, 0L, 0L}
};

X11IMPreedit::X11IMPreedit():IMPreedit(),m_preModKey(0),m_preRetKey(0)
{
}

bool X11IMPreedit::isModifier(u32 keysym)
{
    if (keysym >= XK_Shift_L  && keysym <= XK_Hyper_R)
        return true;
    return false;
}

bool X11IMPreedit::isMatchKeys(int key, int modifier, TriggerKey *trigger)
{
    int i;
    int modifier2;
    int modifier2_mask;

    for (i = 0; trigger[i].keysym != 0; i++) {
        modifier2      = trigger[i].modifier;
        modifier2_mask = trigger[i].modifier_mask;
        if (((KeySym)trigger[i].keysym == key)
            && ((modifier & modifier2_mask) == modifier2))
        return True;
    }
    return False;
}

bool X11IMPreedit::imWants(u32 key, u32 modifier)
{
    if (!isActive()) return false;
    if (m_bCNPun && mapCNPun(key)) return true;
    if (modifier) return false;
    if (m_bStart) return true;
    return (key >= 'a' && key <= 'z');
}

/* modifier
 * ------------------------------------
 *  0: normal key, without modifier key
 *  1: ShiftMask        16: Num Lock
 *  2: LockMask         18: Meta
 *  4: ControlMask      64: Super/Windows
 *  8: Alt             128: Scroll Lock
 *
 * key
 * --------------------------------------
 * XK_0 .. XK_9: '0' .. '9'
 * XK_a .. XK_z: 'a' .. 'z'
 */
int X11IMPreedit::doHandleKey(IC* ic, u32 key, u32 modifier, IMPreeditCallback *callback)
{

    PRINTF("%d,doHandleKey,key(%x,%x), start:%d, triger:%d, cn:%d, CNPun:%d\n",
           ic->id, key, modifier, m_bStart, m_bTrigger, m_bCN, m_bCNPun);

    if (isMatchKeys(key, modifier, ForwardKeys))
        return FORWARD_KEY;

    // Check OnOff
    if (isMatchKeys(key, modifier, OnOffKeys)) {
        if (!m_bTrigger || !m_bCN) {
            m_bTrigger = true;
            m_bCN = true;
            m_bCNPun = true;
            m_bStart = false;
            guiAction(MSG_IM_ON, GUIACT_SYSTRAY | GUIACT_HINT);
            return TRIGGER_ON_KEY;
        }
        m_bTrigger = false;
        m_bCN = false;
        m_bCNPun = false;
        callback->onIMOff(callback->opaque);
        doClose();
        guiAction(MSG_IM_OFF, GUIACT_SYSTRAY);
        return TRIGGER_OFF_KEY;
    }

    if (!m_bTrigger)
        return FORWARD_KEY;

    // Check language
    if (isMatchKeys(key, modifier, CESwitchKeys)) {
        if (m_input != "")
            callback->onCommit(callback->opaque, m_input);
            printf("*** MSG_IM_EN, commit m_input %s **\n", m_input.c_str());
        if (!doSwitchCE())
            guiAction(MSG_IM_EN, GUIACT_SYSTRAY | GUIACT_HINT);
        else
            guiAction(MSG_IM_CN, GUIACT_SYSTRAY | GUIACT_HINT);

        return SWITCH_CE_KEY;
    }
    if (!m_bCN)
        return FORWARD_KEY;

    if (isMatchKeys(key, modifier, CEPSwitchKeys)) {
        doSwitchCEPun();
        return SWITCH_CEP_KEY;
    }

    // Check if start
    if (!m_bStart) {
        if (!modifier && (key >= 'a' && key <= 'z')) {
            doInput(key);
            guiShowCandidate(ic);
            return CONVERT_KEY;
        }
        if (m_bCNPun) {
            string cnPun = mapCNPunToU8Str(key);
            if (cnPun != "") {
                callback->onCommit(callback->opaque, cnPun);
                return COMMIT_KEY;
            }
        }
        return FORWARD_KEY;
    }

    // After this section, without modifier key.
    if (modifier > 0) return FORWARD_KEY;

    if (isMatchKeys(key, modifier, CommitKeys)) {
        if (m_bStart) {
            doCommit(1, callback);
            guiShowCandidate(ic);
            return COMMIT_KEY;
        } else {
            return FORWARD_KEY;
        }
    }

    if (isMatchKeys(key, modifier, PageUpKeys))   {
        doPageup();
        guiShowCandidate(ic);
        return PAGEUP_KEY;
    }

    if (isMatchKeys(key, modifier, PageDownKeys)) {
        doPagedown();
        guiShowCandidate(ic);
        return PAGEDOWN_KEY;
    }

    if (isMatchKeys(key, modifier, CancelKeys)) {
        doClose();
        return CANCEL_KEY;
    }

    if (key >= '0' && key <= '9') {
        int i = key - 0x30;
        doCommit(i, callback);
        guiShowCandidate(ic);
        return COMMIT_KEY;
    }

    if ((key & 0xff) == (XK_BackSpace & 0xff)) {
        if (m_input.empty()) return  FORWARD_KEY;
        doInput(key);
        guiShowCandidate(ic);
        return COMMIT_KEY;
    }

    /* Must at the end.*/
    if (key >= 'a' && key <= 'z') {
        doInput(key);
        guiShowCandidate(ic);
        return CONVERT_KEY;
    }

    return FORWARD_KEY;
}
