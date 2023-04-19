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
#include "XIMSrv.h"
#include "Log.h"
#include "Util.h"

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
          gApp->xim.handleIMDestroyIC(ims, call_data);
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

XIMSrv::XIMSrv():m_ims((XIMS)NULL), m_bDynamicEvent(false), m_imwin(0), m_preModKey(0)
{
	IC* ic = new IC(); // Add a dumpy IC, So don't need check if IC exists every time.
	ic->preedit = new IMPreedit();
	ic->id = 0;
	m_icManager.add(ic, 0);
}

XIMSrv::~XIMSrv()
{
    PRINTF("~XIMSrv\n");
    close();
}

bool XIMSrv::open()
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

	if ((m_dpy = XOpenDisplay(NULL)) == NULL) {
		gLog.e("{XIMSrv} Can't Open Display:\n");
		return false;
	}

    m_imwin = XCreateWindow(m_dpy,
                            DefaultRootWindow(m_dpy),
                            0, 0, 1, 1, 0, 0,
                            InputOnly,
                            CopyFromParent,
                            0, NULL);

    char *imlocale = getenv("LC_CTYPE");
    if (!imlocale) {
        imlocale = (char *)DEF_LOCALATE;
    }

    gLog.d("imlocale  %s\n", imlocale);

    int screen_num = DefaultScreen(m_dpy);
    IC::dpyW  = DisplayWidth(m_dpy, screen_num);
    IC::dpyH  = DisplayHeight(m_dpy, screen_num);

    check(input_styles = (XIMStyles *)malloc(sizeof(XIMStyles)));
    input_styles->count_styles = sizeof(supported_styles)/sizeof(XIMStyle) - 1;
    input_styles->supported_styles = supported_styles;

    check(on_keys = (XIMTriggerKeys *) malloc(sizeof(XIMTriggerKeys)));
    on_keys->count_keys = sizeof(trigger_keys)/sizeof(XIMTriggerKey) - 1;
    on_keys->keylist = trigger_keys;

    check(encodings = (XIMEncodings *)malloc(sizeof(XIMEncodings)));
    encodings->count_encodings = sizeof(zh_encodings)/sizeof(XIMEncoding) - 1;
    encodings->supported_encodings = zh_encodings;

    ims = IMOpenIM(m_dpy,
		   IMModifiers, "Xi18n",
		   IMServerWindow, m_imwin,
		   IMServerName, imname,
		   IMLocale, imlocale,
		   IMServerTransport, transport,
		   IMInputStyles, input_styles,
		   NULL);

    if (ims == (XIMS)NULL) {
        fprintf(stderr, "{XIMSrv} Can't Open Input Method Service:\n");
        gLog.e("{XIMSrv} Can't Open Input Method Service:\n\n");
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
   gLog.d("imopen\n");

   return true;
}

void XIMSrv::eventLoop()
{
	for (;;) {
		XEvent event;
		XNextEvent(m_dpy, &event);
		if (XFilterEvent(&event, None) == True)
			continue;
	}
}

void XIMSrv::close()
{
    if (m_imwin > 0) {
        PRINTF("{XIMSrv} close: XDestroyWindow\n");
        gLog.d("{XIMSrv} close: XDestroyWindow\n");
        XDestroyWindow(m_dpy, m_imwin);
        m_imwin = 0;
    }

    if (m_ims != (XIMS)NULL) {
        PRINTF("{XIMSrv} close:  im\n");
        gLog.d("{XIMSrv} close:  im\n");
        IMCloseIM(m_ims);
        m_ims = (XIMS)NULL;
    }
}

int XIMSrv::handleIMOpen(XIMS ims, IMProtocol *calldata)
{
    return true;
}

int XIMSrv::handleIMCreateIC(XIMS ims, IMProtocol *calldata)
{
    IMChangeICStruct *caller = (IMChangeICStruct *)calldata;
	XIMIC *ic = new XIMIC();
	caller->icid = m_icManager.add(ic);
	//printf("caller id:%d, ic id:%d\n", caller->icid, ic->id);
	ic->set(caller);
    return true;
}

int XIMSrv::handleIMDestroyIC(XIMS ims, IMProtocol *calldata)
{
    m_icManager.destroy(((IMChangeICStruct *)calldata)->icid);
    return true;
}

int XIMSrv::handleIMSetICValues(XIMS ims, IMProtocol *calldata)
{
	IMChangeICStruct *caller = (IMChangeICStruct *)calldata;
	XIMIC *ic = (XIMIC *)m_icManager.get(caller->icid);
	ic->set(caller);
    return true;
}

int XIMSrv::handleGetICValues(XIMS ims, IMProtocol *calldata)
{
	IMChangeICStruct *caller = (IMChangeICStruct *)calldata;
	XIMIC *ic = (XIMIC *)m_icManager.get(caller->icid);
	ic->get(caller);
    return true;
}

