#ifndef _DIC_H_
#define _DIC_H_

#include "IC.h"
#include "GtkIMPreedit.h"

enum DbusClient {
    DBUS_CLIENT_UNKNOWN = 0,
    DBUS_CLIENT_QT,
    DBUS_CLIENT_GTK,
};

class DIC : public IC {
public:
    DIC(DbusClient client = DBUS_CLIENT_QT) {
        m_client = client;
    }
    virtual ~DIC(){}

    virtual IMPreedit *createPreedit(ICManager* icm) {
        if (m_client == DBUS_CLIENT_GTK)
            preedit = new GtkIMPreedit(icm);
        else
            preedit = new QtIMPreedit(icm);
        return preedit;
    }
private:
    DbusClient m_client;
};

#endif
