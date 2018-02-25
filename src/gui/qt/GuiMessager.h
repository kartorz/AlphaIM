#ifndef _GUIMESSAGER_H_
#define _GUIMESSAGER_H_

#include <QtCore/QThread>

#include "MessageQueue.h"

class AimWin;

class GuiMessager: public QObject
{
    Q_OBJECT

public slots:
    void doWork();
    void onExit();

public:
    GuiMessager(AimWin* owner, MessageQueue* q);
    ~GuiMessager();

    void start();
    void abort();

protected:
    AimWin* m_owner;
    QThread *m_thread;
    int m_reqAbort;

private:

    MessageQueue* m_msgQ;
};

#endif
