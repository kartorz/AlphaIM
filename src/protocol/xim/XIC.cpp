/** 
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#include <string.h>
#include <stdio.h>
#include <X11/keysym.h>

#include "aim.h"
#include "XIC.h"
#include "Application.h"

#undef PRINTF
//#define PRINTF(fmt, args...)  printf(fmt, ##args)
#define PRINTF(fmt, args...)

#define setxattr(A, T, V) do {    \
    A.value = (void *)malloc(sizeof(T));    \
    *(T *)A.value = V;                      \
    A.value_length = sizeof(T);             \
} while(0)

StatusAttributes::StatusAttributes()
{
    base_font = NULL;
    memset(&area, 0, sizeof(XRectangle));
    memset(&area_needed, 0, sizeof(XRectangle));
}

StatusAttributes::~StatusAttributes()
{
    if (base_font != NULL) {
        XFree(base_font);
    }
}

void StatusAttributes::set(XICAttribute *xattr, int num)
{
    for (int i = 0; i < num && xattr[i].name; i++) {
        //PRINTF("***********set xattr:name(%s)\n", xattr[i].name);
        if (!strncmp(XNArea, xattr[i].name, xattr[i].name_length))
            area = *(XRectangle *)xattr[i].value;
        else if (!strncmp(XNAreaNeeded, xattr[i].name, xattr[i].name_length))
            area_needed = *(XRectangle*)xattr[i].value;
        else if (!strncmp(XNColormap, xattr[i].name, xattr[i].name_length))
            cmap = *(Colormap*)xattr[i].value;
        else if (!strncmp(XNStdColormap, xattr[i].name, xattr[i].name_length))
            cmap = *(Colormap*)xattr[i].value;
        else if (!strncmp(XNForeground, xattr[i].name, xattr[i].name_length))
            foreground = *(CARD32*)xattr[i].value;
        else if (!strncmp(XNBackground, xattr[i].name, xattr[i].name_length))
            background = *(CARD32*)xattr[i].value;
        else if (!strncmp(XNBackgroundPixmap, xattr[i].name, xattr[i].name_length))
            bg_pixmap = *(Pixmap*)xattr[i].value;
        else if (!strncmp(XNFontSet, xattr[i].name, xattr[i].name_length)) {
            int str_length = strlen((char *)xattr[i].value);
            if (base_font != NULL) {
                if (strcmp(base_font, (char *)xattr[i].value)) {
                    XFree(base_font);
                } else {
                    continue;
                }
            }
            base_font = (char *) malloc(str_length + 1);
            strcpy(base_font, (char *)xattr[i].value);
        } else if (!strncmp(XNLineSpace, xattr[i].name, xattr[i].name_length))
            line_space= *(CARD32*)xattr[i].value;
        else if (!strncmp(XNCursor, xattr[i].name, xattr[i].name_length))
            cursor = *(Cursor*)xattr[i].value;
        else 
            check(xattr, false);
    }
}

void StatusAttributes::get(XICAttribute *xattr, int num)
{
    for (int i = 0; i < num && xattr[i].name; i++) {
        //PRINTF("^^^^^^get: xattr:name(%s)\n", xattr[i].name);
        if (!strncmp(XNArea, xattr[i].name, xattr[i].name_length))
            setxattr(xattr[i], XRectangle, area);
        else if (!strncmp(XNAreaNeeded, xattr[i].name, xattr[i].name_length)) {
            if (area_needed.width == 0) area_needed.width =  ICWIN_W;
            if (area_needed.width == 0) area_needed.height = ICWIN_H;
            setxattr(xattr[i], XRectangle, area_needed);
        } else if (!strncmp(XNColormap, xattr[i].name, xattr[i].name_length))
            setxattr(xattr[i], Colormap, cmap);
        else if (!strncmp(XNStdColormap, xattr[i].name, xattr[i].name_length))
            setxattr(xattr[i], Colormap, cmap);
        else if (!strncmp(XNForeground, xattr[i].name, xattr[i].name_length))
            setxattr(xattr[i], CARD32, foreground);
        else if (!strncmp(XNBackground, xattr[i].name, xattr[i].name_length))
            setxattr(xattr[i], CARD32, background);
        else if (!strncmp(XNBackgroundPixmap, xattr[i].name, xattr[i].name_length))
            setxattr(xattr[i], Pixmap, bg_pixmap);
        else if (!strncmp(XNFontSet, xattr[i].name, xattr[i].name_length)) {
            if (base_font != NULL) {
                CARD16 base_len = (CARD16)strlen(base_font);
                int total_len = sizeof(CARD16) + (CARD16)base_len;
                char *p;
                xattr[i].value = (void *)malloc(total_len);
                p = (char *)xattr[i].value;
                memmove(p, &base_len, sizeof(CARD16));
                p += sizeof(CARD16);
                strncpy(p, base_font, base_len);
                xattr[i].value_length = total_len;
            }
        } else if (!strncmp(XNLineSpace, xattr[i].name, xattr[i].name_length))
            setxattr(xattr[i], CARD32, line_space);
        else if (!strncmp(XNCursor, xattr[i].name, xattr[i].name_length))
            setxattr(xattr[i], Cursor, cursor);
        else 
            check(xattr + i, true);
    }
}

bool PreeditAttributes::check(XICAttribute *xattr, bool r)
{   
    if (!strncmp(XNSpotLocation, xattr->name, xattr->name_length)) {
        //PRINTF("check (%s), (%d)\n", xattr->name, r);
        if (r)
            setxattr((*xattr), XPoint, spot_location);
        else
            spot_location = *(XPoint*)xattr->value;
        return true;
    }
    return false;
}

IC::IC(int icid)
{
    id = icid;
    pre_attr.spot_location.x = -1;/*a check condition*/
    client_win = 0;
    focus_win = 0;
}

