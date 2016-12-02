#include "Application.h"
#include "SysMessager.h"
#include "Log.h"

SysMessager::SysMessager(MessageQueue* queue): Thread(0)
{
    m_msgQ = queue;
}

SysMessager::~SysMessager()
{
    if (!m_stop) {
        stop();
    }
}

void SysMessager::stop()
{
    m_msgQ->push(MSG_QUIT);
    Thread::stop();
}

void SysMessager::onStartup()
{
}

void SysMessager::onExit()
{
    log(LOG_DEBUG, "SysMessager: onExit()\n");
}

void SysMessager::doWork()
{
    processMessage();
}

void SysMessager::processMessage()
{
    Message msg;
    bool ret = m_msgQ->pop(msg);
    if (ret == false) {
        //printf("{SysMessager} no message, exit\n");
        log(LOG_ERROR,"SysMessager: ret == false, exit\n");
        return;
    }
    //log(LOG_DEBUG,"SysMessager: processMessage() id:%d\n", msg.id);
    if (msg.id >= MSG_UI_LAN  && msg.id <= MSG_UI_PUN)
        gApp->xim.handleUIMessage(msg.id);
    else {
        switch (msg.id) {
	    case MSG_QUIT: {
            abort();
            break;
        }

        default:
            break;
        }
    }
	//printf("Message done\n");
}
