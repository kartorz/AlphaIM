/**
 *    @Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#include "Application.h"
#include "X11IMPreedit.h"
#include "TaskManager.h"
#include "PY.h"
#include "Util.h"
#include "Configure.h"
#include "DBusDaemon.h"
#include <boost/filesystem.hpp>
#include <X11/extensions/Xinerama.h>
using namespace boost::filesystem;

Application*  gApp = NULL;

// loop time is 30m.
void SlowJob::doWork()
{
    //printf("slowjob dowork\n");
    m_owner->slowJob();
}

void Application::getDpySize()
{
    Display* dpy;
    if ((dpy = XOpenDisplay(NULL)) == NULL) {
        logger.e("getDpySize: Can't Open Display:\n");
        return;
    }
    int defscrn = DefaultScreen(dpy);
    IC::screens.clear();
    if (XineramaIsActive(dpy)) {
        int count = 0;
        XineramaScreenInfo *info = XineramaQueryScreens(dpy, &count);
        for (int i = 0; i < count; i++) {
            IC::screens.push_back({info[i].x_org, info[i].y_org,
                                   info[i].width, info[i].height});
            logger.i("DpySize: multi-screen, (%d,%d,%d,%d)\n",
                     info[i].x_org, info[i].y_org, info[i].width, info[i].height);
        }
        if (info != NULL)
            XFree(info);
    }
    if (IC::screens.empty()) {
        int w  = DisplayWidth(dpy, defscrn);
        int h  = DisplayHeight(dpy, defscrn);
        IC::screens.push_back({0, 0, w, h});
        logger.i("DpySize: (%d, %d)\n", w, h);
    }
}

Application::Application()
{
//    char *locale = "C,POSIX,POSIX,en_US.utf8"
    m_sysMsgr = new SysMessager();

    TaskManager::getInstance()->start(MAX_WORK_THREAD);
    m_sysMsgr->start();

    TaskManager::getInstance()->addTask(new SlowJob(this), 0);
    py.initialization();

    getDpySize();

    icm.init();

    X11IMPreedit::setTriggerKeys();

    logger.d("start Application ...\n");
}

iIM*  Application::curIM()
{
    return &py;
}

MessageQueue* Application::getMessageQ()
{
    return m_sysMsgr->m_msgQ;
}

void Application::slowJob()
{
    Configure::getRefrence().writeXml();
}

Application::~Application()
{
    //logger.d("~Application\n");
}
