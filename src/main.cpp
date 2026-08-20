#include <boost/locale/utf.hpp>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cerrno>
#include <cstdio>
#include <getopt.h>
#include <unistd.h>

#include "aim.h"
#include "Application.h"
#include "Util.h"
#include "Configure.h"
#include "DBusDaemon.h"

extern void init_singals();

static void usage(char* prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("Options:\n");
    printf("  -f        run at foreground\n");
    printf("  -l <int>  set log level (0=d,1=w,2=e,3=i,4=none)\n");
    printf("  -v        show version\n");
    printf("  -h        show this help\n");
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
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

static void parse_arg(int argc, char* argv[], bool *d, bool *x, LogLevel *l)
{
    int option;
    while ((option = getopt(argc, argv, "fxhl:v")) != -1) {
        switch (option) {
        case 'f':
            *d = false;
            break;
        case 'x':
            *x = true;
            break;
        case 'l': {
            char *end = NULL;
            long value = strtol(optarg, &end, 10);
            if (end == optarg || *end != '\0' ||
                value < LOG_DEBUG || value > LOG_NONE) {
                fprintf(stderr, "Invalid log level: %s\n", optarg);
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            *l = static_cast<LogLevel>(value);
            break;
        }
        case 'v':
            printf("version: %s\n", VERSION);
            exit(EXIT_SUCCESS);
        case 'h':
        case '?':
        default:
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind != argc) {
        fprintf(stderr, "Unexpected argument: %s\n", argv[optind]);
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[])
{
    bool daemon = true;
    bool xim = false;
    LogLevel log_level = LOG_INFO;

    parse_arg(argc, argv, &daemon, &xim, &log_level);
    logger.setLevel(log_level);
    if (daemon) run_as_daemon();

    if (DBusDaemon::getRefrence().setup() != 0) {
        logger.e("Can't register dbus daemon\n");
        return -1;
    }

    Configure::getRefrence().initialization();

    gApp = new Application();

    DBusDaemon::getRefrence().start();

    if (xim) {
        gApp->xim.open();
        gApp->xim.eventLoop();
    }

    DBusDaemon::getRefrence().waitThreadExit();
}
