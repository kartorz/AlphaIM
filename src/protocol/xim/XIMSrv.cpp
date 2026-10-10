/**
 *    @Copyright (c) 2016 joni <joni_lee@outlook.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *--------------------------------------------------
 * Debug: optional: two exports
 *   export LC_CTYPE=zh_CN.UTF-8
 *   export XIM_DEBUG=1
 *   XMODIFIERS=@im=alphaim xterm
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

XIMSrv::XIMSrv():m_ims((XIMS)NULL), m_bDynamicEvent(false), m_imwin(0),
m_preModKey(0), m_icManager(new XIMIC(NULL))
{
}

XIMSrv::~XIMSrv()
{
    PRINTF("~XIMSrv\n");
    close();
}

bool XIMSrv::open()
{
    XIMStyles *input_styles;
    XIMTriggerKeys *on_keys;
    XIMEncodings *encodings;
    const char *transport = "X/";
    const char *imname = IM_ID;
    XIMS ims;
    long filter_mask = KeyPressMask | KeyReleaseMask;

#define check(fun) if ((fun) == NULL) {                                    \
        fprintf(stderr, "%s(%d), IM Can't allocate\n",__FILE__, __LINE__); \
        return false;                                                   \
    }

    if ((m_dpy = XOpenDisplay(NULL)) == NULL) {
        logger.e("{XIMSrv} Can't Open Display:\n");
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

    logger.d("imlocale  %s\n", imlocale);

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
        logger.e("{XIMSrv} Can't Open Input Method Service:\n\n");
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
    ((XIMIC *)m_icManager.get(0))->dpy = m_dpy;
    //XSelectInput(dpy, imwin, StructureNotifyMask);
    //XSetErrorHandler(aim_err_handler);
   logger.d("xim open\n");

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
        logger.d("{XIMSrv} close: XDestroyWindow\n");
        XDestroyWindow(m_dpy, m_imwin);
        m_imwin = 0;
    }

    if (m_ims != (XIMS)NULL) {
        PRINTF("{XIMSrv} close:  im\n");
        logger.d("{XIMSrv} close:  im\n");
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
    XIMIC *ic = new XIMIC(m_dpy);
    caller->icid = m_icManager.add(ic, Util::intToString(ic->client_win).c_str());
    PRINTF("caller id:%d, ic id:%d, clinet_win:%d\n", caller->icid, ic->id, ic->client_win);
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
    MutexLock lock(m_cs);  //Exclude handleUIMessage
    static  CARD32 last_kevtime = 0;

    /* Lookup KeyPress Events only */
    int evtype = calldata->forwardevent.event.type;
    if (evtype != KeyPress && evtype != KeyRelease) {
        IMForwardEvent(ims, (XPointer)calldata);
        return false;
    }

    // Check if a infinite loop.
    //   kev time: 0x5164080 type: 2 state 0 --> code ffe3 --> 0
    //   kev time: 0x5164080 type: 2 state 0 --> code ffe3 --> 0
    //   kev time: 0x5164080 type: 2 state 0 --> code ffe3 --> 0
    XKeyEvent *kev = (XKeyEvent*)&((IMForwardEventStruct *)calldata)->event;
    if (kev->time == last_kevtime || kev->time == 0) {
        logger.e("handleForwardEvent, a infinite loop, type:%d-->%d\n", kev->time, last_kevtime);
        return false;
    }
    last_kevtime = kev->time;

    PRINTF("handleForwardEvent, type:%d\n",  calldata->forwardevent.event.type);

    char strbuf[KEVBUF_LEN];
    KeySym keysym;
    XLookupString(kev, strbuf, KEVBUF_LEN, &keysym, NULL);
    /*PRINTF("kev time: 0x%d type: %d state %d --> code %x --> %x \n",
           kev->time, evtype, kev->state, keysym, strbuf[0]);*/
    if (keysym == 0) {
        IMForwardEvent(ims, (XPointer)calldata);
        return false;
    }

    XIMPriv priv;
    priv.ims = ims;
    priv.calldata = calldata;
    this->opaque = &priv;
    int ret = handleKey(keysym, kev->state, evtype);
    if (ret == FORWARD_KEY) {
        /* Be careful, IMForwardEvent may be hanlded by this function again
           -- a infinite loop */
        IMForwardEvent(ims, (XPointer)calldata);
        return false;
    }
    return true;
}

