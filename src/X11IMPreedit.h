#ifndef _X11IMPREEDIT_H_
#define _X11IMPREEDIT_H_

#include "IMPreedit.h"

class X11IMPreedit : public IMPreedit {
public:
    X11IMPreedit();
    static void setTriggerKeys();
    virtual ~X11IMPreedit(){}
    int doHandleKey(IC* ic, u32 key, u32 modifier, IMPreeditCallback *callback);
    bool isMatchKeys(int key, int modifier, TriggerKey *trigger);
    bool imWants(u32 key, u32 modifier);

protected:
    bool isModifier(u32 keysym);

    int m_preModKey;
    int m_preRetKey;
};

extern TriggerKey ForwardKeys[];
#endif
