#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "Signal.h"

static void signal_handler(int sig)
{
    if (sig == SIGHUP) Signal::panic("FATAL: Program hanged up\n");

    if (sig == SIGSEGV || sig == SIGBUS)
    {
        //Signal::dumpstack();
        Signal::panic("FATAL: %s Fault. Logged StackTrace\n", (sig == SIGSEGV) ? "Segmentation" : ((sig == SIGBUS) ? "Bus" : "Unknown"));
    }

    if (sig == SIGQUIT) Signal::panic("QUIT signal ended program\n");
    if (sig == SIGKILL) Signal::panic("KILL signal ended program\n");
    if (sig == SIGINT) exit(0);
}

void Signal::dumpstack(void)
{
    #define PROGNAME   "AlphaIM"
    char dbx[160];

//    snprintf(dbx, 160,  "echo 'where\ndetach' | dbx -a %d > %s.dump", getpid(), PROGNAME);
//    snprintf(dbx, 160,  "gdb -q -p %d", getpid());

//    system(dbx);
    return;
}

void Signal::panic(const char *fmt, ...)
{
    char buf[50];
    va_list argptr;
    va_start(argptr, fmt);
    vsnprintf(buf, 50, fmt, argptr);
    va_end(argptr);
    fprintf(stderr, buf);
    exit(-1);
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
