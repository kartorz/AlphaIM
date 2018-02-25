/**
 *    @Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#ifndef _DBUSICSRV_H_
#define _DBUSICSRV_H_

#include <systemd/sd-bus.h>
#include <string>

#include "IC.h"
#include "QtIMPreedit.h"
#include "GtkIMPreedit.h"

enum DbusClient {
    DBUS_CLIENT_UNKNOWN = 0,
    DBUS_CLIENT_QT,
    DBUS_CLIENT_GTK,
};

class DIC : public IC {
public:
    DIC(DbusClient client = DBUS_CLIENT_QT) {
        if (client == DBUS_CLIENT_GTK)
            preedit = new GtkIMPreedit();
        else
            preedit = new QtIMPreedit();
    }
    virtual ~DIC(){}
};

class DBusICSrv : public IMPreeditCallback {
public:
    DBusICSrv();
    ~DBusICSrv();
    void processFocusIn(unsigned int id);
    bool processKeyEvent(unsigned int keyval, unsigned int keycode, unsigned int state);
    DbusClient getCurrentClient(const char *name);
    unsigned int createIC(const char *name = AIM_GTK_IC_NAME);
    virtual void onIMOff(void* priv);
    virtual void onCommit(void* priv, string candidate);
    virtual ICRect onGetRect();
    IC* getFocus();
    ICManager icManager;
};

extern int ic_process_keyevent(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int ic_set_cursorlocation(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int ic_focusin(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int ic_focusout(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int ic_reset(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int ic_enable(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int ic_disable(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int ic_is_enabled(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int ic_set_capabilities(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int ic_property_activate(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int ic_set_surroundingtext(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int ic_destroy(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);

#endif
