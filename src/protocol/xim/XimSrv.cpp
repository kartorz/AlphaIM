/** 
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#include <locale.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <boost/algorithm/string.hpp>
#include <signal.h>
#include <exception>

#include "aim.h"
#include "Application.h"
#include "CharUtil.h"
#include "XimSrv.h"
#include "Log.h"
#include "Util.h"

#define IM_NAME "aim"
#define KEVBUF_LEN 64

#undef PRINTF
//#define PRINTF(fmt, args...)  printf(fmt, ##args)
#define PRINTF(fmt, args...)

#define DEF_LOCALATE "zh_CN.UTF-8,zh.UTF-8,zh_CN,zh"

/* Supported Inputstyles */
static XIMStyle supported_styles[] = {
#if 1 // Over the spot
    XIMPreeditPosition|XIMStatusArea,
    XIMPreeditPosition|XIMStatusNothing,
    XIMPreeditPosition|XIMStatusNone,

    XIMPreeditNothing |XIMStatusNothing,
    XIMPreeditNothing |XIMStatusNone,
#endif
#if 0 // On the spot
    XIMPreeditPosition  | XIMStatusNothing,
    XIMPreeditCallbacks | XIMStatusNothing,
    XIMPreeditNothing   | XIMStatusNothing,
    XIMPreeditPosition  | XIMStatusCallbacks,
    XIMPreeditCallbacks | XIMStatusCallbacks,
    XIMPreeditNothing   | XIMStatusCallbacks,
#endif
    0
};

static XIMTriggerKey trigger_keys[] = {
    {XK_space, ShiftMask, ShiftMask},
    {XK_space, ControlMask, ControlMask},
    {0L, 0L, 0L}
};

static XIMEncoding zh_encodings[] = {
    "COMPOUND_TEXT",
    "zh_CN.UTF-8",
    NULL,
};

struct XIMPriv {
    XIMS ims;
    IMProtocol *calldata;
};


int aim_err_handler(Display *d, XErrorEvent *e)
{
    printf("aim_err_handler, type:%d, err_code:%d, req_code:%d\n", e->type, e->error_code, e->request_code);
    return 0;
}

// Running at the same thread.
int aim_proto_handler(XIMS ims, IMProtocol *call_data)
{
    PRINTF("aim_proto_handler 0x%x --> %d\n", call_data->major_code, call_data->major_code);
    switch (call_data->major_code) {
      case XIM_OPEN:
          gApp->xim.handleIMOpen(ims, call_data);
          break;
      case XIM_CREATE_IC:
          gApp->xim.handleIMCreateIC(ims, call_data);
          break;
      case XIM_DESTROY_IC:
          break;
      case XIM_SET_IC_VALUES:
          gApp->xim.handleIMSetICValues(ims, call_data);
          break;
      case XIM_GET_IC_VALUES:
          gApp->xim.handleGetICValues(ims, call_data);
          break;
      case XIM_FORWARD_EVENT:
          gApp->xim.handleForwardEvent(ims, call_data);
          break;
      case XIM_SET_IC_FOCUS:
          gApp->xim.handleSetICFocusEvent(ims, call_data);
          break;
      case XIM_UNSET_IC_FOCUS:
          gApp->xim.handleUnsetICFocusEvent(ims, call_data);
          break;
      case XIM_RESET_IC:
          gApp->xim.handleResetICEvent(ims, call_data);
          break;
      case XIM_TRIGGER_NOTIFY:
          gApp->xim.handleTriggerNotify(ims, call_data);
          break;
      case XIM_PREEDIT_START_REPLY:
          gApp->xim.handlePreeditStartReply(ims, call_data);
          break;
      case XIM_PREEDIT_CARET_REPLY:
          gApp->xim.handlePreeditCaretReply(ims, call_data);
          break;
    }
    return 1;
}

XimSrv::XimSrv():m_ims((XIMS)NULL), m_bDynamicEvent(false), m_imwin(0), m_preModKey(0), m_im(NULL)
{
}

XimSrv::~XimSrv()
{
    PRINTF("~XimSrv\n");
    close();
    if (m_im != NULL) {
        printf("delete m_im\n");
        delete m_im;
    }
}

