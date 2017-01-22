#include "GuiMessager.h"
#include "Application.h"
#include "Log.h"

GuiMessager::GuiMessager(AimWin* owner, MessageQueue* q)
:m_reqAbort(false)
{
    m_owner = owner;
    m_msgQ = q;
}

GuiMessager::~GuiMessager()
{
    if (!m_reqAbort) {
        abort();
    }
}

void GuiMessager::start()
{
    m_thread = new QThread();
    connect(m_thread, SIGNAL(started()), this, SLOT(doWork()));
    connect(m_thread, SIGNAL(finished()), this,SLOT(onExit()));
    //connect(m_thread, SIGNAL(terminated()), this, SLOT()
    moveToThread(m_thread);
    m_thread->start();
}

void GuiMessager::doWork()
{
    do{
        Message msg;
        if (m_msgQ->pop(msg)) {
            //printf("{GuiMessager} MSGID:%d\n", msg.id);
            switch (msg.id) {

            case MSG_IM_INPUT: {
                //QString
                QMetaObject::invokeMethod((QObject *)(m_owner),
                                          "on_messsager_impreedit",
                                          Qt::QueuedConnection,
                                          Q_ARG(int, msg.iArg1),
                                          Q_ARG(int, msg.iArg2),
                                          Q_ARG(int, (int)msg.fArg1),
                                          Q_ARG(int, (int)msg.fArg2),
                                          Q_ARG(std::string, msg.strArg1),
                                          Q_ARG(void*, msg.pArg1));
                break;
            }

            case MSG_IM_ON: {
                QMetaObject::invokeMethod((QObject *)m_owner, "onMesssagerIMOn", Qt::QueuedConnection);
                break;
            }

            case MSG_IM_OFF: {
                QMetaObject::invokeMethod((QObject *)m_owner, "onMesssagerIMOff", Qt::QueuedConnection);

                break;
            }

            case MSG_IM_CLOSE: {
                QMetaObject::invokeMethod((QObject *)m_owner, "onMesssagerIMClose", Qt::QueuedConnection);
            }

            case MSG_IM_COMMIT: {
                QMetaObject::invokeMethod((QObject *)m_owner, "onMesssagerIMCommit", Qt::QueuedConnection);
                break;
            }

            case MSG_IM_CN: {
                QMetaObject::invokeMethod((QObject *)m_owner, "onMesssagerIMLan", Qt::QueuedConnection, Q_ARG(bool, true));
                break;
            }

            case MSG_IM_EN: {
                QMetaObject::invokeMethod((QObject *)m_owner, "onMesssagerIMLan", Qt::QueuedConnection, Q_ARG(bool, false));
                break;
            }

            case MSG_IM_CPUN: {
                QMetaObject::invokeMethod((QObject *)m_owner, "onMesssagerIMPun", Qt::QueuedConnection, Q_ARG(bool, true));

                break;
            }

            case MSG_IM_EPUN: {
                QMetaObject::invokeMethod((QObject *)m_owner, "onMesssagerIMPun", Qt::QueuedConnection, Q_ARG(bool, false));
                break;
            }
                    
            case MSG_QUIT: {
                m_reqAbort = true;
                break;
            }

            default:
                break;
            }

        } else {
            m_reqAbort = true;
            //printf("{GuiMessager} no message eixt\n");
            break;
        }
    }while(!m_reqAbort);
}

void GuiMessager::abort()
{
	if(m_thread && m_thread->isRunning())
	{
	    m_reqAbort = true;
            m_msgQ->push(MSG_QUIT);
            //m_msgQ->unblockAll();
	    m_thread->quit();
	    m_thread->wait();
	}
}

void GuiMessager::onExit()
{
    //g_sysLog.d("GuiMessager::onExit\n");
}
