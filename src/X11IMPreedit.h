#ifndef _X11IMPREEDIT_H_
#define _X11IMPREEDIT_H_

#include "IMPreedit.h"

class X11IMPreedit : public IMPreedit {
public:
    X11IMPreedit();
    virtual ~X11IMPreedit() {};
    virtual int  handleKey(int keycode, int modifier, char *key,  int evtype, IMPreeditCallback *callback);

private:
    u32  mapCNPun(char *key);
    int m_preModKey;
    int m_preRetKey;
};

#endif