bool XimSrv::open(Display *dpy)
{
#define check(fun)  \
    if ((fun) == NULL) {                                                \
        fprintf(stderr, "%s(%d), IM Can't allocate\n",__FILE__, __LINE__); \
        return false;                                                   \
    }

    XIMStyles *input_styles;
    XIMTriggerKeys *on_keys;
    XIMEncodings *encodings;
    const char *transport = "X/";
    const char *imname = IM_NAME;
    XIMS ims;
    long filter_mask = KeyPressMask | KeyReleaseMask;
    m_imwin = XCreateWindow(dpy,
                            DefaultRootWindow(dpy),
                            0, 0, 1, 1, 0, 0,
                            InputOnly,
                            CopyFromParent,
                            0, NULL);

    char *imlocale = getenv("LC_CTYPE");
    if (!imlocale) {
        imlocale = (char *)DEF_LOCALATE;
    }

    log.d("imlocale  %s\n", imlocale);

    int screen_num = DefaultScreen(dpy);
    m_dpy   = dpy;
    m_dpyW  = DisplayWidth(dpy, screen_num);
    m_dpyH  = DisplayHeight(dpy, screen_num);

    check(input_styles = (XIMStyles *)malloc(sizeof(XIMStyles)));
    input_styles->count_styles = sizeof(supported_styles)/sizeof(XIMStyle) - 1;
    input_styles->supported_styles = supported_styles;

    check(on_keys = (XIMTriggerKeys *) malloc(sizeof(XIMTriggerKeys)));
    on_keys->count_keys = sizeof(trigger_keys)/sizeof(XIMTriggerKey) - 1;
    on_keys->keylist = trigger_keys;

    check(encodings = (XIMEncodings *)malloc(sizeof(XIMEncodings)));
    encodings->count_encodings = sizeof(zh_encodings)/sizeof(XIMEncoding) - 1;
    encodings->supported_encodings = zh_encodings;

    ims = IMOpenIM(dpy,
		   IMModifiers, "Xi18n",
		   IMServerWindow, m_imwin,
		   IMServerName, imname,
		   IMLocale, imlocale,
		   IMServerTransport, transport,
		   IMInputStyles, input_styles,
		   NULL);

    if (ims == (XIMS)NULL) {
        fprintf(stderr, "{XimSrv} Can't Open Input Method Service:\n");
        log.e("{XimSrv} Can't Open Input Method Service:\n\n");
        close();
        return false;
    }

    IMSetIMValues(ims,
          IMEncodingList, encodings,
		  IMProtocolHandler, aim_proto_handler,
		  IMFilterEventMask, filter_mask,
		  NULL);

    if (m_bDynamicEvent) {
        IMSetIMValues(ims,
            IMOnKeysList, on_keys,
            IMOffKeysList, on_keys,
            NULL);
    }

    m_ims = ims;
    //XSelectInput(dpy, imwin, StructureNotifyMask);
    //XSetErrorHandler(aim_err_handler);
   log.d("imopen\n");

    return true;
}

void XimSrv::close()
{
    if (m_imwin > 0) {
        PRINTF("{XimSrv} close: XDestroyWindow\n");
        log.d("{XimSrv} close: XDestroyWindow\n");
        XDestroyWindow(m_dpy, m_imwin);
        m_imwin = 0;
    }

    if (m_ims != (XIMS)NULL) {
        PRINTF("{XimSrv} close:  im\n");
        log.d("{XimSrv} close:  im\n");
        IMCloseIM(m_ims);
        m_ims = (XIMS)NULL;
    }
}

int XimSrv::handleIMOpen(XIMS ims, IMProtocol *calldata)
{
    return true;
}

int XimSrv::handleIMCreateIC(XIMS ims, IMProtocol *calldata)
{
    m_icMgr.createIC((IMChangeICStruct *)calldata, m_im);
    return true;
}

int XimSrv::handleIMSetICValues(XIMS ims, IMProtocol *calldata)
{
    m_icMgr.setICValues((IMChangeICStruct *)calldata);
    return true;
}

int XimSrv::handleGetICValues(XIMS ims, IMProtocol *calldata)
{
    m_icMgr.getICValues((IMChangeICStruct *)calldata);
    return true;
}

