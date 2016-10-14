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

Application*  gApp = NULL;

Application::Application()
{
//    char *locale = "C,POSIX,POSIX,en_US.utf8"
    pGuiMsgQ = new MessageQueue("gui");
    pSysMsgQ = new MessageQueue("sys");
    m_sysMsgr = new SysMessager(pSysMsgQ);

    TaskManager::getInstance()->start(MAX_WORK_THREAD);
    m_sysMsgr->start();
    log.d("start Application ...\n");
}

iIM* Application::newIM()
{
    //const char *locale = "C,POSIX,POSIX,en_US.utf8,zh_CN.UTF-8";
   string pyPath = DATADIR;
   pyPath += "/pinyin-utf8.imdb";
   string phPath = DATADIR;
   phPath += "/phrase-utf8.imdb";
   string usrPhPath = home_dir + "/aim_phrase-utf8.imdb";
   string hanPath = DATADIR;
   hanPath += "/han-utf8.imdb";

   return (new PY(pyPath, phPath, usrPhPath, hanPath));
}

Application::~Application()
{
    log.d("~ Application start\n");
    delete m_sysMsgr;
    delete pGuiMsgQ;
    delete pSysMsgQ;
    log.d("~Application done\n");
}
