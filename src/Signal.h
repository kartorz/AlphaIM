#ifndef _SIGNAL_H_
#define _SIGNAL_H_
#include <signal.h>

class Signal
{
public:
    Signal() {}
    ~Signal() {}

    void init();
    void cleanup();

    static void panic(const char *fmt, ...);
    static void dumpstack();
    struct sigaction sigact;
};

#endif
