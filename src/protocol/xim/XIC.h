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
#include <vector>

#include "Ximd/IMdkit.h"
#include "Ximd/Xi18n.h"
#include "IC.h"

//#define MAX_PY_SLICE  50

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

class XIMIC : public IC {
public:
    XIMIC();
    virtual ~XIMIC();
    void set(IMChangeICStruct *calldata);
    void get(IMChangeICStruct *calldata);

    INT32	input_style;	/* input style */
    Window	client_win;	/* client window */
    Window	focus_win;	/* focus window */
    char	*resource_name;	/* resource name */
    char	*resource_class; /* resource class */
    PreeditAttributes pre_attr; /* preedit attributes */
    StatusAttributes sts_attr; /* status attributes */
};

#endif
