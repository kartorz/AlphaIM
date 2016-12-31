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

    static void panic(FILE* f, const char *fmt, ...);
    static void dumpstack(FILE* f);
    struct sigaction sigact;
};

#endif
