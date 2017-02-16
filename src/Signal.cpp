#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <execinfo.h>

#include "aim.h"
#include "Signal.h"

static void signal_handler(int sig)
{
    std::string path = home_dir + "/dump";
    FILE* f = fopen(path.c_str(), "w+");

    //if (sig == SIGHUP) Signal::panic("FATAL: Program hanged up\n");
    if (sig == SIGSEGV || sig == SIGBUS || sig == SIGHUP)
    {
        Signal::dumpstack(f);
        Signal::panic(f, "FATAL: %s Fault. Logged StackTrace\n", (sig == SIGSEGV) ? "Segmentation" : ((sig == SIGBUS) ? "Bus" : "Unknown"));
        fclose(f);

#ifdef AL_DEBUG
       char cmd[256];
       snprintf(cmd, 256,  "gdb -q -p %d", getpid());
       system(cmd);
#endif

    }

    if (sig == SIGQUIT) {
        Signal::panic(f, "QUIT signal ended program\n");
        fclose(f);
        exit(-1);
    }

    if (sig == SIGKILL) {
        Signal::panic(f, "KILL signal ended program\n");
        fclose(f);
        exit(-1);
    }

    if (sig == SIGINT) {
        fclose(f);
        exit(0);
    }

    fclose(f);
}

#if 0
void Signal::log(FILE* f, const char *msg, ...)
{
    // Can't use class Log -- that will be destroied.
    va_list args;
    va_start(args, msg);
    vfprintf(f, msg, args);
    va_end(args);
    fflush(f);
}
#endif

void Signal::dumpstack(FILE* f)
{
    #define PROGNAME   "AlphaIM"


    panic(f, "dumpstack\n");

    void* callstack[512];
    int frames = backtrace(callstack, 512);
    char** strs = backtrace_symbols(callstack, frames);
    for (int i = 0; i < frames; ++i) {
        printf("%s\n", strs[i]);
        panic(f,"%s\n", strs[i]);
    }
    free(strs);
//    snprintf(dbx, 160,  "echo 'where\ndetach' | dbx -a %d > %s.dump", getpid(), PROGNAME);
//    snprintf(dbx, 160,  "gdb -q -p %d", getpid());
    return;
}

void Signal::panic(FILE* f, const char *fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    vfprintf(f, fmt, argptr);
    va_end(argptr);
    fflush(f);
}

void Signal::init()
{
    sigact.sa_handler = signal_handler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, (struct sigaction *)NULL);

    sigaddset(&sigact.sa_mask, SIGSEGV);
    sigaction(SIGSEGV, &sigact, (struct sigaction *)NULL);

    sigaddset(&sigact.sa_mask, SIGBUS);
    sigaction(SIGBUS, &sigact, (struct sigaction *)NULL);

    sigaddset(&sigact.sa_mask, SIGQUIT);
    sigaction(SIGQUIT, &sigact, (struct sigaction *)NULL);

    sigaddset(&sigact.sa_mask, SIGHUP);
    sigaction(SIGHUP, &sigact, (struct sigaction *)NULL);

    sigaddset(&sigact.sa_mask, SIGKILL);
    sigaction(SIGKILL, &sigact, (struct sigaction *)NULL);
}

void Signal::cleanup()
{
    sigemptyset(&sigact.sa_mask);
}