int XIMSrv::handleForwardEvent(XIMS ims, IMProtocol *calldata)
{
    PRINTF("handleForwardEvent\n");
    MutexLock lock(m_cs);  //Exclud handleUIMessage
    static  CARD32 last_kevtime = 0;

    /* Lookup KeyPress Events only */
    int evtype = calldata->forwardevent.event.type;
    if (evtype != KeyPress && evtype != KeyRelease) {
        IMForwardEvent(ims, (XPointer)calldata);
        return true;
    }

    IC* ic = m_icManager.get();
    if (ic->id == 0) {
        IMForwardEvent(ims, (XPointer)calldata);
        return true;
    }

    // Check if a infinite loop.
    //   kev time: 0x5164080 type: 2 state 0 --> code ffe3 --> 0
    //   kev time: 0x5164080 type: 2 state 0 --> code ffe3 --> 0
    //   kev time: 0x5164080 type: 2 state 0 --> code ffe3 --> 0
    XKeyEvent *kev = (XKeyEvent*)&((IMForwardEventStruct *)calldata)->event;
    if (kev->time == last_kevtime || kev->time == 0) {
        PRINTF("A infinite loop, type:%d-->%d\n", kev->time, last_kevtime);
        return true;
    }
    last_kevtime = kev->time;

#ifdef AL_DEBUG
//    gLog.d("handleForwardEvent, type:%d\n",  calldata->forwardevent.event.type);
#endif

    char strbuf[KEVBUF_LEN];
    KeySym keysym;
    XLookupString(kev, strbuf, KEVBUF_LEN, &keysym, NULL);
    //printf("kev time: 0x%d type: %d state %d --> code %x --> %x \n", kev->time, evtype, kev->state, keysym, strbuf[0]);
	if (keysym == 0) {
		return true;
	}

    XIMPriv priv;
    priv.ims = ims;
    priv.calldata = calldata;
    this->opaque = &priv;
    int ret = ic->preedit->handleKey(keysym, kev->state, strbuf, evtype, this);
    if (ret == FORWARD_KEY) {
        // Be careful, IMForwardEvent may be hanlded by this function again -- a infinite loop
        IMForwardEvent(ims, (XPointer)calldata);
    }
    return true;
}

void XIMSrv::handleUIMessage(int msg)
{
    MutexLock lock(m_cs);
    m_icManager.get()->preedit->handleMessage(msg);
}

int XIMSrv::doModifierKeyEvent(XIMS ims, IMProtocol *calldata)
{
    return 0;
}

void XIMSrv::onIMOff(void *priv)
{
    XIMPriv* pri = (XIMPriv*) priv;
    IMPreeditEnd(pri->ims, (XPointer)(pri->calldata));
}

void XIMSrv::onCommit(void *priv, string candidate)
{
    XIMPriv* pri = (XIMPriv*) priv;
    commit(pri->ims, (IMForwardEventStruct *)(pri->calldata), candidate);
}

ICRect XIMSrv::onGetRect()
{
    return getICWinRect();
}

int XIMSrv::handleSetICFocusEvent(XIMS ims, IMProtocol *calldata)
{
    m_icManager.focusIn(((IMChangeFocusStruct *)calldata)->icid);

	XIMPriv priv;
	priv.ims = ims;
	priv.calldata = calldata;
	this->opaque = &priv;
	m_icManager.get()->preedit->guiReload(this);

    return true;
}

int XIMSrv::handleUnsetICFocusEvent(XIMS ims, IMProtocol *calldata)
{
    //focusOut((IMChangeFocusStruct *)calldata->icid);
	m_icManager.focusOut();
    return true;
}

int XIMSrv::handleResetICEvent(XIMS ims, IMProtocol *calldata)
{
	//((IMChangeFocusStruct *)calldata)->icid
    m_icManager.get()->reset();
    return true;
}

int XIMSrv::handleTriggerNotify(XIMS ims, IMProtocol *calldata)
{
    IMTriggerNotifyStruct *notify = (IMTriggerNotifyStruct *)calldata;
    if (notify->flag == 0) {	/* on key */
	/* Here, the start of preediting is notified from IMlibrary, which
	   is the only way to start preediting in case of Dynamic Event
	   Flow, because ON key is mandatary for Dynamic Event Flow. */
	    gApp->getMessageQ()->push(MSG_IM_ON);
        return 1;
    }

    if (notify->flag == 1) {	/* off key */
	/* Here, the end of preediting is notified from the IMlibrary, which
	   happens only if OFF key, which is optional for Dynamic Event Flow,
	   has been registered by IMOpenIM or IMSetIMValues, otherwise,
	   the end of preediting must be notified from the IMserver to the
	   IMlibrary. */
        m_icManager.get()->close();
        return 1;
    }

	/* never happens */
	return 0;
}

int XIMSrv::handlePreeditStartReply(XIMS ims, IMProtocol *calldata)
{
    PRINTF("handlePreeditStartReply\n");
    return true;
}

int XIMSrv::handlePreeditCaretReply(XIMS ims, IMProtocol *calldata)
{
    PRINTF("handlePreeditCaretReply\n");
    return true;
}

/*bool XIMSrv::isMatchKeys(KeySym& keysym, XKeyEvent *kev, XIMTriggerKey *trigger)
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

void XIMSrv::commit(XIMS ims, IMForwardEventStruct* calldata, string candidate)
{
	XTextProperty tp;
	Display *display = ims->core.display;
	char *text = (char *)candidate.c_str();
	//char lang[20];
	printf("XIMSrv::commit %s\n", text);
	//setlocale(LC_CTYPE, "");
	//XmbTextListToTextProperty(display, (char **)&text, 1, XCompoundTextStyle, &tp);
	Xutf8TextListToTextProperty(display, (char **) &text, 1, XCompoundTextStyle, &tp);
	((IMCommitStruct*)calldata)->flag |= XimLookupChars;
	((IMCommitStruct*)calldata)->commit_string = (char *)tp.value;
	IMCommitString(ims, (XPointer)calldata);

	XFree(tp.value);
}

ICRect XIMSrv::getICWinRect()
{
    XIMIC *ic = (XIMIC *) m_icManager.get();
    if (ic->id > 0) {
        int x = 0;
        int y = IC::dpyH;
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
		return IC::adjRect(x, y, w, h);    
    }
    ICRect ret = {0,0, ICWIN_W, ICWIN_H};
    return ret;
}

