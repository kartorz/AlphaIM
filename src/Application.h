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
#include "XIMSrv.h"
#include "QIMSrv.h"
#include "Log.h"
#include "MessageQueue.h"
#include "SysMessager.h"
#include "Signal.h"
#include "iIM.h"
#include "PY.h"

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
    iIM* curIM();
    void slowJob();
	MessageQueue* getMessageQ();

    XIMSrv  xim;
	QIMSrv  qim;
    Signal  sig;

private:
    SysMessager  *m_sysMsgr;
	PY py;
};

extern Application*  gApp;
#endif