int XimSrv::handleForwardEvent(XIMS ims, IMProtocol *calldata)
{
    PRINTF("handleForwardEvent\n");
    MutexLock lock(m_cs);  //Exclud handleUIMessage

    /* Lookup KeyPress Events only */
    int evtype = calldata->forwardevent.event.type;
    if (evtype != KeyPress && evtype != KeyRelease) {
        IMForwardEvent(ims, (XPointer)calldata);
        return true;
    }

    IC* ic = m_icMgr.getIC();
    if (ic == NULL) {
        IMForwardEvent(ims, (XPointer)calldata);
        return true;
    }

    char strbuf[KEVBUF_LEN];
    KeySym keysym;

    XKeyEvent *kev = (XKeyEvent*)&((IMForwardEventStruct *)calldata)->event;
    XLookupString(kev, strbuf, KEVBUF_LEN, &keysym, NULL);
    //printf("kev time: 0x%d type: %d state %d --> code %x --> %x \n", Util::getTimeMS(), evtype, kev->state, keysym, strbuf[0]);

    XIMPriv priv;
    priv.ims = ims;
    priv.calldata = calldata;
    this->opaque = &priv;
    int ret = ic->preedit.handleKey(keysym, kev->state, strbuf, evtype, this);
    if (ret == FORWARD_KEY) {
        IMForwardEvent(ims, (XPointer)calldata);
    }
}

void XimSrv::handleUIMessage(int msg)
{
    MutexLock lock(m_cs);

    IC* ic = m_icMgr.getIC();
    if (ic != NULL) {
        ic->preedit.handleMessage(msg);
    }
}

int XimSrv::doModifierKeyEvent(XIMS ims, IMProtocol *calldata)
{
}

void XimSrv::onIMOff(void *priv)
{
    XIMPriv* pri = (XIMPriv*) priv;
    IMPreeditEnd(pri->ims, (XPointer)(pri->calldata));
}

void XimSrv::onCommit(void *priv, string candidate)
{
    XIMPriv* pri = (XIMPriv*) priv;
    commit(pri->ims, (IMForwardEventStruct *)(pri->calldata), candidate);
}

ICRect XimSrv::onGetRect(void* priv)
{
    XRectangle rect = getICWinRect();

    ICRect r;
    r.x = rect.x;
    r.y = rect.y;
    r.w = rect.width;
    r.h = rect.height;

    return r;
}

int XimSrv::handleSetICFocusEvent(XIMS ims, IMProtocol *calldata)
{
    m_icMgr.setICFocus((IMChangeFocusStruct *)calldata);

    IC* ic = m_icMgr.getIC();
    if (ic != NULL) {
        XIMPriv priv;
        priv.ims = ims;
        priv.calldata = calldata;
        this->opaque = &priv;

        ic->preedit.guiReload(this);
    }

    return true;
}

int XimSrv::handleUnsetICFocusEvent(XIMS ims, IMProtocol *calldata)
{
    m_icMgr.unsetICFocus((IMChangeFocusStruct *)calldata);
    return true;
}

int XimSrv::handleResetICEvent(XIMS ims, IMProtocol *calldata)
{
    m_icMgr.resetICFocus((IMChangeFocusStruct *)calldata);
    return true;
}

int XimSrv::handleTriggerNotify(XIMS ims, IMProtocol *calldata)
{
    IMTriggerNotifyStruct *notify = (IMTriggerNotifyStruct *)calldata;
    if (notify->flag == 0) {	/* on key */
	/* Here, the start of preediting is notified from IMlibrary, which 
	   is the only way to start preediting in case of Dynamic Event
	   Flow, because ON key is mandatary for Dynamic Event Flow. */
        gApp->pGuiMsgQ->push(MSG_IM_ON);
        return 1;
    }

    if (notify->flag == 1) {	/* off key */
	/* Here, the end of preediting is notified from the IMlibrary, which
	   happens only if OFF key, which is optional for Dynamic Event Flow,
	   has been registered by IMOpenIM or IMSetIMValues, otherwise,
	   the end of preediting must be notified from the IMserver to the
	   IMlibrary. */
        m_icMgr.closeIC();
        return 1;
    }

	/* never happens */
	return 0;
}

int XimSrv::handlePreeditStartReply(XIMS ims, IMProtocol *calldata)
{
    PRINTF("handlePreeditStartReply\n");
    return true;
}

int XimSrv::handlePreeditCaretReply(XIMS ims, IMProtocol *calldata)
{
    PRINTF("handlePreeditCaretReply\n");
    return true;
}

