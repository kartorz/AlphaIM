#include "DBusICSrv.h"
#include "Application.h"
#include "DBusDaemon.h"

int ic_process_keyevent(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    bool b;
    unsigned int u1, u2, u3, u4 ;
    /* Read the parameters */
    sd_bus_message_read(m, "uuu", &u1, &u2, &u3);
    b = gApp->dim.processKeyEvent(u1, u2, u3);
    return sd_bus_reply_method_return(m, "b", b);
}

int ic_set_cursorlocation(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    int i1, i2, i3, i4;
    sd_bus_message_read(m, "iiii", &i1, &i2, &i3, &i4);
    gApp->dim.getFocus()->setCursorLocation(i1, i2, i3, i4);
    PRINTF("ic_set_cursorlocation, %d, %d, %d, %d\n", i1, i2, i3, i4);
    return sd_bus_reply_method_return(m, NULL);
}

int ic_focusin(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    unsigned int u;
    sd_bus_message_read(m, "u", &u);
    PRINTF("ic_focusin, %u\n", u);
    gApp->dim.processFocusIn(u);
    return sd_bus_reply_method_return(m, NULL);
}

int ic_focusout(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    PRINTF("ic_focusout\n");
    gApp->dim.icManager.focusOut();
    return sd_bus_reply_method_return(m, NULL);
}

int ic_reset(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    PRINTF("ic_reset\n");
    gApp->dim.getFocus()->reset();
    return sd_bus_reply_method_return(m, NULL);
}

int ic_enable(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    PRINTF("ic_enable\n");
    gApp->dim.getFocus()->enable();
    return sd_bus_reply_method_return(m, NULL);
}

int ic_disable(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    PRINTF("ic_disable\n");
    gApp->dim.getFocus()->disable();
    return sd_bus_reply_method_return(m, NULL);
}

int ic_is_enabled(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    PRINTF("ic_is_enabled\n");
    bool b = gApp->dim.getFocus()->isEnabled();
    return sd_bus_reply_method_return(m, "b", b);
}

int ic_set_capabilities(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    PRINTF("ic_set_capabilities\n");
    unsigned int u;
    sd_bus_message_read(m, "u", &u);
    gApp->dim.getFocus()->setCapabilities(u);
    return sd_bus_reply_method_return(m, NULL);
}

int ic_property_activate(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    PRINTF("ping\n");
    const char *s;
    int i;
    sd_bus_message_read(m, "si", &s, &i);
    gApp->dim.getFocus()->propertyActivate(s, i);
    return sd_bus_reply_method_return(m, NULL);
}

int ic_set_surroundingtext(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    PRINTF("ic_set_surroundingtext\n");
    return sd_bus_reply_method_return(m, NULL);
}

int ic_destroy(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    PRINTF("ic_destroy\n");
    gApp->dim.icManager.destroy();
    return sd_bus_reply_method_return(m, NULL);
}

DBusICSrv::DBusICSrv()
{
    IC *ic = new DIC();  // Add a default IC.
    icManager.add(ic, 0);
}

DBusICSrv::~DBusICSrv()
{
}

DbusClient DBusICSrv::getCurrentClient(const char *name)
{
    if (name == NULL ||
        (strcmp(name, AIM_GTK_IC_NAME) == 0))
        return DBUS_CLIENT_GTK;

    if (strcmp(name, AIM_QT_IC_NAME) == 0) {
        return DBUS_CLIENT_QT;
    }

    return DBUS_CLIENT_UNKNOWN;
}

unsigned int DBusICSrv::createIC(const char *name)
{
    IC *ic = new DIC(getCurrentClient(name));
    return icManager.add(ic);
}

IC* DBusICSrv::getFocus()
{
    return icManager.get();
}

void DBusICSrv::onIMOff(void* priv)
{
}

void DBusICSrv::onCommit(void* priv, string candidate)
{
    //printf("DBusICSrv::onCommit %s\n", candidate.c_str());
    DBusDaemon::getRefrence().dbusIMCommit(candidate);
}

ICRect DBusICSrv::onGetRect()
{
    int x = getFocus()->cursorX;
    int y = getFocus()->cursorY + 40;
    return IC::adjRect(x, y, ICWIN_W, ICWIN_H);
}

void DBusICSrv::processFocusIn(unsigned int id)
{
    icManager.focusIn(id);

    IC *ic = icManager.get();
    if (ic->id > 0)
        ic->preedit->guiReload(this);
    else {
        IC *ic = new DIC();
        icManager.add(ic, id);
        PRINTF("DBusICSrv::processFocusIn, add exists id\n");
    }
}

bool DBusICSrv::processKeyEvent(unsigned int keyval, unsigned int keycode, unsigned int state)
{
    PRINTF("DBusICSrv::ProcessKeyEvent val:0x%x, code:0x%x, state:0x%x\n", keyval, keycode, state);
    if (icManager.get()->preedit->handleKey(keyval, keycode, state, this)  ==  FORWARD_KEY) {
        return false;
    }
    return true; // Eat this key.
}
