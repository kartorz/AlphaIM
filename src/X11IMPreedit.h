#ifndef _X11IMPREEDIT_H_
#define _X11IMPREEDIT_H_

#include "IMPreedit.h"

class X11IMPreedit : public IMPreedit {
public:
    X11IMPreedit();
    virtual ~X11IMPreedit(){}
    virtual int handleKey(unsigned int keycode, unsigned int modifier, char *key, int evtype, IMPreeditCallback *callback);

protected:
    int doHandleKey(unsigned int keysym, unsigned int modifier, unsigned int key, IMPreeditCallback *callback);

    bool isModifier(unsigned int keysym);

    int m_preModKey;
    int m_preRetKey;
};

#endif
