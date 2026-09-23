/**
  * --- GTK -----------------
  * One IC pre input
  * focus a little chaos, click at a input
  *   :43 focus in  <-- focus
  *   :43 reset
  *   :43 cursor, 43 cursor: twice
  *   :42 focus out, : from
  * --- QT  -----------------
  * One IC pre win, click at a input
  *    39 focus in, 39 focus in:  continuous twice
  *    39 cursor, 38 cursor: twice
  *     1 focusout: from
  */

#include "DBusICSrv.h"
#include "Application.h"
#include "DBusDaemon.h"
#include "Configure.h"

int ic_process_keyevent(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    u32 ic;
    int i;
    u32 u1, u2, u3, u4 ;
    /* Read the parameters */
    sd_bus_message_read(m, "uuuu", &ic, &u1, &u2, &u3);
    i = gApp->dim.procKeyEvent(ic, u1, u2, u3);
    return sd_bus_reply_method_return(m, "i", i);
}

int ic_set_cursorlocation(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    u32 ic;
    int i1, i2, i3, i4;
    sd_bus_message_read(m, "uiiii", &ic, &i1, &i2, &i3, &i4);
    gApp->dim.icm[ic]->setCursorLocation(i1, i2, i3, i4);
    PRINTF("%u: cursor (%d,%d,%d,%d)\n", ic, i1,i2,i3,i4);
    return sd_bus_reply_method_return(m, "i", 0);
}

int ic_focusin(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    u32 u;
    sd_bus_message_read(m, "u", &u);
    int active = gApp->dim.icm[u]->preedit->isActive();
    gApp->dim.icm.focusIn(u);
    PRINTF("%u: ic_focusin\n", u);
    return sd_bus_reply_method_return(m, "i", active);
}

int ic_focusout(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    u32 ic;
    sd_bus_message_read(m, "u", &ic);
    gApp->dim.icm.focusOut(ic);
    PRINTF("%u: ic_focusout\n", ic);
    return sd_bus_reply_method_return(m, "i", 0);
}

/**
 * GTK
 *   :1.200  focus in  <-- focus
 *   :1.200  reset
 */
int ic_reset(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    u32 ic;
    sd_bus_message_read(m, "u", &ic);
    PRINTF("%u: ic_reset\n", ic);
    gApp->dim.icm[ic]->reset();
    return sd_bus_reply_method_return(m, "i", 0);
}

int ic_enable(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    u32 ic; sd_bus_message_read(m, "u", &ic);
    //PRINTF("ic_enable\n");
    gApp->dim.icm[ic]->enable();
    return sd_bus_reply_method_return(m, "i", 0);
}

int ic_disable(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    u32 ic; sd_bus_message_read(m, "u", &ic);
    //PRINTF("ic_disable\n");
    gApp->dim.icm[ic]->disable();
    return sd_bus_reply_method_return(m, "i", 0);
}

int ic_is_enabled(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    u32 ic; sd_bus_message_read(m, "u", &ic);
    //PRINTF("ic_is_enabled\n");
    bool b = gApp->dim.icm[ic]->isEnabled();
    return sd_bus_reply_method_return(m, "b", b);
}

int ic_set_capabilities(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    u32 ic, u;
    sd_bus_message_read(m, "uu", &ic, &u);
    gApp->dim.icm[ic]->setCapabilities(u);
    return sd_bus_reply_method_return(m, "i", 0);
}

int ic_property_activate(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    const char *s;
    u32 ic;
    int i;
    sd_bus_message_read(m, "usi", &ic, &s, &i);
    gApp->dim.icm[ic]->propertyActivate(s, i);
    return sd_bus_reply_method_return(m, "i", 0);
}

int ic_set_surroundingtext(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    //u32 ic; sd_bus_message_read(m, "u", &ic);
    //PRINTF("ic_set_surroundingtext\n");
    return sd_bus_reply_method_return(m, "i",  0);
}

