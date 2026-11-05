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
#include <vector>

#include "IMPreedit.h"

#define ICWIN_W  656
#define ICWIN_H  64

using namespace  std;

struct ScreenRect {
    int x, y, w, h;
    int scale = 1;
};

class IC {
public:

    IC();
    virtual ~IC();

    virtual void enable();
    virtual void disable();
    virtual bool isEnabled();
    virtual void setCursorLocation(int x, int y, int w=-1, int h=-1);
    virtual void reset();
    virtual void setCapabilities(u32 cpas);
    virtual void propertyActivate(const char *name, int state);
    virtual void setSurroundingText();
    virtual void close();
    virtual ICRect getRect();

    u32 id;
    ICRect cursor;
    IMPreedit *preedit;

    static vector<ScreenRect> screens;
    static int screenScale;
    static int nativeToLogical(int position, int scale);
    static int screenScaleAt(int nativeX, int nativeY);
};

class ICManager {
public:
    ICManager();
    ~ICManager();
    void init();
    u32 add(IC *ic, const char *app);
    void destroy(u32 id);
    void focusIn(u32 id);
    int focusOut(u32 id);
    int focusId();
    void setMode(int mode);
    int mode();
    IC* get(u32 id);
    IC* getFocus();
    void handleUIMessage(int msg, int arg = 0);

    IC* operator[](u32 id) {
        return get(id);
    }

private:
    u32 add(IC *ic);
    void releasePreedit(IMPreedit *preedit);

    map<int,  IC*> m_ics;
    map<string, IMPreedit*> m_preedits;
    u32  m_icid;
    u32  m_icFocus;
    int  m_icMode;
    MutexCriticalSection m_cs;
};

#endif
