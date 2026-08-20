/**
 *    @Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#ifndef _IC_H_
#define _IC_H_

#include <map>
#include <string>

#include "IMPreedit.h"

#define ICWIN_W  768
#define ICWIN_H  64

using namespace  std;

class IC {
public:
    IC();
    virtual ~IC();

    virtual void enable();
    virtual void disable();
    virtual bool isEnabled();
    virtual void setCursorLocation(int x, int y, int w,  int h);
    virtual void reset();
    virtual void setCapabilities(u32 cpas);
    virtual void propertyActivate(const char *name, int state);
    virtual void setSurroundingText();
    virtual void close();
    // New and set 'IMPreedit *preedit'
    virtual IMPreedit *createPreedit(ICManager* icm);

    ICRect  getRect();

    static ICRect adjRect(int x, int y, int w, int h);

    u32 id;
    int   cursorX;
    int   cursorY;
    IMPreedit *preedit;

    static int dpyW;
    static int dpyH;
};

class ICManager {
public:
    ICManager();
    ~ICManager();

    u32 add(IC *ic);
    u32 add(IC *ic, const char *app);
    void add(IC *ic, int id);
    void destroy(u32 id = 0);
    void focusIn(u32 id = 0);
    void setMode(int mode);
    int mode() const;
    IC* get(u32 id = 0);
    int focusOut(u32 id = 0);

    IC* operator[](u32 id) {
        return get(id);
    }

private:
    map<int,  IC*> m_ics;
    map<string, IMPreedit*> m_preedits;
    u32  m_icid;
    u32  m_icFocus;
    int  m_icMode;
    //MutexCriticalSection m_cs;
};

#endif
