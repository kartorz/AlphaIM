
#include <X11/keysym.h>
#include "QtIMPreedit.h"
//#include "Log.h"

/* Forward Keys List */
static TriggerKey ForwardKeys[] = {
    {XK_Tab, 0, 0},
    {0L, 0L, 0L}
};

QtIMPreedit::QtIMPreedit()
{
}

/* 'w'
   { ProcessKeyEvent val: 0x77,  keycode:0x11,  state:0x0
     ProcessKeyEvent val: 0x77,  keycode:0x11,  state:0x40000000 }+
*/

/* Shift_L
   ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x0
   ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x40000001
*/

/* Shift_L + w  -- one shot
  ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x0 
  ProcessKeyEvent val: 0x57,  keycode:0x11,  state:0x1
  ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x40000001 
  ProcessKeyEvent val: 0x77,  keycode:0x11,  state:0x40000000
*/

/* Shift_L + w --  long press
   ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x0
   {ProcessKeyEvent val: 0x57,  keycode:0x11,  state:0x1
   ProcessKeyEvent val: 0x57,  keycode:0x11,  state:0x40000001}+
   ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x40000001 
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


int QtIMPreedit::handleKey(unsigned int keyval, unsigned int keycode, unsigned int state, IMPreeditCallback *callback)
{
	unsigned int mask = state & 0xff;

    if (isMatchKeys(keyval, mask, ForwardKeys)
		|| ((keyval & 0xff) == (XK_BackSpace & 0xff) && m_input == "")){
        return FORWARD_KEY;
    }

	//printf("QtIMPreedit, doHandleKey keysym(%u), modifier(%u), key(%u)\n", keyval, mask, keyval);

	if ((state & 0x40000000) == 0) { // Press key
		m_preRetKey = doHandleKey(keyval, mask, keyval, callback);
		return NONE_KEY;
	} else {
		return m_preRetKey;
	}
}

