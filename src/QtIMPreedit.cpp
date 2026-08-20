
#include <X11/keysym.h>
#include "QtIMPreedit.h"

QtIMPreedit::QtIMPreedit(ICManager* icm):X11IMPreedit(icm)
{
}

/* 'w' longger press, lots of state pair 0x00 with 0x40000000
   { ProcessKeyEvent val: 0x77,  keycode:0x11,  state:0x0
     ProcessKeyEvent val: 0x77,  keycode:0x11,  state:0x40000000 }+
*/

/* Shift_L
   ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x0
   ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x40000001
*/

/* Shift_L + w  -- one shot
  ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x0
  ProcessKeyEvent val: 0x57,    keycode:0x11,  state:0x1
  ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x40000001
  ProcessKeyEvent val: 0x77,    keycode:0x11,  state:0x40000000
*/

/* Shift_L + w --  long press
   ProcessKeyEvent val: 0xffe1, keycode:0x2a, state:0x0
 { ProcessKeyEvent val: 0x57,   keycode:0x11, state:0x1
   ProcessKeyEvent val: 0x57,   keycode:0x11, state:0x40000001 }+
   ProcessKeyEvent val: 0xffe1, keycode:0x2a, state:0x40000001
*/

/* Ctrl + Shift
   ProcessKeyEvent val: 0xffe3,  keycode:0x1d,  state:0x0
   ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x4
   ProcessKeyEvent val: 0xffe3,  keycode:0x1d,  state:0x40000005
   ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x40000001 // later, release 'Ctrl'
*/

/* Ctrl + SPACE
   ProcessKeyEvent val: 0x20,  keycode:0x39,  state:0x4
   ProcessKeyEvent val: 0x20,  keycode:0x39,  state:0x40000004  // short press, no this event.
*/


int QtIMPreedit::handleKey(u32 ic, u32 keyval, u32 keycode, u32 state, IMPreeditCallback *callback)
{
    u32 mask = state & 0xff;
    if (isMatchKeys(keyval, mask, ForwardKeys)
        || ((keyval & 0xff) == (XK_BackSpace & 0xff) && m_input == "")){
        return FORWARD_KEY;
    }

    PRINTF("QtIMPreedit, key(0x%x), mod(0x%x), code(%u), state:0x%x, active:%d,return:%d\n",
           keyval, mask, keycode, state, m_bTrigger, m_preRetKey);

    if ((state & AIM_RELEASE_MASK) == AIM_RELEASE_MASK)
        return doHandleKey(ic, keyval, mask, keyval, callback);

    if (!m_bTrigger || !m_bCN)
        return FORWARD_KEY;

    return PREEDIT_KEY;
}
