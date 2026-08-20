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

#include "DIC.h"

class DBusICSrv : public IMPreeditCallback {
public:
    DBusICSrv();
    ~DBusICSrv();
    int  processKeyEvent(u32 id, u32 keyval, u32 keycode, u32 state);
    u32 createIC(const char *name, const char *sender);
    virtual void onIMOff(void* priv);
    virtual void onCommit(void* priv, string candidate);
    ICManager icm;
    static DbusClient getClient(const char *name);
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
