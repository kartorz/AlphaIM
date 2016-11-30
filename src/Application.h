/** 
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "config.h"
#include "XimSrv.h"
#include "Log.h"
#include "MessageQueue.h"
#include "SysMessager.h"
#include "Signal.h"
#include "iIM.h"

class Application;

class SlowJob: public Task
{
public:
    SlowJob(Application *owner):m_owner(owner),Task(1000*60*30, true, NULL) {}
    virtual void doWork();
    virtual ~SlowJob(){}
private:
    Application* m_owner;
};

class Application
{
friend class SlowJob;
public:
    Application();
    ~Application();
    iIM* newIM();
    void slowJob();

    XimSrv  xim;
    MessageQueue *pGuiMsgQ; // Message being send to gui
    MessageQueue *pSysMsgQ; // Message being send to sys
    Signal  sig;

private:
    SysMessager  *m_sysMsgr;
};

extern Application*  gApp;
#endif

