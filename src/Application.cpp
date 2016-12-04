/** 
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#include "Application.h"
#include "TaskManager.h"
#include "PY.h"
#include "Util.h"
#include "Configure.h"
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

Application*  gApp = NULL;

// loop time is 30m.
void SlowJob::doWork()
{
    //printf("slowjob dowork\n");
    m_owner->slowJob();
}

Application::Application()
{
//    char *locale = "C,POSIX,POSIX,en_US.utf8"
    pGuiMsgQ = new MessageQueue("gui");
    pSysMsgQ = new MessageQueue("sys");
    m_sysMsgr = new SysMessager(pSysMsgQ);

    TaskManager::getInstance()->start(MAX_WORK_THREAD);
    m_sysMsgr->start();

    TaskManager::getInstance()->addTask(new SlowJob(this), 0);

    log.d("start Application ...\n");
}

iIM* Application::newIM()
{
    //const char *locale = "C,POSIX,POSIX,en_US.utf8,zh_CN.UTF-8";

    PY *py = new PY(); // XimSrv will delete this.
    int ret = py->initialization();
    log.i("newIM:  result of PY init: %d\n", ret);
    return py;
}

void Application::slowJob()
{
    Configure::getRefrence().writeXml();
}

Application::~Application()
{
    log.d("~ Application start\n");
    delete m_sysMsgr;
    delete pGuiMsgQ;
    delete pSysMsgQ;
    log.d("~Application done\n");
}
