#ifndef _GTKIMPREEDIT_H_
#define _GTKIMPREEDIT_H_

#include "QtIMPreedit.h"

class GtkIMPreedit : public QtIMPreedit {
public:
    GtkIMPreedit(ICManager* icm);
    virtual ~GtkIMPreedit() {}
};

#endif