int ic_destroy(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    u32 ic;
    sd_bus_message_read(m, "u", &ic);
    //PRINTF("ic_destroy\n");
    gApp->dim.icm.destroy(ic);
    return sd_bus_reply_method_return(m, "i", 0);
}

DbusClient DBusICSrv::getClient(const char *name)
{
    if (name == NULL ||
        (strcmp(name, AIM_GTK_IC_NAME) == 0))
        return DBUS_CLIENT_GTK;

    if (strcmp(name, AIM_QT_IC_NAME) == 0) {
        return DBUS_CLIENT_QT;
    }

    return DBUS_CLIENT_UNKNOWN;
}

/* The higest bit indict im activate if im state being global
 * Or using im_ping.
 */
u32 DBusICSrv::createIC(const char *name, const char *sender)
{
    PRINTF("DIM:createIC, (%s,%s)\n", name, sender);
    IC *ic = new DIC(getClient(name));
    return icm.add(ic, sender);
}

void DBusICSrv::onIMOff(void* priv)
{
}

void DBusICSrv::onCommit(void* priv, string candidate)
{
    DBusDaemon::getRefrence().dbusIMCommit(candidate);
}

/* Return:
  0: Non-used key fallback to widget.
 >0: Eat this key, give the im activate.
 */
int DBusICSrv::procKeyEvent(u32 id, u32 keyval, u32 keycode, u32 state)
{
    IC *ic = icm.get(id);
    int ret = handleKey(ic, keyval, state, this);
    PRINTF("DBusICSrv, %d-- key(0x%x,%u),state:0x%x, return:%d\n",
           id, keyval, keycode, state, ret);
    if (ret == FORWARD_KEY)
        return 0;
    return ic->preedit->isActive() + 1; // must>0
}

/*  ---------------  Qt  -----------------
 * 'w' longger press, lots of state pair 0x00 with 0x40000000
   { ProcessKeyEvent val: 0x77,  keycode:0x11,  state:0x0
     ProcessKeyEvent val: 0x77,  keycode:0x11,  state:0x40000000 }+

 * Shift_L
   ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x0
   ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x40000001

 * Shift_L + w --  long press
   ProcessKeyEvent val: 0xffe1, keycode:0x2a, state:0x0
 { ProcessKeyEvent val: 0x57,   keycode:0x11, state:0x1
   ProcessKeyEvent val: 0x57,   keycode:0x11, state:0x40000001 } +
   ProcessKeyEvent val: 0xffe1, keycode:0x2a, state:0x40000001

 * Ctrl + Shift
   ProcessKeyEvent val: 0xffe3,  keycode:0x1d,  state:0x0
   ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x4
   ProcessKeyEvent val: 0xffe3,  keycode:0x1d,  state:0x40000005
   ProcessKeyEvent val: 0xffe1,  keycode:0x2a,  state:0x40000001 // later, release 'Ctrl'

 * Ctrl + SPACE
   ProcessKeyEvent val: 0x20,  keycode:0x39,  state:0x4
   ProcessKeyEvent val: 0x20,  keycode:0x39,  state:0x40000004  // short press, no this event.
--------------------     Gtk  --------------------
 'w'  longger press, lots of state:0x00 and a 0x40000000
  { ProcessKeyEvent val: 0x77,  keycode:0x11,  state:0x0 } +   //key press.
  ProcessKeyEvent val: 0x77,  keycode:0x11,  state:0x40000000  //key release
*/
int DBusICSrv::handleKey(IC* ic, u32 key, u32 state, IMPreeditCallback *callback)
{
    u32 mask = state & 0xff;
    if ((state & AIM_RELEASE_MASK) == AIM_RELEASE_MASK)
        return ic->preedit->doHandleKey(ic, key, mask, callback);

    if (ic->preedit->imWants(key))
        return PREEDIT_KEY;

    return FORWARD_KEY;
}
