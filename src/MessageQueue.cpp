#include "Util.h"
#include "MessageQueue.h"

void PushMessageJob::doWork()
{
    //printf("PushMessageJob dowork\n");
    //log.d("slowjob dowork\n");
    if (owner->m_queueTimeout.size() == 0) {
        if (owner->m_qtCond.waitEvent() == -2)
            return;
    }

    //printf("PushMessageJob check\n");
    SpinLock m_lock(owner->m_qtcrs);
    std::list<Message>& Q = owner->m_queueTimeout;
    //printf("front timeout1: %x\n", Q.front().timeout);
    if (Q.front().timeout <=  Util::getTimeMS()) {
        printf("front timeout %d\n", Q.front().timeout);
        owner->push(Q.front());
        Q.pop_front();
    }
}

MessageQueue::MessageQueue(std::string identi)
:m_identify(identi)
{
   // m_pushDelayMsg = new PushMessageJob(this);
    //m_pushDelayMsg->start();
}

MessageQueue::~MessageQueue()
{
    m_qtCond.unblockAll(); // Must be before ...->stop
    printf("~MessageQueue\n");
    //m_pushDelayMsg->stop();

    //delete m_pushDelayMsg;
    flush();
}

void MessageQueue::remove(int id)
{
    do {
         SpinLock m_lock(m_qtcrs);
         std::list<Message>::iterator iter = m_queueTimeout.begin();
         for ( ; iter != m_queueTimeout.end();) {
             if ((*iter).id == id)
                 iter = m_queueTimeout.erase(iter);
             else
                 ++iter;
         }
    } while(0);

    do {
         SpinLock m_lock(m_crs);
         std::list<Message>::iterator iter = m_queue.begin();
         for ( ; iter != m_queue.end(); ++iter) {
             if ((*iter).id == id)
                 iter = m_queue.erase(iter);
             else
                 ++iter;
         }
    } while(0);
}

void MessageQueue::push(Message& msg, int timeout)
{
    SpinLock m_lock(m_qtcrs);
    msg.timeout = Util::getTimeMS() + timeout;

    std::list<Message>::iterator iter = m_queueTimeout.begin();
    for ( ; iter != m_queueTimeout.end(); ++iter) {

        if ((*iter).timeout >= msg.timeout) {
            m_queueTimeout.insert(iter, msg);
            m_qtCond.setEvent();
            return;
        }
    }
    m_queueTimeout.push_back(msg);
    m_qtCond.setEvent();
}

void MessageQueue::push(Message& msg)
{
    produce(&msg);
}

void MessageQueue::push(int id)
{
	Message msg;
    msg.id = id;
    produce(&msg);
}

void MessageQueue::push(int id, int arg1, int arg2)
{
	Message msg;
    msg.id = id;
    msg.iArg1 = arg1;
    msg.iArg2 = arg2;
    produce(&msg);
}

void MessageQueue::push(int id, float arg1, float arg2)
{
	Message msg;
    msg.id = id;
    msg.fArg1 = arg1;
    msg.fArg2 = arg2;
    produce(&msg);
}

void MessageQueue::push(int id, double arg1, double arg2)
{
	Message msg;
    msg.id = id;
    msg.dArg1 = arg1;
    msg.dArg2 = arg2;
    produce(&msg);
}

void MessageQueue::push(int id, std::string arg1, std::string arg2)
{
	Message msg;
    msg.id = id;
    msg.strArg1 = arg1;
    msg.strArg2 = arg2;
    produce(&msg);
}

void MessageQueue::push(int id, std::string& arg1, std::string& arg2)
{
	Message msg;
    msg.id = id;
    msg.strArg1 = arg1;
    msg.strArg2 = arg2;
    produce(&msg);
}

void MessageQueue::push(int id, int arg1, void *arg2, void *arg3)
{
    Message msg;
    msg.id = id;
    msg.iArg1 = arg1;
    msg.pArg1 = arg2;
    msg.pArg2 = arg3;
    produce(&msg);
}

void MessageQueue::onProduce(void *v)
{
    SpinLock m_lock(m_crs);
    if (v != NULL)
        m_queue.push_back(*((Message *)v));
}

bool MessageQueue::pop(Message& msg, bool bwait)
{
    bool ret = false;
    if (bwait) {
        ret = (consume(&msg) == 0);
    } else {
        SpinLock m_lock(m_crs);
    	if (!m_queue.empty()) {
            msg = m_queue.front();
            m_queue.pop_front();
            ret = true;
        }
    }
    //printf("pop a message(%s, %d)\n", m_identify.c_str(), ret);
    return ret;
}

void MessageQueue::onConsume(void* v)
{
    SpinLock m_lock(m_crs);
    //printf("onConsume(%s)\n", m_identify.c_str());
    // The queue may be flush in ~MessageQueue
    if (!m_queue.empty()) {
        *((Message *)v)  = m_queue.front();
        m_queue.pop_front();
    }
}

void MessageQueue::flush()
{
   SpinLock m_lock(m_crs);
   m_queue.clear();
}

void MessageQueue::flush(int id)
{
   SpinLock m_lock(m_crs);
   std::list<Message>::iterator it;
   for (it = m_queue.begin(); it != m_queue.end();) {       
       if (it->id == id) {
           it = m_queue.erase(it);
       } else {
           ++it;
       }
   }
}
