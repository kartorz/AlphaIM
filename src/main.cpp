#include <boost/locale/utf.hpp>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>

#include "iIM.h"
#include "PY.h"

#include "aim.h"
#include "Application.h"
#include "AimWin.h"
#include "Util.h"
#include "Configure.h"

#ifdef GTK3
#include <gdk/gdkx.h>
#include "gui/gtk/AimApp.h"
#endif

std::string system_dir;
std::string home_dir;

extern void init_singals();

void cleanup(void)
{
    if (gApp) {
        log.d("app exit, cleanup in main\n");printf("cleanup\n");
        gApp->xim.close();
    //#ifdef AL_DEBUG
        gApp->sig.cleanup();
    //#endif
        delete gApp;
    }
}

static void gui_activate_callback(Window imwin, Display *dpy)
{
   //printf("joni:%x, %p\n", imwin, dpy);
   log.d("setIM and open\n");

   gApp->xim.setIM(gApp->newIM(), true);
   gApp->xim.open(dpy);
}

static void run_as_daemon()
{
    pid_t pid, sid;
    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
       we can exit the parent process. */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    /* Change the file mode mask */
    umask(0);
    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }
#if 0
    /* Change the current working directory */
    if ((chdir("/")) < 0) {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }
#endif
    /* Close out the standard file descriptors */
//#ifndef AL_DEBUG
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
//#endif
}

int main(int argc, char* argv[])
{
    run_as_daemon();
    atexit(cleanup);

    Configure::getRefrence().initialization();
    system_dir = Configure::getRefrence().m_dataDir;
    home_dir = Configure::getRefrence().m_homeDir;

    gApp = new Application();
//#ifdef AL_DEBUG
    gApp->sig.init();
//#endif

#ifdef GTK3
    return aim_app_main(gApp->pGuiMsgQ, gui_activate_callback, argc, argv);
#else
    return 0;
#endif
}
