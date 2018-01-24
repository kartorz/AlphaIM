#ifndef _MESSAGEQUEUE_H_
#define _MESSAGEQUEUE_H_

#include "Thread.h"
#include "ThdCond.h"
#include "SpinLock.h"

#include <stddef.h>

#include <list>
#include <string>

#include <stdio.h>

class MessageQueue;

class PushMessageJob: public Thread
{
public:
    PushMessageJob(MessageQueue *q):Thread(),owner(q) {}
    virtual void doWork();
    virtual ~PushMessageJob(){}
    MessageQueue *owner;
};

/* [@1]:  specifies 'iArg1' dones't matter.
 * [@2]:  specifies Sender is responsible for the pointer pArg1.
 * [@3]:  specifies Receiver is responsible for the pointer pArg1
 */
class Message
{
public:
    Message()
    :id(-1), iArg1(0), iArg2(0),
     fArg1(0.0),fArg2(0.0),
     dArg1(0.0),dArg2(0.0),
     pArg1(NULL), pArg2(NULL),
     strArg1(""), strArg2(""),
     timeout(0) {
    }

    Message(const Message& m) {
        id = m.id;
        iArg1 = m.iArg1;
        iArg2 = m.iArg2;
        fArg1 = m.fArg1;
        fArg2 = m.fArg2;
        dArg1 = m.dArg1;
        dArg2 = m.dArg2;
        pArg1 = m.pArg1;
        pArg2 = m.pArg2;
        strArg1 = m.strArg1;
        strArg2 = m.strArg2;
        timeout = m.timeout;
    }

    ~Message() { }

	int id;
    int iArg1;
    int iArg2;
    float fArg1;
    float fArg2;
    double dArg1;
    double dArg2;
    void* pArg1;
    void* pArg2;
    std::string strArg1;
    std::string strArg2;
    int timeout;
};

class MessageQueue : public ThdCond
{
friend class PushMessageJob;
public:
    MessageQueue(std::string identi="");
    virtual ~MessageQueue();

    void push(Message& msg);
    void push(int id);
    void push(int id, int arg1, int arg2=0);
    void push(int id, float arg1, float arg2=0.0);
    void push(int id, double arg1, double arg2=0.00);
    void push(int id, std::string arg1, std::string arg2="");
    void push(int id, std::string& arg1, std::string& arg2);
    void push(int id, int arg1, void *arg2, void *arg3=NULL);
    void push(Message& msg, int timeout);

    bool pop(Message& msg, bool bwait=true);
    void flush();
    void flush(int id);

    void remove(int id);
protected:
    virtual void onConsume(void *v);
    virtual void onProduce(void *v);
    virtual bool canConsume(void *v) { return !m_queue.empty(); }

private:
    std::list<Message> m_queue;
    std::list<Message> m_queueTimeout;
    SpinCriticalSection m_qtcrs;
    ThdCond m_qtCond;
    SpinCriticalSection m_crs;
    std::string m_identify;
    PushMessageJob *m_pushDelayMsg;
};


#endif