/*bool XimSrv::isMatchKeys(KeySym& keysym, XKeyEvent *kev, XIMTriggerKey *trigger)
{
    int i;
    int modifier;
    int modifier_mask;

    for (i = 0; trigger[i].keysym != 0; i++) {
	    modifier      = trigger[i].modifier;
	    modifier_mask = trigger[i].modifier_mask;
	    if (((KeySym)trigger[i].keysym == keysym)
	        && ((kev->state & modifier_mask) == modifier))
	    return True;
    }
    return False;
}*/

void XimSrv::commit(XIMS ims, IMForwardEventStruct* calldata, string candidate)
{
	XTextProperty tp;
	Display *display = ims->core.display;
	char *text = (char *)candidate.c_str();
	char lang[20];

	//setlocale(LC_CTYPE, "");
	XmbTextListToTextProperty(display, (char **)&text, 1,
                              XCompoundTextStyle, &tp);
	((IMCommitStruct*)calldata)->flag |= XimLookupChars;
	((IMCommitStruct*)calldata)->commit_string = (char *)tp.value;
	IMCommitString(ims, (XPointer)calldata);
}

void XimSrv::setIM(iIM *im, bool en)
{
    m_im = im;
}

XRectangle XimSrv::getICWinRect()
{
    XRectangle ret = {0,0, ICWIN_W, ICWIN_H};

    IC *ic = m_icMgr.getIC();
    if (ic != NULL) {
        int x = 0;
        int y = m_dpyH;
        int w,h;
        XWindowAttributes clientwin_attr;
        Window win, wid;

        if (!(win = ic->focus_win))
            win = ic->client_win;

        if (ic->pre_attr.spot_location.x != -1) {
            int x2 = ic->pre_attr.spot_location.x;
            int y2 = ic->pre_attr.spot_location.y;
            if (win) {
                try {
                    XTranslateCoordinates(m_dpy, win, XDefaultRootWindow(m_dpy), x2, y2, &x, &y, &wid);  // TODO: The error was 'BadWindow (invalid Window parameter)'
                } catch (exception& e) {
                    printf("x1: %s\n", e.what());
                }
            }
/*
The error was 'BadWindow (invalid Window parameter)'.
  (Details: serial 863 error_code 3 request_code 18 (core protocol) minor_code 0)
  (Note to programmers: normally, X errors are reported asynchronously;
   that is, you will receive the error a while after causing it.
   To debug your program, run it with the GDK_SYNCHRONIZE environment
   variable to change this behavior. You can then get a meaningful
   backtrace from your debugger if you break on the gdk_x_error() function.)

 */
            //w = ic->pre_attr.area.width  == 0 ? ICWIN_W : ic->pre_attr.area.width;
            //h = ic->pre_attr.area.height == 0 ? ICWIN_H : ic->pre_attr.area.height;
            w = ICWIN_W;
            h = ICWIN_H;
            x = x > 0 ? x : 0;
            y = y > 0 ? y : 0;

            //PRINTF("getICWin spot location [%d] (%d, %d, %d ,%d), dpy_w: %d, dpyH, %d\n", ic->client_win, x, y, w, h, m_dpyW, m_dpyH);
        } else if (win) {
            try {
                XGetWindowAttributes(m_dpy, win, &clientwin_attr);
                XTranslateCoordinates(m_dpy, win, XDefaultRootWindow(m_dpy), 0, clientwin_attr.height, &x, &y, &wid);
            }  catch (exception& e) {
                printf("x2: %s\n", e.what());
            }

            //PRINTF("getICWin client win[%d]  (%d, %d, %d ,%d), dpy_w: %d, dpyH, %d\n", ic->client_win, x, y, clientwin_attr.width, clientwin_attr.height, m_dpyW, m_dpyH);
            x += (clientwin_attr.width - ICWIN_W) / 2;
            //y += clientwin_attr.height;
            w = ICWIN_W;/*clientwin_attr.width < ICWIN_W ? ICWIN_W : clientwin_attr.width;*/
            h = ICWIN_H;
        }

        if (x + w > m_dpyW - 20) {
            x = m_dpyW - ICWIN_W - 20;
        }
         
        if (y + h > m_dpyH - 10) {
            y = y - 2*h;
        }
        //PRINTF("getICWin (%d, %d, %d, %d)\n",  x, y, w, h);
        ret.x = x;
        ret.y = y;
        ret.width = w;
        ret.height = h;
        return ret;
    }

    return ret;
}