IC::~IC()
{
}

void IC::set(IMChangeICStruct *calldata)
{
    XICAttribute *xattr = calldata->ic_attr;
    int num = calldata->ic_attr_num;
    register int i;
    for (i = 0; i < (int)num && xattr[i].name; i++) {
        //PRINTF("IC:set %s\n", xattr[i].name);
        if (!strncmp(XNInputStyle, xattr[i].name, xattr[i].name_length))
            input_style = *(INT32*)xattr[i].value;
        else if (!strncmp(XNClientWindow, xattr[i].name, xattr[i].name_length)) {
            client_win = *(Window*)xattr[i].value; PRINTF("set client wind (%d) --> (%x)\n",client_win, client_win);}
        else if (!strncmp(XNFocusWindow, xattr[i].name, xattr[i].name_length))
            focus_win = *(Window*)xattr[i].value;
    }
    //PRINTF("sssssss set pre attr\n");
    pre_attr.set(calldata->preedit_attr, calldata->preedit_attr_num);
    sts_attr.set(calldata->status_attr, calldata->status_attr_num);
}

void IC::get(IMChangeICStruct *calldata)
{
    XICAttribute *xattr = calldata->ic_attr;
    int num = calldata->ic_attr_num;
    register int i;
    for (i = 0; i < num && xattr[i].name; i++) {
        //PRINTF("IC:get %s \n", xattr[i].name);
        if (!strncmp(XNFilterEvents, xattr[i].name, xattr[i].name_length)) {
            xattr[i].value = (void *)malloc(sizeof(CARD32));
            *(CARD32*)xattr[i].value = KeyPressMask|KeyReleaseMask;
            xattr[i].value_length = sizeof(CARD32);
            break;
        }
    }

    pre_attr.get(calldata->preedit_attr, calldata->preedit_attr_num);
    sts_attr.get(calldata->status_attr, calldata->status_attr_num);
}

Xicm::Xicm(): m_icid(0), m_icFocus(-1)
{

}

Xicm::~Xicm()
{
    log.d("~Xicm\n");
    std::map<int, IC*>::iterator iter;
    for (iter = m_ics.begin(); iter != m_ics.end(); iter++)
        delete(iter->second);
}

int Xicm::createIC(IMChangeICStruct *calldata, iIM *im)
{
    IC *ic = new IC(++m_icid);
    calldata->icid = ic->id;
    ic->set(calldata);
    ic->preedit.im = im;

    m_ics[ic->id] =  ic;
    return 0;
}

int Xicm::setICValues(IMChangeICStruct *calldata)
{
    std::map<int, IC*>::iterator iter = m_ics.find(calldata->icid);
    if(iter != m_ics.end()) {
        iter->second->set(calldata);
    }
}

int Xicm::getICValues(IMChangeICStruct *calldata)
{
    std::map<int, IC*>::iterator iter = m_ics.find(calldata->icid);
    if(iter != m_ics.end()) {
        iter->second->get(calldata);
    }
}

int Xicm::setICFocus(IMChangeFocusStruct *calldata)
{
    m_icFocus = calldata->icid;
    PRINTF("setICFocus %d\n", m_icFocus);
    return 0;
}

int Xicm::unsetICFocus(IMChangeFocusStruct *calldata)
{
    PRINTF("unsetICFocus %d, callid:%d\n", m_icFocus, calldata->icid);
    if (m_icFocus == calldata->icid) {
        m_icFocus = -1;
        /*IC *ic = getIC(calldata->icid);
        if (ic != NULL) {
            ic->preedit.close();
        }*/
    }
}

int Xicm::resetICFocus(IMChangeFocusStruct *calldata)
{
    PRINTF("resetICfocus %d icid %d \n",m_icFocus, calldata->icid);
    IC *ic = getIC(calldata->icid);
    if (ic != NULL && m_icFocus == calldata->icid ) {
        ic->preedit.reset();
    }
}

void Xicm::closeIC(int focus)
{
    IC *ic = getIC(focus);
    if (ic != NULL) {
        ic->preedit.close();
    }
}

IC* Xicm::getIC(int focus)
{
    int id =  focus == -1 ? m_icFocus : focus;
    std::map<int, IC*>::iterator iter = m_ics.find(id);
    if(iter != m_ics.end()) {
        return iter->second;
    }
    return NULL;
}
