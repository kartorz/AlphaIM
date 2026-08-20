#ifndef _X11IMPREEDIT_H_
#define _X11IMPREEDIT_H_

#include "IMPreedit.h"

class X11IMPreedit : public IMPreedit {
public:
    X11IMPreedit(ICManager* icm);
    virtual ~X11IMPreedit(){}
    virtual int handleKey(u32 ic, u32 keycode, u32 modifier, char *key, int evtype, IMPreeditCallback *callback);

protected:
    int doHandleKey(u32 ic, u32 keysym, u32 modifier, u32 key, IMPreeditCallback *callback);

    bool isModifier(u32 keysym);

    int m_preModKey;
    int m_preRetKey;
};

extern TriggerKey ForwardKeys[];
#endif