void XIMSrv::handleUIMessage(int msg)
{
    if (m_imwin) {
        MutexLock lock(m_cs);
        m_icManager.getFocus()->preedit->handleMessage(msg);
    }
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

int XIMSrv::handleSetICFocusEvent(XIMS ims, IMProtocol *calldata)
{
    m_icManager.focusIn(((IMChangeFocusStruct *)calldata)->icid);

    XIMPriv priv;
    priv.ims = ims;
    priv.calldata = calldata;
    this->opaque = &priv;
    m_icManager.getFocus()->preedit->guiReload();

    return true;
}

int XIMSrv::handleUnsetICFocusEvent(XIMS ims, IMProtocol *calldata)
{
    //focusOut((IMChangeFocusStruct *)calldata->icid);
    m_icManager.focusOut(m_icManager.focusId());
    return true;
}

int XIMSrv::handleResetICEvent(XIMS ims, IMProtocol *calldata)
{
    //((IMChangeFocusStruct *)calldata)->icid
    m_icManager.getFocus()->reset();
    return true;
}

int XIMSrv::handleTriggerNotify(XIMS ims, IMProtocol *calldata)
{
    IMTriggerNotifyStruct *notify = (IMTriggerNotifyStruct *)calldata;
    if (notify->flag == 0) {    /* on key */
    /* Here, the start of preediting is notified from IMlibrary, which
       is the only way to start preediting in case of Dynamic Event
       Flow, because ON key is mandatary for Dynamic Event Flow. */
        gApp->getMessageQ()->push(MSG_IM_ON);
        return 1;
    }

    if (notify->flag == 1) {    /* off key */
    /* Here, the end of preediting is notified from the IMlibrary, which
       happens only if OFF key, which is optional for Dynamic Event Flow,
       has been registered by IMOpenIM or IMSetIMValues, otherwise,
       the end of preediting must be notified from the IMserver to the
       IMlibrary. */
        m_icManager.getFocus()->close();
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

void XIMSrv::commit(XIMS ims, IMForwardEventStruct* calldata, string candidate)
{
    XTextProperty tp;
    Display *display = ims->core.display;
    char *text = (char *)candidate.c_str();
    //char lang[20];
    PRINTF("XIMSrv::commit %s\n", text);
    //setlocale(LC_CTYPE, "");
    //XmbTextListToTextProperty(display, (char **)&text, 1, XCompoundTextStyle, &tp);
    Xutf8TextListToTextProperty(display, (char **) &text, 1, XCompoundTextStyle, &tp);
    ((IMCommitStruct*)calldata)->flag |= XimLookupChars;
    ((IMCommitStruct*)calldata)->commit_string = (char *)tp.value;
    IMCommitString(ims, (XPointer)calldata);

    XFree(tp.value);
}

/*
 * There are some pains when comes to precess x11 keys.
 * 1) modifier + key (eg, Shit_L + w)
         kev type: 2 mask 0 --> code ffe1
         kev type: 2 mask 1 --> code 57
       { kev type: 3 ... --> code 57} +
 * 2) ctrl + shift
        kev type 2 mask: 0 code: ffe3
        kev type 2 mask: 4 code: ffe1
        kev type 3 mask: 5 code: ffe3
        kev type 3 mask: 1 code: ffe1
 * 3) ASCII key (eg, 'w').
      kev type: 2 mask 0 code:77
    { kev type: 3 mask 0 code:77 }+
 * ----------------------------------------------
 *  #define KeyPress        2
 *  #define KeyRelease      3
 *------------------------------------------------------
 * Tested by xterm, urxvt
 *
 */
int XIMSrv::handleKey(u32 key, u32 modifier, int evtype)
{
    if (evtype == KeyPress) {
        IC* ic = m_icManager.getFocus();
        m_preRetKey = ic->preedit->doHandleKey(ic, key, modifier, this);
    }
    PRINTF("XIMSrv::handleKey, evtype:%d return %d\n", evtype, m_preRetKey);
    // After key released, return the previous result, don't forward directly.
    return m_preRetKey;
}
