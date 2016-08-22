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

Application  gApp;

Application::Application()
{
//    char *locale = "C,POSIX,POSIX,en_US.utf8"
    pGuiMsgQ = new MessageQueue("gui");
    pSysMsgQ = new MessageQueue("sys");
    m_sysMsgr = new SysMessager(pSysMsgQ);

    TaskManager::getInstance()->start(MAX_WORK_THREAD);
    m_sysMsgr->start();
}

Application::~Application()
{
    delete m_sysMsgr;
    delete pGuiMsgQ;
    delete pSysMsgQ;
}
