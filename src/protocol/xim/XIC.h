/** 
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#ifndef _XIC_H_
#define _XIC_H_

#include <X11/Xlib.h>
#include <string>
#include <map>
#include <vector>

#include "Ximd/IMdkit.h"
#include "Ximd/Xi18n.h"
#include "iIM.h"
#include "X11IMPreedit.h"

#define MAX_PY_SLICE  50

using namespace  std;

// Preedit and Status Attribute
class StatusAttributes
{
public:
    StatusAttributes();
    ~StatusAttributes();
    void set(XICAttribute *xattr, int num);
    void get(XICAttribute *xattr, int num);

    virtual bool check(XICAttribute *xattr, bool r) { return false; }


    XRectangle	area;		    /* area */
    XRectangle	area_needed;	/* area needed */
    Colormap	cmap;		    /* colormap */
    CARD32	    foreground;	    /* foreground */
    CARD32	    background;	    /* background */
    Pixmap	    bg_pixmap;	    /* background pixmap */
    char	    *base_font; /* base font of fontset */
    CARD32	    line_space;	    /* line spacing */
    Cursor	    cursor;		    /* cursor */
};

class PreeditAttributes : public StatusAttributes {
public:
    virtual bool check(XICAttribute *xattr, bool r);
    XPoint		spot_location;
};

class  IC {
public:
    IC(int icid);
    void set(IMChangeICStruct *calldata);
    void get(IMChangeICStruct *calldata);

    CARD16	id;		/* ic id */
    INT32	input_style;	/* input style */
    Window	client_win;	/* client window */
    Window	focus_win;	/* focus window */
    char	*resource_name;	/* resource name */
    char	*resource_class; /* resource class */
    PreeditAttributes pre_attr; /* preedit attributes */
    StatusAttributes sts_attr; /* status attributes */
    X11IMPreedit  preedit;
};

class Xicm {
public:
    Xicm();
    ~Xicm();

    int createIC(IMChangeICStruct *calldata, iIM *im);
    int setICValues(IMChangeICStruct *calldata);
    int getICValues(IMChangeICStruct *calldata);
    int setICFocus(IMChangeFocusStruct *calldata);
    int unsetICFocus(IMChangeFocusStruct *calldata);
    int resetICFocus(IMChangeFocusStruct *calldata);
    void closeIC(int focus = -1, bool bReset = false);
    IC* getIC(int focus = -1);

private:
    std::map<int, IC*> m_ics;
    int m_icid;
    int m_icFocus;
};

#endif
