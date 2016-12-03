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
   string phPath = home_dir + "/phrase-utf8.imdb";
   if (!Util::isFileExist(phPath)) {
       log.d("copy phrase db to home\n");
       string phPathOri = system_dir + "/phrase-utf8.imdb";
       copy_file(phPathOri, phPath, copy_option::overwrite_if_exists);
   }

   string hanPath = home_dir + "/han-utf8.imdb";
   if (!Util::isFileExist(hanPath)) {
       log.d("copy han db to home\n");
       string hanPathOri = system_dir + "/han-utf8.imdb";
       copy_file(hanPathOri, hanPath, copy_option::overwrite_if_exists);
       //permissions(file_path, add_perms|owner_write|group_write|others_write);
   }

   string usrPhPath = home_dir + "/user_phrase-utf8.imdb";
   string pyPath = system_dir + "/pinyin-utf8.imdb";
   return (new PY(pyPath, phPath, usrPhPath, hanPath));
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
