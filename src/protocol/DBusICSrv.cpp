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
    i = gApp->dim.processKeyEvent(ic, u1, u2, u3);
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
    u32 ic; sd_bus_message_read(m, "u", &ic);
    //PRINTF("ic_destroy\n");
    gApp->dim.icm.destroy(ic);
    return sd_bus_reply_method_return(m, "i", 0);
}

DBusICSrv::DBusICSrv()
{
}

DBusICSrv::~DBusICSrv()
{
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

/* return:
  0: Non-used key fallback to widget.
 >0: Eat this key, give the im activate.
 */
int DBusICSrv::processKeyEvent(u32 ic, u32 keyval, u32 keycode, u32 state)
{
    int key = icm.get(ic)->preedit->handleKey(ic, keyval, keycode, state, this);
    PRINTF("DBusICSrv, KeyEvent ic:%d, key:0x%x, state:0x%x, return:%d\n", ic, keyval,state,key);
    if (key == FORWARD_KEY)
        return 0;
    return icm.get(ic)->preedit->isActive() + 1; // must>0
}
