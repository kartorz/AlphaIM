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

#ifdef GTK3
#include <gdk/gdkx.h>
#include "gui/gtk/AimApp.h"
#endif

std::string system_dir;
std::string home_dir;

extern void init_singals();

void cleanup(void)
{
#ifdef AL_DEBUG
    gApp.sig.cleanup();
#endif
    //gApp.xim.close();
}

static void gui_activate_callback(Window imwin, Display *dpy)
{
   //printf("joni:%x, %p\n", imwin, dpy);
   //const char *locale = "C,POSIX,POSIX,en_US.utf8,zh_CN.UTF-8";
   string pyPath = DATADIR;
   pyPath += "/pinyin-utf8.imdb";
   string phPath = DATADIR;
   phPath += "/phrase-utf8.imdb";
   string hanPath = DATADIR;
   hanPath += "/han-utf8.tdf";
 
   iIM *pyim = new PY(pyPath, phPath, hanPath);

   gApp.xim.setIM(pyim, true);
   gApp.xim.open(dpy);
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
    //close(STDIN_FILENO);
    //close(STDOUT_FILENO);
    //close(STDERR_FILENO);
}
int main(int argc, char* argv[])
{
    //iPY *pyDB = new PY();
    //pyDB->load("system/pinyin-utf8.imdb");

    //vector<string> items = pyDB->lookup("w")

    system_dir = DATADIR;
    home_dir = "~/.AlphaIM";
    run_as_daemon();
    atexit(cleanup);
//#ifdef AL_DEBUG
    gApp.sig.init();
//#endif

#ifdef GTK3
    return aim_app_main(gApp.pGuiMsgQ, gui_activate_callback, argc, argv);
#else
    return 0;
#endif
}
