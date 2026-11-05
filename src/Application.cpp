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
#include <gdk/gdk.h>
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
    IC::screens.clear();
    IC::screenScale = 1;
    gdk_set_allowed_backends("x11");
    if (gdk_init_check(NULL, NULL)) {
        GdkDisplay *display = gdk_display_get_default();
        GdkScreen *screen = gdk_display_get_default_screen(display);
        IC::screenScale = gdk_window_get_scale_factor(gdk_screen_get_root_window(screen));
        for (int index = 0; index < gdk_display_get_n_monitors(display); ++index) {
            GdkRectangle workarea;
            GdkMonitor *monitor = gdk_display_get_monitor(display, index);
            gdk_monitor_get_workarea(monitor, &workarea);
            const int scale = gdk_monitor_get_scale_factor(monitor);
            IC::screens.push_back({workarea.x, workarea.y, workarea.width, workarea.height,
                                   scale});
            logger.i("DpySize: logical workarea=(%d,%d,%d,%d), scale=%d\n",
                     workarea.x, workarea.y, workarea.width, workarea.height, scale);
        }
        if (!IC::screens.empty())
            return;
    }

    Display* dpy;
    if ((dpy = XOpenDisplay(NULL)) == NULL) {
        logger.e("getDpySize: Can't Open Display:\n");
        return;
    }
    int defscrn = DefaultScreen(dpy);
    if (XineramaIsActive(dpy)) {
        const int sf = IC::screenScale;
        int count = 0;
        XineramaScreenInfo *info = XineramaQueryScreens(dpy, &count);
        for (int i = 0; i < count; i++) {
            IC::screens.push_back({IC::nativeToLogical(info[i].x_org, sf),
                                   IC::nativeToLogical(info[i].y_org, sf),
                                   info[i].width / sf,
                                   info[i].height / sf,
                                   sf});
            logger.i("DpySize: multi-screen, (%d,%d,%d,%d)\n",
                     info[i].x_org, info[i].y_org, info[i].width, info[i].height);
        }
        if (info != NULL)
            XFree(info);
    }
    if (IC::screens.empty()) {
        const int sf = IC::screenScale;
        int w  = DisplayWidth(dpy, defscrn) / sf;
        int h  = DisplayHeight(dpy, defscrn) / sf;
        IC::screens.push_back({0, 0, w, h, sf});
        logger.i("DpySize: (%d, %d)\n", w, h);
    }
    XCloseDisplay(dpy);
}

Application::Application()
{
//    char *locale = "C,POSIX,POSIX,en_US.utf8"
    m_sysMsgr = new SysMessager();

    TaskManager::getInstance()->start(MAX_WORK_THREAD);
    m_sysMsgr->start();

    TaskManager::getInstance()->addTask(new SlowJob(this), 1000);
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
    py.syncUserPhrases();
    Configure::getRefrence().writeXml();
}

Application::~Application()
{
    //logger.d("~Application\n");
}
