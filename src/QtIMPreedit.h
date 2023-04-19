#ifndef _QTIMPREEDIT_H_
#define _QTIMPREEDIT_H_

#include "X11IMPreedit.h"

class QtIMPreedit : public X11IMPreedit {
public:
    QtIMPreedit();
    virtual ~QtIMPreedit() {}
    virtual int handleKey(unsigned int keyval, unsigned int keycode, unsigned int state, IMPreeditCallback *callback);

private:
    unsigned int  m_preMask;
};

#endif
