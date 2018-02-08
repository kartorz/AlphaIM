#include <boost/locale/utf.hpp>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>

#include "aim.h"
#include "Application.h"
#include "Util.h"
#include "Configure.h"
#include "DBusDaemon.h"

extern void init_singals();

void cleanup(void)
{
    if (gApp) {
        log.d("app exit, cleanup in main\n");
		DBusDaemon::getRefrence().stop();
        gApp->xim.close();
    //#ifdef AL_DEBUG
        gApp->sig.cleanup();
    //#endif
        delete gApp;
    }
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
	if (DBusDaemon::getRefrence().setup() != 0) {
		log.e("Can't register dbus daemon\n");
		return -1;
	}

	DBusDaemon::getRefrence().start();

    Configure::getRefrence().initialization();

    gApp = new Application();
//#ifdef AL_DEBUG
    gApp->sig.init();
//#endif
	gApp->xim.open();
	gApp->xim.eventLoop();
}
