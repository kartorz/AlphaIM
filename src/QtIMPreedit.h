#ifndef _QTIMPREEDIT_H_
#define _QTIMPREEDIT_H_

#include "X11IMPreedit.h"

class QtIMPreedit : public X11IMPreedit {
public:
    QtIMPreedit(ICManager* icm);
    virtual ~QtIMPreedit() {}
    virtual int handleKey(u32 ic, u32 keyval,u32 keycode, u32 state, IMPreeditCallback *callback);

private:
    u32  m_preMask;
};

#endif
