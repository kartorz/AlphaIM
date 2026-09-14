#ifndef _DIC_H_
#define _DIC_H_

#include "IC.h"
#include "X11IMPreedit.h"

enum DbusClient {
    DBUS_CLIENT_UNKNOWN = 0,
    DBUS_CLIENT_QT,
    DBUS_CLIENT_GTK,
};

class DIC : public IC {
public:
    DIC(DbusClient client = DBUS_CLIENT_UNKNOWN) {
        m_client = client;
    }
    virtual ~DIC() {}

    virtual IMPreedit *createPreedit() {
        preedit = new X11IMPreedit();
        return preedit;
    }
private:
    DbusClient m_client;
};

#endif
