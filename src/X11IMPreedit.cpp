
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "Application.h"
#include "X11IMPreedit.h"
#include "Log.h"

#undef PRINTF
//#define PRINTF(fmt, args...)  printf(fmt, ##args)
#define PRINTF(fmt, args...)

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
    {XK_Shift_L, ControlMask, ControlMask},
	{XK_Control_L, ShiftMask, ShiftMask},
    {0L, 0L, 0L}
};

static TriggerKey CEPSwitchKeys[] = {
    {XK_Shift_R, ShiftMask, ShiftMask},
    {0L, 0L, 0L}
};

X11IMPreedit::X11IMPreedit():m_preModKey(0),m_preRetKey(0)
{
}

bool X11IMPreedit::isModifier(unsigned int keysym)
{
	if (keysym >= XK_Shift_L  && keysym <= XK_Hyper_R)
		return true;
	return false;
}


/*
 * There are some pains when comes to precess x11 keys.
 * 1) modifier + key (eg, Shit_L + w)
         kev type: 2 state 0 --> code ffe1 --> 0
		 kev type: 2 state 1 --> code 57 --> 57
		 {kev type: 2 state 1 --> code 57 --> 57}+
		 kev type: 3 state 1 --> code 57 --> 57
		 kev type: 3 state 1 --> code ffe1 --> 0
 * 2) ctrl + shift
        kev type 2 state: 0 code: ffe3  key: 0
        kev type 2 state: 4 code: ffe1  key: 0
        kev type 3 state: 5 code: ffe3  key: 0
        kev type 3 state: 1 code: ffe1  key: 0
 * 3) ASCII key (eg, 'w').
      kev  type: 2 state 0 code:77
     {kev type: 2 state 0 code:77 }+
     kev type: 3 state 0  code 77
 * 4) Forward key.
      Forward 'Release key event' not working.
 */
int X11IMPreedit::handleKey(unsigned int keysym, unsigned int modifier, char *key, int evtype, IMPreeditCallback *callback)
{
    //MutexLock lock(m_cs);
    if (isMatchKeys(keysym, modifier, ForwardKeys)
		|| ((keysym & 0xff) == (XK_BackSpace & 0xff) && m_input == "")){
        return FORWARD_KEY;
    }

    // Check Shift + key
    if (keysym == XK_Shift_L || keysym == XK_Shift_R) {
        if (evtype == KeyPress) {
            if (modifier != ControlMask) // by pass: Ctrl + shift
                return FORWARD_KEY;
        } else {
            //printf("m_preModKey == %d --> %d\n", m_preModKey, ShiftMask);
            if ((m_preModKey & ShiftMask) == ShiftMask) {
                m_preModKey = 0;
                return FORWARD_KEY;
            }
        }
    } else {
        //  #define KeyPress		2
        //  #define KeyRelease		3
        if (evtype == KeyRelease) {
            m_preModKey = modifier;
            return m_preRetKey;
        }
        //printf("m_preModKey == %d, current == %d\n", m_preModKey, modifier);
    }
    m_preRetKey = NONE_KEY;

	return doHandleKey(keysym,  modifier, *key,  callback);
}

int X11IMPreedit::doHandleKey(unsigned int keysym, unsigned int modifier, unsigned int key, IMPreeditCallback *callback)
{
	PRINTF("doHandleKey keysym(%u), modifier(%u), key(%u-->0x%x)\n", keysym, modifier, key, key);

    // Check OnOff
    if (isMatchKeys(keysym, modifier, OnOffKeys)) {
        if (!m_bTrigger) {
            m_bTrigger = true;
            m_bCN = true;
            m_bCNPun = true;
			m_bStart = false;
            guiAction(MSG_IM_ON);
            return TRIGGER_ON_KEY;
        }
        m_bTrigger = false;
		m_bCN = false;
		m_bCNPun = false;
        callback->onIMOff(callback->opaque);
        doClose();
        guiAction(MSG_IM_OFF);
        return TRIGGER_OFF_KEY;
    }
    if (!m_bTrigger) {
        m_preRetKey = FORWARD_KEY;
        return FORWARD_KEY;
    }
	PRINTF("doHandleKey has checked trigger\n");

    // Check  language 
    if (isMatchKeys(keysym, modifier, CESwitchKeys)) {
        doSwitchCE(callback);
        return SWITCH_CE_KEY;
    }
    if (!m_bCN) {
        m_preRetKey = FORWARD_KEY;printf("X11IMPreedit bCN false\n");
        return FORWARD_KEY;
    }

    if (isMatchKeys(keysym, modifier, CEPSwitchKeys)) {
        doSwitchCEPun();
        return SWITCH_CEP_KEY;
    }
	PRINTF("doHandleKey has checked language, m_bStart:%d\n", m_bStart);

    // Check if start
    if (!m_bStart) {
        if (modifier == 0  && isalpha(key)) {
            doInput(key);
            guiShowCandidate(callback);
            return CONVERT_KEY;
        }
		PRINTF("doHandleKey, not start, check Pun char, m_bCNPun:%d, key:%d\n", m_bCNPun, key);
        if (m_bCNPun) {
            string cnPun = mapCNPunToU8Str(key);
			if (cnPun == "" && isascii(key) && isgraph(key))  // forward key
				cnPun = key;
            if (cnPun != "") {
                callback->onCommit(callback->opaque, cnPun);
                return COMMIT_KEY;
            }
        }
		PRINTF("doHandleKey, not start, forward key\n");

        m_preRetKey = FORWARD_KEY;
        return FORWARD_KEY;
    }
	PRINTF("doHandleKey has checked starting\n");

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
	PRINTF("doHandleKey has checked some control keys.\n");

    if (modifier == 0 || modifier == 1) {
        if (isdigit(key)) {
			printf("is digit key\n");
            int i = key - 0x30;
            doCommit(i, callback);
            guiShowCandidate(callback);
            return COMMIT_KEY;
        }
		PRINTF("doHandleKey has checked digit key\n");

		//if (*key == (XK_BackSpace & 0xff))
        if ((keysym & 0xff) == (XK_BackSpace & 0xff)) {
            doInput(key);
			guiShowCandidate(callback);
			return COMMIT_KEY;
        }

		PRINTF("doHandleKey has checked backspace, now do input\n");
        // Must at the end. 
        if (isascii(key) && isgraph(key)/* 'Shift' returns true */) {
            doInput(key);
            guiShowCandidate(callback);
            return CONVERT_KEY;
        }
    }
    m_preRetKey = FORWARD_KEY;
    return FORWARD_KEY;
}

