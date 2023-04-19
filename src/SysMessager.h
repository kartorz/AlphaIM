#ifndef _SYSMESSAGER_H_
#define _SYSMESSAGER_H_

#include "Thread.h"
#include "MessageQueue.h"

class SysMessager: public Thread
{
public:
    SysMessager(MessageQueue* queuq);
    SysMessager();
    virtual ~SysMessager();
    
    void processMessage();
    virtual void stop();

    MessageQueue* m_msgQ;

protected:
    virtual void doWork();
    virtual void onStartup();
    virtual void onExit();
};


#endif
